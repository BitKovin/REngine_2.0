// Tier "chase" (escaping a fight) and tier "squad" (flanking). These reproduce what was seen in manual_10.savg:
// the guards never lose the player, and a flanker that runs to its doorway and then just stands.
//
// Every scenario of both tiers also runs the invariant "no knowledge without perception" (Ctx::CheckKnowledge). The
// fight is not expected to end at a fixed time: it has to end within the designed window after the last time an NPC
// saw or heard the puppet; a puppet that is found again says so in the log and keeps the fight going.

#include "ScenarioKit.h"

#ifndef DISTRIBUTION

#include "../../../Entities/Npc/NpcBase.h"
#include "../../../Entities/Npc/Brain/NpcSounds.h"
#include "../../../Entities/Npc/Combat/CombatCoordinator.h"
#include "../../../Entities/Npc/Combat/CombatTuning.h"
#include "../../../Entities/Npc/Combat/TargetBelief.h"

#include <algorithm>
#include <functional>
#include <map>
#include <string>

namespace AiTest
{
	namespace
	{
		bool IsRole(const Snapshot& s, const char* role) { return s.role == role; }
		bool IsLane(const Snapshot& s, const char* lane) { return s.lane == lane; }

		// The longest a fight may go on after the last evidence of the puppet: the squad believes it is hiding for up to
		// concealedMax, checks the spot for up to lostMaxTime, then searches for the search timeout.
		double FightWindow()
		{
			const CombatTuningValues& tuning = CombatTuning::Get();
			return tuning.belief.concealedMax + tuning.belief.lostMaxTime + tuning.search.timeout + 5.0;
		}

		// The puppet went out of reach at `hiddenAt` (hid, left, flew away). The fight has to end within FightWindow() of the
		// last time an NPC saw or heard it. Being found again restarts that clock; a puppet still being perceived at the end
		// is a fight that rightly goes on. `upkeep` runs on every step (noise the puppet keeps making, for instance).
		void ExpectFightEnds(Ctx& ctx, double hiddenAt, const char* what, const std::function<void()>& upkeep = nullptr)
		{
			const double window = FightWindow();
			const double giveUp = hiddenAt + 150.0;
			bool found = false;

			while (Kit::CountAttacking(ctx) > 0)
			{
				found |= ctx.LastPerceived() > hiddenAt + 0.5;

				const double evidence = std::max(ctx.LastPerceived(), hiddenAt);
				if (ctx.Now() - evidence > window || ctx.Now() > giveUp) break;

				if (upkeep) upkeep();
				ctx.Step();
			}

			const double sinceEvidence = ctx.Now() - std::max(ctx.LastPerceived(), hiddenAt);
			const int attacking = Kit::CountAttacking(ctx);

			if (found)
				ctx.Info("found: an NPC saw or heard the puppet %.1f s ago (it was out of reach from %.1f s)", ctx.Now() - ctx.LastPerceived(), hiddenAt);

			ctx.Metric(std::string("seconds_from_last_evidence_to_fight_over_") + what, attacking == 0 ? sinceEvidence : -1.0);

			if (attacking == 0)
				ctx.Check(true, "%s: the fight ended %.1f s after the last evidence of the puppet (window %.0f s)", what, sinceEvidence, window);
			else if (ctx.Now() - ctx.LastPerceived() <= 2.0)
				ctx.Info("%s: %d still attacking and the puppet is still being perceived: the fight goes on", what, attacking);
			else
				ctx.Check(false, "%s: %d still attacking %.1f s after the last evidence of the puppet (window %.0f s)", what, attacking, sinceEvidence, window);
		}

		int CountMoving(Ctx& ctx)
		{
			int count = 0;
			for (const Snapshot& s : Kit::Alive(ctx))
			{
				if (s.speed > 0.5f) ++count;
			}
			return count;
		}

		// ── Escape ───────────────────────────────────────────────────────────────────────────────────────────

		// The puppet gets out of sight (a hidden spot far from every guard) and stays there, quiet.
		void EscapeToHiddenSpot(Ctx& ctx)
		{
			if (Kit::EngageSquad(ctx, 3) == false) return;

			ctx.Wait(5.0f); // fight for a while first

			const glm::vec3 centre = Kit::Centroid(ctx);
			glm::vec3 spot;
			if (Kit::HiddenFromEveryone(ctx, centre, 40.0f, spot) == false)
			{
				ctx.Info("the map has no spot out of every guard's sight 12 m or more from each");
				return;
			}

			ctx.Info("escape spot %.1f %.1f %.1f, %.1f m from the squad", spot.x, spot.y, spot.z, glm::distance(spot, centre));

			// Runs there (footsteps, sight lines and all), then crouches still and quiet.
			if (ctx.Travel(spot, Posture::Run, 60.0f) == false) return;

			ctx.puppet.Hold(Posture::CrouchStill);
			const double hiddenAt = ctx.Now();
			ctx.Info("arrived; %d guards attacking, %d seeing it", Kit::CountAttacking(ctx), Kit::CountSeeing(ctx));

			ExpectFightEnds(ctx, hiddenAt, "hidden_spot");

			// Whoever starts looking for it again a while later knows something it should not.
			const double evidenceBefore = ctx.LastPerceived();
			ctx.Wait(30.0f);
			if (ctx.LastPerceived() > evidenceBefore)
				ctx.Info("30 s later: an NPC perceived the puppet again, so a new fight is allowed");
			else
				ctx.Check(Kit::CountAttacking(ctx) == 0, "30 s later nobody is attacking again (%d are)", Kit::CountAttacking(ctx));
		}

		// "I flew away from the map": the puppet leaves the level entirely, waits, and comes back.
		void FlyAwayAndReturn(Ctx& ctx)
		{
			if (Kit::EngageSquad(ctx, 3) == false) return;

			ctx.Wait(5.0f);

			const glm::vec3 home = ctx.puppet.Position();
			ctx.puppet.Park(home + glm::vec3(300.0f, 60.0f, 300.0f));

			ExpectFightEnds(ctx, ctx.Now(), "out_of_map");

			ctx.Wait(30.0f);
			ctx.Info("30 s later: %d attacking, %d moving", Kit::CountAttacking(ctx), CountMoving(ctx));
			ctx.Check(CountMoving(ctx) <= 2, "after the fight the guards stop running around (%d still moving)", CountMoving(ctx));

			// Back again, but out of sight of everybody: nothing from the old fight tells them where it is.
			glm::vec3 hidden;
			if (Kit::HiddenFromEveryone(ctx, home, 12.0f, hidden))
			{
				ctx.puppet.TeleportTo(hidden);
				ctx.puppet.Hold(Posture::Still);
				ctx.Wait(5.0f);

				float highest = 0.0f;
				for (const Snapshot& s : Kit::Alive(ctx))
					highest = std::max(highest, s.detection);
				ctx.Check(Kit::CountAttacking(ctx) == 0 && highest < 0.2f, "back out of sight, the guards know nothing of it (%d attacking, highest awareness %.2f)",
					Kit::CountAttacking(ctx), highest);
			}
			else
			{
				ctx.Info("no spot out of the squad's sight to come back to");
			}

			// Then into the open where it fought: seen again, it takes the time a fresh sighting takes (sense tier), not zero.
			ctx.puppet.TeleportTo(home);
			ctx.puppet.Hold(Posture::Still);
			const double returned = ctx.Now();
			const bool fought = ctx.WaitUntil([&] { return Kit::CountAttacking(ctx) > 0; }, 10.0f);
			const double after = ctx.Now() - returned;
			ctx.Metric("seconds_until_attack_after_returning_in_view", fought ? after : -1.0);
			ctx.Check(fought == false || after >= 0.3, "an armed player stepping back into view is not attacked at once (%.2f s)", after);
		}

		// "I flew straight up": the puppet climbs high above the squad, over open ground with nothing in the way, and hovers.
		// Nothing can reach it and it is far out of the sight and hearing ranges: the fight has to end.
		void FlyStraightUp(Ctx& ctx)
		{
			if (Kit::EngageSquad(ctx, 3) == false) return;
			ctx.Wait(3.0f);

			glm::vec3 position = ctx.puppet.Position();
			const float climbSpeed = 8.0f;
			const float height = 100.0f;

			for (float climbed = 0.0f; climbed < height; climbed += climbSpeed * StepSeconds)
			{
				position.y += climbSpeed * StepSeconds;
				ctx.puppet.TeleportTo(position);
				ctx.Step();
			}

			ctx.puppet.Park(position); // stays up there
			ctx.Info("hovering %.0f m up; %d attacking, %d seeing it", height, Kit::CountAttacking(ctx), Kit::CountSeeing(ctx));

			ExpectFightEnds(ctx, ctx.Now(), "flying_up");

			// And back down where it was: a quiet player in the open, not an old fight.
			ctx.Wait(10.0f);
			ctx.Check(Kit::CountAttacking(ctx) == 0, "10 s after the fight ended nobody is attacking again (%d are)", Kit::CountAttacking(ctx));
		}

		// Running away in the open: noisy and fast. Out of earshot the guards must give up too.
		void RunAway(Ctx& ctx)
		{
			if (Kit::EngageSquad(ctx, 3) == false) return;

			ctx.Wait(4.0f);

			const glm::vec3 centre = Kit::Centroid(ctx);
			const glm::vec3 from = ctx.puppet.Position();
			const float away = World::YawDegrees(from - centre);

			glm::vec3 target;
			bool found = false;
			for (float distance : { 40.0f, 30.0f, 22.0f, 15.0f })
			{
				if (ctx.world.FindClearSpot(from, distance, away, 35.0f, target, 3.0f))
				{
					found = true;
					break;
				}
			}

			if (found == false)
			{
				ctx.Info("no open ground to run away over from the fight position");
				return;
			}

			ctx.Info("running %.1f m away to %.1f %.1f", glm::distance(from, target), target.x, target.z);
			if (ctx.Travel(target, Posture::Run, 30.0f) == false) return;

			// Then a hidden spot far from the squad.
			glm::vec3 hidden;
			if (Kit::HiddenFromEveryone(ctx, ctx.puppet.Position(), 25.0f, hidden) && ctx.Travel(hidden, Posture::Run, 60.0f) == false)
				return;

			ctx.puppet.Hold(Posture::CrouchStill);
			ExpectFightEnds(ctx, ctx.Now(), "run_away");
		}

		// Sounds from far outside the guards' hearing must not keep a fight going.
		void NoiseFarAwayDoesNotExtendFight(Ctx& ctx)
		{
			if (Kit::EngageSquad(ctx, 3) == false) return;

			ctx.Wait(3.0f);

			const glm::vec3 home = ctx.puppet.Position();
			ctx.puppet.Park(home + glm::vec3(300.0f, 60.0f, 300.0f));

			// A footstep and a shot every 2 s from 300 m away: a footstep radius is 8 m, a shot 40 m.
			double nextNoise = ctx.Now();
			ExpectFightEnds(ctx, ctx.Now(), "far_noise", [&]
				{
					if (ctx.Now() < nextNoise) return;
					nextNoise = ctx.Now() + 2.0;
					ctx.puppet.Emit(NpcSoundType::Footstep);
					ctx.puppet.Shoot();
				});
		}

		// ── Flanking ─────────────────────────────────────────────────────────────────────────────────────────

		// The puppet fights in the open, then ducks out of sight close to the squad. Flankers should go round, stack at
		// a doorway and breach. The check is that none of them stands still on the way for long.
		void FlankersImpl(Ctx& ctx, int attackersNeeded)
		{
			if (Kit::EngageSquad(ctx, attackersNeeded) == false) return;

			ctx.Wait(4.0f);

			// Duck out of every guard's sight at once, as near the fight as the map allows (the squad then has to come for it).
			glm::vec3 hidden;
			bool found = false;
			for (float distance : { 10.0f, 6.0f })
			{
				found = ctx.world.FindSpotHiddenFromAll(Kit::Eyes(ctx), ctx.puppet.Position(), distance, 40.0f, hidden);
				if (found) break;
			}

			if (found == false)
			{
				ctx.Info("no spot within 40 m of the fight that none of the guards can see");
				return;
			}

			ctx.puppet.TeleportTo(hidden);
			ctx.puppet.Hold(Posture::CrouchStill);
			ctx.Info("puppet hides at %.1f %.1f %.1f", hidden.x, hidden.y, hidden.z);

			struct Track
			{
				double stillSince = -1.0;
				double longestStill = 0.0;
				bool everFlank = false;
				bool breached = false;
				bool laneDone = false;
				bool stacked = false;
				double stackedSince = -1.0;
				double longestStacked = 0.0;
				float closestWhileStacked = 1.0e9f;
				double longestWaiting = 0.0;   // ordered to flank, in any lane phase before breaching, and not moving
				double waitingSince = -1.0;
			};

			std::map<std::string, Track> tracks;
			const double started = ctx.Now();
			float closest = 1.0e9f;
			double firstBreach = -1.0;
			double firstFlankOrder = -1.0;
			const double duration = 45.0;

			while (ctx.Now() - started < duration)
			{
				ctx.Step();

				for (const Snapshot& s : Kit::Alive(ctx))
				{
					Track& track = tracks[s.name + "/" + s.id];

					const bool flanking = IsRole(s, "flank");
					const bool movingLane = IsLane(s, "moving");

					if (flanking)
					{
						track.everFlank = true;
						if (firstFlankOrder < 0.0) firstFlankOrder = ctx.Now() - started;
					}
					if (IsLane(s, "stacked"))
					{
						track.stacked = true;
						if (track.stackedSince < 0.0) track.stackedSince = ctx.Now();
						track.longestStacked = std::max(track.longestStacked, ctx.Now() - track.stackedSince);
						track.closestWhileStacked = std::min(track.closestWhileStacked, s.distanceToPlayer);
					}
					else
					{
						track.stackedSince = -1.0;
					}

					// Ordered to flank, hasn't breached yet, and standing: whatever the lane phase is called.
					if (flanking && s.speed < 0.3f && IsLane(s, "breaching") == false && IsLane(s, "done") == false)
					{
						if (track.waitingSince < 0.0) track.waitingSince = ctx.Now();
						track.longestWaiting = std::max(track.longestWaiting, ctx.Now() - track.waitingSince);
					}
					else
					{
						track.waitingSince = -1.0;
					}
					if (IsLane(s, "breaching"))
					{
						track.breached = true;
						if (firstBreach < 0.0) firstBreach = ctx.Now() - started;
					}
					if (IsLane(s, "done")) track.laneDone = true;

					// "Stalled": ordered to flank and walking its lane, yet not moving.
					if (flanking && movingLane && s.speed < 0.3f)
					{
						if (track.stillSince < 0.0) track.stillSince = ctx.Now();
						track.longestStill = std::max(track.longestStill, ctx.Now() - track.stillSince);
					}
					else
					{
						track.stillSince = -1.0;
					}

					closest = std::min(closest, s.distanceToPlayer);
				}
			}

			int flankers = 0, breachers = 0, stackers = 0;
			double worst = 0.0, worstStacked = 0.0, worstWaiting = 0.0;
			float stackedClosest = 1.0e9f;
			for (const auto& entry : tracks)
			{
				worstStacked = std::max(worstStacked, entry.second.longestStacked);
				worstWaiting = std::max(worstWaiting, entry.second.longestWaiting);
				stackedClosest = std::min(stackedClosest, entry.second.closestWhileStacked);
				if (entry.second.everFlank) ++flankers;
				if (entry.second.breached) ++breachers;
				if (entry.second.stacked) ++stackers;
				worst = std::max(worst, entry.second.longestStill);
			}

			ctx.Metric("flankers_ordered", flankers);
			ctx.Metric("flankers_stacked", stackers);
			ctx.Metric("flankers_breached", breachers);
			ctx.Metric("first_flank_order_seconds", firstFlankOrder);
			ctx.Metric("first_breach_seconds", firstBreach);
			ctx.Metric("longest_stall_while_moving_lane_seconds", worst);
			ctx.Metric("closest_guard_to_puppet_m", closest);
			ctx.Metric("longest_wait_in_stack_seconds", worstStacked);
			ctx.Metric("longest_flanker_standing_before_breach_seconds", worstWaiting);
			ctx.Metric("closest_to_puppet_while_stacked_m", stackedClosest < 1.0e8f ? stackedClosest : -1.0f);

			ctx.Check(flankers > 0, "the squad sends at least one flanker against a hidden target (%d)", flankers);
			ctx.Check(worst <= 6.0, "no flanker stands still more than 6 s on its way (worst %.1f s)", worst);
			ctx.Check(worstWaiting <= 6.0, "no flanker waits more than 6 s before breaching, in any lane phase (worst %.1f s, %.1f s of it stacked)", worstWaiting, worstStacked);
			ctx.Check(flankers == 0 || breachers > 0, "a breach happens within %.0f s of the puppet hiding (%d breached)", duration, breachers);
			ctx.Check(closest < 8.0f, "someone gets within 8 m of the hidden puppet within %.0f s (closest %.1f m)", duration, closest);
		}

		void FlankersDoNotStall(Ctx& ctx) { FlankersImpl(ctx, 4); }

		// The same, with only two guards left: a smaller squad still has to do something.
		void TwoGuardsStillFlank(Ctx& ctx)
		{
			// Keep the two guards nearest to the start, remove the rest.
			const glm::vec3 start = ctx.puppet.Position();
			std::vector<NpcBase*> npcs = ctx.world.Npcs();
			std::sort(npcs.begin(), npcs.end(), [&](NpcBase* a, NpcBase* b)
				{
					return glm::distance(a->Position, start) < glm::distance(b->Position, start);
				});
			for (size_t i = 2; i < npcs.size(); ++i)
				npcs[i]->Destroy();

			ctx.Step();
			FlankersImpl(ctx, 2);
		}
	}

	void RegisterChaseScenarios()
	{
		Scenario scenario;

		scenario.tier = "chase";
		scenario.watchKnowledge = true;
		scenario.timeLimit = 360.0f;

		scenario.name = "chase_escape_to_hidden_spot";
		scenario.run = EscapeToHiddenSpot;
		Register(scenario);

		scenario.name = "chase_fly_away_and_return";
		scenario.run = FlyAwayAndReturn;
		Register(scenario);

		scenario.name = "chase_fly_straight_up";
		scenario.run = FlyStraightUp;
		Register(scenario);

		scenario.name = "chase_run_away";
		scenario.run = RunAway;
		Register(scenario);

		scenario.name = "chase_noise_far_away";
		scenario.run = NoiseFarAwayDoesNotExtendFight;
		Register(scenario);

		scenario.tier = "squad";
		scenario.timeLimit = 200.0f;

		scenario.name = "squad_flankers_do_not_stall";
		scenario.run = FlankersDoNotStall;
		Register(scenario);

		scenario.name = "squad_two_guards_flank";
		scenario.run = TwoGuardsStillFlank;
		Register(scenario);
	}
}

#endif

// Tier "stealth": M1 of NPC_AI_DESIGN.md (§6.2, §12, §22) on a spawned `npc` guard: its routine, the way it looks into a
// noise, how its alert rises and falls, save and load in the middle of an investigation, and the two walkthroughs of
// section 22 that decide whether stealth is playable (W1: crouching behind a patrolling guard, W2: spotted at distance,
// then out of sight before it knows who).
//
// Every number a check compares against comes from GameData/ai/*.json, with a tolerance for the update granularity.

#include "ScenarioKit.h"

#ifndef DISTRIBUTION

#include <AiPerception/AiPerceptionSystem.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace AiTest
{
	namespace
	{
		constexpr float WalkTolerance = 0.2f;   // measured pace against the profile's
		constexpr double TimeTolerance = 0.35;  // seconds against the data, for a reaction measured in steps and a body that has to get going

		bool Starts(const std::string& text, const char* prefix)
		{
			return text.compare(0, std::strlen(prefix), prefix) == 0;
		}

		// A guard with a post of its own: a task point under it, so that the routine has something to come back to.
		struct PostedGuard
		{
			Npc* guard = nullptr;
			glm::vec3 post{};   // the floor of its post
			float yaw = 0.0f;
		};

		PostedGuard SpawnPostedGuard(Ctx& ctx, const glm::vec3& floor, float yaw)
		{
			Kit::SpawnTaskPoint(ctx, "post", floor);
			PostedGuard posted;
			posted.guard = &Kit::SpawnGuard(ctx, "guard", floor, yaw, "post");
			posted.post = floor;
			posted.yaw = yaw;
			ctx.Wait(3.0f); // settles on its post
			return posted;
		}

		// What an investigation looks like from outside, step by step.
		struct Watch
		{
			double noiseAt = 0.0;          // game time of the noise
			double glanceAt = -1.0;        // the head turned to it
			double walkAt = -1.0;          // the guard started to move
			double lookAt = -1.0;          // the look-around began
			double doneAt = -1.0;          // it went back to its routine (the concern is settled)
			bool wasInvestigating = false;
			std::vector<float> paces;      // speed while walking to the post
			float distanceAtLook = -1.0f;
			float closest = 1.0e9f;        // nearest it came to the point
			int investigators = 0;         // most investigators the director had at once
			int situations = 0;            // most investigate situations at once
		};

		// One step of the scenario while a guard looks into `point`.
		void Observe(Ctx& ctx, Npc& guard, const glm::vec3& point, Watch& watch)
		{
			ctx.Step();

			const Snapshot* snapshot = ctx.probe.Find(guard.Name);
			const double now = Time::GameTime;
			const std::string skill = guard.CurrentSkillId();
			const std::string description = guard.CurrentSkillDescription(now);

			if (watch.glanceAt < 0.0 && guard.GetBody().Glancing(now))
				watch.glanceAt = now;

			if (watch.walkAt < 0.0 && snapshot && snapshot->speed > 0.5f)
				watch.walkAt = now;

			const bool investigating = skill == "investigate";

			if (investigating && Starts(description, "approach") && snapshot && snapshot->speed > 0.5f && watch.walkAt >= 0.0 && now - watch.walkAt > 0.6)
				watch.paces.push_back(snapshot->speed);

			if (investigating && Starts(description, "look around") && watch.lookAt < 0.0)
			{
				watch.lookAt = now;
				watch.distanceAtLook = Kit::Flat(guard.Position, point);
			}

			if (watch.wasInvestigating && investigating == false && watch.doneAt < 0.0 && guard.GetMind().TopConcern() == nullptr)
				watch.doneAt = now;
			watch.wasInvestigating |= investigating;

			if (watch.lookAt < 0.0)
				watch.closest = std::min(watch.closest, Kit::Flat(guard.Position, point));

			if (const NpcDirector* director = NpcDirector::Get())
			{
				int investigators = 0;
				int situations = 0;
				for (const auto& situation : director->Situations())
				{
					if (situation->Kind() != SituationKind::Investigate) continue;
					++situations;
					for (const SituationMember& member : situation->Members())
						investigators += member.order.role == NpcRole::Investigator ? 1 : 0;
				}
				watch.investigators = std::max(watch.investigators, investigators);
				watch.situations = std::max(watch.situations, situations);
			}
		}

		// A loud noise (a thrown object with a 20 m radius) the guard has to turn round for: the puppet makes it from 8.5 m behind it and is gone (so nothing is left to see
		// when the investigator gets there). Returns the place of the noise; false (with the reason said) when the map has none.
		bool MakeNoise(Ctx& ctx, const Npc& guard, float yaw, NpcSoundType type, glm::vec3& place, float distance = 8.5f)
		{
			if (ctx.world.FindClearSpot(guard.Position, distance, yaw + 180.0f, 25.0f, place, 1.0f) == false)
			{
				ctx.Abort("no open ground %.1f m behind the guard", distance);
				return false;
			}

			ctx.puppet.TeleportTo(place);
			ctx.puppet.Hold(Posture::Still);
			ctx.Step();
			ctx.puppet.Emit(type, 20.0f);
			ctx.Step();
			ctx.puppet.Park(place + Kit::FarAway);
			return true;
		}

		// ── Routine ──────────────────────────────────────────────────────────────────────────────────────────

		// A guard with a post walks to it and holds it.
		void RoutinePost(Ctx& ctx)
		{
			const glm::vec3 floor = Kit::CleanRoom(ctx);

			glm::vec3 spot;
			if (ctx.world.FindClearSpot(floor, 8.0f, 0.0f, 180.0f, spot, 2.0f) == false)
				ctx.Abort("no open ground 8 m from the start");

			const glm::vec3 post = spot - glm::vec3(0.0f, 1.4f, 0.0f);
			Kit::SpawnTaskPoint(ctx, "post_far", post);
			Npc& guard = Kit::SpawnGuard(ctx, "guard", floor, 0.0f, "post_far");

			const double started = ctx.Now();
			const float distance = Kit::Flat(floor, post);
			const float walk = guard.GetProfile().walkSpeed;
			const bool arrived = ctx.WaitUntil([&] { return Kit::Flat(guard.Position, post) < 1.0f; }, distance / walk * 1.6f + 6.0f);

			ctx.Metric("seconds_to_reach_the_post", arrived ? ctx.Now() - started : -1.0);
			ctx.Check(arrived, "the guard walks %.1f m to its post at %.1f m/s (%.1f s)", distance, walk, ctx.Now() - started);
			if (arrived == false) return;

			ctx.Check(ctx.Now() - started >= distance / (walk * 1.3f), "and at about that pace, not by magic (%.1f s for %.1f m)", ctx.Now() - started, distance);

			// Held: stands on it, still, doing its routine, for as long as it is left alone (after a moment to come to a stop).
			ctx.Wait(1.5f);
			const bool held = ctx.HoldsFor([&]
			{
				const Snapshot* snapshot = ctx.probe.Find("guard");
				return Kit::Flat(guard.Position, post) < 1.2f && snapshot && snapshot->speed < 0.5f && std::string(guard.CurrentSkillId()) == "routine";
			}, 10.0f);

			ctx.Check(held, "it holds the post for 10 s: on it, still, running its routine (%s, %.1f m off)", guard.CurrentSkillId(), Kit::Flat(guard.Position, post));
			ctx.Check(guard.GetMind().alert == NpcAlert::Calm, "and stays calm");
		}

		// Three task points in a loop: the guard visits them in order, lap after lap, and never stalls on the way.
		void RoutinePatrol(Ctx& ctx)
		{
			const glm::vec3 floor = Kit::CleanRoom(ctx);

			glm::vec3 second, third;
			if (ctx.world.FindClearSpot(floor, 8.0f, 0.0f, 180.0f, second, 2.0f) == false)
				ctx.Abort("no open ground 8 m from the start");

			const glm::vec3 secondFloor = second - glm::vec3(0.0f, 1.4f, 0.0f);

			bool found = false;
			for (const float turn : { 110.0f, -110.0f, 70.0f, -70.0f, 150.0f, -150.0f })
			{
				if (ctx.world.FindClearSpot(secondFloor, 8.0f, World::YawDegrees(secondFloor - floor) + turn, 25.0f, third, 2.0f))
				{
					found = true;
					break;
				}
			}
			if (found == false)
				ctx.Abort("no third corner for a patrol");

			const glm::vec3 thirdFloor = third - glm::vec3(0.0f, 1.4f, 0.0f);

			const glm::vec3 corners[3] = { floor, secondFloor, thirdFloor };
			const char* names[3] = { "corner_a", "corner_b", "corner_c" };
			const float wait = 1.0f;

			Kit::SpawnTaskPoint(ctx, names[0], corners[0], names[1], wait);
			Kit::SpawnTaskPoint(ctx, names[1], corners[1], names[2], wait);
			Kit::SpawnTaskPoint(ctx, names[2], corners[2], names[0], wait);

			Npc& guard = Kit::SpawnGuard(ctx, "guard", floor, 0.0f, names[0]);

			// Visits: the corner it is within a metre of, in the order it gets there.
			std::vector<int> visits;
			std::vector<double> times;
			int stalled = 0;
			const int wanted = 7; // two laps and back at the start
			const float walk = guard.GetProfile().walkSpeed;

			double lastMoved = ctx.Now();
			const double limit = ctx.Now() + 3.0 * (Kit::Flat(corners[0], corners[1]) + Kit::Flat(corners[1], corners[2]) + Kit::Flat(corners[2], corners[0])) / walk * 2.0 + 40.0;

			while (static_cast<int>(visits.size()) < wanted && ctx.Now() < limit)
			{
				ctx.Step();

				for (int i = 0; i < 3; ++i)
				{
					if (Kit::Flat(guard.Position, corners[i]) < 1.0f && (visits.empty() || visits.back() != i))
					{
						visits.push_back(i);
						times.push_back(ctx.Now());
					}
				}

				const Snapshot* snapshot = ctx.probe.Find("guard");
				if (snapshot && snapshot->speed > 0.3f) lastMoved = ctx.Now();
				if (ctx.Now() - lastMoved > wait + 3.0)
				{
					++stalled;
					lastMoved = ctx.Now();
				}
			}

			ctx.Check(static_cast<int>(visits.size()) == wanted, "the guard visits %d corners within the time two laps take (%d)", wanted, static_cast<int>(visits.size()));

			bool inOrder = true;
			for (size_t i = 0; i < visits.size(); ++i)
				inOrder &= visits[i] == static_cast<int>(i % 3);
			ctx.Check(inOrder, "in the order of the loop, a, b, c, a, b, c, a");

			// Each leg takes the walk and the wait, no more than 1.6 times what the pace says plus a margin for turning.
			for (size_t i = 1; i < times.size(); ++i)
			{
				const int from = visits[i - 1];
				const int to = visits[i];
				const float length = Kit::Flat(corners[from], corners[to]);
				const double expected = length / walk + wait;
				const double took = times[i] - times[i - 1];

				char name[64];
				snprintf(name, sizeof(name), "leg_%d_seconds", static_cast<int>(i));
				ctx.Metric(name, took);

				ctx.Check(took <= expected * 1.6 + 2.5, "leg %d (%.1f m) takes %.1f s, the pace and the wait say %.1f s", static_cast<int>(i), length, took, expected);
			}

			ctx.Check(stalled == 0, "the guard never stands still on its way for longer than a wait (%d times)", stalled);
		}

		// ── Investigate ──────────────────────────────────────────────────────────────────────────────────────

		// The noise, from the first glance to the guard back on its post, against the numbers of the profile (§12).
		void InvestigateNoise(Ctx& ctx)
		{
			const NpcInvestigateTuning tuning = NpcProfile::Get("guard").investigate;

			const glm::vec3 floor = Kit::CleanRoom(ctx);
			const PostedGuard posted = SpawnPostedGuard(ctx, floor, 0.0f);
			Npc& guard = *posted.guard;

			glm::vec3 place;
			if (MakeNoise(ctx, guard, posted.yaw, NpcSoundType::ThrownObject, place) == false) return;

			Watch watch;
			watch.noiseAt = Time::GameTime;

			const double limit = ctx.Now() + 40.0;
			while (watch.doneAt < 0.0 && ctx.Now() < limit)
				Observe(ctx, guard, place, watch);

			ctx.Check(watch.glanceAt >= 0.0 && watch.glanceAt - watch.noiseAt <= 0.25, "the head turns to the noise at once (%.2f s)", watch.glanceAt - watch.noiseAt);

			const double reaction = watch.walkAt - watch.noiseAt;
			ctx.Metric("seconds_from_noise_to_first_step", reaction);
			ctx.Check(watch.walkAt >= 0.0 && reaction >= tuning.reactionTime - 0.1 && reaction <= tuning.reactionTime + 0.6,
				"it sets off after its reaction time (%.2f s, the data says %.2f)", reaction, tuning.reactionTime);

			if (watch.paces.empty() == false)
			{
				std::sort(watch.paces.begin(), watch.paces.end());
				const float pace = watch.paces[watch.paces.size() / 2];
				ctx.Metric("investigation_walk_speed", pace);
				ctx.Check(std::abs(pace - tuning.speed) <= WalkTolerance * tuning.speed, "it walks at the investigation pace (%.2f m/s, the data says %.2f)", pace, tuning.speed);
			}
			else
			{
				ctx.Check(false, "it walks to the noise (no steps measured on the way)");
			}

			ctx.Check(watch.lookAt >= 0.0, "it gets to a spot and looks around");
			if (watch.lookAt >= 0.0)
			{
				ctx.Check(watch.distanceAtLook >= tuning.postMin - 1.0f && watch.distanceAtLook <= tuning.postMax + 1.0f,
					"from a spot %.1f to %.1f m from the noise (it stands %.1f m away)", tuning.postMin, tuning.postMax, watch.distanceAtLook);

				const double looked = watch.doneAt - watch.lookAt;
				ctx.Metric("look_around_seconds", looked);
				ctx.Check(watch.doneAt >= 0.0 && std::abs(looked - tuning.lookAroundTime) <= TimeTolerance,
					"the look around takes its time (%.2f s, the data says %.2f)", looked, tuning.lookAroundTime);
			}

			ctx.Check(watch.doneAt >= 0.0 && guard.GetMind().TopConcern() == nullptr, "it gives up: nothing is left to look into");
			ctx.Check(watch.investigators == 1 && watch.situations == 1, "the director had one investigate situation with one investigator (%d, %d)", watch.situations, watch.investigators);

			// And back to the post.
			const float way = Kit::Flat(guard.Position, posted.post);
			const bool home = ctx.WaitUntil([&] { return Kit::Flat(guard.Position, posted.post) < 1.0f && std::string(guard.CurrentSkillId()) == "routine"; }, way / guard.GetProfile().walkSpeed * 1.6f + 6.0f);
			ctx.Check(home, "it walks back to its post and carries on (%.1f m off)", Kit::Flat(guard.Position, posted.post));

			const NpcDirector* director = NpcDirector::Get();
			ctx.Check(director != nullptr && director->Situations().empty(), "and no situation is left open");
		}

		// ── Alert ────────────────────────────────────────────────────────────────────────────────────────────

		// Calm, then Wary or Alarmed by what happened, then calm again on the timings of the data.
		void AlertCycle(Ctx& ctx)
		{
			const NpcAlertTuning alert = NpcPerceptionTuning::Get().alert;
			const NpcSearchTuning search = NpcSituationTuning::Get().search;

			const glm::vec3 floor = Kit::CleanRoom(ctx);

			// A noise: Wary at once, back to Calm a minute after the investigation is over.
			{
				const PostedGuard posted = SpawnPostedGuard(ctx, floor, 0.0f);
				Npc& guard = *posted.guard;

				ctx.Check(guard.GetMind().alert == NpcAlert::Calm, "a guard on its post is calm");

				glm::vec3 place;
				if (MakeNoise(ctx, guard, posted.yaw, NpcSoundType::ThrownObject, place) == false) return;

				const double noiseAt = Time::GameTime;
				ctx.Wait(0.5f);
				ctx.Check(guard.GetMind().alert == NpcAlert::Wary, "a noise makes it wary within half a second");

				Watch watch;
				watch.noiseAt = noiseAt;
				const double limit = ctx.Now() + 40.0;
				while (watch.doneAt < 0.0 && ctx.Now() < limit)
					Observe(ctx, guard, place, watch);

				ctx.Check(watch.doneAt >= 0.0, "the investigation ends");
				ctx.Check(guard.GetMind().alert == NpcAlert::Wary, "it stays wary when it ends");

				const double calmFrom = Time::GameTime;
				const bool calm = ctx.WaitUntil([&] { return guard.GetMind().alert == NpcAlert::Calm; }, alert.wearyToCalm + 10.0f);
				const double took = Time::GameTime - calmFrom;
				ctx.Metric("seconds_wary_to_calm", calm ? took : -1.0);
				ctx.Check(calm && std::abs(took - alert.wearyToCalm) <= 2.0, "and calm again %.0f s later (%.1f s)", alert.wearyToCalm, took);
			}

			// A shot: Alarmed at once, Wary when the search is over, Calm a minute later.
			{
				ctx.world.RemoveNpcs();

				const PostedGuard posted = SpawnPostedGuard(ctx, floor, 0.0f);
				Npc& guard = *posted.guard;

				glm::vec3 place;
				// Out of its sight: a shot it cannot see the shooter of is worth a search (one in plain view with nobody there is not).
				if (ctx.world.FindHiddenSpot(guard.observer->position, 10.0f, 25.0f, place) == false)
					ctx.Abort("no spot 10 to 25 m from the guard that it cannot see");

				ctx.puppet.TeleportTo(place);
				ctx.puppet.Hold(Posture::Still);
				ctx.Step();
				ctx.puppet.Shoot();
				ctx.Step();
				ctx.puppet.Park(place + Kit::FarAway);

				const double shotAt = Time::GameTime;
				const bool alarmed = ctx.WaitUntil([&] { return guard.GetMind().alert >= NpcAlert::Alarmed; }, 3.0f);
				ctx.Metric("seconds_from_shot_to_alarmed", alarmed ? Time::GameTime - shotAt : -1.0);
				ctx.Check(alarmed && Time::GameTime - shotAt <= 1.0, "a shot makes it alarmed within a second (%.2f s)", Time::GameTime - shotAt);

				const double window = search.timeout + 20.0;
				const bool wary = ctx.WaitUntil([&] { return guard.GetMind().alert == NpcAlert::Wary; }, static_cast<float>(window));
				const double searched = Time::GameTime - shotAt;
				ctx.Metric("seconds_alarmed", wary ? searched : -1.0);
				ctx.Check(wary, "the search ends and it is wary (%.1f s after the shot, the search lasts at most %.0f s)", searched, search.timeout);

				const double calmFrom = Time::GameTime;
				const bool calm = ctx.WaitUntil([&] { return guard.GetMind().alert == NpcAlert::Calm; }, alert.wearyToCalm + 10.0f);
				const double took = Time::GameTime - calmFrom;
				ctx.Metric("seconds_wary_to_calm_after_the_search", calm ? took : -1.0);
				ctx.Check(calm && std::abs(took - alert.wearyToCalm) <= 2.0, "and calm again %.0f s later (%.1f s)", alert.wearyToCalm, took);
			}
		}

		// ── Save and load ────────────────────────────────────────────────────────────────────────────────────

		// The entities save as the level saves them (Serialize) and load as it loads them (Deserialize on the same entity), with
		// the game time put back to where it was, in the middle of the walk to a noise. What they hold has to be the same, and the
		// investigation has to go on to its end.
		void SaveLoadMidInvestigation(Ctx& ctx)
		{
			const NpcInvestigateTuning tuning = NpcProfile::Get("guard").investigate;

			const glm::vec3 floor = Kit::CleanRoom(ctx);
			const PostedGuard posted = SpawnPostedGuard(ctx, floor, 0.0f);
			Npc& guard = *posted.guard;
			NpcDirector& director = NpcDirector::Ensure();

			glm::vec3 place;
			if (MakeNoise(ctx, guard, posted.yaw, NpcSoundType::ThrownObject, place) == false) return;

			Watch watch;
			watch.noiseAt = Time::GameTime;

			// Into the walk.
			while (watch.walkAt < 0.0 || Time::GameTime - watch.walkAt < 1.0)
			{
				Observe(ctx, guard, place, watch);
				if (ctx.Now() > 20.0) break;
			}

			ctx.Check(std::string(guard.CurrentSkillId()) == "investigate" && Starts(guard.CurrentSkillDescription(Time::GameTime), "approach"),
				"saved while walking to the noise (%s: %s)", guard.CurrentSkillId(), guard.CurrentSkillDescription(Time::GameTime).c_str());

			const double savedAt = Time::GameTime;
			const double savedNoPause = Time::GameTimeNoPause;
			const glm::vec3 savedPosition = guard.Position;

			json guardSaved, directorSaved;
			guard.Serialize(guardSaved);
			director.Serialize(directorSaved);

			ctx.Check(guardSaved.contains("npcBrain") && directorSaved.contains("npcDirector"), "the guard and the director have something to save");

			// Time goes on and the state moves away from what was saved.
			ctx.Wait(2.5f);

			Time::GameTime = savedAt;
			Time::GameTimeNoPause = savedNoPause;

			json guardLoad = guardSaved;
			json directorLoad = directorSaved;
			guard.Deserialize(guardLoad);
			director.Deserialize(directorLoad);

			json guardAfter, directorAfter;
			guard.Serialize(guardAfter);
			director.Serialize(directorAfter);

			std::string where;
			const bool mindSame = Kit::JsonNear(guardSaved["npcBrain"], guardAfter["npcBrain"], 1.0e-3, where);
			ctx.Check(mindSame, "the Mind and the skills are the same after the load%s%s", mindSame ? "" : " (differs at ", mindSame ? "" : (where + ")").c_str());

			where.clear();
			const bool directorSame = Kit::JsonNear(directorSaved["npcDirector"], directorAfter["npcDirector"], 1.0e-3, where);
			ctx.Check(directorSame, "and so are the director's situations%s%s", directorSame ? "" : " (differs at ", directorSame ? "" : (where + ")").c_str());

			ctx.Check(Kit::Flat(guard.Position, savedPosition) < 0.5f, "the guard is where it was (%.2f m)", Kit::Flat(guard.Position, savedPosition));
			ctx.Check(director.Situations().size() == 1 && director.Situations()[0]->Kind() == SituationKind::Investigate, "the investigation is still there");

			// And on from here.
			Watch rest;
			rest.noiseAt = watch.noiseAt;
			rest.walkAt = watch.walkAt;
			const double limit = ctx.Now() + 40.0;
			while (rest.doneAt < 0.0 && ctx.Now() < limit)
				Observe(ctx, guard, place, rest);

			ctx.Check(rest.lookAt >= 0.0 && rest.distanceAtLook >= tuning.postMin - 1.0f && rest.distanceAtLook <= tuning.postMax + 1.0f,
				"it gets to its spot and looks around (%.1f m from the noise)", rest.distanceAtLook);
			ctx.Check(rest.doneAt >= 0.0 && guard.GetMind().TopConcern() == nullptr, "and gives up as it would have");

			const bool home = ctx.WaitUntil([&] { return Kit::Flat(guard.Position, posted.post) < 1.0f && std::string(guard.CurrentSkillId()) == "routine"; }, 40.0f);
			ctx.Check(home && director.Situations().empty(), "it goes back to its post and the situation is closed");
		}

		// ── W1: crouching behind a patrolling guard ──────────────────────────────────────────────────────────

		// The player follows a walking guard 3 to 5 m behind, crouched. Its eyes look the other way: the awareness never gets to
		// Notice. The control is the same guard with the player in front of it, who does notice.
		void WalkthroughCrouchBehindPatrol(Ctx& ctx)
		{
			const NpcMeterTuning meter = NpcPerceptionTuning::Get().meter;

			const glm::vec3 floor = Kit::CleanRoom(ctx);

			glm::vec3 spot;
			if (ctx.world.FindClearSpot(floor, 14.0f, 0.0f, 180.0f, spot, 3.0f) == false)
				ctx.Abort("no open ground 14 m from the start");

			const glm::vec3 far = spot - glm::vec3(0.0f, 1.4f, 0.0f);
			const float yaw = World::YawDegrees(far - floor);

			Kit::SpawnTaskPoint(ctx, "patrol_end", far);
			Npc& guard = Kit::SpawnGuard(ctx, "guard", floor, yaw, "patrol_end");

			const std::string playerId = ctx.puppet.Id();

			// The puppet is 4 m behind it, crouched and still, until it moves off.
			glm::vec3 behind;
			if (ctx.world.FindClearSpot(guard.Position, 4.0f, yaw + 180.0f, 30.0f, behind, 1.0f) == false)
				ctx.Abort("no open ground 4 m behind the guard");

			ctx.puppet.TeleportTo(behind);
			ctx.puppet.Hold(Posture::CrouchStill);

			float highest = 0.0f;
			float nearest = 1.0e9f;
			float farthest = 0.0f;
			float walked = 0.0f;
			glm::vec3 last = guard.Position;
			double nextMove = 0.0;

			const double limit = ctx.Now() + 30.0;
			while (Kit::Flat(guard.Position, far) > 1.0f && ctx.Now() < limit)
			{
				// Keeps its distance: 4 m behind, along the way the guard walks.
				if (ctx.Now() >= nextMove)
				{
					nextMove = ctx.Now() + 0.4;

					const glm::vec3 toGuard = guard.Position - ctx.puppet.Position();
					if (Kit::Flat(guard.Position, ctx.puppet.Position()) > 4.4f)
					{
						const glm::vec3 direction = glm::normalize(glm::vec3(far.x - guard.Position.x, 0.0f, far.z - guard.Position.z));
						glm::vec3 wanted = guard.Position - direction * 3.8f;
						glm::vec3 snapped;
						if (ctx.world.SnapToFloor(wanted, snapped))
							ctx.puppet.MoveTo(snapped + glm::vec3(0.0f, 1.4f, 0.0f), Posture::CrouchWalk);
					}
					else if (glm::length(toGuard) < 3.4f)
					{
						ctx.puppet.Hold(Posture::CrouchStill);
					}
				}

				ctx.Step();

				highest = std::max(highest, guard.GetMind().MeterValue(playerId));
				const float distance = Kit::Flat(guard.Position, ctx.puppet.Position());
				nearest = std::min(nearest, distance);
				farthest = std::max(farthest, distance);
				walked += Kit::Flat(guard.Position, last);
				last = guard.Position;
			}

			ctx.Metric("highest_meter_behind_the_guard", highest);
			ctx.Check(walked > 10.0f, "the guard patrolled %.1f m with the puppet behind it (%.1f to %.1f m away)", walked, nearest, farthest);
			ctx.Check(farthest < 8.0f && nearest > 1.5f, "close enough to matter: it never got more than 8 m away or within the near zone (%.1f to %.1f m)", nearest, farthest);
			ctx.Check(highest < meter.notice, "the awareness stays below Notice (%.2f against %.2f)", highest, meter.notice);
			ctx.Check(guard.GetMind().Concerns().empty() && guard.GetMind().alert == NpcAlert::Calm, "the guard has no concern and is calm");

			// Control: now the player walks round in front of it, standing up: it does notice.
			glm::vec3 front;
			if (ctx.world.FindClearSpot(guard.Position, 6.0f, World::YawDegrees(guard.observer->forward), 25.0f, front, 1.5f) == false)
			{
				ctx.Info("no open ground 6 m in front of the guard: no control");
				return;
			}

			ctx.Travel(front, Posture::Walk, 20.0f);
			ctx.puppet.Hold(Posture::Still);

			float controlHighest = 0.0f;
			ctx.WaitUntil([&]
			{
				controlHighest = std::max(controlHighest, guard.GetMind().MeterValue(playerId));
				return controlHighest >= meter.notice;
			}, 8.0f);

			ctx.Check(controlHighest >= meter.notice, "control: in front of the same guard it does notice (%.2f)", controlHighest);
		}

		// ── W2: spotted at distance, then out of sight before it knows who ───────────────────────────────────

		// The player stands in the open 16 to 22 m from a guard, one step from cover. The awareness fills; before it is full the
		// player steps out of sight. The guard goes to where it saw something and looks: an investigation, not a fight.
		void WalkthroughSpottedThenGone(Ctx& ctx)
		{
			const NpcMeterTuning meter = NpcPerceptionTuning::Get().meter;
			const NpcInvestigateTuning tuning = NpcProfile::Get("guard").investigate;

			const glm::vec3 floor = Kit::CleanRoom(ctx);

			// Where the guard stands is decided by the map: try the start, then the places round it.
			glm::vec3 seen, hidden;
			Npc* probe = &Kit::SpawnGuard(ctx, "guard", floor, 0.0f);
			ctx.Wait(1.0f);

			if (ctx.world.FindEdgeSpot(probe->observer->position, 16.0f, 22.0f, 1.6f, seen, hidden) == false)
			{
				ctx.Abort("no spot 16 to 22 m from the guard with cover a step away");
				return;
			}

			ctx.world.RemoveNpcs();

			const float yaw = World::YawDegrees(seen - floor);
			Kit::SpawnTaskPoint(ctx, "post", floor);
			Npc& guard = Kit::SpawnGuard(ctx, "guard", floor, yaw, "post");
			ctx.Wait(3.0f);

			const std::string playerId = ctx.puppet.Id();
			const float distance = Kit::Flat(guard.Position, seen);

			ctx.puppet.TeleportTo(seen);
			ctx.puppet.Hold(Posture::CrouchStill);

			// Seen: the meter climbs. At 0.75 (past Suspect) the player steps behind cover.
			const bool filling = ctx.WaitUntil([&] { return guard.GetMind().MeterValue(playerId) >= 0.75f; }, 20.0f);
			ctx.Check(filling, "spotted at %.0f m the awareness fills (%.2f)", distance, guard.GetMind().MeterValue(playerId));
			if (filling == false) return;

			ctx.puppet.MoveTo(hidden, Posture::CrouchWalk);

			float highest = 0.0f;
			bool threat = false;
			bool combat = false;
			bool curious = false;
			bool hiddenNow = false;

			auto note = [&]
			{
				highest = std::max(highest, guard.GetMind().MeterValue(playerId));
				combat |= guard.GetMind().alert == NpcAlert::Combat;
				for (const NpcConcern& concern : guard.GetMind().Concerns())
				{
					threat |= concern.type == NpcConcernType::Threat;
					curious |= concern.type == NpcConcernType::Curious;
				}
			};

			// The step out of sight: walked until the puppet arrives (it stops within 0.6 m of the spot), then the last bit is closed.
			const double limit = ctx.Now() + 8.0;
			while (ctx.Now() < limit && hiddenNow == false)
			{
				ctx.Step();
				note();

				if (ctx.puppet.Moving() == false)
				{
					ctx.puppet.TeleportTo(hidden);
					ctx.puppet.Hold(Posture::CrouchStill);
					hiddenNow = true;
				}
			}

			ctx.Wait(0.5f);
			note();
			const Snapshot* afterStep = ctx.probe.Find("guard");
			ctx.Check(hiddenNow && afterStep && afterStep->perceives == false, "the player gets out of sight");
			ctx.Check(highest < meter.identify, "before the awareness is full (%.2f of %.2f)", highest, meter.identify);

			ctx.puppet.Park(seen + Kit::FarAway);


			// The guard looks into it.
			Watch watch;
			watch.noiseAt = Time::GameTime;
			const double investigateLimit = ctx.Now() + 40.0;
			while (watch.doneAt < 0.0 && ctx.Now() < investigateLimit)
			{
				Observe(ctx, guard, seen, watch);
				note();
			}

			ctx.Check(curious, "the guard is curious about it");
			ctx.Check(threat == false && combat == false, "and never takes it for a fight (threat %d, combat %d)", threat ? 1 : 0, combat ? 1 : 0);
			ctx.Check(watch.closest <= tuning.postMax + 1.5f, "it goes to where it saw something (down to %.1f m of it)", watch.closest);
			ctx.Check(watch.lookAt >= 0.0 && watch.doneAt >= 0.0, "looks around and gives up");

			const bool home = ctx.WaitUntil([&] { return Kit::Flat(guard.Position, floor) < 1.0f && std::string(guard.CurrentSkillId()) == "routine"; }, 40.0f);
			ctx.Check(home, "and goes back to its post");
		}
	}

	void RegisterStealthScenarios()
	{
		Scenario scenario;
		scenario.tier = "stealth";
		scenario.timeLimit = 400.0f;

		scenario.name = "stealth_routine_post";
		scenario.run = RoutinePost;
		Register(scenario);

		scenario.name = "stealth_routine_patrol";
		scenario.run = RoutinePatrol;
		Register(scenario);

		scenario.name = "stealth_investigate_noise";
		scenario.run = InvestigateNoise;
		Register(scenario);

		scenario.name = "stealth_alert_cycle";
		scenario.run = AlertCycle;
		Register(scenario);

		scenario.name = "stealth_save_load_mid_investigation";
		scenario.run = SaveLoadMidInvestigation;
		Register(scenario);

		scenario.name = "stealth_w1_crouch_behind_patrol";
		scenario.run = WalkthroughCrouchBehindPatrol;
		Register(scenario);

		scenario.name = "stealth_w2_spotted_then_gone";
		scenario.run = WalkthroughSpottedThenGone;
		Register(scenario);
	}
}

#endif

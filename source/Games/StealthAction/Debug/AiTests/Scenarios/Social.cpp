// Tier "social": M2 core of NPC_AI_DESIGN.md (§7, §10, §11, §12) on spawned `npc` guards: what they tell each other and
// how far it carries, who takes which part in an investigation or a search, and the way a guard that loses a hostile
// goes through chase, search and giving up. The invariant of the tier is the one of the whole design: no knowledge without
// perception (the runner checks it on every step of these scenarios).
//
// A message is queued the way a mind queues it (its outbox); everything after that is the director's.

#include "ScenarioKit.h"

#ifndef DISTRIBUTION

#include <AiPerception/AiPerceptionSystem.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace AiTest
{
	namespace
	{
		// The body centre is this far above the floor (the helpers of the world hand out centre points).
		const glm::vec3 CentreOffset(0.0f, 1.4f, 0.0f);

		glm::vec3 FloorOf(const glm::vec3& centre)
		{
			return centre - CentreOffset;
		}

		const NpcRecord* RecordOf(const Npc& npc, const std::string& subject)
		{
			return npc.GetMind().memory.Find(subject, NpcRecordKind::Person);
		}

		// A hostile somebody saw at `at`, as the sender knew it (the record every message of these scenarios carries).
		NpcMessage MessageAbout(NpcMessageKind kind, const std::string& subject, const glm::vec3& at)
		{
			NpcRecord record;
			record.subject = subject;
			record.kind = NpcRecordKind::Person;
			record.position = at;
			record.time = Time::GameTime;
			record.lastSeen = Time::GameTime;
			record.source = NpcRecordSource::Saw;
			record.certainty = 1.0f;
			record.detail = "hostile";

			NpcMessage message;
			message.kind = kind;
			message.records.push_back(record);
			return message;
		}

		// The sender queues it; the director takes it from there.
		void Send(Npc& sender, const NpcMessage& message)
		{
			sender.TestMind().outbox.messages.push_back(message);
		}

		// Steps until `npc` knows about `subject`; returns the seconds it took (-1 when it did not within `timeout`).
		double SecondsUntilTold(Ctx& ctx, const Npc& npc, const std::string& subject, float timeout)
		{
			const double started = Time::GameTime;
			const bool told = ctx.WaitUntil([&] { return RecordOf(npc, subject) != nullptr; }, timeout);
			return told ? Time::GameTime - started : -1.0;
		}

		// `count` floor points round `centre` (a floor point), `distance` away with a view of it, one every 360 / count degrees.
		bool Ring(Ctx& ctx, const glm::vec3& centre, int count, float distance, float firstYaw, std::vector<glm::vec3>& floors)
		{
			floors.clear();
			for (int i = 0; i < count; ++i)
			{
				glm::vec3 spot;
				if (ctx.world.FindClearSpot(centre, distance, firstYaw + 360.0f * static_cast<float>(i) / static_cast<float>(count), 30.0f, spot, 1.5f) == false)
					return false;
				floors.push_back(FloorOf(spot));
			}
			return true;
		}

		const SearchSituation* FindSearch(const std::string& subject)
		{
			const NpcDirector* director = NpcDirector::Get();
			if (director == nullptr) return nullptr;
			return dynamic_cast<const SearchSituation*>(director->FindSituation(SituationKind::Search, subject));
		}

		int CountRole(const NpcSituation& situation, NpcRole role)
		{
			int count = 0;
			for (const SituationMember& member : situation.Members())
				count += member.order.role == role ? 1 : 0;
			return count;
		}

		// ── Shouts ───────────────────────────────────────────────────────────────────────────────────────────

		// A shout carries as far as a sound does: a guard in the open within the radius is told after the delay, one beyond it is
		// not, and one behind a wall is not either (the wall adds to the distance).
		void ShoutReachesRange(Ctx& ctx)
		{
			const NpcMessageTuning& tuning = NpcSituationTuning::Get().messages;

			const glm::vec3 floor = Kit::CleanRoom(ctx);
			Npc& shouter = Kit::SpawnGuard(ctx, "shouter", floor, 0.0f);

			glm::vec3 nearSpot, farSpot;
			if (ctx.world.FindClearSpot(floor, 8.0f, 0.0f, 180.0f, nearSpot, 2.0f) == false)
				ctx.Abort("no open ground 8 m from the shouter");
			if (ctx.world.FindClearSpot(floor, tuning.shout.radius + 9.0f, 0.0f, 180.0f, farSpot, 3.0f) == false)
				ctx.Abort("no open ground %.0f m from the shouter", tuning.shout.radius + 9.0f);

			Npc& inRange = Kit::SpawnGuard(ctx, "in_range", FloorOf(nearSpot), 0.0f);
			Npc& outOfRange = Kit::SpawnGuard(ctx, "out_of_range", FloorOf(farSpot), 0.0f);

			// Close by, but with a wall between: the straight distance says in range, the muffled one does not.
			Npc* walled = nullptr;
			glm::vec3 behind;
			if (ctx.world.FindHiddenSpot(shouter.observer->position, 7.0f, tuning.shout.radius - 1.0f, behind))
				walled = &Kit::SpawnGuard(ctx, "behind_wall", FloorOf(behind), 0.0f);

			ctx.Wait(2.0f);

			const float muffled = walled ? AiPerceptionSystem::MuffledDistance(shouter.Position, walled->observer->position) : 0.0f;
			const bool wallCounts = walled != nullptr && muffled > tuning.shout.radius;
			if (walled && wallCounts == false)
				ctx.Info("the spot behind a wall is %.1f m muffled, inside the radius: that guard is not part of the check", muffled);

			const std::string subject = "intruder_1";
			const double sentAt = Time::GameTime;
			Send(shouter, MessageAbout(NpcMessageKind::Shout, subject, floor + glm::vec3(3.0f, 0.0f, 0.0f)));

			const double told = SecondsUntilTold(ctx, inRange, subject, 3.0f);
			ctx.Metric("seconds_to_hear_the_shout", told);
			ctx.Check(told >= tuning.shout.delay - 0.1 && told <= tuning.shout.delay + 0.35, "the guard in range (%.1f m, %.1f m muffled) is told after the shout's delay (%.2f s, the data says %.2f)",
				Kit::Flat(shouter.Position, inRange.Position), AiPerceptionSystem::MuffledDistance(shouter.Position, inRange.observer->position), told, tuning.shout.delay);

			ctx.Wait(1.0f);

			const NpcRecord* record = RecordOf(inRange, subject);
			if (record != nullptr)
			{
				ctx.Check(record->source == NpcRecordSource::Told, "what it knows is marked as told");
				ctx.Check(std::abs(record->certainty - tuning.toldCertainty) < 0.01f, "with the teller's certainty times %.1f (%.2f)", tuning.toldCertainty, record->certainty);
				ctx.Check(std::abs(record->time - sentAt) < 0.1, "and the time it was true, not the time it was told (%.2f s before the message)", record->time - sentAt);
			}
			ctx.Check(inRange.GetMind().alert >= NpcAlert::Alarmed, "a hostile somebody saw makes it alarmed (%s)", NpcAlertName(inRange.GetMind().alert));

			ctx.Check(RecordOf(outOfRange, subject) == nullptr && outOfRange.GetMind().alert == NpcAlert::Calm,
				"the guard out of range (%.1f m, the radius is %.0f) is told nothing and stays calm", Kit::Flat(shouter.Position, outOfRange.Position), tuning.shout.radius);

			if (wallCounts)
			{
				ctx.Check(RecordOf(*walled, subject) == nullptr && walled->GetMind().alert == NpcAlert::Calm,
					"the guard behind a wall (%.1f m straight, %.1f m muffled) is told nothing", Kit::Flat(shouter.Position, walled->Position), muffled);
			}

			ctx.Check(NpcDirector::Get() != nullptr && NpcDirector::Get()->PendingMessages() == 0, "nothing is left in the queue");
		}

		// A shouter that cannot act when its shout is due says nothing: dead or destroyed, the message is dropped and the dead
		// one holds no part in anything.
		void ShouterGoneBeforeDelivery(Ctx& ctx, bool destroy)
		{
			const NpcMessageTuning& tuning = NpcSituationTuning::Get().messages;

			const glm::vec3 floor = Kit::CleanRoom(ctx);

			std::vector<glm::vec3> floors;
			if (Ring(ctx, floor, 2, 6.0f, 0.0f, floors) == false)
				ctx.Abort("no open ground 6 m round the listener");

			Npc& listener = Kit::SpawnGuard(ctx, "listener", floor, 0.0f);
			Npc& control = Kit::SpawnGuard(ctx, "control", floors[0], 0.0f);
			Npc& doomed = Kit::SpawnGuard(ctx, "doomed", floors[1], 0.0f);
			const std::string doomedId = doomed.Id;

			ctx.Wait(2.0f);

			// The control: the same shout from a guard that lives is heard.
			Send(control, MessageAbout(NpcMessageKind::Shout, "intruder_alive", floor));
			const double heard = SecondsUntilTold(ctx, listener, "intruder_alive", 3.0f);
			ctx.Check(heard >= 0.0, "control: a shout from a living guard is heard (%.2f s)", heard);

			// The shout is out, collected and waiting for its time; then the shouter is gone.
			Send(doomed, MessageAbout(NpcMessageKind::Shout, "intruder_dead", floor));
			ctx.Step();

			const NpcDirector* director = NpcDirector::Get();
			ctx.Check(director != nullptr && director->PendingMessages() == 1, "the shout waits in the queue for its delay (%d message)", director ? director->PendingMessages() : -1);

			if (destroy)
				doomed.Destroy();
			else
				doomed.Death();

			ctx.Wait(tuning.shout.delay + 1.5f);

			ctx.Check(RecordOf(listener, "intruder_dead") == nullptr, "nobody is told by a shouter that was %s before its shout was due", destroy ? "destroyed" : "killed");
			ctx.Check(listener.GetMind().Concerns().empty() || RecordOf(listener, "intruder_dead") == nullptr, "and it has no concern about it");
			ctx.Check(director != nullptr && director->PendingMessages() == 0, "the message is gone from the queue");

			std::vector<std::pair<SituationKind, std::string>> parts;
			if (director) director->MembershipsOf(doomedId, parts);
			ctx.Check(parts.empty(), "and the one that is %s is part of no situation (%d)", destroy ? "destroyed" : "dead", static_cast<int>(parts.size()));

			if (destroy == false)
			{
				ctx.Check(doomed.CanAct() == false, "the dead guard cannot act");
				ctx.Check(doomed.GetMind().outbox.messages.empty() && doomed.GetMind().outbox.requests.empty(), "and has nothing queued to send");
			}
		}

		void DeadShouter(Ctx& ctx) { ShouterGoneBeforeDelivery(ctx, false); }
		void DestroyedShouter(Ctx& ctx) { ShouterGoneBeforeDelivery(ctx, true); }

		// ── Message kinds ────────────────────────────────────────────────────────────────────────────────────

		// A report is for the ones standing next to the teller; a radio is for the whole squad, through walls, but only a profile
		// with a radio can use it.
		void MessageKinds(Ctx& ctx)
		{
			const NpcMessageTuning& tuning = NpcSituationTuning::Get().messages;

			const glm::vec3 floor = Kit::CleanRoom(ctx);

			glm::vec3 close, apart, hidden;
			if (ctx.world.FindClearSpot(floor, 1.8f, 0.0f, 180.0f, close, 0.4f) == false)
				ctx.Abort("no open ground 1.8 m from the teller");
			if (ctx.world.FindClearSpot(floor, 6.0f, 0.0f, 180.0f, apart, 1.0f) == false)
				ctx.Abort("no open ground 6 m from the teller");

			Npc& teller = Kit::SpawnGuard(ctx, "teller", floor, 0.0f);
			Npc& beside = Kit::SpawnGuard(ctx, "beside", FloorOf(close), 0.0f);
			Npc& distant = Kit::SpawnGuard(ctx, "distant", FloorOf(apart), 0.0f);

			Npc* officer = nullptr;
			if (ctx.world.FindHiddenSpot(teller.observer->position, 18.0f, 45.0f, hidden))
				officer = &Kit::SpawnGuard(ctx, "officer", FloorOf(hidden), 0.0f, "", "officer");
			else
				ctx.Abort("no spot 18 to 45 m from the teller out of its sight for the officer");

			ctx.Wait(2.0f);

			// Report: face to face.
			Send(teller, MessageAbout(NpcMessageKind::Report, "report_1", floor));
			const double reported = SecondsUntilTold(ctx, beside, "report_1", 4.0f);
			ctx.Metric("seconds_to_hear_the_report", reported);
			ctx.Check(reported >= tuning.report.delay - 0.1 && reported <= tuning.report.delay + 0.35, "a report reaches the one beside it (%.1f m) after its delay (%.2f s, the data says %.2f)",
				Kit::Flat(teller.Position, beside.Position), reported, tuning.report.delay);
			ctx.Check(RecordOf(distant, "report_1") == nullptr && RecordOf(*officer, "report_1") == nullptr,
				"and nobody farther than %.1f m (%.1f m) hears it", tuning.report.radius, Kit::Flat(teller.Position, distant.Position));

			// Radio from a guard without one: nothing is sent.
			Send(teller, MessageAbout(NpcMessageKind::Radio, "radio_guard", floor));
			ctx.Wait(tuning.radio.delay + 1.5f);
			ctx.Check(RecordOf(beside, "radio_guard") == nullptr && RecordOf(distant, "radio_guard") == nullptr && RecordOf(*officer, "radio_guard") == nullptr,
				"a guard without a radio cannot call on one: nobody hears it");

			// Radio from the officer: everybody of the faction, wherever they stand.
			const double sentAt = Time::GameTime;
			Send(*officer, MessageAbout(NpcMessageKind::Radio, "radio_officer", floor));
			const double radioed = SecondsUntilTold(ctx, distant, "radio_officer", 4.0f);
			ctx.Metric("seconds_to_hear_the_radio", radioed);
			ctx.Check(radioed >= tuning.radio.delay - 0.1 && radioed <= tuning.radio.delay + 0.35, "a radio call reaches a guard %.0f m away through walls after its delay (%.2f s, the data says %.2f)",
				Kit::Flat(officer->Position, distant.Position), radioed, tuning.radio.delay);

			ctx.Wait(0.5f);
			ctx.Check(RecordOf(teller, "radio_officer") != nullptr && RecordOf(beside, "radio_officer") != nullptr, "and the others of the squad (%.2f s after it was sent)", Time::GameTime - sentAt);
		}

		// ── Roles ────────────────────────────────────────────────────────────────────────────────────────────

		// A noise three guards hear: one of them investigates (the nearest), a second covers because the noise made them wary,
		// the third glances and keeps its routine.
		void InvestigateRoles(Ctx& ctx)
		{
			const NpcInvestigateRolesTuning& tuning = NpcSituationTuning::Get().investigate;

			const glm::vec3 floor = Kit::CleanRoom(ctx);

			// The noise is made at `floor`; the guards stand at 8, 11 and 14 m from it, each with a view of the spot.
			glm::vec3 first, second, third;
			if (ctx.world.FindClearSpot(floor, 8.0f, 0.0f, 180.0f, first, 1.0f) == false
				|| ctx.world.FindClearSpot(floor, 11.0f, 120.0f, 180.0f, second, 1.0f) == false
				|| ctx.world.FindClearSpot(floor, 14.0f, 240.0f, 180.0f, third, 1.5f) == false)
				ctx.Abort("no open ground 8, 11 and 14 m from the noise");

			const glm::vec3 spots[3] = { FloorOf(first), FloorOf(second), FloorOf(third) };
			const char* names[3] = { "nearest", "middle", "farthest" };
			Npc* guards[3];
			for (int i = 0; i < 3; ++i)
				guards[i] = &Kit::SpawnGuard(ctx, names[i], spots[i], 0.0f);

			ctx.Wait(2.5f);

			const glm::vec3 start[3] = { guards[0]->Position, guards[1]->Position, guards[2]->Position };
			ctx.puppet.TeleportTo(floor);
			ctx.puppet.Hold(Posture::Still);
			ctx.Step();
			ctx.puppet.Emit(NpcSoundType::ThrownObject, 30.0f);
			ctx.Step();
			ctx.puppet.Park(floor + Kit::FarAway);

			bool glanced[3] = { false, false, false };
			bool otherSkill[3] = { false, false, false };
			float moved[3] = { 0.0f, 0.0f, 0.0f };
			int investigators = 0, backups = 0, situations = 0, memberCount = 0;
			NpcRole roles[3] = { NpcRole::None, NpcRole::None, NpcRole::None };

			for (int step = 0; step < 150; ++step)
			{
				ctx.Step();

				for (int i = 0; i < 3; ++i)
				{
					glanced[i] |= guards[i]->GetBody().Glancing(Time::GameTime);
					moved[i] = std::max(moved[i], Kit::Flat(guards[i]->Position, start[i]));
				}

				const NpcDirector* director = NpcDirector::Get();
				int investigatorsNow = 0, backupsNow = 0, situationsNow = 0;
				for (const auto& situation : director->Situations())
				{
					if (situation->Kind() != SituationKind::Investigate) continue;
					++situationsNow;
					investigatorsNow += CountRole(*situation, NpcRole::Investigator);
					backupsNow += CountRole(*situation, NpcRole::Backup);
					memberCount = std::max(memberCount, static_cast<int>(situation->Members().size()));
				}

				investigators = std::max(investigators, investigatorsNow);
				backups = std::max(backups, backupsNow);
				situations = std::max(situations, situationsNow);

				for (int i = 0; i < 3; ++i)
				{
					const NpcOrder order = director->FindSituation(SituationKind::Investigate, ctx.puppet.Id()) ? director->FindSituation(SituationKind::Investigate, ctx.puppet.Id())->OrderOf(guards[i]->Id) : NpcOrder();
					if (order.role != NpcRole::None) roles[i] = order.role;
					if (order.role == NpcRole::None && std::string(guards[i]->CurrentSkillId()) != "routine") otherSkill[i] = true;
				}
			}

			ctx.Check(situations == 1, "one investigate situation for one noise (%d)", situations);
			ctx.Check(memberCount == 3, "all three guards that heard it are in it (%d)", memberCount);
			ctx.Check(investigators == tuning.investigators, "with %d investigator at a time (%d)", tuning.investigators, investigators);
			ctx.Check(backups == tuning.backupsWhenWary, "and %d backup, because the noise made them wary (%d)", tuning.backupsWhenWary, backups);
			ctx.Check(roles[0] == NpcRole::Investigator, "the nearest guard is the investigator (%s)", NpcRoleName(roles[0]));
			ctx.Check(roles[1] == NpcRole::Backup, "the next one covers (%s)", NpcRoleName(roles[1]));
			ctx.Check(roles[2] == NpcRole::None, "the farthest has no part (%s)", NpcRoleName(roles[2]));
			ctx.Check(glanced[2] && moved[2] < 0.5f && otherSkill[2] == false, "it glances at the noise and keeps its routine (glanced %d, moved %.2f m, other skill %d)", glanced[2] ? 1 : 0, moved[2], otherSkill[2] ? 1 : 0);
			ctx.Check(moved[0] > 2.0f, "the investigator walked to look (%.1f m)", moved[0]);
		}

		// An alarm four guards hear: one checks the place of the noise, the others search, at most two of them hold a choke point,
		// and no two of the searchers are sent to the same corner.
		void SearchRoles(Ctx& ctx)
		{
			const NpcSearchTuning& search = NpcSituationTuning::Get().search;

			const glm::vec3 floor = Kit::CleanRoom(ctx);

			std::vector<glm::vec3> ring;
			if (Ring(ctx, floor, 3, 5.0f, 30.0f, ring) == false)
				ctx.Abort("no open ground 5 m round the first guard");

			Npc* guards[4];
			guards[0] = &Kit::SpawnGuard(ctx, "g0", floor, 0.0f);
			for (int i = 0; i < 3; ++i)
			{
				char name[8];
				snprintf(name, sizeof(name), "g%d", i + 1);
				guards[i + 1] = &Kit::SpawnGuard(ctx, name, ring[i], 0.0f);
			}

			glm::vec3 shot;
			if (ctx.world.FindHiddenSpot(guards[0]->observer->position, 14.0f, 30.0f, shot) == false)
				ctx.Abort("no spot 14 to 30 m from the guards that they cannot see");

			ctx.Wait(2.5f);

			ctx.puppet.TeleportTo(shot);
			ctx.puppet.Hold(Posture::Still);
			ctx.Step();
			ctx.puppet.Shoot();
			ctx.Step();
			ctx.puppet.Park(shot + Kit::FarAway);

			const std::string subject = ctx.puppet.Id();

			bool opened = ctx.WaitUntil([&] { return FindSearch(subject) != nullptr; }, 3.0f);
			ctx.Check(opened, "a shot nobody saw the shooter of opens a search");
			if (opened == false) return;

			ctx.Wait(1.2f);

			const SearchSituation* situation = FindSearch(subject);
			if (situation == nullptr)
			{
				ctx.Check(false, "the search is still on a second after it opened");
				return;
			}

			const int members = static_cast<int>(situation->Members().size());
			const int checkers = CountRole(*situation, NpcRole::Checker);
			const int watchers = CountRole(*situation, NpcRole::Watcher);
			const int searchers = CountRole(*situation, NpcRole::Searcher);

			ctx.Check(members == 4, "all four guards are in the search (%d)", members);
			ctx.Check(situation->GetPhase() == SearchSituation::Phase::Search, "and, with nobody to chase, it is a search");
			ctx.Check(checkers == 1, "one of them checks the place of the shot (%d)", checkers);
			ctx.Check(watchers <= search.maxWatchers && searchers >= 1 && checkers + watchers + searchers == 4,
				"the rest search or hold a choke point, at most %d watching (%d watchers, %d searchers)", search.maxWatchers, watchers, searchers);

			// Claims: where the searchers were sent, kept apart.
			std::vector<glm::vec3> claims;
			for (const SituationMember& member : situation->Members())
			{
				if (member.order.role == NpcRole::Searcher && member.claim >= 0 && member.claim < situation->GetMap().Count())
					claims.push_back(situation->GetMap().Floor(member.claim));
			}

			float closest = 1.0e9f;
			for (size_t i = 0; i < claims.size(); ++i)
			{
				for (size_t j = i + 1; j < claims.size(); ++j)
					closest = std::min(closest, Kit::Flat(claims[i], claims[j]));
			}

			ctx.Check(claims.empty() == false, "the searchers have somewhere to go (%d claims)", static_cast<int>(claims.size()));
			ctx.Check(closest >= search.claimSpacing - 0.5f, "and not the same place: at least %.1f m between claims (%.1f m)", search.claimSpacing, claims.size() > 1 ? closest : 0.0f);
		}

		// ── Lost target ──────────────────────────────────────────────────────────────────────────────────────

		// What the director's search looked like from outside, step by step.
		struct SearchWatch
		{
			bool opened = false;
			double chaseAt = -1.0;
			double searchAt = -1.0;
			double endedAt = -1.0;
			bool chaseSkill = false;
			bool searchSkill = false;
			NpcAlert highest = NpcAlert::Calm;
			int gazeFrames = 0;
			int gazeOnTarget = 0;
			float gazeWorst = 0.0f;
			float floodError = -1.0f;   // metres between where the chase flood started and what the guards last knew
		};

		void Watch(Ctx& ctx, Npc& guard, const std::string& subject, SearchWatch& watch)
		{
			const SearchSituation* situation = FindSearch(subject);
			const double now = Time::GameTime;

			if (situation != nullptr)
			{
				watch.opened = true;

				const NpcRecord* known = NpcDirector::Get()->Freshest(*situation);
				if (watch.floodError < 0.0f && known != nullptr && situation->GetPhase() == SearchSituation::Phase::Chase && situation->GetFlood().WarmCells() > 0)
					watch.floodError = Kit::Flat(situation->GetFlood().Origin(), known->position);
				if (situation->GetPhase() == SearchSituation::Phase::Chase && watch.chaseAt < 0.0) watch.chaseAt = now;
				if (situation->GetPhase() == SearchSituation::Phase::Search && watch.searchAt < 0.0) watch.searchAt = now;
			}
			else if (watch.opened && watch.endedAt < 0.0)
			{
				watch.endedAt = now;
			}

			const std::string skill = guard.CurrentSkillId();
			watch.chaseSkill |= skill == "chase";
			watch.searchSkill |= skill == "search";
			watch.highest = std::max(watch.highest, guard.GetMind().alert);

			// The gaze of a searcher is on the edge of what is not yet known.
			if (skill == "search")
			{
				NpcOrder order;
				std::string orderSubject;
				if (NpcDirector::FindOrder(guard.Id, SituationKind::Search, order, orderSubject) && order.hasLook)
				{
					const float error = Kit::GazeError(guard, order.look);
					++watch.gazeFrames;
					watch.gazeOnTarget += error <= 25.0f ? 1 : 0;
					watch.gazeWorst = std::max(watch.gazeWorst, error);
				}
			}
		}

		// The guard sees a hostile, then it is gone: it runs to where the chase says, the map is searched from
		// there, it gives up and goes back to its post.
		void LostTargetChaseSearch(Ctx& ctx)
		{
			const NpcSearchTuning& search = NpcSituationTuning::Get().search;

			const glm::vec3 floor = Kit::CleanRoom(ctx);

			// The map decides where the guard stands: try the start and take what has a spot in the open a step from cover.
			glm::vec3 seen, hidden;
			Npc& probe = Kit::SpawnGuard(ctx, "probe", floor, 0.0f);
			ctx.Wait(1.0f);
			if (ctx.world.FindEdgeSpot(probe.observer->position, 8.0f, 12.0f, 2.0f, seen, hidden) == false)
				ctx.Abort("no spot 8 to 12 m from the guard with cover a step away");
			ctx.world.RemoveNpcs();

			Kit::SpawnTaskPoint(ctx, "post", floor);
			Npc& guard = Kit::SpawnGuard(ctx, "guard", floor, World::YawDegrees(seen - floor), "post");
			ctx.Wait(3.0f);

			const std::string subject = ctx.puppet.Id();

			// In the open in front of it, armed: it identifies a hostile.
			ctx.puppet.TeleportTo(seen);
			ctx.puppet.Hold(Posture::Still);

			const bool identified = ctx.WaitUntil([&] { return guard.GetMind().SeesHostile(); }, 15.0f);
			ctx.Check(identified, "the guard identifies the armed player in the open");
			if (identified == false) return;

			ctx.Check(FindSearch(subject) == nullptr, "while it sees the hostile nothing is searched for");

			// Out of sight in one step, and gone.
			ctx.puppet.MoveTo(hidden, Posture::Run);
			ctx.WaitUntil([&] { return ctx.puppet.Moving() == false; }, 5.0f);
			ctx.puppet.TeleportTo(hidden);
			ctx.puppet.Hold(Posture::Still);
			ctx.Wait(0.4f);
			ctx.puppet.Park(hidden + Kit::FarAway);

			const double lostAt = Time::GameTime;
			SearchWatch watch;

			const double limit = ctx.Now() + search.timeout + 60.0;
			while (ctx.Now() < limit && (watch.endedAt < 0.0))
			{
				ctx.Step();
				Watch(ctx, guard, subject, watch);
			}

			ctx.Check(watch.opened, "losing a hostile opens a search");
			ctx.Check(watch.chaseAt >= 0.0 && watch.chaseSkill, "it starts with a chase: the flood says where it went and the guard runs there (phase %s, skill %s)",
				watch.chaseAt >= 0.0 ? "yes" : "no", watch.chaseSkill ? "yes" : "no");
			ctx.Check(watch.floodError >= 0.0 && watch.floodError <= 2.0, "the chase floods from what the guard knew (%.1f m from the last record)", watch.floodError);
			ctx.Check(watch.searchAt >= 0.0, "then the search of the map (the chasers found nothing)");
			ctx.Check(watch.chaseAt >= 0.0 && watch.searchAt > watch.chaseAt, "in that order (chase at %.1f s, search at %.1f s)", watch.chaseAt - lostAt, watch.searchAt - lostAt);
			ctx.Check(watch.highest >= NpcAlert::Alarmed, "alarmed all the way (%s)", NpcAlertName(watch.highest));

			ctx.Metric("seconds_lost_to_give_up", watch.endedAt >= 0.0 ? watch.endedAt - lostAt : -1.0);
			ctx.Check(watch.endedAt >= 0.0 && watch.endedAt - lostAt <= search.timeout + 15.0, "it gives up within the timeout (%.1f s, the data says %.0f)", watch.endedAt - lostAt, search.timeout);

			ctx.Wait(0.3f);
			ctx.Check(guard.GetMind().TopConcern() == nullptr, "no concern is left");

			const bool home = ctx.WaitUntil([&] { return Kit::Flat(guard.Position, floor) < 1.2f && std::string(guard.CurrentSkillId()) == "routine"; }, 60.0f);
			ctx.Check(home, "it goes back to its post and its routine (%.1f m off, %s)", Kit::Flat(guard.Position, floor), guard.CurrentSkillId());
			ctx.Check(NpcDirector::Get()->Situations().empty(), "and no situation is left open");
		}

		// An alarm out of sight: one guard walks to look, then searches the map; wherever it looks, it looks at the edge of what
		// is not yet known.
		void SearchGaze(Ctx& ctx)
		{
			const NpcSearchTuning& search = NpcSituationTuning::Get().search;

			const glm::vec3 floor = Kit::CleanRoom(ctx);
			Kit::SpawnTaskPoint(ctx, "post", floor);
			Npc& guard = Kit::SpawnGuard(ctx, "guard", floor, 0.0f, "post");
			ctx.Wait(3.0f);

			glm::vec3 shot;
			if (ctx.world.FindHiddenSpot(guard.observer->position, 18.0f, 35.0f, shot) == false)
				ctx.Abort("no spot 18 to 35 m from the guard that it cannot see");

			ctx.puppet.TeleportTo(shot);
			ctx.puppet.Hold(Posture::Still);
			ctx.Step();
			ctx.puppet.Shoot();
			ctx.Step();
			ctx.puppet.Park(shot + Kit::FarAway);

			const std::string subject = ctx.puppet.Id();
			SearchWatch watch;

			const double limit = ctx.Now() + search.timeout + 30.0;
			while (ctx.Now() < limit && (watch.opened == false || watch.endedAt < 0.0))
			{
				ctx.Step();
				Watch(ctx, guard, subject, watch);
			}

			ctx.Check(watch.opened && watch.endedAt >= 0.0, "the search opens and ends (%s)", watch.endedAt >= 0.0 ? "ended" : "still on");
			ctx.Check(watch.gazeFrames >= 20, "the searcher had a point to look at (%d frames)", watch.gazeFrames);

			const float onTarget = watch.gazeFrames > 0 ? static_cast<float>(watch.gazeOnTarget) / static_cast<float>(watch.gazeFrames) : 0.0f;
			ctx.Metric("gaze_on_frontier", onTarget);
			ctx.Check(onTarget >= 0.8f, "its gaze is on the frontier at least 80%% of the time (%.0f%% of %d frames, worst %.0f degrees)", onTarget * 100.0f, watch.gazeFrames, watch.gazeWorst);
		}

		// While the puppet is out of sight, wherever it really is, the search is built from what the guard knew: its record of the
		// puppet does not move, and the flood starts from it. The runner's invariant (no knowledge without perception) watches the
		// rest.
		void SearchDoesNotLeak(Ctx& ctx)
		{
			const NpcSearchTuning& search = NpcSituationTuning::Get().search;

			const glm::vec3 floor = Kit::CleanRoom(ctx);
			Kit::SpawnTaskPoint(ctx, "post", floor);
			Npc& guard = Kit::SpawnGuard(ctx, "guard", floor, 0.0f, "post");
			ctx.Wait(3.0f);

			// The places the puppet will be at in turn: all out of the guard's sight, near and far.
			std::vector<glm::vec3> places;
			for (const std::pair<float, float> range : { std::pair<float, float>{ 10.0f, 16.0f }, { 16.0f, 24.0f }, { 24.0f, 40.0f } })
			{
				glm::vec3 place;
				if (ctx.world.FindHiddenSpot(guard.observer->position, range.first, range.second, place))
					places.push_back(place);
			}
			if (places.size() < 2)
				ctx.Abort("fewer than two places out of the guard's sight to hide in (%d)", static_cast<int>(places.size()));

			// A shot from the first one: the only thing the guard learns.
			ctx.puppet.TeleportTo(places[0]);
			ctx.puppet.Hold(Posture::Still);
			ctx.Step();
			ctx.puppet.Shoot();
			ctx.Step();

			const std::string subject = ctx.puppet.Id();
			ctx.Check(ctx.WaitUntil([&] { return FindSearch(subject) != nullptr && RecordOf(guard, subject) != nullptr; }, 3.0f), "the shot opens a search and the guard has a record of it");

			const NpcRecord* known = RecordOf(guard, subject);
			if (known == nullptr) return;

			glm::vec3 knownPosition = known->position;
			double knownTime = known->time;

			// The puppet moves between the other places, quietly, one every 4 s, and never makes a sound.
			size_t at = 0;
			double nextMove = ctx.Now() + 1.0;
			int changes = 0, steps = 0;
			float flood = -1.0f;

			const double limit = ctx.Now() + search.timeout + 30.0;
			while (ctx.Now() < limit && FindSearch(subject) != nullptr)
			{
				if (ctx.Now() >= nextMove)
				{
					at = (at + 1) % places.size();
					ctx.puppet.TeleportTo(places[at]);
					ctx.puppet.Hold(Posture::CrouchStill);
					nextMove = ctx.Now() + 4.0;
				}

				ctx.Step();
				++steps;

				const NpcRecord* now = RecordOf(guard, subject);
				if (now != nullptr && (glm::distance(now->position, knownPosition) > 0.01f || std::abs(now->time - knownTime) > 0.001))
				{
					// News is only legitimate when somebody perceived the puppet a moment ago.
					if (ctx.LastPerceived() < ctx.Now() - 0.5)
						++changes;
					knownPosition = now->position;
					knownTime = now->time;
				}

				if (const SearchSituation* situation = FindSearch(subject))
				{
					if (situation->GetPhase() == SearchSituation::Phase::Chase && situation->GetFlood().Done() == false)
						flood = Kit::Flat(situation->GetFlood().Origin(), knownPosition);
				}
			}

			ctx.Check(steps > 60, "the search went on while the puppet moved about (%d steps)", steps);
			ctx.Check(changes == 0, "the guard's record of the puppet changed %d times without a sight or a sound", changes);
			ctx.Check(flood < 0.0f || flood <= 2.0f, "a chase floods from what the guard knew (%.1f m from the record)", flood);
		}
	}

	void RegisterSocialScenarios()
	{
		Scenario scenario;
		scenario.tier = "social";
		scenario.timeLimit = 400.0f;
		scenario.watchKnowledge = true;

		scenario.name = "social_shout_reaches_range";
		scenario.run = ShoutReachesRange;
		Register(scenario);

		scenario.name = "social_dead_shouter";
		scenario.run = DeadShouter;
		Register(scenario);

		scenario.name = "social_destroyed_shouter";
		scenario.run = DestroyedShouter;
		Register(scenario);

		scenario.name = "social_message_kinds";
		scenario.run = MessageKinds;
		Register(scenario);

		scenario.name = "social_investigate_roles";
		scenario.run = InvestigateRoles;
		Register(scenario);

		scenario.name = "social_search_roles";
		scenario.run = SearchRoles;
		Register(scenario);

		scenario.name = "social_lost_target_chase_search";
		scenario.run = LostTargetChaseSearch;
		Register(scenario);

		scenario.name = "social_search_gaze";
		scenario.run = SearchGaze;
		Register(scenario);

		scenario.name = "social_search_no_leak";
		scenario.run = SearchDoesNotLeak;
		Register(scenario);
	}
}

#endif

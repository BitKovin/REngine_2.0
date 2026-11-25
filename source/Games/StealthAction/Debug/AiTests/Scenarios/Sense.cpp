// Tier "sense": what a single NPC sees and hears (NPC_AI_DESIGN.md §5, §6.2), and what the HUD reads. The checks are
// derived from GameData/ai/perception.json and the formulas of the Mind, with tolerances for the update granularity:
//
//   strength = zone factor x distance factor x visibility x light            (Observer, the vision profile)
//   fill/s   = fillRate x strength x motion x state x acuity x suspicion     (NpcMind::SenseSight)
//
// So the meter's rise has to match the sum of what the formula gives for what the guard's eyes report each step, and
// the eyes' strength has to match the formula for the reported zone and distance. Distance, crouching and motion
// matter because the numbers in the file say so.

#include "ScenarioKit.h"

#ifndef DISTRIBUTION

#include "../../../Entities/Npc/Npc.h"

#include <AiPerception/Observer.h>
#include <AiPerception/ObservationTarget.h>
#include <Time.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>

namespace AiTest
{
	namespace
	{
		constexpr const char* NpcClass = "npc";
		constexpr const char* LegacyClass = "npc_guard_hybrid";
		constexpr const char* GuardName = "test_guard";

		constexpr float FillTolerance = 0.15f;     // meter rise against the formula
		constexpr float StrengthTolerance = 0.08f; // eyes against the formula
		constexpr float TimeTolerance = 0.20f;     // seconds of attention against the formula

		struct Placement
		{
			glm::vec3 position{};
			float yaw = 0.0f;
		};

		// A fresh guard of `className` on the floor where the puppet starts, after the map's own NPCs are gone. The guard
		// settles and turns to its post; the placement says where it ended up looking. The puppet is parked far away.
		Placement PlaceGuard(Ctx& ctx, const char* className, const glm::vec3& start)
		{
			ctx.world.RemoveNpcs();

			glm::vec3 floor;
			if (ctx.world.SnapToFloor(start, floor) == false)
				ctx.Abort("the start position has no navigation grid cell");

			const glm::vec3 spawn = floor + glm::vec3(0.0f, 1.4f, 0.0f);
			if (ctx.world.SpawnNpc(className, GuardName, spawn, 0.0f) == nullptr)
				ctx.Abort("could not spawn %s", className);

			ctx.puppet.Park(spawn + glm::vec3(300.0f, 100.0f, 300.0f));
			ctx.Wait(2.0f);

			const Snapshot* snapshot = ctx.probe.Find(GuardName);
			if (snapshot == nullptr)
				ctx.Abort("the spawned guard is not in the level");

			return { snapshot->position, snapshot->yaw };
		}

		Npc& GuardNpc(Ctx& ctx)
		{
			Npc* npc = dynamic_cast<Npc*>(ctx.world.FindNpc(GuardName));
			if (npc == nullptr)
				ctx.Abort("the guard is not an npc");
			return *npc;
		}

		float MeterOf(Ctx& ctx)
		{
			return GuardNpc(ctx).GetMind().MeterValue(ctx.puppet.Id());
		}

		// What the guard's eyes report about the puppet this step (null when it is not seen).
		const SightResult* SightOf(Ctx& ctx)
		{
			const Npc& guard = GuardNpc(ctx);
			if (guard.observer == nullptr) return nullptr;

			for (const SightResult& result : guard.observer->sightResults)
			{
				if (result.sense == SenseKind::Sight && result.target && result.target->ownerId == ctx.puppet.Id())
					return &result;
			}
			return nullptr;
		}

		float Saturate(float value) { return std::clamp(value, 0.0f, 1.0f); }

		// The vision profile's strength of a sighting (Observer::UpdateVisibilityProfiled) for the zone and distance it
		// reports, as it is once the target has been in view for a while (the zone's range grows by the hysteresis).
		float ExpectedStrength(const SightResult& result, float rangeScale)
		{
			const NpcVisionTuning& vision = NpcPerceptionTuning::Get().vision;

			float zoneFactor = vision.nearFactor;
			float zoneRange = vision.nearRange;
			if (result.zone == VisionZone::Focus)
			{
				zoneFactor = 1.0f;
				zoneRange = vision.focusRange * rangeScale;
			}
			else if (result.zone == VisionZone::Peripheral)
			{
				zoneFactor = vision.peripheralFactor;
				zoneRange = vision.peripheralRange * rangeScale;
			}
			zoneRange *= 1.0f + vision.hysteresis;

			const float distance = result.distance / result.target->noticeMaxDistanceMultiplier;
			float distanceFactor = 1.0f;
			if (distance > vision.closeDistance && zoneRange > vision.closeDistance)
				distanceFactor = glm::mix(1.0f, vision.farFactor, Saturate((distance - vision.closeDistance) / (zoneRange - vision.closeDistance)));

			const float light = glm::mix(vision.darkFactor, 1.0f, Saturate(result.target->lightLevel / std::max(vision.litLevel, 0.001f)));
			return zoneFactor * distanceFactor * Saturate(result.target->visibility) * light;
		}

		float MotionFactor(TargetMotion motion)
		{
			const NpcMeterTuning& meter = NpcPerceptionTuning::Get().meter;
			switch (motion)
			{
			case TargetMotion::Still: return meter.motionStill;
			case TargetMotion::Run: return meter.motionRun;
			case TargetMotion::CrouchWalk: return meter.motionCrouchWalk;
			case TargetMotion::CrouchStill: return meter.motionCrouchStill;
			default: return meter.motionWalk;
			}
		}

		TargetMotion MotionOf(Posture posture)
		{
			switch (posture)
			{
			case Posture::Run: return TargetMotion::Run;
			case Posture::CrouchWalk: return TargetMotion::CrouchWalk;
			case Posture::CrouchStill: return TargetMotion::CrouchStill;
			case Posture::Walk: return TargetMotion::Walk;
			default: return TargetMotion::Still;
			}
		}

		// Watches the meter fill. Every step it adds what the Mind's formula gives for what the eyes reported, so the sum
		// can be held against the real rise of the meter.
		struct Gauge
		{
			double expectedRise = 0.0;     // sum of fill x dt
			double strengthSeconds = 0.0;  // integral of the sighting strength
			double seconds = 0.0;          // time the puppet was seen
			double motionMatches = 0.0;    // seconds the puppet's motion was the one asked for
			double steps = 0.0;
			float start = 0.0f;
			float lastStrength = 0.0f;

			void Begin(Ctx& ctx) { *this = Gauge(); start = MeterOf(ctx); }

			// One step of the scenario with the bookkeeping.
			void Step(Ctx& ctx, TargetMotion wanted)
			{
				const Npc& guard = GuardNpc(ctx);
				const NpcMeterTuning& meter = NpcPerceptionTuning::Get().meter;
				const NpcAlert alert = guard.GetMind().alert; // what the Mind multiplied by during this step

				ctx.Step();

				const SightResult* sight = SightOf(ctx);
				steps += StepSeconds;
				if (sight == nullptr) return;

				const TargetMotion motion = sight->target->motion;
				const float fill = meter.fillRate * sight->strength * MotionFactor(motion) * meter.stateMul[static_cast<int>(alert)]
					* guard.GetProfile().acuity * guard.GetProfile().defaultSuspicion;

				expectedRise += fill * StepSeconds;
				strengthSeconds += sight->strength * StepSeconds;
				seconds += StepSeconds;
				lastStrength = sight->strength;
				if (motion == wanted) motionMatches += StepSeconds;
			}

			float Rise(Ctx& ctx) const { return MeterOf(ctx) - start; }
		};

		// Steps until the meter reaches `level` or `timeout` runs out; returns the seconds it took (-1: never).
		double FillTo(Ctx& ctx, Gauge& gauge, TargetMotion wanted, float level, float timeout, const std::function<void()>& upkeep = nullptr)
		{
			const double started = ctx.Now();
			while (MeterOf(ctx) < level)
			{
				if (ctx.Now() - started > timeout) return -1.0;
				if (upkeep) upkeep();
				gauge.Step(ctx, wanted);
			}
			return ctx.Now() - started;
		}

		void CheckGauge(Ctx& ctx, const Gauge& gauge, const char* what)
		{
			const float rise = gauge.Rise(ctx);
			ctx.Check(std::abs(rise - gauge.expectedRise) <= FillTolerance * std::max(rise, 0.1f),
				"%s: the meter rose %.2f, the formula (fillRate x strength x motion x state x acuity x suspicion) gives %.2f", what, rise, gauge.expectedRise);
		}

		// ── Sight ────────────────────────────────────────────────────────────────────────────────────────────

		// A still player in front of the guard, 5, 12 and 20 m away: the nearer, the faster, as the distance factor says.
		void SightDistances(Ctx& ctx)
		{
			const glm::vec3 start = ctx.puppet.Position();
			const float distances[3] = { 5.0f, 12.0f, 20.0f };
			double times[3] = { -1.0, -1.0, -1.0 };
			float strengths[3] = { 0.0f, 0.0f, 0.0f };

			for (int i = 0; i < 3; ++i)
			{
				// A fresh guard for every trial: one that has already noticed keeps its meter up for a long time.
				const Placement placed = PlaceGuard(ctx, NpcClass, start);

				glm::vec3 spot;
				if (ctx.world.FindClearSpot(placed.position, distances[i], placed.yaw, 12.0f, spot) == false)
				{
					ctx.Info("no open ground %.0f m in front of the guard", distances[i]);
					continue;
				}

				ctx.puppet.TeleportTo(spot);
				ctx.puppet.Hold(Posture::Still);

				Gauge gauge;
				gauge.Begin(ctx);
				times[i] = FillTo(ctx, gauge, TargetMotion::Still, 0.6f, 40.0f);

				char name[64];
				snprintf(name, sizeof(name), "seconds_to_reach_0.6_standing_%.0fm", distances[i]);
				ctx.Metric(name, times[i]);

				const SightResult* sight = SightOf(ctx);
				ctx.Check(times[i] > 0.0 && sight != nullptr, "a standing player %.0f m in front of the guard fills its meter to 0.6 (%.1f s)", distances[i], times[i]);
				if (sight == nullptr) continue;

				strengths[i] = sight->strength;
				ctx.Check(sight->zone == VisionZone::Focus, "%.0f m in front is seen by the focus zone", distances[i]);

				const float expected = ExpectedStrength(*sight, 1.0f);
				ctx.Check(std::abs(sight->strength - expected) <= StrengthTolerance * expected,
					"%.0f m: the eyes report strength %.2f, the vision profile gives %.2f at %.1f m", distances[i], sight->strength, expected, sight->distance);

				CheckGauge(ctx, gauge, name);
			}

			if (times[0] > 0.0 && times[1] > 0.0)
				ctx.Check(times[0] < times[1], "closer is faster: 5 m %.1f s, 12 m %.1f s", times[0], times[1]);
			if (times[1] > 0.0 && times[2] > 0.0)
				ctx.Check(times[1] < times[2], "closer is faster: 12 m %.1f s, 20 m %.1f s", times[1], times[2]);
			if (times[0] > 0.0 && times[2] > 0.0 && strengths[0] > 0.0f && strengths[2] > 0.0f)
			{
				const double ratio = times[2] / times[0];
				const double expected = strengths[0] / strengths[2];
				ctx.Check(std::abs(ratio - expected) <= TimeTolerance * expected, "20 m takes %.2f times as long as 5 m, the strengths give %.2f", ratio, expected);
			}
		}

		// The zones of the vision profile: what each reports at a place around the guard.
		void SightZones(Ctx& ctx)
		{
			const NpcVisionTuning& vision = NpcPerceptionTuning::Get().vision;
			const glm::vec3 start = ctx.puppet.Position();

			struct Place
			{
				const char* what;
				float distance;
				float yawOffset;
				VisionZone expected;
			};

			const Place places[] = {
				{ "in front", 6.0f, 0.0f, VisionZone::Focus },
				{ "to the side", 6.0f, 85.0f, VisionZone::Peripheral },
				{ "behind", 6.0f, 180.0f, VisionZone::None },
				{ "in front beyond the focus range", vision.focusRange + 6.0f, 0.0f, VisionZone::None }
			};

			Placement placed;

			for (const Place& place : places)
			{
				// A fresh guard each time: one that has noticed the puppet turns to it.
				placed = PlaceGuard(ctx, NpcClass, start);

				glm::vec3 spot;
				if (ctx.world.FindClearSpot(placed.position, place.distance, placed.yaw + place.yawOffset, 15.0f, spot, 2.5f) == false)
				{
					ctx.Info("no open ground %s (%.0f m)", place.what, place.distance);
					continue;
				}

				ctx.puppet.TeleportTo(spot);
				ctx.puppet.Hold(Posture::Still);
				ctx.Wait(0.8f);

				const SightResult* sight = SightOf(ctx);
				const VisionZone zone = sight ? sight->zone : VisionZone::None;
				ctx.Check(zone == place.expected, "%s (%.0f m): zone %d, expected %d (0 none, 1 focus, 2 peripheral)", place.what, place.distance,
					static_cast<int>(zone), static_cast<int>(place.expected));

				if (sight != nullptr && zone == place.expected)
				{
					const float expected = ExpectedStrength(*sight, 1.0f);
					ctx.Check(std::abs(sight->strength - expected) <= StrengthTolerance * expected, "%s: strength %.2f, the profile gives %.2f", place.what, sight->strength, expected);
				}
				if (place.expected == VisionZone::None)
					ctx.Check(MeterOf(ctx) <= 0.01f, "%s: nothing fills the meter (%.2f)", place.what, MeterOf(ctx));
			}

			// Behind a wall.
			placed = PlaceGuard(ctx, NpcClass, start);

			glm::vec3 hidden;
			if (ctx.world.FindHiddenSpot(placed.position, 4.0f, 14.0f, hidden))
			{
				ctx.puppet.TeleportTo(hidden);
				ctx.puppet.Hold(Posture::Still);
				ctx.Wait(8.0f);
				ctx.Check(SightOf(ctx) == nullptr && MeterOf(ctx) <= 0.01f, "a player out of line of sight fills nothing (%.2f)", MeterOf(ctx));
			}
			else
			{
				ctx.Info("no spot out of the guard's sight 4 to 14 m away");
			}
		}

		// Posture and motion, 14 to 20 m in front (outside the 8 m a footstep is heard at): the strength-seconds the meter needs
		// per unit are 1 / (fillRate x motion factor), so crouching is slower than standing, walking faster, running fastest.
		void SightMotion(Ctx& ctx)
		{
			const glm::vec3 start = ctx.puppet.Position();
			const NpcMeterTuning& meter = NpcPerceptionTuning::Get().meter;
			Placement placed = PlaceGuard(ctx, NpcClass, start);

			glm::vec3 nearSpot, farSpot;
			if (ctx.world.FindClearSpot(placed.position, 14.0f, placed.yaw, 10.0f, nearSpot) == false
				|| ctx.world.FindClearSpot(placed.position, 20.0f, placed.yaw, 10.0f, farSpot) == false)
			{
				ctx.Info("no open ground 14 and 20 m in front of the guard");
				return;
			}

			const Posture postures[] = { Posture::CrouchStill, Posture::CrouchWalk, Posture::Still, Posture::Walk, Posture::Run };
			const float level = 0.6f;
			double needed[5] = {};

			for (int i = 0; i < 5; ++i)
			{
				const Posture posture = postures[i];
				const bool moving = posture == Posture::Walk || posture == Posture::Run || posture == Posture::CrouchWalk;

				placed = PlaceGuard(ctx, NpcClass, start);

				ctx.puppet.TeleportTo(nearSpot);
				ctx.puppet.Hold(posture);

				bool toFar = true;
				auto pace = [&]
				{
					if (moving && ctx.puppet.Moving() == false)
					{
						ctx.puppet.MoveTo(toFar ? farSpot : nearSpot, posture);
						toFar = !toFar;
					}
				};

				Gauge gauge;
				gauge.Begin(ctx);
				const double seconds = FillTo(ctx, gauge, MotionOf(posture), level, 60.0f, pace);

				char name[64];
				snprintf(name, sizeof(name), "seconds_to_reach_0.6_%s", PostureName(posture));
				ctx.Metric(name, seconds);

				ctx.Check(seconds > 0.0, "%s: the meter reaches %.1f (%.1f s)", PostureName(posture), level, seconds);
				if (seconds <= 0.0) continue;

				CheckGauge(ctx, gauge, PostureName(posture));

				ctx.Check(gauge.motionMatches >= 0.7 * gauge.seconds, "%s: the puppet looked like it (%.0f%% of the time)", PostureName(posture),
					100.0 * gauge.motionMatches / std::max(gauge.seconds, 1.0e-3));

				// Strength-seconds needed for the level: level / (fillRate x motion) when it stays Calm.
				needed[i] = gauge.strengthSeconds;
				const double expected = level / (meter.fillRate * MotionFactor(MotionOf(posture)));
				const double tolerance = moving ? 0.35 : TimeTolerance; // a moving puppet spends its first seconds getting to pace
				snprintf(name, sizeof(name), "strength_seconds_to_reach_0.6_%s", PostureName(posture));
				ctx.Metric(name, needed[i]);
				ctx.Check(std::abs(needed[i] - expected) <= tolerance * expected, "%s: %.2f strength-seconds to reach %.1f, the motion factor gives %.2f",
					PostureName(posture), needed[i], level, expected);
			}

			// Crouching still < crouch-walking < standing < walking < running, by the motion factors.
			for (int i = 0; i + 1 < 5; ++i)
			{
				if (needed[i] > 0.0 && needed[i + 1] > 0.0)
					ctx.Check(needed[i] > needed[i + 1], "%s needs more strength-seconds (%.2f) than %s (%.2f)", PostureName(postures[i]), needed[i],
						PostureName(postures[i + 1]), needed[i + 1]);
			}
		}

		// From a first glimpse to being identified at 8 m: noticed, suspected (the alert goes up and the fill with it),
		// identified. The strength-seconds follow the staged formula.
		void SightIdentify(Ctx& ctx)
		{
			const NpcMeterTuning& meter = NpcPerceptionTuning::Get().meter;
			const Placement placed = PlaceGuard(ctx, NpcClass, ctx.puppet.Position());

			glm::vec3 spot;
			if (ctx.world.FindClearSpot(placed.position, 8.0f, placed.yaw, 10.0f, spot) == false)
			{
				ctx.Info("no open ground 8 m in front of the guard");
				return;
			}

			ctx.puppet.TeleportTo(spot);
			ctx.puppet.Hold(Posture::Still);

			Gauge gauge;
			gauge.Begin(ctx);

			const double noticed = FillTo(ctx, gauge, TargetMotion::Still, meter.notice, 30.0f);
			const double suspected = FillTo(ctx, gauge, TargetMotion::Still, meter.suspect, 30.0f);
			const double identified = FillTo(ctx, gauge, TargetMotion::Still, meter.identify - 0.05f, 30.0f);
			ctx.Metric("seconds_to_notice", noticed);
			ctx.Metric("seconds_to_suspect_total", suspected);
			ctx.Metric("seconds_to_identify_total", identified);

			ctx.Check(noticed > 0.0 && suspected > 0.0 && identified > 0.0, "noticed %.1f s, suspected %.1f s, identified %.1f s after the first glimpse", noticed, suspected, identified);
			if (identified <= 0.0) return;

			CheckGauge(ctx, gauge, "identify");

			// Strength-seconds: up to Suspect in Calm, the rest at the Wary multiplier (the Suspect concern raises the alert).
			const double base = meter.fillRate * MotionFactor(TargetMotion::Still) * GuardNpc(ctx).GetProfile().acuity * GuardNpc(ctx).GetProfile().defaultSuspicion;
			const double expected = meter.suspect / (base * meter.stateMul[0]) + (meter.identify - 0.05 - meter.suspect) / (base * meter.stateMul[1]);
			ctx.Check(std::abs(gauge.strengthSeconds - expected) <= TimeTolerance * expected, "%.2f strength-seconds to be identified, the staged formula gives %.2f",
				gauge.strengthSeconds, expected);

			ctx.Check(ctx.WaitUntil([&] { return GuardNpc(ctx).GetMind().TopConcern() != nullptr
				&& GuardNpc(ctx).GetMind().TopConcern()->type == NpcConcernType::Threat; }, 3.0f), "a seen, identified armed player is a threat to the guard");

			// Save and load: the concern comes back, still pointing at the place the Person record holds.
			const NpcMind& mind = GuardNpc(ctx).GetMind();
			const NpcConcern* top = mind.TopConcern();
			if (top == nullptr) return;

			NpcMind loaded;
			loaded.Deserialize(mind.Serialize(Time::GameTime), Time::GameTime);

			const NpcConcern* back = loaded.TopConcern();
			ctx.Check(back != nullptr && back->subject == top->subject && back->type == top->type, "the saved concern is loaded back");
			ctx.Check(back != nullptr && glm::distance(loaded.ConcernPosition(*back), mind.ConcernPosition(*top)) < 0.01f,
				"and it points at the same place (%.1f m apart)", back ? glm::distance(loaded.ConcernPosition(*back), mind.ConcernPosition(*top)) : -1.0f);
			ctx.Check(std::abs(loaded.MeterValue(top->subject) - mind.MeterValue(top->subject)) < 0.001f, "and so is the meter (%.2f vs %.2f)",
				loaded.MeterValue(top->subject), mind.MeterValue(top->subject));
		}

		// ── HUD ──────────────────────────────────────────────────────────────────────────────────────────────

		// The HUD reads the same value the guard holds (both NPC classes feed UiNpcAwareness through GetAwareness).
		void HudMeterFollowsDetection(Ctx& ctx, const char* className)
		{
			const Placement placed = PlaceGuard(ctx, className, ctx.puppet.Position());

			glm::vec3 spot;
			if (ctx.world.FindClearSpot(placed.position, 8.0f, placed.yaw, 10.0f, spot) == false)
			{
				ctx.Info("no open ground 8 m in front of the guard");
				return;
			}

			NpcBase* guard = ctx.world.FindNpc(GuardName);
			ctx.puppet.TeleportTo(spot);
			ctx.puppet.Hold(Posture::Still);
			ctx.Wait(3.0f);

			std::vector<NpcBase::AwarenessReading> readings;
			guard->GetAwareness(readings);

			float hudValue = 0.0f;
			for (const auto& reading : readings)
			{
				if (reading.subject == ctx.puppet.Id()) hudValue = reading.value;
			}

			const Snapshot* snapshot = ctx.probe.Find(GuardName);
			ctx.Check(guard->SeesTarget() || (snapshot && snapshot->perceives), "the guard sees the player in the open");
			ctx.Check(hudValue > 0.05f, "the HUD has a meter value for a guard that is watching the player (%.2f)", hudValue);
			ctx.Check(snapshot != nullptr && std::abs(hudValue - snapshot->detection) < 0.15f, "the HUD value matches the guard's detection (%.2f vs %.2f)", hudValue,
				snapshot ? snapshot->detection : -1.0f);
		}

		void HudMeterNpc(Ctx& ctx) { HudMeterFollowsDetection(ctx, NpcClass); }
		void HudMeterLegacy(Ctx& ctx) { HudMeterFollowsDetection(ctx, LegacyClass); }

		// ── Hearing ──────────────────────────────────────────────────────────────────────────────────────────

		// A gunshot is heard inside its radius (40 m) and not from outside the world: an Alarmed guard goes to look.
		void HearingGunshot(Ctx& ctx)
		{
			const Placement placed = PlaceGuard(ctx, NpcClass, ctx.puppet.Position());
			const glm::vec3 home = placed.position;

			auto reacted = [&]
			{
				const Snapshot* guard = ctx.probe.Find(GuardName);
				return guard && (guard->speed > 0.5f || glm::distance(guard->position, home) > 1.0f);
			};

			// Far outside the world: not heard.
			ctx.puppet.Park(home + glm::vec3(400.0f, 100.0f, 400.0f));
			ctx.Wait(1.0f);
			ctx.puppet.Shoot();
			ctx.Wait(8.0f);
			ctx.Check(reacted() == false && GuardNpc(ctx).GetAlert() == NpcAlert::Calm, "a shot 560 m away is not heard");

			// 20 m away, a wall or not: heard (the gunshot radius is 40 m).
			glm::vec3 spot;
			if (ctx.world.FindClearSpot(home, 20.0f, placed.yaw, 180.0f, spot, 4.0f) == false
				&& ctx.world.FindHiddenSpot(home, 15.0f, 25.0f, spot) == false)
			{
				ctx.Info("no floor 15 to 25 m from the guard");
				return;
			}

			ctx.puppet.TeleportTo(spot);
			ctx.Wait(1.0f);
			ctx.puppet.Shoot();
			const bool heard = ctx.WaitUntil(reacted, 8.0f);
			ctx.Check(heard, "a gunshot 20 m away makes the guard react within 8 s");
			ctx.Check(GuardNpc(ctx).GetAlert() >= NpcAlert::Alarmed, "and an alarm sound makes it at least Alarmed");
		}

		// A footstep is heard inside its radius (8 m) and adds to the awareness of whoever made it, though it never identifies.
		void HearingFootsteps(Ctx& ctx)
		{
			const NpcSoundTuning& footstep = NpcPerceptionTuning::Get().SoundFor("footstep");
			const glm::vec3 start = ctx.puppet.Position();

			for (const float distance : { footstep.radius * 0.6f, footstep.radius * 2.0f })
			{
				const Placement placed = PlaceGuard(ctx, NpcClass, start);

				glm::vec3 spot;
				if (ctx.world.FindClearSpot(placed.position, distance, placed.yaw + 180.0f, 20.0f, spot, 2.0f) == false)
				{
					ctx.Info("no open ground %.1f m behind the guard", distance);
					continue;
				}

				ctx.puppet.TeleportTo(spot);
				ctx.puppet.Hold(Posture::Still);
				ctx.Wait(1.0f);
				ctx.puppet.Emit(NpcSoundType::Footstep);
				ctx.Wait(0.5f);

				const float value = MeterOf(ctx);
				if (distance < footstep.radius)
					ctx.Check(value > 0.02f && value < 1.0f, "a footstep %.1f m behind the guard (radius %.0f m) raises its awareness without identifying (%.2f)", distance, footstep.radius, value);
				else
					ctx.Check(value <= 0.01f, "a footstep %.1f m away (radius %.0f m) is not heard (%.2f)", distance, footstep.radius, value);
			}
		}
	}

	void RegisterSenseScenarios()
	{
		Scenario scenario;
		scenario.tier = "sense";
		scenario.timeLimit = 400.0f;

		scenario.name = "sense_sight_distances";
		scenario.run = SightDistances;
		Register(scenario);

		scenario.name = "sense_sight_zones";
		scenario.run = SightZones;
		Register(scenario);

		scenario.name = "sense_sight_motion";
		scenario.run = SightMotion;
		Register(scenario);

		scenario.name = "sense_sight_identify";
		scenario.run = SightIdentify;
		Register(scenario);

		scenario.name = "sense_hud_meter_npc";
		scenario.run = HudMeterNpc;
		Register(scenario);

		scenario.name = "sense_hud_meter_legacy";
		scenario.run = HudMeterLegacy;
		Register(scenario);

		scenario.name = "sense_hearing_gunshot";
		scenario.run = HearingGunshot;
		Register(scenario);

		scenario.name = "sense_hearing_footsteps";
		scenario.run = HearingFootsteps;
		Register(scenario);
	}
}

#endif

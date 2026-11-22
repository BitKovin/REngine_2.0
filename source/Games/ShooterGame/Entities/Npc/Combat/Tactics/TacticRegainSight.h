#pragma once

#include "TacticHelpers.h"
#include "CombatMoves.h"
#include "../CombatBehaviorBase.h"
#include "../CombatCoordinator.h"
#include "../CombatTacticBase.h"
#include "../CoverFinder.h"
#include "../CoverPlanner.h"
#include "../RangedFireControl.h"
#include "../../NpcBase.h"
#include "../CombatTuning.h"

#include <RandomHelper.h>

#include <algorithm>
#include <cmath>

// A gun NPC moving to see the target from somewhere else. One tactic, four
// situations:
//
//   Lane       the squad plan gave it a lane (a doorway or corridor of the
//              approach map, §8.7) to flank or push through: sprint to the
//              hidden stack spot short of the entry (gun ready), wait there
//              crouched with the gun on the entry, and when the squad breaches
//              walk in through it firing. Pushers holding for their turn wait
//              crouched where they are, aimed at the entry.
//   Flank      the squad plan sent it round (SquadRole::Flank): run to a spot
//              65..115 degrees round from the squad's axis on its given side,
//              by the route that stays out of the target's sight the most.
//              Works whether the target is visible (pincer while the rest
//              keep it busy) or hiding (go round the cover it hides behind).
//              Done when it gets there, or earlier once it sees the target
//              from well round the side; then it fights from there.
//   Push       the target hides and the plan has this NPC push up
//              (SquadRole::Push): move up on the believed spot from its own
//              side while the suppressors keep the target down.
//   Reacquire  the squad sees the target but this NPC doesn't (behind a wall,
//              wrong side of a pillar) and it has no cover spot to go to:
//              move to a reachable spot that sees the target, close by.
//
// Close to a hiding target (CombatCoordinator::CloseQuartersDistance) a pusher
// works round the corner the target is behind: short crouched steps sideways
// to spots that see further round it, gun on the spot, closing in when there
// is no room to the side. It never hops between far-off random spots.
// Further out, if a spot doesn't show the target after a moment it picks
// another one.
class TacticRegainSight : public CombatTacticBase
{
public:
	explicit TacticRegainSight(NpcBase* body) : body(body) {}

	const char* Id() const override { return "regain_sight"; }

	float Score(const CombatContext& ctx) const override
	{
		switch (ModeFor(ctx))
		{
		case Mode::Lane:      return 1.25f; // the plan's assault comes before anything the NPC would pick
		case Mode::Flank:     return 1.2f;  // above cover, kiting and strafing: the plan wants it round
		case Mode::Reacquire: return 1.15f;
		case Mode::Push:      return 1.0f;
		default:              return 0.0f;
		}
	}

	void Start(const CombatContext& ctx) override
	{
		mode = ModeFor(ctx);
		ResetSearch();
	}

	void Update(const CombatContext& ctx, float) override
	{
		const double now = Time::GameTime;
		const vec3 threat = ctx.threatPosition;

		// The role can change while running (flank ordered, target re-seen).
		const Mode current = ModeFor(ctx);
		if (current != mode && current != Mode::None)
		{
			mode = current;
			ResetSearch();
		}

		if (mode == Mode::Lane)
		{
			UpdateLane(ctx, now);
			return;
		}

		if (mode == Mode::Push && TacticHelpers::DistanceXZ(body->Position, threat) < CombatTuning::Get().squad.closeQuartersDistance)
		{
			UpdateCloseQuarters(ctx, now);
			return;
		}

		// A visible target that moved a lot since the spot was picked: pick again.
		if (hasPosition && TacticHelpers::DistanceXZ(threat, searchedAgainst) > CombatTuning::Get().regainSight.retargetDistance)
		{
			hasPosition = false;
			nextSearch = std::min(nextSearch, now);
		}

		if (!hasPosition && now >= nextSearch)
		{
			hasPosition = FindPosition(ctx, position);
			searchedAgainst = threat;
			nextSearch = now + CombatTuning::Get().regainSight.searchRetry + RandomHelper::RandomFloat() * 0.5;
			arrivedAt = 0.0;

			if (!hasPosition) failedSearches++;
		}

		// No way round from here: the plan gives the job to somebody else.
		if (mode == Mode::Flank && failedSearches >= CombatTuning::Get().regainSight.maxFailedSearches)
			FinishFlank();

		if (ctx.Hunting() || mode != Mode::Reacquire)
		{
			body->SetAimPoint(threat);
			body->SetWeaponAimed();
		}
		body->UpdateAimAndAttackPosition(false);

		if (ctx.fire->IsReloading())
			body->SetWeaponLowered(true);

		const bool arrived = hasPosition && TacticHelpers::DistanceXZ(body->Position, position) < CombatTuning::Get().regainSight.arriveDistance;

		if (arrived)
		{
			if (arrivedAt == 0.0) arrivedAt = now;

			if (mode == Mode::Flank)
				FinishFlank();

			// A spot that turned out not to show the target: try another.
			if (now - arrivedAt > CombatTuning::Get().regainSight.noSightRetryTime && !body->SeesTarget())
			{
				hasPosition = false;
				nextSearch = now;
			}
		}

		// Round the side with the target in sight and in range: that is the flank.
		if (mode == Mode::Flank && body->SeesTarget() && FlankedEnough(ctx))
			FinishFlank();

		body->SetAttackPosition(hasPosition ? position : FallbackPosition(ctx));

		const bool hunting = ctx.Hunting();
		const bool close = TacticHelpers::DistanceXZ(body->Position, threat) < CombatTuning::Get().regainSight.creepDistance;
		const bool creep = hunting && (close || arrived);

		CombatMoveOrder order;
		order.active = true;
		order.stance = creep ? NpcStance::Crouched : NpcStance::Standing;

		if (creep)
		{
			order.speed = CombatTuning::Get().movement.crouchWalkSpeed;
			order.facing = NpcFacing::Point;
			order.lookAt = threat;
		}
		else if (arrived || (mode == Mode::Reacquire && body->SeesTarget()))
		{
			order.speed = CombatTuning::Get().movement.strafeSpeed;
			order.facing = NpcFacing::Point;
			order.lookAt = threat;
		}
		else
		{
			// Getting there: a sprint, not a stroll.
			order.speed = CombatTuning::Get().movement.runSpeed;
			order.facing = NpcFacing::Movement;
		}

		body->SetCombatMoveOrder(order);
	}

private:
	enum class Mode { None, Reacquire, Flank, Push, Lane };

	float stalledTime = 0.0f;

	NpcBase* body = nullptr;
	Mode mode = Mode::None;
	bool hasPosition = false;
	vec3 position{};
	vec3 searchedAgainst{};
	double nextSearch = 0.0;
	double arrivedAt = 0.0;
	int failedSearches = 0;

	double breachStart = 0.0;

	bool hasSlice = false;
	vec3 slice{};
	double nextSlice = 0.0;
	float sliceSign = 0.0f;

	// "Slicing the pie": each step is a short crouched sidestep, on the same side
	// as the last, to a spot that sees the believed position, or (no such spot)
	// a step straight in. The gun stays on the spot all the time; the brain fires
	// the moment the target shows.
	void UpdateCloseQuarters(const CombatContext& ctx, double now)
	{
		const vec3 threat = ctx.threatPosition;
		const float distance = TacticHelpers::DistanceXZ(body->Position, threat);

		if (sliceSign == 0.0f)
			sliceSign = TacticHelpers::StableRandom01(body->Id, 31) < 0.5f ? -1.0f : 1.0f;

		const bool reached = hasSlice && TacticHelpers::DistanceXZ(body->Position, slice) < 0.4f;

		if (now >= nextSlice || reached)
		{
			float sign = sliceSign;
			hasSlice = CombatMoves::PickSidestep(body, threat, ctx.threatEye,
				CombatTuning::Get().regainSight.sliceMinDistance, std::max(CombatTuning::Get().regainSight.sliceMinDistance, distance), CombatTuning::Get().regainSight.sliceStep, sign, slice);

			if (hasSlice)
			{
				// PickSidestep hands back the other side for next time; keep working
				// round the side it actually stepped to.
				sliceSign = -sign;
			}
			else if (distance > CombatTuning::Get().regainSight.sliceMinDistance)
			{
				const vec3 in = MathHelper::Normalized(MathHelper::XZ(threat - body->Position));
				hasSlice = CombatMoves::WalkablePoint(body, body->Position + in * std::min(CombatTuning::Get().regainSight.sliceStep, distance - CombatTuning::Get().regainSight.sliceMinDistance), slice);
			}

			nextSlice = now + CombatTuning::Get().regainSight.sliceInterval;
		}

		body->SetAimPoint(threat);
		body->UpdateAimAndAttackPosition(false);
		body->SetAttackPosition(hasSlice ? slice : body->Position);

		CombatMoveOrder move;
		move.active = true;
		move.stance = NpcStance::Crouched;
		move.speed = CombatTuning::Get().movement.crouchWalkSpeed;
		move.facing = NpcFacing::Point;
		move.lookAt = threat;
		move.careful = true;
		body->SetCombatMoveOrder(move);
	}

	void ResetSearch()
	{
		hasPosition = false;
		nextSearch = 0.0;
		arrivedAt = 0.0;
		failedSearches = 0;
	}

	// Lane order (§8.7): stack, then breach on the squad's call.
	void UpdateLane(const CombatContext& ctx, double now)
	{
		CombatBehaviorBase* brain = body->GetCombatBehavior();
		const SquadOrder& order = ctx.squad;
		const vec3 threat = ctx.threatPosition;
		const vec3 entryAim = order.entry + vec3(0, CombatTuning::Get().regainSight.entryAimHeight, 0);

		CombatMoveOrder move;
		move.active = true;

		if (order.breach && brain->GetLanePhase() != LanePhase::Breaching)
		{
			brain->ReportLanePhase(LanePhase::Breaching);
			breachStart = now;
		}

		if (brain->GetLanePhase() == LanePhase::Breaching)
		{
			// Through the entry toward the target, gun up and firing.
			const vec3 inward = MathHelper::Normalized(MathHelper::XZ(threat - order.entry));
			body->SetAttackPosition(order.entry + inward * CombatTuning::Get().regainSight.breachDepth);
			body->SetAimPoint(threat);

			move.speed = CombatTuning::Get().movement.strafeSpeed;
			move.facing = NpcFacing::Point;
			move.lookAt = threat;
			move.careful = true;

			const bool inSight = body->SeesTarget() && TacticHelpers::DistanceXZ(body->Position, order.entry) < CombatTuning::Get().regainSight.breachDoneDistance;
			if (inSight || now - breachStart > CombatTuning::Get().regainSight.breachTime)
				brain->ReportLanePhase(LanePhase::Done);
		}
		else if (order.waitTurn)
		{
			// Another pusher is moving up: hold here, gun on the entry.
			body->SetAttackPosition(body->Position);
			body->SetAimPoint(entryAim);

			move.stance = NpcStance::Crouched;
			move.speed = CombatTuning::Get().movement.crouchWalkSpeed;
			move.facing = NpcFacing::Point;
			move.lookAt = entryAim;
			move.careful = true;
		}
		else
		{
			const float toStack = TacticHelpers::DistanceXZ(body->Position, order.stack);
			body->SetAttackPosition(order.stack);

			// Got as close as the navmesh lets it: that is its place in the stack.
			const bool stalled = toStack < CombatTuning::Get().regainSight.stackApproachDistance
				&& glm::length(MathHelper::XZ(body->GetControllerVelocity())) < CombatTuning::Get().regainSight.stallSpeed;
			stalledTime = stalled ? stalledTime + static_cast<float>(Time::DeltaTimeF) : 0.0f;

			if (toStack < CombatTuning::Get().regainSight.stackArriveDistance || stalledTime > CombatTuning::Get().regainSight.stallTime)
				brain->ReportLanePhase(LanePhase::Stacked);

			// Waiting for a way in that no longer leads to the target (it moved
			// round): the stack is pointless. The plan sends it round another way
			// if there is one; otherwise it rejoins the base of fire.
			if (brain->GetLanePhase() == LanePhase::Stacked && ctx.isFlanker && !StillThroughEntry(ctx))
			{
				brain->ReportLaneStale();
				body->SetAttackPosition(body->Position);
			}
			else if (toStack > CombatTuning::Get().regainSight.stackLeftDistance && brain->GetLanePhase() == LanePhase::Stacked)
				brain->ReportLanePhase(LanePhase::Moving);

			// Spotted on the way or at the stack, the surprise is gone: no hiding.
			// From well round the side that is the flank, fight from right here;
			// otherwise go in through the entry now (which starts the squad's breach).
			if (ctx.isFlanker && body->SeesTarget())
			{
				if (FlankedEnough(ctx))
				{
					brain->ReportFlankEngaged();
					body->SetAttackPosition(body->Position);
				}
				else if (TacticHelpers::DistanceXZ(body->Position, order.entry) < CombatTuning::Get().regainSight.spottedBreachDistance)
				{
					brain->ReportLanePhase(LanePhase::Breaching);
					breachStart = now;
				}
			}

			if (toStack < CombatTuning::Get().regainSight.stackApproachDistance)
			{
				// The last stretch and the wait: crouched, gun on the entry.
				body->SetAimPoint(entryAim);

				move.stance = NpcStance::Crouched;
				move.speed = CombatTuning::Get().movement.crouchWalkSpeed;
				move.facing = NpcFacing::Point;
				move.lookAt = entryAim;
				move.careful = true;
			}
			else
			{
				body->SetAimPoint(threat);

				move.speed = CombatTuning::Get().movement.runSpeed;
				move.facing = NpcFacing::Movement;
			}
		}

		body->UpdateAimAndAttackPosition(false);
		body->SetCombatMoveOrder(move);
	}

	// No spot found (yet): a pusher or a reacquiring NPC doesn't wait for one, it
	// closes in on the target, which opens most lines of sight. A flanker stays
	// put until its search succeeds or it gives the flank up.
	vec3 FallbackPosition(const CombatContext& ctx) const
	{
		if (mode == Mode::Flank) return body->Position;

		// No spot with a line found: a squad member (not pushing) steps sideways
		// to look round whatever is in the way; it doesn't walk at the target.
		if (mode == Mode::Reacquire)
		{
			vec3 sidestep;
			const vec3 toThreat = MathHelper::Normalized(MathHelper::XZ(ctx.threatPosition - body->Position));
			const vec3 side = MathHelper::Normalized(glm::cross(toThreat, vec3(0, 1, 0)));
			const float sign = TacticHelpers::StableRandom01(body->Id, 51) < 0.5f ? -1.0f : 1.0f;

			for (const float s : { sign, -sign })
			{
				if (CombatMoves::WalkablePoint(body, body->Position + side * (s * 3.0f), sidestep))
					return sidestep;
			}
			return body->Position;
		}

		const vec3 threat = ctx.threatPosition;
		vec3 away = MathHelper::Normalized(MathHelper::XZ(body->Position - threat));
		if (glm::length(away) < 0.001f) return body->Position;

		const float distance = TacticHelpers::DistanceXZ(body->Position, threat);
		const float keep = mode == Mode::Push ? CombatTuning::Get().regainSight.fallbackPushDistance : std::min(body->GetAttackDesiredRange(), distance - 2.0f);

		return threat + away * std::max(keep, CombatTuning::Get().regainSight.fallbackPushDistance);
	}

	void FinishFlank()
	{
		if (CombatBehaviorBase* brain = body->GetCombatBehavior())
			brain->ReportFlankFinished();
	}

	// Is the target still somewhere through the entry this NPC waits at?
	bool StillThroughEntry(const CombatContext& ctx) const
	{
		const vec3 toEntry = MathHelper::XZ(ctx.squad.entry - body->Position);
		const vec3 toThreat = MathHelper::XZ(ctx.threatPosition - body->Position);
		if (glm::length(toEntry) < 0.001f || glm::length(toThreat) < 0.001f) return true;

		const float angle = glm::degrees(std::acos(glm::clamp(glm::dot(MathHelper::Normalized(toEntry), MathHelper::Normalized(toThreat)), -1.0f, 1.0f)));
		return angle <= CombatTuning::Get().regainSight.stackStaleAngleDegrees && glm::length(toThreat) > glm::length(toEntry) - 0.5f;
	}

	bool FlankedEnough(const CombatContext& ctx) const
	{
		const vec3 to = MathHelper::Normalized(MathHelper::XZ(body->Position - ctx.threatPosition));
		const float cosAngle = glm::dot(to, ctx.squad.axis);

		return cosAngle < std::cos(glm::radians(CombatTuning::Get().regainSight.flankedAngleDegrees))
			&& TacticHelpers::DistanceXZ(body->Position, ctx.threatPosition) < body->GetAttackDesiredRange() + 4.0f;
	}

	Mode ModeFor(const CombatContext& ctx) const
	{
		if (ctx.meleeMode || !ctx.target.Valid() || ctx.Lost()) return Mode::None;

		const CombatBehaviorBase* brain = body->GetCombatBehavior();
		const bool laneOpen = brain && ctx.squad.hasLane && brain->GetLanePhase() != LanePhase::Done;

		if (ctx.isFlanker && brain && !brain->FlankFinished())
			return laneOpen ? Mode::Lane : Mode::Flank;

		if (ctx.Concealed())
		{
			if (!ctx.isPusher) return Mode::None;
			return laneOpen ? Mode::Lane : Mode::Push;
		}

		const bool someoneSees = ctx.contact == TargetContact::Visible;
		const bool iDontSee = !body->SeesTarget() && ctx.fire->UnseenTime() > CombatTuning::Get().regainSight.reacquireDelay;
		const bool noCoverToGoTo = !ctx.cover || !ctx.cover->HasCover();

		if (someoneSees && iDontSee && noCoverToGoTo) return Mode::Reacquire;

		return Mode::None;
	}

	bool FindPosition(const CombatContext& ctx, vec3& out) const
	{
		FlankQuery query;
		query.self = body;
		query.npcPosition = body->Position;
		query.npcCenterHeight = body->GetCenterHeight();
		query.estimatePosition = ctx.threatPosition;
		query.threatEye = ctx.threatEye;

		const float desired = body->GetAttackDesiredRange();

		switch (mode)
		{
		case Mode::Flank:
			query.baseDirection = ctx.squad.axis;
			query.side = ctx.squad.side;
			query.minAngleDegrees = 65.0f;
			query.maxAngleDegrees = 115.0f;
			if (ctx.Hunting())
			{
				query.minDistance = 5.0f;
				query.maxDistance = 12.0f;
			}
			else
			{
				query.minDistance = std::max(6.0f, desired - 4.0f);
				query.maxDistance = std::min(16.0f, desired + 2.0f);
			}
			query.maxPathLength = 40.0f;
			break;

		case Mode::Push:
			query.minAngleDegrees = 10.0f;
			query.maxAngleDegrees = 70.0f;
			query.minDistance = 4.0f;
			query.maxDistance = 10.0f;
			query.routeClearance = 2.0f;
			break;

		default: // Reacquire: a short move sideways or back, never much closer
		{
			const float distance = TacticHelpers::DistanceXZ(body->Position, ctx.threatPosition);
			query.minAngleDegrees = 0.0f;
			query.maxAngleDegrees = 75.0f;
			query.minDistance = std::max(5.0f, distance - CombatTuning::Get().regainSight.reacquireMaxCloser);
			query.maxDistance = std::max(query.minDistance + 3.0f, std::max(desired, distance) + 3.0f);
			query.maxPathLength = 25.0f;
			query.exposedRouteCost = 0.0f; // it wants to be seen-and-seeing
			break;
		}
		}

		return CoverFinder::FindFlankPosition(query, out);
	}
};

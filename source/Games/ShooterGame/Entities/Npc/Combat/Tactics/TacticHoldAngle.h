#pragma once

#include "CombatMoves.h"
#include "TacticHelpers.h"
#include "../CombatTacticBase.h"
#include "../CoverFinder.h"
#include "../CoverMap.h"
#include "../CoverPlanner.h"
#include "../RangedFireControl.h"
#include "../../NpcBase.h"
#include "../CombatTuning.h"

#include <Physics.h>

#include <algorithm>

// The target ducked out of sight and the squad believes it is still there:
// the main force's NPCs hold where they are (in cover if they are in cover),
// keep the gun on a way the target can come out (the front entry, or a point
// over or beside its cover), and shoot the moment it does. A holder with no
// line on that point (a wall in between) moves a short way to a spot that has
// one instead of staring at the wall. One the target has left out of its
// fighting range moves up to a spot within range that sees it. When the target
// turned up somewhere else (heard or tracked, belief.seekUntil) the holders go
// to find it: each moves to a spot that sees the new position, closer if need be.
class TacticHoldAngle : public CombatTacticBase
{
public:
	explicit TacticHoldAngle(NpcBase* body) : body(body) {}

	const char* Id() const override { return "hold_angle"; }

	float Score(const CombatContext& ctx) const override
	{
		if (ctx.meleeMode || !ctx.target.Valid() || !ctx.belief || !ctx.cover) return 0.0f;
		if (!ctx.Concealed() || !ctx.isHolder) return 0.0f;

		// With cover on the way, getting there comes first.
		if (ctx.cover->HasCover() && !ctx.cover->Arrived()) return 0.0f;

		return 1.0f;
	}

	void Update(const CombatContext& ctx, float) override
	{
		// The way the target would come out at the squad, when the map knows it;
		// otherwise a point over or beside its cover.
		const int index = static_cast<int>(TacticHelpers::StableRandom01(body->Id) * TargetBelief::PeekPointCount) % TargetBelief::PeekPointCount;
		// Seeking a target that turned up elsewhere: the gun goes to where it was heard.
		const bool seeking = Time::GameTime < ctx.belief->seekUntil;
		const vec3 peekPoint = seeking ? ctx.threatEye
			: (ctx.squad.hasLane ? ctx.squad.entry + vec3(0, 1.2f, 0) : ctx.belief->peekPoints[index]);

		body->SetAimPoint(peekPoint);
		body->SetWeaponAimed();
		body->UpdateAimAndAttackPosition(false);

		if (ctx.fire->IsReloading())
			body->SetWeaponLowered(true);

		const bool inCover = ctx.cover->Arrived();
		const CoverType coverType = inCover ? ctx.cover->Spot().type : CoverType::None;

		UpdateLine(ctx, peekPoint);

		CombatMoveOrder order;
		order.active = true;
		order.facing = NpcFacing::Point;
		order.lookAt = peekPoint;

		if (hasLineSpot)
		{
			// Off to a spot with a line on it (runs: it can't shoot from here anyway).
			body->SetAttackPosition(lineSpot);
			order.speed = CombatTuning::Get().movement.strafeSpeed;
			order.stance = NpcStance::Standing;
		}
		else
		{
			body->SetAttackPosition(inCover ? ctx.cover->Spot().position : body->Position);
			order.speed = CombatTuning::Get().movement.crouchWalkSpeed;
			order.stance = coverType == CoverType::Full ? NpcStance::Standing : NpcStance::Crouched;
			order.careful = true;
		}

		body->SetCombatMoveOrder(order);

		// It showed up where this NPC was watching: no first-shot warning miss, and a
		// better aim for a moment.
		if (body->SeesTarget())
			ctx.fire->OnHoldReaction();
	}

private:
	NpcBase* body = nullptr;
	static constexpr double LineSpotTimeout = 8.0; // seconds to get there before looking again

	bool hasLineSpot = false;
	vec3 lineSpot{};
	double lineSpotSince = 0.0;
	vec3 lineSpotFor{};
	double nextLineSearch = 0.0;

	// No line on the watched point from here: find a spot close by that has one.
	void UpdateLine(const CombatContext& ctx, const vec3& point)
	{
		const auto& tune = CombatTuning::Get().hold;
		const double now = Time::GameTime;

		// There, given up on (unreachable), or picked for where the target no
		// longer is: look again.
		if (hasLineSpot && (TacticHelpers::DistanceXZ(body->Position, lineSpot) < 0.8f || now - lineSpotSince > LineSpotTimeout
			|| TacticHelpers::DistanceXZ(lineSpotFor, ctx.threatPosition) > CombatTuning::Get().belief.relocateDistance))
			hasLineSpot = false;

		if (hasLineSpot || now < nextLineSearch) return;
		nextLineSearch = now + tune.lineSearchInterval;

		// Out of range: get back within fighting range of it, line or not.
		const float range = body->GetAttackRange() * CombatTuning::Get().cover.pursuitRangeFraction;
		const float toThreat = TacticHelpers::DistanceXZ(body->Position, ctx.threatPosition);
		if (toThreat > range)
		{
			FlankQuery query;
			query.self = body;
			query.npcPosition = body->Position;
			query.npcCenterHeight = body->GetCenterHeight();
			query.estimatePosition = ctx.threatPosition;
			query.threatEye = ctx.threatEye;
			query.minAngleDegrees = 0.0f;
			query.maxAngleDegrees = 75.0f;
			query.minDistance = range * tune.closeInBandFraction;
			query.maxDistance = range;
			query.maxPathLength = tune.closeInMaxPath;
			query.routeClearance = 2.0f;

			hasLineSpot = CoverFinder::FindFlankPosition(query, lineSpot);
			lineSpotSince = now;
			lineSpotFor = ctx.threatPosition;

			// Nothing with a line in range: head that way anyway.
			if (!hasLineSpot)
			{
				lineSpot = CombatMoves::ApproachPoint(body, ctx.threatPosition, range * 0.8f, 2.0f);
				hasLineSpot = true;
			}
			return;
		}

		// Seeking (the target turned up elsewhere): it is the new position that
		// needs a line, not the old angle.
		const bool seeking = ctx.belief && Time::GameTime < ctx.belief->seekUntil;
		const vec3 watched = seeking ? ctx.threatEye : point;

		const vec3 eye = body->Position + vec3(0, 0.6f, 0);
		const auto hit = Physics::LineTrace(eye, watched, CoverMap::BlockerMask());
		if (!hit.hasHit || glm::distance(hit.position, watched) <= tune.lineMargin) return;

		FlankQuery query;
		query.self = body;
		query.npcPosition = body->Position;
		query.npcCenterHeight = body->GetCenterHeight();
		query.estimatePosition = point;
		query.threatEye = ctx.threatEye;
		query.minAngleDegrees = 0.0f;
		query.maxAngleDegrees = 90.0f;
		const float distance = TacticHelpers::DistanceXZ(body->Position, point);
		query.minDistance = std::max(tune.minDistance, distance - 1.0f);
		query.maxDistance = std::max(query.minDistance + 1.0f, distance + 3.0f);
		query.maxPathLength = tune.maxPath;
		query.exposedRouteCost = 0.0f;
		query.routeClearance = 2.0f;

		// Seeking: any spot that sees the new position, closer if need be;
		// otherwise hardly closer than it stands.
		if (seeking)
		{
			query.estimatePosition = ctx.threatPosition;
			query.minDistance = tune.minDistance;
			query.maxDistance = std::max(query.minDistance + 1.0f, std::min(distance + 3.0f,
				body->GetAttackRange() * CombatTuning::Get().cover.pursuitRangeFraction));
			query.maxPathLength = tune.closeInMaxPath;
		}

		hasLineSpot = CoverFinder::FindFlankPosition(query, lineSpot);
		lineSpotSince = now;
		lineSpotFor = ctx.threatPosition;

		// Seeking and nothing sees it from round here: head for it.
		if (!hasLineSpot && seeking)
		{
			lineSpot = CombatMoves::ApproachPoint(body, ctx.threatPosition, tune.minDistance + 2.0f, 2.0f);
			hasLineSpot = true;
		}
	}
};

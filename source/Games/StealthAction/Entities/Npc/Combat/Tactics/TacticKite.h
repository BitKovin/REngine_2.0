#pragma once

#include "CombatMoves.h"
#include "TacticHelpers.h"
#include "../CombatTacticBase.h"
#include "../CoverMap.h"
#include "../RangedFireControl.h"
#include "../../NpcBase.h"
#include "../CombatTuning.h"

#include <Physics.h>
#include <RandomHelper.h>

// A gunner against a target that can't shoot back (a blade, empty hands):
// there is nothing to hide from, so it fights in the open, and its whole job
// is to keep shooting while staying out of reach. COMBAT_AI_DESIGN.md §7.1.
//
//   far (> StrafeMax + FarSlack) bounds in to the band: sprint, stop and shoot
//   out of reach                 sidesteps every couple of seconds, alternating
//                                sides, while holding StrafeMin..StrafeMax from
//                                the target, shooting all the while
//   closing (< KiteMinDistance)  backs off while still shooting
//   on top of it (< PanicDistance) turns and runs to make room
//   in reach (< ShoveRange)      kicks the attacker away, then keeps going
//   cornered                     no way back: stands and shoots, kicks when it can
class TacticKite : public CombatTacticBase
{
public:
	explicit TacticKite(NpcBase* body) : body(body) {}

	const char* Id() const override { return "kite"; }

	// Outranks strafing and cover: those are for a target that shoots.
	float Score(const CombatContext& ctx) const override
	{
		return !ctx.meleeMode && ctx.target.Valid() && !ctx.Hunting() && !ctx.target.hasGun ? 1.1f : 0.0f;
	}

	void Start(const CombatContext&) override
	{
		hasRetreat = false;
		nextRetreatSearch = 0.0;
		hasStrafePoint = false;
		nextStrafe = 0.0;
		strafeSign = TacticHelpers::StableRandom01(body->Id, 7) < 0.5f ? -1.0f : 1.0f;
		bounding.Reset();
	}

	void Update(const CombatContext& ctx, float) override
	{
		const double now = Time::GameTime;
		const vec3 target = ctx.threatPosition;
		const float distance = TacticHelpers::DistanceXZ(body->Position, target);

		const bool backingOff = distance < CombatTuning::Get().kite.kiteMinDistance;
		body->UpdateAimAndAttackPosition(false);

		if (ctx.fire->IsReloading())
			body->SetWeaponLowered(true);

		CombatMoveOrder order;
		order.active = true;
		order.stance = NpcStance::Standing;
		order.facing = NpcFacing::Target;
		order.speed = CombatTuning::Get().movement.strafeSpeed;

		// Mid-kick: stand still until it is over.
		if (body->IsPlayingActionAnimation())
		{
			body->SetAttackPosition(body->Position);
			body->SetCombatMoveOrder(order);
			return;
		}

		if (distance < CombatTuning::Get().shove.range && body->CanShove())
		{
			body->PerformShove();
			body->SetAttackPosition(body->Position);
			body->SetCombatMoveOrder(order);
			return;
		}

		// Far out: close in to the band fast, stopping to shoot on the way.
		if (distance > CombatTuning::Get().kite.strafeMaxDistance + CombatTuning::Get().kite.farSlack)
		{
			hasRetreat = false;
			hasStrafePoint = false;

			const vec3 goal = CombatMoves::ApproachPoint(body, target, CombatTuning::Get().kite.strafeMaxDistance - 1.0f, CombatTuning::Get().kite.spreadOffset);
			bounding.Update(body, goal, target, body->SeesTarget() && !ctx.fire->IsReloading(), order);

			body->SetCombatMoveOrder(order);
			return;
		}

		bounding.Reset();

		if (!backingOff)
		{
			hasRetreat = false;

			const bool reachedStrafePoint = hasStrafePoint && TacticHelpers::DistanceXZ(body->Position, strafePoint) < 0.6f;
			if (now >= nextStrafe || reachedStrafePoint)
			{
				hasStrafePoint = CombatMoves::PickSidestep(body, target, ctx.threatEye,
					CombatTuning::Get().kite.strafeMinDistance, CombatTuning::Get().kite.strafeMaxDistance, CombatTuning::Get().kite.strafeStep, strafeSign, strafePoint);
				nextStrafe = now + CombatTuning::Get().kite.strafeInterval + RandomHelper::RandomFloat() * CombatTuning::Get().kite.strafeJitter;
			}

			body->SetAttackPosition(hasStrafePoint ? strafePoint : body->Position);
		}
		else if (now >= nextRetreatSearch)
		{
			hasRetreat = FindRetreat(target, retreatPoint);
			nextRetreatSearch = now + CombatTuning::Get().kite.retreatSearchInterval;
		}

		if (hasRetreat && distance < CombatTuning::Get().kite.panicDistance)
		{
			body->SetAttackPosition(retreatPoint);
			order.speed = CombatTuning::Get().movement.runSpeed;
			order.facing = NpcFacing::Movement;
		}
		else if (hasRetreat)
		{
			// Slow enough to keep shooting (NpcBase::RangedMaxFireSpeed).
			body->SetAttackPosition(retreatPoint);
		}
		else if (backingOff)
		{
			// Cornered: hold the ground and shoot.
			body->SetAttackPosition(body->Position);
		}

		body->SetCombatMoveOrder(order);
	}

private:

	NpcBase* body = nullptr;
	CombatMoves::BoundingApproach bounding;
	bool hasRetreat = false;
	vec3 retreatPoint{};
	double nextRetreatSearch = 0.0;

	bool hasStrafePoint = false;
	vec3 strafePoint{};
	double nextStrafe = 0.0;
	float strafeSign = 1.0f;

	// The open direction that takes the NPC furthest from the target, preferring
	// straight back. False when every way back is blocked (cornered).
	bool FindRetreat(const vec3& target, vec3& out) const
	{
		vec3 away = MathHelper::Normalized(MathHelper::XZ(body->Position - target));
		if (glm::length(away) < 0.001f) away = vec3(1, 0, 0);

		const float centerHeight = body->GetCenterHeight();
		const float groundY = body->Position.y - centerHeight;

		bool found = false;
		float bestScore = -1.0e9f;

		for (float degrees : { 0.0f, 35.0f, -35.0f, 70.0f, -70.0f, 105.0f, -105.0f })
		{
			const float angle = glm::radians(degrees);
			const vec3 direction(
				away.x * std::cos(angle) - away.z * std::sin(angle),
				0.0f,
				away.x * std::sin(angle) + away.z * std::cos(angle));

			const vec3 wanted = body->Position + direction * CombatTuning::Get().kite.retreatStep;

			const auto ground = Physics::LineTrace(wanted + vec3(0, 1.0f, 0), wanted - vec3(0, 3.0f, 0), BodyType::World);
			if (!ground.hasHit || std::abs(ground.position.y - groundY) > CombatTuning::Get().kite.maxRetreatStepHeight) continue;

			const vec3 candidate = ground.position + vec3(0, centerHeight, 0);

			if (Physics::CylinderTrace(body->Position, candidate, 0.5f, 0.8f, BodyType::World | BodyType::MainBody).hasHit) continue;

			const float score = TacticHelpers::DistanceXZ(candidate, target) - std::abs(degrees) * 0.01f;
			if (score > bestScore)
			{
				bestScore = score;
				out = candidate;
				found = true;
			}
		}

		return found;
	}
};

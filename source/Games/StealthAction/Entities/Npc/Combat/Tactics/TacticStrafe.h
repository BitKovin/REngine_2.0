#pragma once

#include "CombatMoves.h"
#include "../CombatBehaviorBase.h"
#include "../CombatTacticBase.h"
#include "../RangedFireControl.h"
#include "../../NpcBase.h"
#include "../CombatTuning.h"

#include <RandomHelper.h>

// Fighting a gun-armed target in the open (no cover spot to use): the NPC
// keeps the fight going from the range it is at and never stands still; it
// only closes in when the target is out of its fighting range.
//
//   far (> engage range + boundSlack)  bounds in: sprint, stop and shoot
//                                (kneeling half the time), sprint again
//   out of range (> engage range)  walks in firing
//                                Both head for this NPC's own side of the
//                                target so the squad spreads out.
//   in range                     sidesteps one step after another, alternating
//                                sides, keeping a line of sight and never more
//                                than maxCloserPerStep closer; boxed in, it
//                                backs off a step (pressInOnReload: steps in
//                                while the target reloads)
//
// The engage range is strafe.engageFraction of the attack range (CombatTuning).
// Walking speed stays under shot.maxFireSpeed so it shoots while it
// moves (the brain fires whenever the target is in sight). Strafing is the
// fallback when no cover is found.
class TacticStrafe : public CombatTacticBase
{
public:
	explicit TacticStrafe(NpcBase* body) : body(body) {}

	const char* Id() const override { return "strafe"; }

	// Not while the target hides: positions are picked against where the target is.
	float Score(const CombatContext& ctx) const override { return ctx.meleeMode || ctx.Hunting() ? 0.0f : 1.0f; }

	void Start(const CombatContext&) override
	{
		hasStep = false;
		pressingIn = false;
		nextStep = 0.0;
		bounding.Reset();
		sign = TacticHelpers::StableRandom01(body->Id, 3) < 0.5f ? -1.0f : 1.0f;
	}

	void Update(const CombatContext& ctx, float) override
	{
		body->UpdateAimAndAttackPosition(false);

		CombatMoveOrder order;
		order.active = true;
		order.stance = NpcStance::Standing;
		order.facing = NpcFacing::Target;
		order.speed = CombatTuning::Get().movement.strafeSpeed;

		if (!ctx.target.Valid())
		{
			body->SetCombatMoveOrder(order);
			return;
		}

		const double now = Time::GameTime;
		const vec3 target = ctx.threatPosition;
		const vec3 targetEye = ctx.threatEye;
		const auto& tune = CombatTuning::Get().strafe;
		const float distance = TacticHelpers::DistanceXZ(body->Position, target);

		// The main force fights from the range it is at: it only closes in when
		// the target is beyond its fighting range.
		const float engage = body->GetAttackRange() * tune.engageFraction;
		const vec3 approach = CombatMoves::ApproachPoint(body, target, engage - tune.advanceSlack, tune.spreadOffset);

		if (distance > engage + tune.boundSlack)
		{
			hasStep = false;
			bounding.Update(body, approach, target, body->SeesTarget() && !ctx.fire->IsReloading(), order);
		}
		else if (distance > engage)
		{
			// Just out of range: walk in firing, off to this NPC's own side.
			body->SetAttackPosition(approach);
			hasStep = false;
			bounding.Reset();
		}
		else
		{
			bounding.Reset();

			const bool reachedStep = hasStep && TacticHelpers::DistanceXZ(body->Position, step) < 0.6f;
			const bool pressIn = tune.pressInOnReload && ctx.target.isReloading && distance > tune.minDistance + tune.stepLength;

			if (now >= nextStep || reachedStep || (pressIn && !pressingIn))
			{
				// Sideways, never more than a little closer (unless pressing in on a reload).
				const float minDistance = pressIn ? tune.minDistance : std::max(tune.minDistance, distance - tune.maxCloserPerStep);
				const float maxDistance = pressIn ? distance - tune.stepLength : std::min(engage, distance + tune.stepLength);

				hasStep = CombatMoves::PickSidestep(body, target, targetEye,
					minDistance, std::max(minDistance, maxDistance), tune.stepLength, sign, step);

				// Boxed in sideways: back off a step rather than stand.
				if (!hasStep)
				{
					vec3 radial = MathHelper::Normalized(MathHelper::XZ(body->Position - target));
					hasStep = CombatMoves::WalkablePoint(body, body->Position + radial * tune.stepLength, step)
						&& CombatMoves::SeesFrom(step, body->GetCenterHeight(), targetEye);
				}

				pressingIn = pressIn;
				nextStep = now + CombatTuning::Get().strafe.stepInterval + RandomHelper::RandomFloat() * CombatTuning::Get().strafe.stepJitter;
			}

			body->SetAttackPosition(hasStep ? step : body->Position);
		}

		body->SetCombatMoveOrder(order);
	}

private:

	NpcBase* body = nullptr;
	CombatMoves::BoundingApproach bounding;
	bool hasStep = false;
	bool pressingIn = false;
	vec3 step{};
	double nextStep = 0.0;
	float sign = 1.0f;
};

#pragma once

#include "CombatMoves.h"
#include "TacticHelpers.h"
#include "../CombatBehaviorBase.h"
#include "../CombatTacticBase.h"
#include "../CombatTuning.h"
#include "../../NpcBase.h"

#include <algorithm>

// Locked in after a flank (§8.7): the NPC got round, so it fights from there
// for squad.postFlankPressTime, no cover, no new orders. It keeps a line of
// sight on the target at all costs:
//
//   sees the target   short sidesteps every press.stepInterval (never closer
//                     than press.minDistance), each one to a spot that still
//                     sees it; the brain fires all the while
//   lost the line     goes straight for the believed position (running: it
//                     can't shoot anyway) until the target shows or it is
//                     press.closeInDistance away
class TacticPress : public CombatTacticBase
{
public:
	explicit TacticPress(NpcBase* body) : body(body) {}

	const char* Id() const override { return "press"; }

	// Above every other gun tactic, lane orders included.
	float Score(const CombatContext& ctx) const override
	{
		const CombatBehaviorBase* brain = body->GetCombatBehavior();
		if (ctx.meleeMode || !ctx.target.Valid() || ctx.Lost() || !brain) return 0.0f;

		return brain->IsPressingAttack() ? 1.3f : 0.0f;
	}

	void Start(const CombatContext&) override
	{
		hasStep = false;
		nextStep = 0.0;
		sign = TacticHelpers::StableRandom01(body->Id, 41) < 0.5f ? -1.0f : 1.0f;
	}

	void Update(const CombatContext& ctx, float) override
	{
		const auto& tune = CombatTuning::Get().press;
		const double now = Time::GameTime;
		const vec3 threat = ctx.threatPosition;
		const float distance = TacticHelpers::DistanceXZ(body->Position, threat);

		body->SetAimPoint(threat);
		body->UpdateAimAndAttackPosition(false);

		CombatMoveOrder order;
		order.active = true;
		order.stance = NpcStance::Standing;
		order.speed = CombatTuning::Get().movement.strafeSpeed;
		order.facing = NpcFacing::Point;
		order.lookAt = threat;

		if (body->SeesTarget())
		{
			const bool reached = hasStep && TacticHelpers::DistanceXZ(body->Position, step) < 0.5f;

			if (now >= nextStep || reached)
			{
				hasStep = CombatMoves::PickSidestep(body, threat, ctx.threatEye,
					tune.minDistance, std::max(tune.minDistance, distance), tune.stepLength, sign, step);
				nextStep = now + tune.stepInterval;
			}

			body->SetAttackPosition(hasStep ? step : body->Position);
		}
		else
		{
			// Get the line back: straight for where it is believed to be.
			hasStep = false;

			const vec3 away = MathHelper::Normalized(MathHelper::XZ(body->Position - threat));
			body->SetAttackPosition(distance > tune.closeInDistance && glm::length(away) > 0.001f
				? threat + away * tune.closeInDistance
				: body->Position);
		}

		body->SetCombatMoveOrder(order);
	}

private:
	NpcBase* body = nullptr;
	bool hasStep = false;
	vec3 step{};
	double nextStep = 0.0;
	float sign = 1.0f;
};

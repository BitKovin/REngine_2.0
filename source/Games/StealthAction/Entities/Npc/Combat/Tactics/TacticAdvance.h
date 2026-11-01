#pragma once

#include "TacticHelpers.h"
#include "../CombatTacticBase.h"
#include "../../NpcBase.h"
#include "../CombatTuning.h"

// A melee NPC (or a hybrid with its sword out) while the target is out of
// sight: close in on the spot the squad believes it is at, run while far, walk
// and stop a few metres short for a moment (a target that jumps out walks into
// the blade), then push in on the spot itself to clear it. It never just waits
// for the target to come out.
class TacticAdvance : public CombatTacticBase
{
public:
	explicit TacticAdvance(NpcBase* body) : body(body) {}

	const char* Id() const override { return "advance"; }

	float Score(const CombatContext& ctx) const override
	{
		return ctx.meleeMode && ctx.target.Valid() && ctx.Concealed() ? 1.0f : 0.0f;
	}

	void Start(const CombatContext&) override { reachedStandOffAt = 0.0; }

	void Update(const CombatContext& ctx, float) override
	{
		const double now = Time::GameTime;
		const vec3 estimate = ctx.threatPosition;
		const float distance = TacticHelpers::DistanceXZ(body->Position, estimate);

		if (distance <= CombatTuning::Get().melee.standOffDistance + CombatTuning::Get().melee.standOffSlack && reachedStandOffAt == 0.0)
			reachedStandOffAt = now;

		const double pause = CombatTuning::Get().melee.pauseTime + TacticHelpers::StableRandom01(body->Id) * CombatTuning::Get().melee.pauseJitter;
		const bool pushIn = reachedStandOffAt != 0.0 && now - reachedStandOffAt > pause;

		vec3 goal = body->Position;
		if (pushIn)
		{
			goal = vec3(estimate.x, body->Position.y, estimate.z);
		}
		else if (distance > CombatTuning::Get().melee.standOffDistance)
		{
			const vec3 away = glm::normalize(vec3(body->Position.x - estimate.x, 0, body->Position.z - estimate.z));
			goal = vec3(estimate.x, body->Position.y, estimate.z) + away * CombatTuning::Get().melee.standOffDistance;
		}

		body->SetAttackPosition(goal);

		CombatMoveOrder order;
		order.active = true;
		order.stance = NpcStance::Standing;
		order.speed = distance > CombatTuning::Get().melee.advanceWalkDistance ? CombatTuning::Get().movement.runSpeed : CombatTuning::Get().movement.strafeSpeed;

		if (distance < CombatTuning::Get().melee.advanceWalkDistance + 2.0f)
		{
			order.facing = NpcFacing::Point;
			order.lookAt = estimate;
		}
		else
		{
			order.facing = NpcFacing::Movement;
		}

		body->SetCombatMoveOrder(order);
	}

private:

	NpcBase* body = nullptr;
	double reachedStandOffAt = 0.0;
};

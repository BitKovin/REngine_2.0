#pragma once

#include "TacticHelpers.h"
#include "../CombatTacticBase.h"
#include "../../NpcBase.h"
#include "../CombatTuning.h"

// The target is reloading or stunned and a melee NPC (or a hybrid that just
// drew its sword) is close enough: charge. It runs with a slight sideways
// offset instead of straight down the barrel, then the duel takes over.
class TacticRush : public CombatTacticBase
{
public:
	explicit TacticRush(NpcBase* body) : body(body) {}

	const char* Id() const override { return "rush"; }

	float Score(const CombatContext& ctx) const override
	{
		if (!ctx.meleeMode || !ctx.target.Valid() || ctx.Hunting()) return 0.0f;
		if (!ctx.target.isReloading && !ctx.target.isStunned) return 0.0f;
		if (ctx.wantsMeleeSlot && !ctx.hasMeleeSlot) return 0.0f;

		const float distance = TacticHelpers::DistanceXZ(body->Position, ctx.target.position);

		return distance > CombatTuning::Get().melee.rushMinDistance && distance < CombatTuning::Get().melee.rushMaxDistance ? 1.2f : 0.0f;
	}

	void Update(const CombatContext& ctx, float) override
	{
		const vec3 toTarget = glm::normalize(vec3(ctx.target.position.x - body->Position.x, 0, ctx.target.position.z - body->Position.z));
		const vec3 side = glm::cross(toTarget, vec3(0, 1, 0));
		const float sign = TacticHelpers::StableRandom01(body->Id) < 0.5f ? -1.0f : 1.0f;

		body->SetAttackPosition(ctx.target.position + side * (CombatTuning::Get().melee.rushSideOffset * sign));

		CombatMoveOrder order;
		order.active = true;
		order.stance = NpcStance::Standing;
		order.facing = NpcFacing::Movement;
		order.speed = CombatTuning::Get().movement.rushSpeed;

		body->SetCombatMoveOrder(order);
	}

private:

	NpcBase* body = nullptr;
};

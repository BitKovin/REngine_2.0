#pragma once

#include "../CombatTacticBase.h"
#include "../CoverPlanner.h"
#include "../CombatBehaviorBase.h"
#include "../RangedFireControl.h"
#include "../../NpcBase.h"
#include "../CombatTuning.h"

// Sprints to the cover spot the planner picked: a dash between covers, not a
// stand-off, so it doesn't stop to shoot on the way. Low cover is approached
// crouch-walking for the last stretch, so the NPC arrives already ducked. Once
// it stands in the spot the cover-fire tactics take over.
class TacticTakeCover : public CombatTacticBase
{
public:
	explicit TacticTakeCover(NpcBase* body) : body(body) {}

	const char* Id() const override { return "take_cover"; }

	float Score(const CombatContext& ctx) const override
	{
		if (ctx.meleeMode || !ctx.target.Valid() || !ctx.cover || ctx.Lost()) return 0.0f;

		return ctx.cover->HasCover() && !ctx.cover->Arrived() ? 1.0f : 0.0f;
	}

	void Update(const CombatContext& ctx, float) override
	{
		const CoverSpot& spot = ctx.cover->Spot();

		// A hiding target is not where it really is as far as the squad knows.
		if (ctx.Hunting())
			body->SetAimPoint(ctx.threatPosition);

		body->UpdateAimAndAttackPosition(false);
		body->SetAttackPosition(spot.position);

		const float distance = glm::distance(MathHelper::XZ(body->Position), MathHelper::XZ(spot.position));
		const bool sneakIn = spot.type == CoverType::Low && distance < CombatTuning::Get().melee.takeCoverCrouchDistance;

		// Reloading on the way: the weapon is down, same as anywhere else.
		if (ctx.fire->IsReloading())
			body->SetWeaponLowered(true);

		CombatMoveOrder order;
		order.active = true;
		order.stance = sneakIn ? NpcStance::Crouched : NpcStance::Standing;
		order.facing = sneakIn ? NpcFacing::Point : NpcFacing::Movement;
		order.lookAt = ctx.threatPosition;
		order.speed = sneakIn ? CombatTuning::Get().movement.crouchWalkSpeed : CombatTuning::Get().movement.runSpeed;
		order.careful = sneakIn;

		body->SetCombatMoveOrder(order);
	}

private:

	NpcBase* body = nullptr;
};

#pragma once

#include "../CombatTacticBase.h"
#include "../../NpcGuardMelee.h"
#include "../CombatTuning.h"

// Melee duel: pressure, wind-up, bait, block, punish, guard break. Wraps the
// state machine in NpcGuardMelee::RunMeleeAttack; the state machine moves into
// this tactic when slots and group play (M4) rework it. The tactic owns the
// movement order.
class TacticMeleeDuel : public CombatTacticBase
{
public:
	explicit TacticMeleeDuel(NpcGuardMelee* body) : body(body) {}

	const char* Id() const override { return "melee_duel"; }

	// The duel needs a visible target and, against the player, an attack slot.
	float Score(const CombatContext& ctx) const override
	{
		if (!ctx.meleeMode || ctx.Hunting()) return 0.0f;

		return !ctx.wantsMeleeSlot || ctx.hasMeleeSlot ? 1.0f : 0.0f;
	}

	void Update(const CombatContext& ctx, float) override
	{
		if (ctx.target.Valid())
		{
			// Run at the target until the exchange starts, then quick footwork. The
			// default order would walk the whole attack range at strafing speed.
			CombatMoveOrder order = body->MakeDefaultCombatMoveOrder();

			const float distance = glm::distance(MathHelper::XZ(body->Position), MathHelper::XZ(ctx.target.position));
			order.speed = distance > NpcGuardMelee::EngageDistance ? CombatTuning::Get().movement.runSpeed : CombatTuning::Get().movement.meleeFootworkSpeed;

			body->SetCombatMoveOrder(order);
		}

		body->RunMeleeAttack();
	}

private:
	NpcGuardMelee* body = nullptr;
};

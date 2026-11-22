#pragma once

#include "TacticHelpers.h"
#include "../CombatTacticBase.h"
#include "../../NpcBase.h"
#include "../CombatTuning.h"

#include <Physics.h>

// A melee NPC that has no attack slot on the player: waits on a ring around
// him, facing him, spread out from the others by its own fixed angle. Takes a
// slot (and with it the melee duel) as soon as one is free.
class TacticWaitForSlot : public CombatTacticBase
{
public:
	explicit TacticWaitForSlot(NpcBase* body) : body(body) {}

	const char* Id() const override { return "wait_for_slot"; }

	float Score(const CombatContext& ctx) const override
	{
		return ctx.meleeMode && ctx.target.Valid() && !ctx.Hunting() && ctx.wantsMeleeSlot && !ctx.hasMeleeSlot ? 1.0f : 0.0f;
	}

	void Start(const CombatContext&) override { nextRingUpdate = 0.0; }

	void Update(const CombatContext& ctx, float) override
	{
		const double now = Time::GameTime;

		if (now >= nextRingUpdate)
		{
			nextRingUpdate = now + CombatTuning::Get().melee.ringUpdateInterval;
			ringPosition = PickRingPosition(ctx);
		}

		body->SetAttackPosition(ringPosition);

		const float distanceToRing = TacticHelpers::DistanceXZ(body->Position, ringPosition);

		CombatMoveOrder order;
		order.active = true;
		order.stance = NpcStance::Standing;
		order.facing = NpcFacing::Target;
		order.speed = distanceToRing > CombatTuning::Get().melee.ringRunDistance ? CombatTuning::Get().movement.runSpeed : CombatTuning::Get().movement.strafeSpeed;

		body->SetCombatMoveOrder(order);
	}

private:

	NpcBase* body = nullptr;
	vec3 ringPosition{};
	double nextRingUpdate = 0.0;

	vec3 PickRingPosition(const CombatContext& ctx) const
	{
		const float angle = TacticHelpers::StableRandom01(body->Id) * 2.0f * glm::pi<float>();
		const vec3 direction(std::cos(angle), 0.0f, std::sin(angle));
		const vec3 wanted = ctx.target.position + direction * CombatTuning::Get().melee.ringRadius;

		// Keep the ring point only if there is floor under it and the way there is open.
		const auto ground = Physics::LineTrace(wanted + vec3(0, 1.0f, 0), wanted - vec3(0, 3.0f, 0), BodyType::World);
		if (!ground.hasHit) return ringPosition == vec3() ? body->Position : ringPosition;

		const vec3 point = ground.position + vec3(0, body->GetCenterHeight(), 0);

		if (Physics::LineTrace(body->Position, point, BodyType::World).hasHit)
			return ringPosition == vec3() ? body->Position : ringPosition;

		return point;
	}
};

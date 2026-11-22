#pragma once

#include "TacticHelpers.h"
#include "../CombatTacticBase.h"
#include "../RangedFireControl.h"
#include "../../NpcBase.h"
#include "../CombatTuning.h"

// The squad has evidence the target left (or waited long enough): one NPC
// creeps up to the spot where it was believed to hide, a second covers it for
// a moment, everybody else goes straight to searching. With an approach map
// the checker comes in through a lane other than the front one (it runs to
// that lane's stack spot first), and the watcher covers from the front's. Each NPC ends the
// fight on its own (NpcBase::TargetLost), which starts the regular search.
class TacticCheckLastKnown : public CombatTacticBase
{
public:
	explicit TacticCheckLastKnown(NpcBase* body) : body(body) {}

	const char* Id() const override { return "check_last_known"; }

	float Score(const CombatContext& ctx) const override
	{
		return ctx.target.Valid() && ctx.belief && ctx.Lost() && !finished ? 1.3f : 0.0f;
	}

	void Start(const CombatContext&) override
	{
		finished = false;
		arrivedAt = 0.0;
		laneReached = false;
	}

	void Update(const CombatContext& ctx, float) override
	{
		const double now = Time::GameTime;
		const vec3 estimate = ctx.threatPosition;
		const double lostFor = now - ctx.belief->lostSince;

		body->SetAimPoint(estimate);
		body->SetWeaponAimed();
		body->UpdateAimAndAttackPosition(false);

		if (ctx.fire->IsReloading())
			body->SetWeaponLowered(true);

		CombatMoveOrder order;
		order.active = true;
		order.facing = NpcFacing::Point;
		order.lookAt = estimate;
		order.stance = NpcStance::Standing;
		order.speed = CombatTuning::Get().movement.strafeSpeed;

		bool done = lostFor > CombatTuning::Get().search.timeout;

		if (ctx.isChecker && ctx.squad.hasLane && !laneReached)
		{
			// Round to the other way in first, at a run.
			body->SetAttackPosition(ctx.squad.stack);
			laneReached = TacticHelpers::DistanceXZ(body->Position, ctx.squad.stack) < CombatTuning::Get().search.laneReachedDistance;
		}
		else if (ctx.isChecker)
		{
			body->SetAttackPosition(estimate);

			order.stance = NpcStance::Crouched;
			order.speed = CombatTuning::Get().movement.crouchWalkSpeed;
			order.careful = true;

			if (TacticHelpers::DistanceXZ(body->Position, estimate) < CombatTuning::Get().search.checkDistance)
			{
				if (arrivedAt == 0.0) arrivedAt = now;
				done = done || now - arrivedAt > CombatTuning::Get().search.lookAroundTime;
			}
		}
		else if (ctx.isWatcher && ctx.squad.hasLane)
		{
			// Covers the obvious way in from just short of it.
			body->SetAttackPosition(ctx.squad.stack);
			done = done || lostFor > CombatTuning::Get().search.watcherWaitTime;
		}
		else
		{
			// Everybody else holds their ground.
			body->SetAttackPosition(body->Position);

			done = done || lostFor > (ctx.isWatcher ? CombatTuning::Get().search.watcherWaitTime : CombatTuning::Get().search.othersWaitTime);
		}

		body->SetCombatMoveOrder(order);

		if (done)
		{
			finished = true;
			body->TargetLost();
		}
	}

private:

	NpcBase* body = nullptr;
	bool finished = false;
	double arrivedAt = 0.0;
	bool laneReached = false;
};

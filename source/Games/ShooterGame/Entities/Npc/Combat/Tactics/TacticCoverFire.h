#pragma once

#include "../CombatBehaviorBase.h"
#include "../CombatTacticBase.h"
#include "../CoverPlanner.h"
#include "../RangedFireControl.h"
#include "../../NpcBase.h"
#include "../CombatTuning.h"

#include <RandomHelper.h>

// Fighting from a cover spot the NPC stands in: hidden for a while, then
// exposed for a burst, then hidden again.
//
//   Low cover:    crouched behind it while hidden; stands up to shoot.
//   Full cover:   stands behind it while hidden; steps out sideways to shoot.
//
// Reloading, a flinch or heavy suppression send the NPC back (or keep it) in
// hiding. Every NPC peeks on its own rhythm (nobody waits for a turn); when
// nobody in the squad is out shooting, a hidden NPC comes out after
// ReturnFireDelay and even through moderate suppression, so the squad never
// all sits behind cover at once. The subclasses decide when they apply and
// how long the phases last.
class TacticCoverFire : public CombatTacticBase
{
public:
	explicit TacticCoverFire(NpcBase* body) : body(body) {}

	float Score(const CombatContext& ctx) const override
	{
		if (ctx.meleeMode || !ctx.target.Valid() || !ctx.cover) return 0.0f;
		if (!ContactAllowed(ctx)) return 0.0f;
		if (RequiresCover() && !ctx.cover->Arrived()) return 0.0f;

		return Applies(ctx) ? 1.0f : 0.0f;
	}

	void Start(const CombatContext&) override
	{
		exposed = false;
		timer = RandomRange(0.3f, 0.8f);
	}

	float MinCommitTime() const override { return 1.0f; }

	void Update(const CombatContext& ctx, float deltaTime) override
	{
		const bool inCover = ctx.cover->Arrived();
		const CoverSpot spot = inCover ? ctx.cover->Spot() : OpenSpot();
		const Profile profile = GetProfile(ctx);

		vec3 aimPoint;
		const bool aimsAtPoint = AimPoint(ctx, aimPoint);

		if (aimsAtPoint)
		{
			body->SetAimPoint(aimPoint);
			body->SetWeaponAimed();
		}

		body->UpdateAimAndAttackPosition(false);

		const bool reloading = ctx.fire->IsReloading();

		// Somebody has to keep shooting.
		const bool nobodyShooting = ctx.squadExposed == 0;

		const bool suppressed = ctx.suppression > CombatTuning::Get().brain.duckSuppression;
		const bool assaulting = ctx.belief && Time::GameTime < ctx.belief->assaultUntil;
		const bool ducking = ctx.flinched
			|| (suppressed && !assaulting && !(nobodyShooting && ctx.suppression < CombatTuning::Get().coverFight.braveSuppression));

		if (reloading)
			body->SetWeaponLowered(true);

		timer -= deltaTime;

		if (!exposed && nobodyShooting)
			timer = std::min(timer, Patient(ctx) ? CombatTuning::Get().coverFight.patientReturnFireDelay : CombatTuning::Get().coverFight.returnFireDelay);

		// The squad breaches (§8.7): everybody comes out shooting while the stack
		// goes in, unless reloading or just hit.
		if (assaulting && !reloading && !ctx.flinched)
		{
			if (!exposed) timer = 0.0f;
			else timer = std::max(timer, static_cast<float>(ctx.belief->assaultUntil - Time::GameTime));
		}

		if (!inCover)
		{
			// Nothing to hide behind: stand and shoot.
			exposed = !reloading && !ducking;
		}
		else if (exposed && (timer <= 0.0f || reloading || ducking))
		{
			exposed = false;
			timer = RandomRange(profile.hiddenMin, profile.hiddenMax) + (ducking ? CombatTuning::Get().coverFight.duckExtraHideTime : 0.0f);
		}
		else if (!exposed && timer <= 0.0f && !reloading && !ducking)
		{
			exposed = true;
			timer = RandomRange(profile.exposedMin, profile.exposedMax);
		}

		CombatMoveOrder order;
		order.active = true;
		order.facing = NpcFacing::Target;
		order.speed = CombatTuning::Get().movement.strafeSpeed;

		if (spot.type == CoverType::Low)
		{
			order.stance = exposed ? NpcStance::Standing : NpcStance::Crouched;
			if (order.stance == NpcStance::Crouched) order.speed = CombatTuning::Get().movement.crouchWalkSpeed;

			body->SetAttackPosition(spot.position);
		}
		else
		{
			order.stance = NpcStance::Standing;

			body->SetAttackPosition(exposed ? spot.peekPosition : spot.position);
		}

		// Face the believed threat when the target itself is out of sight.
		if (aimsAtPoint)
		{
			order.facing = NpcFacing::Point;
			order.lookAt = aimPoint;
		}

		body->SetCombatMoveOrder(order);

		if (exposed)
		{
			ctx.fire->NoteExposed();
			Shoot(ctx);

			// Peeking without ever seeing the target means the spot is useless now.
			if (RequiresCover() && inCover && !body->SeesTarget())
			{
				blindExposedTime += deltaTime;
				if (blindExposedTime > CombatTuning::Get().coverFight.blindPeekLimit)
				{
					blindExposedTime = 0.0f;
					ctx.cover->Abandon();
				}
			}
			else
			{
				blindExposedTime = 0.0f;
			}
		}
	}

protected:
	using Profile = CombatCoverRhythm;

	virtual Profile GetProfile(const CombatContext& ctx) const = 0;
	virtual bool Applies(const CombatContext& ctx) const = 0;

	// False for tactics that also work out in the open.
	virtual bool RequiresCover() const { return true; }

	// Holding back while the squad's flankers get round: hidden ones come out
	// slower when nobody is shooting.
	virtual bool Patient(const CombatContext&) const { return false; }

	// Which contact levels the tactic works in; by default only while the target is not hiding.
	virtual bool ContactAllowed(const CombatContext& ctx) const { return !ctx.Hunting(); }

	// A point to aim at instead of the target (the squad's estimate); false = aim at the target.
	virtual bool AimPoint(const CombatContext&, vec3&) const { return false; }

	// Extra shooting while exposed. Shots at the target need nothing here: the
	// brain fires whenever the target is in sight (CombatBehaviorBase::FireIfTargetVisible).
	virtual void Shoot(const CombatContext&) {}

	NpcBase* body = nullptr;

private:

	bool exposed = false;
	float timer = 0.0f;
	float blindExposedTime = 0.0f;

	CoverSpot OpenSpot() const
	{
		CoverSpot spot;
		spot.type = CoverType::Full;
		spot.position = body->Position;
		spot.floor = body->Position - vec3(0, body->GetCenterHeight(), 0);
		spot.peekPosition = body->Position;
		return spot;
	}

	static float RandomRange(float min, float max)
	{
		return min + RandomHelper::RandomFloat() * (max - min);
	}
};

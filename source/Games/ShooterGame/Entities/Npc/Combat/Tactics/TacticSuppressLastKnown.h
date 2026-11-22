#pragma once

#include "TacticCoverFire.h"

// A suppressor of a squad whose target ducked out of sight (the plan makes up
// to CombatCoordinator::MaxSuppressors of the NPCs with a line on the spot
// suppressors for a few seconds): keeps shooting at where it disappeared so
// it stays down while the flankers go round and the others push up. Fires
// from cover when it has some, from where it stands when it has not, out most
// of the time. The shots are not aimed at the target: they chip the cover it
// hides behind (the brain shoots the target itself the moment it shows).
class TacticSuppressLastKnown : public TacticCoverFire
{
public:
	using TacticCoverFire::TacticCoverFire;

	const char* Id() const override { return "suppress_last_known"; }

protected:
	Profile GetProfile(const CombatContext&) const override { return CombatTuning::Get().coverFight.suppress; }

	bool Applies(const CombatContext& ctx) const override { return ctx.Concealed() && ctx.isSuppressor; }

	bool RequiresCover() const override { return false; }

	bool ContactAllowed(const CombatContext& ctx) const override { return ctx.Concealed(); }

	bool AimPoint(const CombatContext& ctx, vec3& out) const override
	{
		out = ctx.threatPosition;
		return true;
	}

	// The target itself, when it shows, is shot by the brain; until then the cover gets it.
	void Shoot(const CombatContext& ctx) override
	{
		if (body->SeesTarget()) return;

		if (body->ReadyToFireAtPoint(ctx.threatPosition))
			ctx.fire->TrySuppressShot(ctx, ctx.threatPosition);
	}
};

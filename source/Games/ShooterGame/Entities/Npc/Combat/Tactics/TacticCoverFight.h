#pragma once

#include "TacticCoverFire.h"

// Fighting from cover against a visible target: hide, peek, shoot, hide.
//
// One tactic whether or not the NPC holds an attack token, so a token changing
// hands never interrupts a peek: the token only decides how well the shots aim
// (RangedFireControl). Every NPC that can see the target shoots; it spends most
// of its time out shooting. Behind low cover the hidden part is long enough to
// read as ducking down, not a twitch.
class TacticCoverFight : public TacticCoverFire
{
public:
	using TacticCoverFire::TacticCoverFire;

	const char* Id() const override { return "cover_fight"; }

protected:
	// While flankers are out (before the breach) the base of fire is patient:
	// longer hidden, shorter out. It keeps the target busy without forcing the
	// issue; the breach brings everyone out at once.
	Profile GetProfile(const CombatContext& ctx) const override
	{
		const auto& tune = CombatTuning::Get().coverFight;

		if (Patient(ctx)) return tune.patient;

		if (ctx.cover && ctx.cover->Arrived() && ctx.cover->Spot().type == CoverType::Low)
			return tune.lowCover;

		return tune.normal;
	}

	bool Patient(const CombatContext& ctx) const override
	{
		return ctx.belief && ctx.belief->flankUnderWay && !ctx.isFlanker;
	}

	bool Applies(const CombatContext&) const override { return true; }
};

#pragma once

#include "CombatBehaviorBase.h"

class NpcGuardMelee;
class NpcGuardHybrid;

// Shared by the archetypes: the tactic sets they pick from.
class ArchetypeCombatBehavior : public CombatBehaviorBase
{
protected:
	using CombatBehaviorBase::CombatBehaviorBase;

	// Cover, peeking, suppression, holding angles, flanking, kiting a target that
	// can't shoot. Strafing is the fallback.
	void AddGunTactics(NpcBase* body);

	// Dueling, waiting for an attack slot, rushing an opening, advancing on a hiding target.
	void AddMeleeTactics(NpcGuardMelee* body);

	// Checking the spot when the target is lost; every archetype does it.
	void AddSearchTactics(NpcBase* body);
};

// Gun fighters (guards, bandits).
class RangedCombatBehavior : public ArchetypeCombatBehavior
{
public:
	explicit RangedCombatBehavior(NpcBase* owner);

	bool CanFlinch() const override { return true; }
};

// Sword fighters.
class MeleeCombatBehavior : public ArchetypeCombatBehavior
{
public:
	explicit MeleeCombatBehavior(NpcGuardMelee* owner);

	bool IsMeleeMode() const override { return true; }
};

// Gun and sword: the weapon mode follows the NPC's distance rules, and the
// tactics of the active mode run.
class HybridCombatBehavior : public ArchetypeCombatBehavior
{
public:
	explicit HybridCombatBehavior(NpcGuardHybrid* owner);

	bool IsMeleeMode() const override;

protected:
	void PreSelect() override;

private:

	NpcGuardHybrid* hybrid = nullptr;
};

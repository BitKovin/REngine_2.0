#include "CombatBehaviors.h"

#include "CombatCoordinator.h"

#include <AiPerception/Observer.h>
#include "../NpcGuardHybrid.h"
#include "Tactics/TacticAdvance.h"
#include "Tactics/TacticCheckLastKnown.h"
#include "Tactics/TacticCoverFight.h"
#include "Tactics/TacticHoldAngle.h"
#include "Tactics/TacticKite.h"
#include "Tactics/TacticMeleeDuel.h"
#include "Tactics/TacticPress.h"
#include "Tactics/TacticRegainSight.h"
#include "Tactics/TacticRush.h"
#include "Tactics/TacticStrafe.h"
#include "Tactics/TacticSuppressLastKnown.h"
#include "Tactics/TacticTakeCover.h"
#include "Tactics/TacticWaitForSlot.h"
#include "CombatTuning.h"

void ArchetypeCombatBehavior::AddGunTactics(NpcBase* body)
{
	AddTactic(std::make_unique<TacticStrafe>(body), CombatTuning::Get().brain.strafeWeight);
	AddTactic(std::make_unique<TacticTakeCover>(body));
	AddTactic(std::make_unique<TacticCoverFight>(body));
	AddTactic(std::make_unique<TacticHoldAngle>(body));
	AddTactic(std::make_unique<TacticSuppressLastKnown>(body), CombatTuning::Get().brain.suppressLastKnownWeight);
	AddTactic(std::make_unique<TacticRegainSight>(body));
	AddTactic(std::make_unique<TacticKite>(body));
	AddTactic(std::make_unique<TacticPress>(body));
}

void ArchetypeCombatBehavior::AddMeleeTactics(NpcGuardMelee* body)
{
	AddTactic(std::make_unique<TacticMeleeDuel>(body));
	AddTactic(std::make_unique<TacticWaitForSlot>(body));
	AddTactic(std::make_unique<TacticRush>(body));
	AddTactic(std::make_unique<TacticAdvance>(body));
}

void ArchetypeCombatBehavior::AddSearchTactics(NpcBase* body)
{
	AddTactic(std::make_unique<TacticCheckLastKnown>(body));
}

RangedCombatBehavior::RangedCombatBehavior(NpcBase* owner) : ArchetypeCombatBehavior(owner)
{
	AddGunTactics(owner);
	AddSearchTactics(owner);
}

MeleeCombatBehavior::MeleeCombatBehavior(NpcGuardMelee* owner) : ArchetypeCombatBehavior(owner)
{
	AddMeleeTactics(owner);
	AddSearchTactics(owner);
}

HybridCombatBehavior::HybridCombatBehavior(NpcGuardHybrid* owner) : ArchetypeCombatBehavior(owner), hybrid(owner)
{
	AddGunTactics(owner);
	AddMeleeTactics(owner);
	AddSearchTactics(owner);
}

bool HybridCombatBehavior::IsMeleeMode() const
{
	return hybrid->GetCombatMode() == CombatMode::Melee;
}

void HybridCombatBehavior::PreSelect()
{
	const CombatTargetView& target = GetTargetView();

	if (!target.Valid() || hybrid->IsRangedWeaponDisabled())
	{
		hybrid->SetForcedMode(NpcGuardHybrid::ForcedMode::None);
		hybrid->UpdateCombatMode();
		return;
	}

	const float distance = glm::distance(hybrid->Position, target.position);
	const bool swordOut = IsMeleeMode();
	bool sword = false;

	if (target.hasGun)
	{
		const bool safe = target.isReloading || target.isStunned || !target.isAimingAtMe;
		sword = swordOut ? distance < CombatTuning::Get().hybrid.swordLeaveDistance : (distance < CombatTuning::Get().hybrid.swordDistance && safe);
	}
	else
	{
		sword = distance < (swordOut ? CombatTuning::Get().hybrid.swordLeaveDistance : CombatTuning::Get().hybrid.corneredDistance);
	}

	hybrid->SetForcedMode(sword ? NpcGuardHybrid::ForcedMode::Melee : NpcGuardHybrid::ForcedMode::Ranged);

	hybrid->UpdateCombatMode();
}

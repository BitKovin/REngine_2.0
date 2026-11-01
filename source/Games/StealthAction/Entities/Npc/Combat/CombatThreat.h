#pragma once

#include <string>

class NpcBase;

// How dangerous a known target is to one NPC (COMBAT_AI_DESIGN.md §4.2):
// danger x proximity x attention. NpcBase uses it to pick the primary target
// among attackers, so a guard fighting a bandit turns to the player once the
// player starts shooting at it.
namespace CombatThreat
{
	// A challenger must beat the current primary target by this factor.
	constexpr float SwitchFactor = 1.3f;

	// 0 when the target no longer exists.
	float Score(const NpcBase* viewer, const std::string& targetId);
}

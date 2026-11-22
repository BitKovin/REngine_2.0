#pragma once

#include <glm.h>

class Entity;
class NpcBase;

// Snapshot of what a combat brain needs to know about one of its targets.
// The target can be the player or another NPC; the brain never assumes which.
// Refreshed by the brain every tick (see COMBAT_AI_DESIGN.md §4).
struct CombatTargetView
{
	Entity* entity = nullptr;
	bool isPlayer = false;

	vec3 position{};
	vec3 eyePosition{};
	vec3 velocity{};

	bool isMeleeArmed = false;
	bool hasGun = false; // can shoot back; a target without one is only dangerous up close
	bool isReloading = false;
	bool isBlocking = false;
	bool isParrying = false;
	bool isCrouched = false;
	bool isAimingAtMe = false;
	bool isStunned = false;

	float health01 = 1.0f;

	bool Valid() const { return entity != nullptr; }

	// Fills the view for `target` as seen by `viewer`. Returns false (and leaves
	// the view invalid) when the target no longer exists.
	static bool Fill(CombatTargetView& out, const NpcBase* viewer, Entity* target);
};

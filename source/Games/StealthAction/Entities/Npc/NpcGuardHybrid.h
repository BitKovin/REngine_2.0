#pragma once

#include "NpcGuardMelee.h"
#include "Animators/NpcHybridAnimator.h"

// ─────────────────────────────────────────────────────────────────────────────
//  NpcGuardHybrid
//
//  Seamlessly switches between melee and ranged combat based on target distance:
//
//   distance < meleeThreshold (4 m)   → switch to Melee
//   distance > rangedThreshold (10 m) → switch back to Ranged
//
//  The hysteresis gap (4–10 m) prevents rapid mode flickering.
//
//  Ranged weapon can be disabled at runtime (e.g. disarmed by the player).
//  When disabled the NPC is locked to melee until the weapon is restored.
//
//  Two weapon meshes are managed:
//    weaponMesh      (inherited from NpcBase) – the ranged weapon
//    meleeWeaponMesh (added here)             – the melee weapon
//
//  Both meshes are shown/hidden automatically on mode transitions.
// ─────────────────────────────────────────────────────────────────────────────

class NpcGuardHybrid : public NpcGuardMelee
{
public:

	// ── Tuning ───────────────────────────────────────────────────────────────
	float meleeThreshold  = 4.0f;   // switch to melee when closer than this
	float rangedThreshold = 10.0f;  // switch to ranged when farther than this

	float rangedAttackRange    = 20.0f;
	float rangedDesiredRange   = 10.0f;

	// ── Construction / lifecycle ──────────────────────────────────────────────
	NpcGuardHybrid();
	void Start() override;

	// ── Disarm interface ──────────────────────────────────────────────────────
	// When the ranged weapon is disabled the NPC is permanently locked to
	// melee until EnableRangedWeapon() is called.
	void DisableRangedWeapon();
	void EnableRangedWeapon();
	bool IsRangedWeaponDisabled() const { return rangedDisabled; }

	// ── Entity overrides ──────────────────────────────────────────────────────
	vec3 FindAttackLocation()                          override;
	void UpdateAnimations(bool forceFullUpdate = false) override;

	void UpdateWeaponMesh() override;

	// ── Mode management (driven by the hybrid combat behavior) ────────────────
	// Evaluates the target distance and flips the weapon mode with hysteresis.
	void UpdateCombatMode();
	CombatMode GetCombatMode() const { return currentMode; }

	// The combat behavior can override the distance rule: sword out for a charge,
	// a corner or a rescue; gun out against a target that can't shoot back.
	enum class ForcedMode { None, Melee, Ranged };
	void SetForcedMode(ForcedMode mode) { forcedMode = mode; }

protected:

	StaticMesh* meleeWeaponMesh = nullptr; // melee weapon (sword, etc.)
	// weaponMesh (from NpcBase) is used for the ranged weapon

	bool       rangedDisabled = false;
	ForcedMode forcedMode     = ForcedMode::None;
	CombatMode currentMode    = CombatMode::Ranged;

	void SwitchToMode(CombatMode mode);
	void UpdateWeaponMeshVisibility();

	void LoadAssets() override;

	std::shared_ptr<CombatBehaviorBase> CreateCombatBehavior() override;

	// ── Typed animator accessor ───────────────────────────────────────────────
	inline NpcHybridAnimator* GetHybridAnimator() const
	{
		return static_cast<NpcHybridAnimator*>(animator.get());
	}
};

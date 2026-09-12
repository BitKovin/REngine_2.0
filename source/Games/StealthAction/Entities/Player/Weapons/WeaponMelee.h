#pragma once

#include "WeaponBase.h"
#include "../Player.hpp"

#include <ParticleSystems/particle_system_meleeTrail.hpp>
#include <Entities/Npc/NpcGuardMelee.h>
#include "../../Enemy/IEnemy.h"

// Tunables for the default attack/block/parry behaviour below. A brand new
// simple melee weapon (crowbar, knife, whatever) can just fill these in and
// use WeaponMelee as-is with no overrides at all. Richer weapons (see
// weapon_twinsword) override the StartAttack/PerformAttack/StartBlock/
// EndBlock virtuals instead and can ignore whichever of these don't apply.
struct MeleeParams
{
	std::string modelPath;
	std::string modelPathTp;
	std::string texturesLocation;

	float damage = 25.0f;

	std::string attackSoundEvent = "event:/Weapons/knife/knife_attack";
	std::string hitSoundEvent = "";
	std::string parrySoundEvent = "event:/Weapons/cane/cane_parry";

	std::string attackAnim = "attack";
	std::string blockStartAnim = "block_start";
	std::string blockEndAnim = "block_end";
	std::string counterAnim = "attack_counter";
	std::string drawAnim = "draw";

	float attackDelayTime = 0.4f;    // min time between swings
	float attackWindowStart = 0.15f; // delay after StartAttack before the hit-trace opens
	float attackWindowEnd = 0.35f;   // delay after StartAttack before the hit-trace window closes
	float reAttackDelayTime = 0.55f; // keeps CanChangeSlot() false (still mid-swing/combo)
	float blockStartDelayTime = 0.2f;
	float parrySpamWindowTime = 0.4f; // can't re-parry for this long after releasing block
	float counterWindowTime = 1.1f;   // how long a successful parry's counter opportunity stays open

	float attackRange = 1.3f;
	float attackRadius = 0.35f;
};

// Shared skeleton for melee weapons: attack timing, block/parry/counter-attack
// windows, and single- or dual-wielded viewmodel handling. Modeled on
// weapon_twinsword, the fullest-featured melee weapon - a new weapon can
// derive from this and just set `params` (see MeleeParams) to get a fully
// working attack/block/parry/counter loop for free. Set DualWield = true in
// the ctor for a twinsword-style two-blade weapon (loads/animates
// viewmodel_l/arms_l too); leave it false for a single blade like a sword.
class WeaponMelee : public Weapon
{
public:

	MeleeParams params;

	bool DualWield = false;

	// Right-hand (or only) blade
	SkeletalMesh* viewmodel_r = nullptr;
	SkeletalMesh* arms_r = nullptr;

	// Left-hand blade - only loaded/used when DualWield is true
	SkeletalMesh* viewmodel_l = nullptr;
	SkeletalMesh* arms_l = nullptr;

	vec3 weaponOffset = vec3(0.0f);

	Delay attackDelay;
	Delay pendingAttackStartDelay;
	Delay pendingAttackEndDelay;
	Delay reAttackDelay;
	Delay parryWindow;
	Delay parrySpamWindow;
	Delay blockStartDelay;
	Delay counterWindow;    // how long a successful parry's counter opportunity stays open
	Delay blockingWindow;   // available for richer parry-window logic, see weapon_twinsword
	Delay forceBlockInput;  // available for richer parry-window logic, see weapon_twinsword
	Delay startTrailDelay;  // for UpdateTrail() overrides: delay between StartAttack and the blade trail actually starting

	bool pendingAttack = false;
	bool isBlocking = false;
	bool soundToggle = false;
	bool counterAvailable = false; // set true after a successful parry
	bool pendingCounterAttack = false;
	bool pendingTrailRight = false; // for UpdateTrail() overrides on a DualWield weapon
	bool pendingTrailLeft = false;

	SoundPlayer* fireSoundPlayer = nullptr;
	SoundPlayer* fireSoundPlayer2 = nullptr;
	SoundPlayer* hitSoundPlayer = nullptr;

	WeaponMelee();

	WeaponRole GetRole() const override { return WeaponRole::Melee; }
	bool CanChangeSlot() override { return !reAttackDelay.Wait(); }

	void Start() override;
	void LoadAssets() override;
	void Update() override;
	void AsyncUpdate() override;
	void LateUpdate() override;

	WeaponSlotData GetDefaultData() override
	{
		WeaponSlotData data;
		data.className = "weapon_melee";
		return data;
	}

	// Plays `anim` on viewmodel_r, and on viewmodel_l too if DualWield.
	void PlayBoth(const std::string& anim, bool loop, float blend);

	// Damage multiplier applied in the default PerformAttack() - the default
	// gives a bonus for landing the counter-attack right after a parry, like
	// weapon_twinsword's 3x. Override to add more conditions (power-ups,
	// combo tiers, etc).
	virtual float GetDamageMultiplier() const;

	virtual void StartAttack();
	virtual void PerformAttack();
	virtual void StartBlock();
	virtual void EndBlock();
	void OnParried() override;
	virtual void WarnAboutAttack();

	// Called from LateUpdate() every frame - default is a no-op since blade
	// trail bone names differ per model. Override to spawn/update/stop a
	// particle_system_meleeTrail the way weapon_sword/weapon_twinsword do.
	virtual void UpdateTrail() {}

};

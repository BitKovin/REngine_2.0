#pragma once

#include "WeaponBase.h"
#include "../Player.hpp"

#include <Animation.h>
#include "Animators/WeaponAnimator.h"

#include <string>

#include <Particle/ParticleSystem.hpp>

// Multiplier applied to every time-based value that drives the weapon smoke
// trail's lifetime (emitter recycle threshold, stop-emitting delay, forced
// fade-out point on StopTrail, and destroy delay). 1.0f = default duration;
// >1.0f makes the smoke last longer, <1.0f makes it fade/clean up sooner.
// NOTE: keep this in sync with WEAPONSMOKE_DURATION_FACTOR in WeaponSmoke.cpp -
// that copy drives the particle/emitter's own internal timeline, and the two
// need to agree or the firearm's recycle/stop logic will drift out of step
// with the actual smoke fade curve.
constexpr float WEAPONSMOKE_DURATION_FACTOR = 0.4f;

struct FirearmParams 
{
	WeaponAmmoType ammoType = WeaponAmmoType::None;
	std::string modelPath = "";
	std::string texturesLocation = "";
	std::string modelPathTp = "";
	std::string texturesLocationTp = "";
	std::string fireSoundEvent = "event:/Weapons/pistol/pistol_fire";
	std::string fireAnimation = "fire";
	std::string drawAnimation = "draw";
	std::string boneMuzzle = "weapon_fire_point";
	std::string bulletClass = "bullet";
	float fireVolume = 0.5f;
	bool fireSoundIs2D = true;
	bool useOneshotSound = false;
	float pitchModifier = 1.0f;
	float baseSpread = 0.1f;
	float spreadIncreasePerShot = 0.1f;
	float spreadDecreaseSpeed = 2.0f;
	float maxActiveSpread = 0.5f;
	float velocitySpreadDivisor = 57.0f;
	float attackDelayTime = 0.3f;
	float switchDelayTime = 0.35f;
	float switchDelayOnAttack = 0.2f;
	vec3 weaponOffset = vec3(0.0f, 0.0f, 0.0f);
	float bulletSpeed = 200.0f;
	float bulletDamage = 20.0f;
	int bulletsPerShot = 1;
	float range = 80.0f;
	float muzzleMix = 0.6f;
	float muzzleForwardOffset = 0.1f;
	float fireAnimInterpInTime = 0.08f;
	bool hasActiveSpread = true;
	bool notifyNpcs = true;
	float npcNotifyRadius = 20.0f;
	bool activateViolenceCrime = false;
	float violenceCrimeDelay = 0.3f;
	bool hasRecoilModelOffset = false;
	float recoilModelTarget = 0.0f;
	float recoilModelInterpIn = 2.0f;
	float recoilModelInterpOut = 7.0f;
	bool hasRandomRecoilStrength = false;
	CameraShake recoilShake = CameraShake(
		0.13f, 0.5f,
		vec3(0.0f, 0.0f, -0.1f),
		vec3(0.0f, 0.0f, 3.4f),
		vec3(-3.0f, 0.15f, 0.0f),
		vec3(-1.5f, 18.8f, 0.0f),
		0.5f,
		CameraShake::ShakeType::SingleWave
	);
	bool lateUpdateWhenPaused = true;
	std::string spreadType = "random";
	float gridSpreadSize = 4.0f;
	float gridStep = 2.0f;
	float gridMaxLength = 4.7f;

	std::string debuffOnHit = "";
	float debuffStacksOnHit = 0.0f;

	// ── Reloading ────────────────────────────────────────────────────────────
	// How many rounds this weapon's magazine holds. Reloading tops
	// Data.magazineAmmo up to this value, clamped to however much ammo the
	// player actually has in their shared pool.
	int magazineSize = 8;
	std::string reloadAnimation = "reload";

};

class WeaponFirearm : public Weapon {
public:
	FirearmParams params;

	// Right-hand
	SkeletalMesh* viewmodel = nullptr;
	SkeletalMesh* arms = nullptr;

	// Left-hand (always loaded)
	SkeletalMesh* viewmodelLeft = nullptr;
	SkeletalMesh* armsLeft = nullptr;

	Delay attackDelay;
	SoundPlayer* fireSoundPlayer = nullptr;
	float activeSpread = 0.0f;
	float recoilModelOffset = 0.0f;

	SkeletalMesh* thirdPersonModel = nullptr;
	float weaponAim = 0.0f;
	float oldWeaponAim = 0.0f;
	AnimationPose lastAppliedPose;

	std::unique_ptr<WeaponAnimator> thirdPersonAnimator;

	bool fireLeftNext = false;

	// Runtime akimbo state
	bool akimbo = false;
	bool akimboPrev = false;
	bool alternateFire = true;

	// ── Reloading ────────────────────────────────────────────────────────────
	// Rounds currently chambered/loaded in THIS weapon instance, independent of
	// (but drawn from) the player's shared ammo pool for params.ammoType. This
	// lives directly on Data (Data.magazineAmmo) rather than a separate field,
	// because Data is exactly the record Player persists per weapon instance
	// (Player::ownedWeapons) - a separate field would need manual syncing and
	// could drift out of sync with what actually gets saved. See SetData().
	bool reloading = false;

	WeaponFirearm(const FirearmParams& initialParams = FirearmParams());

	void Start() override;
	void LoadAssets() override;
	void Update() override;
	virtual void PerformAttack();
	virtual void FireSingleBullet(const vec3& startLoc, const vec4& gridOffset = vec4(0));
	void NotifyNpcs();
	void AsyncUpdate() override;
	void LateUpdate() override;
	WeaponSlotData GetDefaultData() override;

	// Called whenever this weapon is (re)equipped, with either fresh pickup data
	// or a previously-owned instance's persisted data (see Player::ownedWeapons).
	// Resolves Data.magazineAmmo: tops it up fresh if this is a never-before-seen
	// instance (magazineAmmo < 0), otherwise restores the persisted count,
	// re-clamped in case the shared pool has dropped since it was last equipped.
	void SetData(WeaponSlotData data) override;

	// Whether StartReload() may currently succeed (not already reloading, magazine
	// not already as full as the pool allows, pool has ammo, offhand weapon isn't
	// currently mid-action, player is free to act).
	virtual bool CanReload();

	// Begins the reload: plays the reload animation and marks the weapon as
	// reloading. Firing is blocked until the reload animation reports ~0.2s
	// remaining, at which point Data.magazineAmmo is topped up and reloading clears.
	virtual void StartReload();

	bool SupressOffhandWeapon() override
	{
		return reloading;
	}

	void Destroy() override;

	virtual AnimationPose ApplyWeaponAnimation(AnimationPose thirdPersonPose);

	// Runtime akimbo functions
	void SetAkimbo(bool enabled);

	void Serialize(json& target);
	void Deserialize(json& source) override;

	virtual bool IsInUltimateAkimbo();

protected:

	void SnapTrailPositions();

	// float (not bool): HideWeapon can now be a continuous 0..1 blend for
	// two-handed weapons mid-handoff with an offhand weapon, and this needs to
	// preserve that, not collapse it to true/false (see AsyncUpdate()).
	float lastFrameHide = 0.0f;

	void StopTrail(ParticleSystem* trail);

	float akimboDistanceProgress = 0.0f;

	ParticleSystem* smokeTrail = nullptr;
	Delay stopEmittingSmokeDelay;

	ParticleSystem* smokeTrailL = nullptr;
	Delay stopEmittingSmokeDelayL;

};

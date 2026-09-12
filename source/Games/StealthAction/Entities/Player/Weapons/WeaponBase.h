#pragma once

#include <Entity.h>
#include <json.hpp>
#include <Helpers/JsonHelper.hpp>
#include <Animation.h>
#include <algorithm>

enum class WeaponAmmoType : uint8_t
{
	None = 0,
	PistolBullets,
	ShotgunShells,
	CannonBullets
};

// What role a weapon plays in the currentWeapon role-switching system.
// Exactly one of these (or None) is ever "current" at a time - see
// Player::currentWeaponRole.
enum class WeaponRole : uint8_t
{
	None = 0,
	Firearm,
	Melee,
	Tool
};

struct WeaponSlotData
{
	string className = "";

	int startAmmo = 8;
	WeaponAmmoType AmmoType = WeaponAmmoType::None;

	std::string inventoryUUID = ""; // For inventory system tracking

	auto operator<=>(const WeaponSlotData&) const = default;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponSlotData, className, AmmoType)
};


class Player;

class Weapon : public Entity
{

protected:

	Delay SwitchDelay;

public:

	uint8_t weaponHandlingType = 0; //0 default with left hand holding, 1 for left hand resting, 2 for 2 separate hands holding 2 copies of weapon

	float HideWeapon = 0;

	WeaponSlotData Data;

	static inline std::string ArmsModelPath = "GameData/models/player/arms_a.glb";

	std::string thirdPersonModelPath = "GameData/models/player/weapons/pistol/pistol_tp.glb";

	bool Illegal = false;

	float bobScale = 1.0f;

	Player* owner = nullptr;

	float Spread = 0.0f;

	virtual bool CanAttack();

	bool ForceFirstPerson = false;

	bool Parrying = false;
	bool Blocking = false;

	// ── Procedural draw / hidden-carry pose ────────────────────────────────
	// When SkipDrawAnimation is true (the default), the weapon's viewmodel
	// animation is snapped straight to the end of its draw clip on load (see
	// LoadAssets()) instead of playing it - no canned draw ANIMATION plays.
	// The physical "raising the weapon" motion is instead done procedurally
	// here: DrawProgress (0 = fully hidden pose, 1 = fully drawn/held pose)
	// is ramped by each weapon's own Update() (see UpdateDrawProgress), and
	// Player::UpdateWeapon() blends the weapon's world transform between
	// HiddenPose*** and its normal held pose accordingly, rotating around
	// HiddenPoseRotationPoint exactly like the existing run-pose
	// (weaponRunRotation / runRotatePoint) trick.
	//
	// For firearms, DrawProgress doubles as "readied" - reaching 1.0 is what
	// allows firing (holding attack2 raises it, releasing lowers it). For
	// melee/tools, DrawProgress just tracks presented-vs-hidden and is
	// driven by Player's manual-hide / idle-hide state instead.
	//
	// DrawTime / HideTime are in seconds and freely tunable per weapon.
	bool SkipDrawAnimation = true;
	float DrawTime = 0.2f;
	float HideTime = 0.4f;
	float DrawProgress = 0.0f;

	vec3 HiddenPosePosition = vec3(0.0f, -0.02f, 0.04f);       // extra offset applied at DrawProgress = 0
	vec3 HiddenPoseRotation = vec3(45.0f, 12.0f, -6.0f);      // rotation (deg) applied at DrawProgress = 0
	vec3 HiddenPoseRotationPoint = vec3(-0.05f, -0.12f, 0.4f); // pivot for HiddenPoseRotation, weapon-local

	// Ramps DrawProgress toward 1 (wantDrawn) or 0, at 1/DrawTime or
	// 1/HideTime per second respectively. Call this once per Update() from
	// whichever code decides wantDrawn (WeaponFirearm computes it from
	// attack2; WeaponMelee/WeaponTool from WantsPresented() below).
	void UpdateDrawProgress(bool wantDrawn);

	// ── Auto-hide after use ─────────────────────────────────────────────────
	// The weapon stays presented (WantsPresented() == true) for
	// AutoHideWaitTime seconds after its last use, then is allowed to lower
	// itself back to the hidden pose. The countdown only ticks down while
	// CanChangeSlot() is true ("switch is allowed" - i.e. not mid-attack),
	// so pressing hide mid-swing just means "hide as soon as this attack
	// ends" rather than interrupting it. There's no separate "manually
	// hidden" flag - RequestHide() just collapses the same countdown.
	//
	// This applies uniformly to every weapon type (a raised pistol, a sword
	// held ready, a tool - nobody should be walking around with their fists
	// up or a hammer cocked back at all times, it reads as unnatural). A
	// specific weapon that genuinely should stay presented indefinitely once
	// drawn can just set AutoHideWaitTime very high in its ctor instead of
	// needing a separate on/off flag - keeps this one mechanism instead of two.
	//
	// Defaults to 0 (not presented) - WeaponMelee/WeaponTool set this to
	// AutoHideWaitTime in their ctor so they're immediately visible the
	// moment they're equipped (no "hold to ready" input for those types).
	// WeaponFirearm leaves this at 0: it has no business being presented
	// until attack2 is actually held or it fires.
	float AutoHideWaitTime = 3.0f;
	float autoHideTimer = 0.0f;

	// Call once per Update(). Only decrements while CanChangeSlot() is true.
	void UpdateAutoHideTimer()
	{
		if (CanChangeSlot())
			autoHideTimer -= Time::DeltaTimeF;
	}

	// Call when the weapon performs its use action (fires / swings /
	// triggers) - resets the auto-hide countdown back to AutoHideWaitTime.
	void NotifyUsed() { autoHideTimer = AutoHideWaitTime; }

	// Manual hide: collapses the countdown to almost zero instead of
	// forcing DrawProgress down directly, so if this lands mid-attack it
	// naturally waits for CanChangeSlot() before it actually starts hiding.
	void RequestHide() { autoHideTimer = std::min(autoHideTimer, 0.01f); }

	bool WantsPresented() const { return autoHideTimer > 0.0f; }

	virtual void OnParried(){}
	virtual void OnBlocked() {}

	// What kind of weapon this is for the currentWeapon role-switching system.
	// Firearms report Firearm via WeaponFirearm's override; everything else
	// defaults to Melee unless it overrides this (see weapon_cane -> Tool).
	virtual WeaponRole GetRole() const
	{
		return WeaponRole::Melee;
	}

	// Back-compat helpers for any external/NPC code that asks about a
	// weapon's category by name instead of via GetRole().
	bool IsMelee()   { return GetRole() == WeaponRole::Melee; }
	bool IsFirearm() { return GetRole() == WeaponRole::Firearm; }
	bool IsTool()    { return GetRole() == WeaponRole::Tool; }

	virtual void SetData(WeaponSlotData data)
	{
		Data = data;
	}

	// Gates BOTH: (a) whether Player is allowed to switch away from this
	// weapon, and (b) whether Player is allowed to auto-return from this
	// weapon back to the previous one when it's a Tool. Individual weapons
	// override this to stay "busy" mid-swing / mid-reload / mid-throw.
	virtual bool CanChangeSlot()
	{
		return SwitchDelay.Wait() == false;
	}

	virtual WeaponSlotData GetDefaultData()
	{
		return WeaponSlotData();
	}

	WeaponAmmoType GetAmmoType()
	{
		return Data.AmmoType;
	}

	virtual AnimationPose ApplyWeaponAnimation(AnimationPose thirdPersonPose)
	{
		return thirdPersonPose;
	}

	void LoadAssets() override;


private:

};


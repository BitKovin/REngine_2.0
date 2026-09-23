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

struct WeaponSlotData
{
	string className = "";
	int slot = 0;
	int priority = 0;

	int startAmmo = 8;
	WeaponAmmoType AmmoType = WeaponAmmoType::None;

	bool offhand = false;

	// Stable per-instance ID for this specific owned weapon. Used both by the
	// Inventory system (matches InventoryItem::uid - main and offhand share one
	// ID there, since they belong to the same inventory item) and, together with
	// `offhand` as a composite key, by Player::ownedWeapons, the mode-agnostic
	// registry of every weapon instance the player currently holds.
	std::string inventoryUUID = "";

	// Rounds currently loaded in this specific weapon instance's magazine.
	// -1 = never persisted yet (a fresh pickup: the weapon should top itself up
	// from the shared ammo pool the first time it's equipped, see
	// WeaponFirearm::SetData). Kept in sync with the live weapon's magazine and
	// written back to Player::ownedWeapons whenever the weapon is unequipped, so
	// it survives switching away and - later - dropping/picking the weapon back up.
	int magazineAmmo = -1;

	auto operator<=>(const WeaponSlotData&) const = default;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponSlotData, className, slot, priority, offhand, AmmoType, startAmmo, inventoryUUID, magazineAmmo)
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

	bool SupportsOffhandWeapon = true;

	// ── Offhand / two-handed left-hand ownership ────────────────────────────────
	//
	// UsesLeftHand() means two different things depending on which slot the weapon
	// is currently occupying (Player::UpdateWeapon() is what interprets it):
	//  - As the MAIN weapon: a general, mostly-static property — true means this
	//    weapon needs both hands to hold/operate normally (e.g. an SMG/rifle), so
	//    it does NOT want to give up its left hand to an offhand item by default.
	//    False (the default) means the weapon is one-handed and never needed the
	//    left hand in the first place (pistols, swords, ...) — those weapons keep
	//    their existing behaviour untouched (left hand always free whenever any
	//    offhand item is equipped).
	//  - As the OFFHAND weapon: a dynamic, per-frame property — true means it is
	//    currently mid-action (e.g. swinging), false while idle/holstered.
	//
	// Only when BOTH the main weapon and the offhand weapon report true at the same
	// time does the main weapon smoothly hide its left hand and the offhand weapon
	// smoothly draw itself; see LeftHandBlend / UpdateLeftHandBlend below.
	virtual bool UsesLeftHand()
	{
		return false;
	}

	// True while this weapon's current state forbids using an offhand weapon at all
	// right now (e.g. mid-reload). While true, the offhand weapon should refuse to
	// start any new action of its own (see WeaponOffhand::IsSuppressedByMainWeapon).
	// This is distinct from SupportsOffhandWeapon, which is a permanent per-weapon
	// "never allow offhand" flag — SupressOffhandWeapon() is meant to be transient.
	virtual bool SupressOffhandWeapon()
	{
		return false;
	}

	// Smoothed 0..1 blend driven by Player::UpdateWeapon() once per frame:
	//  - On a two-handed MAIN weapon: 1 = its own left hand is hidden (grip
	//    released) to make room for the offhand weapon, 0 = both hands are on
	//    this weapon normally.
	//  - On an OFFHAND weapon: 1 = fully drawn/visible, 0 = hidden/holstered.
	float LeftHandBlend = 0.0f;

	// Hiding is slightly faster than drawing, per design: 0.15s to hide, 0.2s to
	// draw. Both the main weapon (hiding its hand) and the offhand weapon (drawing
	// itself) use these same two timings.
	static constexpr float LeftHandHideTime = 0.05f;
	static constexpr float LeftHandDrawTime = 0.1f;

	// Advances LeftHandBlend toward 1 (show/engage) or 0 (hide), at the rate above.
	// Call once per frame.
	void UpdateLeftHandBlend(bool show, float deltaTime)
	{
		float target = show ? 1.0f : 0.0f;
		float rate = (target > LeftHandBlend) ? (1.0f / LeftHandDrawTime) : (1.0f / LeftHandHideTime);

		if (LeftHandBlend < target)
			LeftHandBlend = std::min(target, LeftHandBlend + rate * deltaTime);
		else if (LeftHandBlend > target)
			LeftHandBlend = std::max(target, LeftHandBlend - rate * deltaTime);
	}

	virtual void OnParried(){}
	virtual void OnBlocked() {}

	virtual bool IsMelee()
	{
		return false;
	}

	virtual void SetData(WeaponSlotData data)
	{
		Data = data;
	}

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

// Base class for weapons meant to be equipped in the offhand slot (e.g. the cane).
// Adds helpers shared by offhand weapons; concrete offhand weapons should still
// override UsesLeftHand() themselves to report when they're mid-action.
class WeaponOffhand : public Weapon
{
public:

	// True while the player's main weapon is currently in a state that forbids
	// starting a new offhand action (e.g. mid-reload). Offhand weapons should
	// gate the start of any of their own actions (attack, parry, throw, ...) on
	// this. Safe to call when the player has no main weapon equipped at all
	// (offhand-only loadout) — that case is never suppressed.
	bool IsSuppressedByMainWeapon() const;

};


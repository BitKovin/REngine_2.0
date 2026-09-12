#pragma once

#include "WeaponBase.h"
#include "../Player.hpp"

// Shared skeleton for Tool-type weapons: a single dedicated "useTool" key
// press triggers StartUse() once attackDelay has cleared. Player
// automatically switches back to whatever was equipped before, once
// CanChangeSlot() allows it again (see Player::UpdateWeaponRoleInput) -
// exactly like everything else, a Tool subclass just needs to eventually let
// CanChangeSlot() return true once it's done. Handles viewmodel/arms loading
// (with the usual SkipDrawAnimation handling) and basic positioning;
// StartUse() itself is weapon-specific - see weapon_cane's throw/grab/
// retrieve state machine for a fuller example.
class WeaponTool : public Weapon
{
public:

	std::string modelPath;
	std::string texturesLocation;
	std::string drawAnim = "draw";

	SkeletalMesh* viewmodel = nullptr;
	SkeletalMesh* arms = nullptr;

	vec3 weaponOffset = vec3(0.0f);

	Delay attackDelay; // general-purpose "busy" cooldown between uses

	WeaponTool();

	WeaponRole GetRole() const override { return WeaponRole::Tool; }

	void Start() override;
	void LoadAssets() override;
	void Update() override;
	void AsyncUpdate() override;
	void LateUpdate() override;

	void SetViewmodelScaleFactor(float factor);

	// Called once "useTool" is pressed and attackDelay has cleared. This is
	// where a concrete tool does its thing (throw a projectile, pop a gadget
	// UI, whatever). Not called automatically if a subclass overrides
	// Update() with its own multi-state input handling - see weapon_cane.
	virtual void StartUse() {}

	WeaponSlotData GetDefaultData() override
	{
		WeaponSlotData data;
		data.className = "weapon_tool";
		return data;
	}
};

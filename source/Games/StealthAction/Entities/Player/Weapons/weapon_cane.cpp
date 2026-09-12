#include "WeaponTool.h"

// Cane is a Tool: press "useTool" once to snap into a quick parry stance,
// then automatically return to whatever was equipped before once the parry
// finishes (CanChangeSlot() clears) - same auto-return every Tool gets, see
// Player::UpdateWeaponRoleInput.
class weapon_cane : public WeaponTool
{
public:

	Delay parryWindow;

	weapon_cane()
	{
		modelPath = "GameData/models/player/weapons/cane/cane.glb";
		texturesLocation = "GameData/models/player/weapons/cane/cane.glb/";
		weaponOffset = vec3(0.0, 0.00, -0.0);

		thirdPersonModelPath = "GameData/models/player/weapons/cane/cane_tp.glb";
	}

	// Busy for the duration of the parry stance - Player's Tool auto-return
	// only switches away once this clears.
	bool CanChangeSlot() override
	{
		return !attackDelay.Wait();
	}

	void StartUse() override
	{
		viewmodel->PlayAnimation("attack", false, 0.1f);

		parryWindow.AddDelay(0.3f);
		attackDelay.AddDelay(0.75f);

		SoundPlayer::PlayOneshot("event:/Weapons/knife/knife_attack", 2, 1, false);
	}

	// Called by the engine when an enemy attack lands during the parry window.
	void OnParried() override
	{
		Time::AddTimeScaleEffect(0.3, 0.1, true, "parry", 0.02f, 0.1f);

		SoundPlayer::PlayOneshot("event:/Weapons/cane/cane_parry", 1.0f, 1.0f, false);

		if (viewmodel->GetAnimationTime() < 0.18f)
			viewmodel->SetAnimationTime(0.18f);
	}

	void Update() override
	{
		WeaponTool::Update();

		Parrying = parryWindow.Wait();
	}

	WeaponSlotData GetDefaultData() override
	{
		WeaponSlotData data;

		data.className = "weapon_cane";

		return data;
	}

};

REGISTER_ENTITY(weapon_cane, "weapon_cane")

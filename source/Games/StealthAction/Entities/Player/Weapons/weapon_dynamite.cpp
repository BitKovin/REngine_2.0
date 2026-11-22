#include "WeaponThrowableBase.h"

// Dynamite: lit with the lighter, burns down a fuse and goes off wherever it is. Thrown or held too long.
class weapon_dynamite : public WeaponThrowableBase
{
public:

	weapon_dynamite()
	{
		params.viewmodelPath = "GameData/models/player/weapons/dynamite/dynamite.glb";
		params.projectileClass = "dynamiteProjectile";
		params.drawSound = "event:/Weapons/dynamite/draw";
		params.throwSound = "event:/Weapons/dynamite/throw";
		params.burnSound = "event:/Weapons/dynamite/burning";

		// Lit at frame 26 of throw_start. Four seconds from there: about three seconds after the arm is cocked.
		params.fuseTime = 4.0f;
		params.minThrowSpeed = 6.0f;
		params.maxThrowSpeed = 20.0f;

		thirdPersonModelPath = "GameData/models/player/weapons/dynamite/dynamite_tp.glb";
	}

	WeaponSlotData GetDefaultData() override
	{
		WeaponSlotData data;
		data.className = "weapon_dynamite";
		data.slot = 4;
		return data;
	}
};

REGISTER_ENTITY(weapon_dynamite, "weapon_dynamite")

#include "WeaponThrowableBase.h"

// Molotov cocktail: the rag is lit with the lighter, the bottle breaks on the first thing it hits.
// It has no fuse, so it can be held lit as long as the player likes.
class weapon_molotov : public WeaponThrowableBase
{
public:

	weapon_molotov()
	{
		params.viewmodelPath = "GameData/models/player/weapons/molotov/molotov.glb";
		params.projectileClass = "molotovProjectile";
		params.drawSound = "event:/Weapons/molotov/draw";
		params.throwSound = "event:/Weapons/molotov/throw";
		params.burnSound = "event:/Weapons/molotov/burning";

		params.fuseTime = -1.0f;
		params.minThrowSpeed = 5.0f;
		params.maxThrowSpeed = 18.0f;

		thirdPersonModelPath = "GameData/models/player/weapons/molotov/molotov_tp.glb";
	}

	WeaponSlotData GetDefaultData() override
	{
		WeaponSlotData data;
		data.className = "weapon_molotov";
		data.slot = 4;
		return data;
	}
};

REGISTER_ENTITY(weapon_molotov, "weapon_molotov")

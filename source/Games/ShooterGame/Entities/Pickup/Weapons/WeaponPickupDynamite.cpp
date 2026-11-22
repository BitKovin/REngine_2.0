#include "WeaponPickupBase.hpp"

class WeaponPickupDynamite : public WeaponPickupBase
{
public:
	WeaponPickupDynamite()
	{
		weaponName = "weapon_dynamite";
		modelPath = "GameData/models/player/weapons/dynamite/dynamite_thrown.glb";
	}
};

REGISTER_ENTITY(WeaponPickupDynamite, "item_weapon_dynamite")

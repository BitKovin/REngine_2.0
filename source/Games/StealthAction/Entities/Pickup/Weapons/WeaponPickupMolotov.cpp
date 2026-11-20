#include "WeaponPickupBase.hpp"

class WeaponPickupMolotov : public WeaponPickupBase
{
public:
	WeaponPickupMolotov()
	{
		weaponName = "weapon_molotov";
		modelPath = "GameData/models/player/weapons/molotov/molotov_thrown.glb";
	}
};

REGISTER_ENTITY(WeaponPickupMolotov, "item_weapon_molotov")

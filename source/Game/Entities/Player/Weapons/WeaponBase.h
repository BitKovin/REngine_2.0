#pragma once

#include <Entity.hpp>
#include <json.hpp>
#include <Helpers/JsonHelper.hpp>

struct WeaponSlotData
{
	string className = "";

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponSlotData, className)
};

class Weapon : public Entity
{
public:

	float HideWeapon = 0;


private:

};


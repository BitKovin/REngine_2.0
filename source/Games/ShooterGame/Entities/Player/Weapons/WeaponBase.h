#pragma once

#include <Entity.h>
#include <json.hpp>
#include <Helpers/JsonHelper.hpp>

struct WeaponSlotData
{
	string className = "";
	int slot = 0;
	int priority = 0;

	auto operator<=>(const WeaponSlotData&) const = default;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeaponSlotData, className,slot,priority)
};

class Weapon : public Entity
{

protected:

	Delay SwitchDelay;

public:

	float HideWeapon = 0;

	WeaponSlotData Data;

	std::string ArmsModelPath = "GameData/models/player/arms.glb";

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

private:

};


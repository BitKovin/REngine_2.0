#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <map>
#include <string>

// How a weapon can be held in two hands (two copies, one per hand).
enum class AkimboMode : uint8_t
{
	None,      // single only
	WithPerk,  // the pair appears for characters with AkimboPerk
	Always,    // the pair appears for every character
	Only       // only the pair exists, no single version
};

// Static, per-class weapon data for the weapon grid and its icons.
// Read from GameData/tables/weapons/*.csv. One row per weapon class.
struct WeaponDef
{
	std::string className;
	int slot = 0;                           // grid column 0..5 (slot key 1..6); -1 = left-hand item (the column before them, key slotMelee)
	int slotOrder = 0;                      // row order inside the column, smaller is higher
	AkimboMode akimbo = AkimboMode::None;
	std::string displayName;                // localisation key, e.g. "${WEAPON_PISTOL}"
	std::string iconModel;                  // model the icon is rendered from
	glm::vec3 iconRotation = glm::vec3(0);  // yaw, pitch, roll in degrees, applied before the icon camera looks at the model

	std::string IconPath() const { return "GameData/textures/ui/weapon_icons/" + className + ".png"; }

	bool HasSingle() const { return akimbo != AkimboMode::Only; }
	bool HasPair(bool akimboPerk) const
	{
		return akimbo == AkimboMode::Always || akimbo == AkimboMode::Only || (akimbo == AkimboMode::WithPerk && akimboPerk);
	}
};

class WeaponDefs
{
public:

	static void Load();

	// nullptr when the class has no row (logged once per class).
	static const WeaponDef* Find(const std::string& className);

	static const std::map<std::string, WeaponDef>& All() { return defs; }

private:

	static void LoadFile(const std::string& path);

	static inline std::map<std::string, WeaponDef> defs;

};

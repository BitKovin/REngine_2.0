#pragma once

#include <map>
#include <string>

// A playable character. Read from GameData/tables/characters/*.csv.
struct PlayerCharacterDef
{
	std::string id;
	std::string displayName;      // localisation key
	bool akimboPerk = false;      // adds the pair (akimbo) entries of WithPerk weapons to the weapon grid
	std::string hudSilhouette;    // the body figure of the health display (a PNG: the grey figure, tinted red from the feet as health drops)
};

class PlayerCharacters
{
public:

	static constexpr const char* DefaultId = "default";

	static void Load();

	// nullptr when there's no row with this id (logged).
	static const PlayerCharacterDef* Find(const std::string& id);

private:

	static void LoadFile(const std::string& path);

	static inline std::map<std::string, PlayerCharacterDef> defs;

};

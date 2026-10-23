#pragma once

// The baked world nav file (GameData/sim/world.nav) as the game side uses it: where it lives, how to load it, how a
// level maps to a world level, and the stale check (addendum 4.5, SIM_CONTRACT section 4).

#include <Sim/Nav/WorldNavData.h>

#include <string>
#include <vector>

namespace SimWorldNavFile
{
	constexpr const char* Path = "GameData/sim/world.nav";
	constexpr const char* MapFolder = "GameData/maps/";
	constexpr const char* DefaultManifest = "GameData/sim/world.manifest";

	// The maps of a manifest, in order (one map file name per line; blank lines and lines starting with # are ignored).
	// Empty when the file is missing. The first map is the layout root and the game's startup map.
	std::vector<std::string> ReadManifest(const std::string& manifestPath);

	inline std::string MapPath(const std::string& levelName) { return MapFolder + levelName; }

	// "GameData/maps/village.bsp" -> "village.bsp". Empty when the path is not inside the map folder.
	// Slashes and letter case of the folder do not matter.
	std::string LevelNameOfMapPath(const std::string& mapPath);

	// Index of the world level for a map path (case-insensitive), or -1.
	int FindLevel(const sim::nav::WorldNavData& data, const std::string& mapPath);

	// Reads and parses the world file. `error` says why not (missing file, wrong format, corrupt).
	bool Load(sim::nav::WorldNavData& out, std::string& error);

#ifndef DISTRIBUTION
	// Stale check for development builds: compares each baked level's source version with the map's modification time
	// and logs one loud error per level that changed, with the command that fixes it. Returns the number of stale levels.
	int CheckFresh(const sim::nav::WorldNavData& data);

	// What the game does at load: reads the world file and runs CheckFresh; a missing or unreadable file is logged just as
	// loudly. Returns true only when the file loads and no level is stale.
	bool CheckAtGameLoad();
#endif
}

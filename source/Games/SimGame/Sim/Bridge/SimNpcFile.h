#pragma once

// GameData/sim/npcs.json: the authored NPCs (SLICE_M2D_NPCS.md). The file is content, edited by hand, so every problem is reported
// with the NPC and the stop it is in, and one bad NPC never takes the others down.
//
// {
//   "npcs": [
//     { "id": "warden",                  // unique, up to 23 characters
//       "name": "Warden Harlan",          // shown in tools
//       "model": "guard",                 // "base" or "guard"
//       "speed": 1.0,                     // optional, multiplies the gait's table speed (0.2 to 3)
//       "gait": "walk",                   // optional, "walk" or "run"
//       "routine": "loop",                // optional, "loop" or "pingpong"
//       "level": "sim_test_a.bsp",        // the level of the stops that do not name one
//       "stops": [
//         { "at": [8, 0.2, 14], "wait": [3, 6] },          // level-local x y z, stay 3 to 6 seconds
//         { "spawn": "start", "wait": 5 },                   // or a spawn point by name
//         { "level": "sim_test_road.bsp", "at": [-18, 0.2, 3], "wait": 4 }
//       ] } ] }

#include <Sim/Npcs/SimNpcs.h>

#include <string>
#include <vector>

namespace SimNpcFile
{
	constexpr const char* Path = "GameData/sim/npcs.json";

	// Parses the text. NPCs that parse go to `npcs`; every problem goes to `errors`.
	void Parse(const std::string& text, const sim::nav::WorldNavData& world, std::vector<sim::npcs::NpcDefinition>& npcs, std::vector<std::string>& errors);

	// Reads Path and parses it. A missing file is not an error (no authored NPCs), unless `required`.
	void Load(const sim::nav::WorldNavData& world, std::vector<sim::npcs::NpcDefinition>& npcs, std::vector<std::string>& errors, bool required = false);
}

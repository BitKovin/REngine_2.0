#pragma once

// Data of the world navigation (addendum section 4): the inputs of the bake, the baked world file, and the reader for
// the engine's per-level .nav files.

#include <Sim/Core/SimArchive.h>

#include <Navigation/Detour/DetourNavMesh.h>

#include <glm/vec3.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace sim::nav
{
	// One level's baked navigation as the engine writes it (<map>.bsp.nav): mesh params and the raw tile blobs.
	struct LevelNavData
	{
		dtNavMeshParams params{};
		uint32_t sourceVersion = 0; // the map's modification time when the file was baked
		std::vector<std::vector<uint8_t>> tiles;
	};

	// Parses the compressed .nav file bytes (NavigationFileHelper format). The tile cache part is ignored.
	bool ReadLevelNavFile(const std::vector<uint8_t>& fileBytes, LevelNavData& out, std::string& error);

	// A level transition authored in a map (sim_portal): the trigger, and where it lands.
	struct PortalInput
	{
		std::string name;          // the trigger's targetname, for messages
		glm::vec3 position{ 0.0f };// level-local; projected onto the level's mesh by the bake
		std::string targetLevel;   // a level name from the manifest
		std::string targetSpawn;   // a spawn point name in the target level
	};

	struct SpawnInput
	{
		std::string name;
		glm::vec3 position{ 0.0f };
		bool playerStart = false; // the level's default player start
	};

	struct LevelInput
	{
		std::string name; // map file name, e.g. "village.bsp"
		LevelNavData nav;
		std::vector<PortalInput> portals;
		std::vector<SpawnInput> spawns;
	};

	struct BakeSettings
	{
		float linkRadius = 1.0f;            // off-mesh connection radius (landing search, horizontal)
		glm::vec3 projectExtents{ 2.0f, 4.0f, 2.0f }; // half extents when snapping portals and spawns to the mesh
		uint8_t linkArea = 1;
		uint16_t linkFlags = 1;
		bool createLinks = true;            // false only in tests (proves levels do not stitch by themselves)
	};

	// ---- The world file -----------------------------------------------------------------------------------------

	struct WorldLevel
	{
		std::string name;
		uint32_t sourceVersion = 0;
		glm::vec3 offset{ 0.0f };  // nav position = level-local position + offset
		int32_t tileShiftX = 0;    // world tile = local tile + shift
		int32_t tileShiftY = 0;
		int32_t tileCount = 0;
		template<class Ar> void Visit(Ar& ar)
		{
			ar("name", name);
			ar("sourceVersion", sourceVersion);
			ar("offset", offset);
			ar("tileShiftX", tileShiftX);
			ar("tileShiftY", tileShiftY);
			ar("tileCount", tileCount);
		}
	};

	// One authored transition, as linked in the world mesh.
	struct WorldLink
	{
		std::string portal;
		uint16_t fromLevel = 0;
		uint16_t toLevel = 0;
		glm::vec3 start{ 0.0f }; // nav space
		glm::vec3 end{ 0.0f };
		bool linked = false;       // false: the layout could not put the two ends in neighbouring tiles
		bool bidirectional = false;
		template<class Ar> void Visit(Ar& ar)
		{
			ar("portal", portal);
			ar("fromLevel", fromLevel);
			ar("toLevel", toLevel);
			ar("start", start);
			ar("end", end);
			ar("linked", linked);
			ar("bidirectional", bidirectional);
		}
	};

	// A named arrival point (sim_spawn) snapped onto the world mesh; the demo population and the player start use them.
	struct WorldSpawn
	{
		std::string name;
		uint16_t level = 0;
		glm::vec3 position{ 0.0f }; // nav space
		bool playerStart = false;
		template<class Ar> void Visit(Ar& ar)
		{
			ar("name", name);
			ar("level", level);
			ar("position", position);
			ar("playerStart", playerStart);
		}
	};

	struct WorldTile
	{
		uint16_t level = 0;
		std::vector<uint8_t> data; // Detour tile blob, already translated and linked
		template<class Ar> void Visit(Ar& ar) { ar("level", level); ar("data", data); }
	};

	struct WorldNavData
	{
		static constexpr uint32_t FormatVersion = 2; // 2: spawns

		float tileWidth = 0.0f;
		int32_t maxTiles = 0;
		int32_t maxPolys = 0;
		float walkableHeight = 0.0f;
		float walkableRadius = 0.0f;
		float walkableClimb = 0.0f;
		std::vector<WorldLevel> levels;
		std::vector<WorldLink> links;
		std::vector<WorldSpawn> spawns;
		std::vector<WorldTile> tiles;

		template<class Ar> void Visit(Ar& ar)
		{
			ar("tileWidth", tileWidth);
			ar("maxTiles", maxTiles);
			ar("maxPolys", maxPolys);
			ar("walkableHeight", walkableHeight);
			ar("walkableRadius", walkableRadius);
			ar("walkableClimb", walkableClimb);
			ar("levels", levels);
			ar("links", links);
			ar("spawns", spawns);
			ar("tiles", tiles);
		}

		std::vector<uint8_t> Serialize() const;
		static bool Deserialize(const std::vector<uint8_t>& bytes, WorldNavData& out, std::string& error);

		int FindLevel(const std::string& name) const;

		// Identifies the layout a saved sim is tied to: levels (name, offset, tile shift) and links. A rebake that moves a
		// level changes it; spawns and tile contents do not (saved positions only depend on the layout).
		uint64_t Fingerprint() const;
	};

	// A level whose map changed (or disappeared) since the world file was baked: its physical nav mesh and the
	// world mesh may no longer agree (addendum 4.5).
	struct StaleLevel
	{
		std::string name;
		uint32_t baked = 0;   // the map's modification time when the world file was baked
		uint32_t current = 0; // now; 0 when the map file does not exist
		std::string Describe() const;
	};

	// `mapVersion` returns a map's current modification time (0 for a missing map). Result is in level order.
	std::vector<StaleLevel> FindStaleLevels(const WorldNavData& data, const std::function<uint32_t(const std::string&)>& mapVersion);
}

#pragma once

// The world nav bake (addendum section 4.3) as a pure function: levels in, world file out, every validation failure
// reported by name. The console command that loads the levels through the engine is a thin shell around it.
//
// Steps:
//   1. Parse every level's tiles; all levels must share tile size and agent profile.
//   2. Snap portals (trigger positions) and their target spawn points onto their level's mesh.
//   3. Layout: a breadth-first spanning tree of the level graph from the first level. Each level gets a whole-tile
//      shift that puts it as close as possible to its partner's portal while keeping at least one empty tile between
//      the levels' tile bounding boxes, so levels never stitch together outside the portals.
//   4. Translate tiles and add one off-mesh connection per portal (trigger to the named spawn point). Connections are
//      "remote" (DT_OFFMESH_SIDE_REMOTE): stock Detour only lands a connection in the start tile or its neighbours, so
//      dtNavMesh::connectRemoteOffMeshLinks (an engine addition) lands them by position after all tiles are added.
//   5. Build the combined mesh and validate: every link connects, every level's portal ends reach each other, every
//      level can reach every other level.

#include "WorldNavData.h"

#include <string>
#include <vector>

class dtNavMesh;

namespace sim::nav
{
	struct BakeReport
	{
		std::vector<std::string> errors;
		std::vector<std::string> warnings;
		std::vector<std::string> notes;

		bool Ok() const { return errors.empty(); }
		std::string Text() const;
	};

	bool BakeWorldNav(const std::vector<LevelInput>& levels, const BakeSettings& settings, WorldNavData& out, BakeReport& report);

	// Copies the tiles of a live mesh (the engine's level mesh after it built or loaded the level's nav).
	LevelNavData CaptureLevelNav(const dtNavMesh& mesh, uint32_t sourceVersion);
}

#pragma once

// Real baked levels shared by the nav and movement tests (addendum 17.2: round-trip tests on real fixtures).

#include "SimTestFramework.h"

#ifndef DISTRIBUTION

#include <Sim/Nav/WorldNav.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace sim::test
{
	extern const char* const Fixtures[8];

	// Reads GameData/maps/<name>.bsp.nav; a failure is a failed check.
	bool LoadFixture(TestContext& t, const std::string& name, sim::nav::LevelInput& out);

	// A level's own mesh, as the engine would hold it.
	struct OriginalMesh
	{
		dtNavMesh* mesh = nullptr;
		dtNavMeshQuery* query = nullptr;
		~OriginalMesh();
		bool Build(const sim::nav::LevelNavData& nav);
	};

	// Ground polygons of the largest walkable region around the busiest tile (avoids rooftops and islands).
	std::vector<std::pair<dtPolyRef, glm::vec3>> MainRegionPolys(OriginalMesh& original);

	// A chain of levels joined east to west by synthetic portals at the edges of their main walkable region.
	struct Chain
	{
		std::vector<sim::nav::LevelInput> levels;
		std::vector<glm::vec3> east, west; // portal points per level, level-local
	};

	bool BuildChain(TestContext& t, const std::vector<const char*>& names, Chain& chain);

	// A chain baked into a world mesh: the loaded WorldNav and the portal spots of every level in nav space
	// (points: east and west of each level, level by level).
	struct ChainWorld
	{
		Chain chain;
		sim::nav::WorldNavData data;
		std::unique_ptr<sim::nav::WorldNav> nav;
		std::vector<glm::vec3> points;

		// The first name is the layout root. Default: the four fixtures the movement tests use.
		bool Build(TestContext& t, const std::vector<const char*>& names = { "lvl1", "test", "example", "test_interior" });
	};
}

#endif

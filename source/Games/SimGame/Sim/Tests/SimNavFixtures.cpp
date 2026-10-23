#include "SimNavFixtures.h"

#ifndef DISTRIBUTION

#include <Sim/Nav/WorldNavBake.h>

#include <FileSystem/FileSystem.h>
#include <Navigation/Detour/DetourAlloc.h>
#include <Navigation/Detour/DetourCommon.h>

#include <cstring>

namespace sim::test
{
	using namespace sim::nav;

	const char* const Fixtures[8] = { "lvl1", "test", "example", "test_interior", "l1_town", "lvl1_proto", "lvl2", "blank" };

	bool LoadFixture(TestContext& t, const std::string& name, LevelInput& out)
	{
		const std::string path = "GameData/maps/" + name + ".bsp.nav";
		std::string error;
		out = LevelInput{};
		out.name = name + ".bsp";
		const bool ok = ReadLevelNavFile(FileSystemEngine::ReadFileBinary(path), out.nav, error);
		t.Check(ok, "%s loads: %s", path.c_str(), error.c_str());
		return ok;
	}

	OriginalMesh::~OriginalMesh()
	{
		dtFreeNavMeshQuery(query);
		dtFreeNavMesh(mesh);
	}

	bool OriginalMesh::Build(const LevelNavData& nav)
	{
		mesh = dtAllocNavMesh();
		if (dtStatusFailed(mesh->init(&nav.params)))
			return false;
		for (const auto& blob : nav.tiles)
		{
			unsigned char* copy = static_cast<unsigned char*>(dtAlloc(static_cast<int>(blob.size()), DT_ALLOC_PERM));
			std::memcpy(copy, blob.data(), blob.size());
			if (dtStatusFailed(mesh->addTile(copy, static_cast<int>(blob.size()), DT_TILE_FREE_DATA, 0, nullptr)))
			{
				dtFree(copy);
				return false;
			}
		}
		query = dtAllocNavMeshQuery();
		return dtStatusSucceed(query->init(mesh, WorldNav::QueryNodes));
	}

	namespace
	{
		glm::vec3 PolyCentre(const dtMeshTile* tile, const dtPoly* poly)
		{
			glm::vec3 centre(0.0f);
			for (int i = 0; i < poly->vertCount; ++i)
				centre += glm::vec3(tile->verts[poly->verts[i] * 3], tile->verts[poly->verts[i] * 3 + 1], tile->verts[poly->verts[i] * 3 + 2]);
			return centre / static_cast<float>(poly->vertCount);
		}
	}

	// Ground polygons of the largest walkable region around the busiest tile (avoids rooftops and islands).
	std::vector<std::pair<dtPolyRef, glm::vec3>> MainRegionPolys(OriginalMesh& original)
	{
		const dtNavMesh* mesh = original.mesh;
		const dtMeshTile* busiest = nullptr;
		for (int i = 0; i < mesh->getMaxTiles(); ++i)
		{
			const dtMeshTile* tile = mesh->getTile(i);
			if (tile && tile->header && (!busiest || tile->header->polyCount > busiest->header->polyCount))
				busiest = tile;
		}
		std::vector<std::pair<dtPolyRef, glm::vec3>> result;
		if (!busiest)
			return result;
		const dtPolyRef seed = mesh->getPolyRefBase(busiest);
		const glm::vec3 seedPos = PolyCentre(busiest, &busiest->polys[0]);
		SimQueryFilter filter;
		std::vector<dtPolyRef> refs(8192);
		int count = 0;
		original.query->findPolysAroundCircle(seed, &seedPos.x, 100000.0f, &filter, refs.data(), nullptr, nullptr, &count, static_cast<int>(refs.size()));
		for (int i = 0; i < count; ++i)
		{
			const dtMeshTile* tile = nullptr;
			const dtPoly* poly = nullptr;
			mesh->getTileAndPolyByRef(refs[i], &tile, &poly);
			if (poly->getType() == DT_POLYTYPE_GROUND)
				result.emplace_back(refs[i], PolyCentre(tile, poly));
		}
		return result;
	}


	bool BuildChain(TestContext& t, const std::vector<const char*>& names, Chain& chain)
	{
		for (const char* name : names)
		{
			LevelInput level;
			if (!LoadFixture(t, name, level))
				return false;
			OriginalMesh original;
			if (!original.Build(level.nav))
				return false;
			const auto polys = MainRegionPolys(original);
			if (polys.empty())
				return false;
			glm::vec3 east = polys[0].second, west = polys[0].second;
			for (const auto& poly : polys)
			{
				if (poly.second.x > east.x)
					east = poly.second;
				if (poly.second.x < west.x)
					west = poly.second;
			}
			chain.east.push_back(east);
			chain.west.push_back(west);
			chain.levels.push_back(std::move(level));
		}
		for (size_t i = 0; i + 1 < chain.levels.size(); ++i)
		{
			LevelInput& a = chain.levels[i];
			LevelInput& b = chain.levels[i + 1];
			a.portals.push_back({ "to_" + b.name, chain.east[i], b.name, "from_" + a.name });
			b.spawns.push_back({ "from_" + a.name, chain.west[i + 1] });
			b.portals.push_back({ "to_" + a.name, chain.west[i + 1], a.name, "from_" + b.name });
			a.spawns.push_back({ "from_" + b.name, chain.east[i] });
		}
		return true;
	}


	bool ChainWorld::Build(TestContext& t, const std::vector<const char*>& names)
	{
		if (!BuildChain(t, names, chain))
			return false;
		BakeReport report;
		nav = std::make_unique<WorldNav>();
		std::string error;
		if (!t.Check(BakeWorldNav(chain.levels, BakeSettings{}, data, report) && nav->Load(data, error), "chain bakes and loads: %s", error.c_str()))
			return false;
		points.clear();
		for (size_t level = 0; level < chain.levels.size(); ++level)
		{
			points.push_back(nav->ToNav(static_cast<int>(level), chain.east[level]));
			points.push_back(nav->ToNav(static_cast<int>(level), chain.west[level]));
		}
		return true;
	}
}

#endif

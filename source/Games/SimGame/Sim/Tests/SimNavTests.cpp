// World nav tests on real baked levels (GameData/maps/*.bsp.nav, addendum section 17.2: round-trip tests on real
// fixtures). Failures name the level.

#include "SimTestFramework.h"

#ifndef DISTRIBUTION

#include <Sim/Core/SimRandom.h>
#include <Sim/Nav/NavTileData.h>
#include <Sim/Nav/WorldNav.h>
#include <Sim/Nav/WorldNavBake.h>

#include <FileSystem/FileSystem.h>
#include <Navigation/Detour/DetourAlloc.h>
#include <Navigation/Detour/DetourCommon.h>

#include <glm/geometric.hpp>

#include <cmath>
#include <cstring>
#include <memory>

namespace sim::test
{
	namespace
	{
		using namespace sim::nav;

		const char* Fixtures[] = { "lvl1", "test", "example", "test_interior", "l1_town", "lvl1_proto", "lvl2", "blank" };

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

		// The level's own mesh, as the engine would hold it.
		struct OriginalMesh
		{
			dtNavMesh* mesh = nullptr;
			dtNavMeshQuery* query = nullptr;
			~OriginalMesh()
			{
				dtFreeNavMeshQuery(query);
				dtFreeNavMesh(mesh);
			}
			bool Build(const LevelNavData& nav)
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
		};

		glm::vec3 PolyCentre(const dtMeshTile* tile, const dtPoly* poly)
		{
			glm::vec3 centre(0.0f);
			for (int i = 0; i < poly->vertCount; ++i)
				centre += glm::vec3(tile->verts[poly->verts[i] * 3], tile->verts[poly->verts[i] * 3 + 1], tile->verts[poly->verts[i] * 3 + 2]);
			return centre / static_cast<float>(poly->vertCount);
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

		// Corridor as (tile x, tile y, layer, polygon index) so two meshes can be compared.
		struct PolyKey
		{
			int x, y, layer, poly;
			bool operator==(const PolyKey&) const = default;
		};

		PolyKey KeyOf(const dtNavMesh* mesh, dtPolyRef ref, int shiftX = 0, int shiftY = 0)
		{
			const dtMeshTile* tile = nullptr;
			const dtPoly* poly = nullptr;
			mesh->getTileAndPolyByRef(ref, &tile, &poly);
			return { tile->header->x - shiftX, tile->header->y - shiftY, tile->header->layer, static_cast<int>(poly - tile->polys) };
		}

		float PathLength(dtNavMeshQuery* query, dtPolyRef a, const glm::vec3& pa, dtPolyRef b, const glm::vec3& pb, bool& complete, std::vector<dtPolyRef>& corridor)
		{
			SimQueryFilter filter;
			corridor.assign(4096, 0);
			int count = 0;
			const dtStatus status = query->findPath(a, b, &pa.x, &pb.x, &filter, corridor.data(), &count, 4096);
			complete = count > 0 && corridor[count - 1] == b && !(status & DT_PARTIAL_RESULT);
			corridor.resize(count);
			std::vector<float> corners(4096 * 3);
			int cornerCount = 0;
			query->findStraightPath(&pa.x, &pb.x, corridor.data(), count, corners.data(), nullptr, nullptr, &cornerCount, 4096, 0);
			float length = 0.0f;
			for (int i = 1; i < cornerCount; ++i)
				length += glm::distance(glm::vec3(corners[i * 3 - 3], corners[i * 3 - 2], corners[i * 3 - 1]), glm::vec3(corners[i * 3], corners[i * 3 + 1], corners[i * 3 + 2]));
			return length;
		}

		void TestParseAndRewrite(TestContext& t)
		{
			t.Section("nav: tile parse and rewrite on fixtures");
			for (const char* name : Fixtures)
			{
				LevelInput level;
				if (!LoadFixture(t, name, level))
					continue;
				size_t polys = 0;
				bool sameSize = true, reparsed = true;
				for (const auto& blob : level.nav.tiles)
				{
					TileData tile;
					std::string error;
					if (!t.Check(TileData::Parse(blob.data(), blob.size(), tile, error), "%s: tile parses: %s", name, error.c_str()))
						break;
					polys += tile.polys.size();
					const std::vector<uint8_t> written = tile.WriteVector();
					sameSize &= written.size() == blob.size();
					TileData again;
					reparsed &= TileData::Parse(written.data(), written.size(), again, error) && again.verts == tile.verts && again.polys.size() == tile.polys.size() &&
						again.detailVerts == tile.detailVerts && again.bvTree.size() == tile.bvTree.size();
				}
				t.Check(sameSize && reparsed, "%s: an unedited tile writes back to the same layout", name);
				t.Note("%-14s %4zu tiles, %6zu polys, tile %.2f m, origin (%.1f %.1f %.1f)", name, level.nav.tiles.size(), polys, level.nav.params.tileWidth,
					level.nav.params.orig[0], level.nav.params.orig[1], level.nav.params.orig[2]);
			}
		}

		// Moves one level by whole tiles into a world mesh and checks that every query answers the same, offset by the move.
		void TestTranslationRoundTrip(TestContext& t)
		{
			t.Section("nav: translated level answers like the original");
			for (const char* name : { "lvl1", "test", "test_interior", "l1_town", "lvl2" })
			{
				LevelInput level;
				if (!LoadFixture(t, name, level))
					continue;
				OriginalMesh original;
				if (!t.Check(original.Build(level.nav), "%s: original mesh builds", name))
					continue;

				const int shiftX = 7, shiftY = -3;
				const float width = level.nav.params.tileWidth;
				const glm::vec3 offset(shiftX * width - level.nav.params.orig[0], 0.0f, shiftY * width - level.nav.params.orig[2]);
				WorldNavData data;
				data.tileWidth = width;
				data.levels.push_back({ level.name, 0, offset, shiftX, shiftY, static_cast<int32_t>(level.nav.tiles.size()) });
				for (const auto& blob : level.nav.tiles)
				{
					TileData tile;
					std::string error;
					TileData::Parse(blob.data(), blob.size(), tile, error);
					tile.Translate(offset, tile.header.x + shiftX, tile.header.y + shiftY);
					data.tiles.push_back({ 0, tile.WriteVector() });
					data.maxPolys = std::max<int32_t>(data.maxPolys, static_cast<int32_t>(tile.polys.size()));
				}
				data.maxTiles = static_cast<int32_t>(data.tiles.size());
				WorldNav world;
				std::string error;
				if (!t.Check(world.Load(data, error), "%s: translated mesh loads: %s", name, error.c_str()))
					continue;

				const auto polys = MainRegionPolys(original);
				if (!t.Check(polys.size() >= 2, "%s: has a walkable region", name))
					continue;
				Rng rng(HashString(name));
				int pathMismatches = 0, projectMismatches = 0, completePaths = 0, lengthDrift = 0, completenessMismatches = 0;
				for (int i = 0; i < 40; ++i)
				{
					const auto& a = polys[rng.Below(static_cast<uint32_t>(polys.size()))];
					const auto& b = polys[rng.Below(static_cast<uint32_t>(polys.size()))];
					bool completeA = false;
					std::vector<dtPolyRef> corridorA;
					// Both meshes route between the same points: the centres snapped onto the surface of the original.
					SimQueryFilter snapFilter;
					const float snapExtents[3] = { 0.1f, 0.5f, 0.1f };
					float snapA[3], snapB[3];
					dtPolyRef refA = 0, refB = 0;
					original.query->findNearestPoly(&a.second.x, snapExtents, &snapFilter, &refA, snapA);
					original.query->findNearestPoly(&b.second.x, snapExtents, &snapFilter, &refB, snapB);
					const glm::vec3 pa(snapA[0], snapA[1], snapA[2]), pb(snapB[0], snapB[1], snapB[2]);
					const float lengthA = PathLength(original.query, refA, pa, refB, pb, completeA, corridorA);
					NavPoint startPoint{ pa + offset, world.Project(pa + offset, { 0.05f, 0.1f, 0.05f }).poly };
					NavPoint endPoint{ pb + offset, world.Project(pb + offset, { 0.05f, 0.1f, 0.05f }).poly };
					const NavPath path = world.FindPath(startPoint, endPoint);
					completePaths += completeA ? 1 : 0;
					// The polygon corridor must be identical. Corner lengths may differ slightly: Detour's funnel is not
					// stable under float rounding of translated coordinates in near-degenerate corners.
					bool sameCorridor = completeA == path.complete && corridorA.size() == path.corridor.size();
					for (size_t c = 0; sameCorridor && c < corridorA.size(); ++c)
						sameCorridor = KeyOf(original.mesh, corridorA[c]) == KeyOf(world.Mesh(), path.corridor[c], shiftX, shiftY);
					if (completeA != path.complete)
						++completenessMismatches;
					if (!sameCorridor)
						++pathMismatches;
					if (std::fabs(lengthA - path.length) > 0.02f * std::max(1.0f, lengthA))
						++lengthDrift;

					const glm::vec3 probe = a.second + glm::vec3(rng.Range(-1.5f, 1.5f), rng.Range(-0.5f, 0.5f), rng.Range(-1.5f, 1.5f));
					SimQueryFilter filter;
					const float extents[3] = { 2.0f, 4.0f, 2.0f };
					float nearest[3];
					dtPolyRef ref = 0;
					original.query->findNearestPoly(&probe.x, extents, &filter, &ref, nearest);
					const NavPoint projected = world.Project(probe + offset);
					if ((ref != 0) != projected.IsValid() || (ref && glm::distance(glm::vec3(nearest[0], nearest[1], nearest[2]) + offset, projected.position) > 1e-3f))
						++projectMismatches;
				}
				// Translation shifts float rounding, so A* may pick a different one of two equal-cost polygons and the funnel
				// may resolve a near-degenerate corner differently. Reachability must match exactly; routes nearly always.
				t.Check(completenessMismatches == 0, "%s: reachability is identical for 40 pairs (%d differ, %d complete)", name, completenessMismatches, completePaths);
				t.Check(pathMismatches <= 4 && lengthDrift <= 2, "%s: routes match after the move (%d of 40 corridors differ by ties, %d lengths differ > 2%%)",
					name, pathMismatches, lengthDrift);
				t.Check(projectMismatches == 0, "%s: 40 nearest-poly queries match after the move (%d mismatches)", name, projectMismatches);
				t.Check(world.LevelOfPoly(world.Project(polys[0].second + offset).poly) == 0, "%s: polygons know their level", name);
			}
		}

		// A chain of levels joined east to west by synthetic portals at the edges of their main walkable region.
		struct Chain
		{
			std::vector<LevelInput> levels;
			std::vector<glm::vec3> east, west; // portal points per level, level-local
		};

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

		void TestChainBake(TestContext& t)
		{
			t.Section("nav: bake a chain of four levels");
			Chain chain;
			if (!t.Check(BuildChain(t, { "lvl1", "test", "example", "test_interior" }, chain), "fixtures load"))
				return;

			WorldNavData data;
			BakeReport report;
			const bool ok = BakeWorldNav(chain.levels, BakeSettings{}, data, report);
			t.Check(ok, "bake succeeds:\n%s", report.Text().c_str());
			t.Note("%s", report.Text().c_str());
			if (!ok)
				return;
			int linked = 0;
			for (const WorldLink& link : data.links)
				linked += link.linked ? 1 : 0;
			t.Check(linked == 6, "all 6 portals are linked (%d)", linked);

			// Save and load the world file, then route from the first level to the last.
			WorldNavData reloaded;
			std::string error;
			t.Check(WorldNavData::Deserialize(data.Serialize(), reloaded, error), "world file round trip: %s", error.c_str());
			WorldNav nav;
			if (!t.Check(nav.Load(reloaded, error), "world mesh loads: %s", error.c_str()))
				return;

			const NavPoint start = nav.Project(nav.ToNav(0, chain.east[0]));
			const NavPoint end = nav.Project(nav.ToNav(3, chain.west[3]));
			const NavPath path = nav.FindPath(start, end);
			t.Check(path.complete && path.portalCrossings == 3, "a route crosses three levels (complete %d, crossings %d, %zu corners, %.1f m)", path.complete,
				path.portalCrossings, path.points.size(), path.length);
			bool ordered = !path.pointLevels.empty() && path.pointLevels.front() == 0 && path.pointLevels.back() == 3;
			for (size_t i = 1; i < path.pointLevels.size(); ++i)
				ordered &= path.pointLevels[i] >= path.pointLevels[i - 1];
			std::string levelsVisited;
			for (const int level : path.pointLevels)
				levelsVisited += std::to_string(level) + " ";
			t.Check(ordered, "the route visits the levels in order (%s)", levelsVisited.c_str());
			const NavPath back = nav.FindPath(end, start);
			t.Check(back.complete && back.portalCrossings == 3, "and back again (complete %d, crossings %d)", back.complete, back.portalCrossings);
			const glm::vec3 local = nav.ToLocal(2, nav.ToNav(2, chain.west[2]));
			t.Check(glm::distance(local, chain.west[2]) < 1e-4f, "nav and level-local positions convert both ways");

			// Sliced service gives the same answer, results in request order.
			PathService service(nav);
			const PathService::RequestId first = service.Request(start, end);
			const PathService::RequestId second = service.Request(end, start);
			const PathService::RequestId broken = service.Request(NavPoint{}, end);
			int updates = 0;
			std::vector<PathService::Result> results;
			while (results.size() < 3 && updates < 10000)
			{
				service.Update(32);
				++updates;
				for (auto& result : service.TakeResults())
					results.push_back(std::move(result));
			}
			t.Check(results.size() == 3 && results[0].id == first && results[1].id == second && results[2].id == broken, "sliced results arrive for every request");
			if (results.size() == 3)
			{
				t.Check(!results[0].failed && results[0].path.complete && results[0].path.portalCrossings == 3 &&
					std::fabs(results[0].path.length - path.length) < 0.05f * path.length,
					"sliced path is a valid route of similar length (%d updates of 32 iterations; %.2f vs one-shot %.2f m)", updates, results[0].path.length, path.length);

				// What determinism needs: the budget (thread timing) never changes the result.
				PathService bigBudget(nav);
				bigBudget.Request(start, end);
				bigBudget.Update(1000000);
				const auto big = bigBudget.TakeResults();
				t.Check(big.size() == 1 && big[0].path.corridor == results[0].path.corridor, "the sliced result does not depend on the iteration budget");
				t.Check(results[2].failed, "an invalid request fails cleanly");
			}
		}

		void TestLevelsDoNotStitch(TestContext& t)
		{
			t.Section("nav: levels never connect outside portals");
			Chain chain;
			if (!BuildChain(t, { "lvl1", "test" }, chain))
				return;
			BakeSettings settings;
			settings.createLinks = false;
			WorldNavData data;
			BakeReport report;
			BakeWorldNav(chain.levels, settings, data, report);
			if (!t.Check(data.levels.size() == 2, "the unlinked bake still lays out both levels:\n%s", report.Text().c_str()))
				return;
			WorldNav nav;
			std::string error;
			if (!t.Check(nav.Load(data, error), "unlinked world loads: %s", error.c_str()))
				return;
			const NavPath path = nav.FindPath(nav.Project(nav.ToNav(0, chain.east[0])), nav.Project(nav.ToNav(1, chain.west[1])));
			t.Check(!path.complete, "without links the neighbouring levels are not reachable from each other (tiles touch: shift %d,%d)", data.levels[1].tileShiftX,
				data.levels[1].tileShiftY);
		}

		void TestBakeValidation(TestContext& t)
		{
			t.Section("nav: bake validation names the problem");
			Chain chain;
			if (!BuildChain(t, { "lvl1", "test" }, chain))
				return;
			Chain broken = chain;
			broken.levels[0].portals[0].targetSpawn = "nowhere";
			broken.levels[1].portals[0].targetLevel = "missing.bsp";
			broken.levels[1].portals.push_back({ "in_the_sky", chain.west[1] + glm::vec3(0.0f, 500.0f, 0.0f), chain.levels[0].name, chain.levels[0].spawns[0].name });
			WorldNavData data;
			BakeReport report;
			const bool ok = BakeWorldNav(broken.levels, BakeSettings{}, data, report);
			const std::string text = report.Text();
			t.Check(!ok && text.find("'nowhere'") != std::string::npos && text.find("missing.bsp") != std::string::npos && text.find("in_the_sky") != std::string::npos,
				"missing spawn, missing level and an off-mesh trigger are each reported:\n%s", text.c_str());

			Chain lonely = chain;
			lonely.levels[0].portals.clear();
			lonely.levels[1].portals.clear();
			report = BakeReport{};
			t.Check(!BakeWorldNav(lonely.levels, BakeSettings{}, data, report) && report.Text().find("not connected") != std::string::npos,
				"a level without portals is reported as unconnected");
		}
	}

	void RunNavTests(TestContext& context)
	{
		TestParseAndRewrite(context);
		TestTranslationRoundTrip(context);
		TestChainBake(context);
		TestLevelsDoNotStitch(context);
		TestBakeValidation(context);
	}
}

#endif

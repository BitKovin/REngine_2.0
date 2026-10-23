// World nav tests on real baked levels (GameData/maps/*.bsp.nav, addendum section 17.2: round-trip tests on real
// fixtures). Failures name the level.

#include "SimNavFixtures.h"

#ifndef DISTRIBUTION

#include <Sim/Bridge/SimWorldNavFile.h>
#include <Sim/Core/SimRandom.h>
#include <Sim/Nav/NavTileData.h>
#include <Sim/Nav/WorldNav.h>
#include <Sim/Nav/WorldNavBake.h>
#include <Sim/Nav/WorldNavDebug.h>

#include <FileSystem/FileSystem.h>
#include <Navigation/Detour/DetourAlloc.h>
#include <Navigation/Detour/DetourCommon.h>

#include <entt/container/dense_map.hpp>
#include <glm/common.hpp>
#include <glm/geometric.hpp>

#include <cmath>
#include <cstring>
#include <memory>

namespace sim::test
{
	namespace
	{
		using namespace sim::nav;

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
			{
				// Cost of the longest route of the chain (budget for path requests per tick).
				constexpr int Repeats = 200;
				Stopwatch timer;
				for (int i = 0; i < Repeats; ++i)
					nav.FindPath(start, end);
				t.Note("one-shot path across 3 portals (%zu corners, %d polygons): %.0f us", path.points.size(), path.polyCount, timer.Seconds() / Repeats * 1e6);
			}
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

		void TestStaleCheck(TestContext& t)
		{
			t.Section("nav: stale world file check");
			WorldNavData data;
			for (const char* name : { "a.bsp", "b.bsp", "c.bsp", "d.bsp" })
			{
				WorldLevel level;
				level.name = name;
				level.sourceVersion = 100 + static_cast<uint32_t>(data.levels.size());
				data.levels.push_back(level);
			}
			// a: unchanged, b: edited after the bake, c: deleted, d: restored to an older version.
			entt::dense_map<std::string, uint32_t> versions;
			versions.emplace("a.bsp", 100);
			versions.emplace("b.bsp", 500);
			versions.emplace("d.bsp", 7);
			auto lookup = [&](const std::string& name)
			{
				const auto found = versions.find(name);
				return found == versions.end() ? 0u : found->second;
			};
			const auto stale = FindStaleLevels(data, lookup);
			t.Check(stale.size() == 3 && stale[0].name == "b.bsp" && stale[1].name == "c.bsp" && stale[2].name == "d.bsp", "exactly the changed, missing and older maps are stale, in level order (%zu)", stale.size());
			if (stale.size() == 3)
			{
				t.Check(stale[0].baked == 101 && stale[0].current == 500 && stale[0].Describe().find("b.bsp") != std::string::npos, "a changed map reports both versions: %s", stale[0].Describe().c_str());
				t.Check(stale[1].current == 0 && stale[1].Describe().find("missing") != std::string::npos, "a deleted map is reported as missing: %s", stale[1].Describe().c_str());
			}
			data.levels.erase(data.levels.begin() + 1, data.levels.end());
			t.Check(FindStaleLevels(data, lookup).empty(), "an unchanged world file has no stale level");

			t.Check(SimWorldNavFile::LevelNameOfMapPath("GameData/maps/village.bsp") == "village.bsp", "map path to level name");
			t.Check(SimWorldNavFile::LevelNameOfMapPath("gamedata\\Maps\\Village.bsp") == "Village.bsp", "folder case and slashes do not matter");
			t.Check(SimWorldNavFile::LevelNameOfMapPath("GameData/other/village.bsp").empty() && SimWorldNavFile::LevelNameOfMapPath("GameData/maps/").empty(), "paths outside the map folder map to nothing");
			t.Check(SimWorldNavFile::FindLevel(data, "GameData/maps/A.BSP") == 0 && SimWorldNavFile::FindLevel(data, "GameData/maps/zzz.bsp") == -1, "levels are found by map path, ignoring case");
		}

		void LevelBounds(const LevelNavData& nav, glm::vec3& low, glm::vec3& high)
		{
			low = glm::vec3(1e30f);
			high = glm::vec3(-1e30f);
			for (const auto& blob : nav.tiles)
			{
				const dtMeshHeader* header = reinterpret_cast<const dtMeshHeader*>(blob.data());
				low = glm::min(low, glm::vec3(header->bmin[0], header->bmin[1], header->bmin[2]));
				high = glm::max(high, glm::vec3(header->bmax[0], header->bmax[1], header->bmax[2]));
			}
		}

		void TestDebugSegments(TestContext& t)
		{
			t.Section("nav: debug draw geometry");
			Chain chain;
			if (!t.Check(BuildChain(t, { "lvl1", "test", "example", "test_interior" }, chain), "fixtures load"))
				return;
			WorldNavData data;
			BakeReport report;
			WorldNav nav;
			std::string error;
			if (!t.Check(BakeWorldNav(chain.levels, BakeSettings{}, data, report) && nav.Load(data, error), "chain bakes and loads: %s", error.c_str()))
				return;

			std::vector<DebugSegment> world;
			CollectDebugSegments(nav, -1, glm::vec3(0.0f), 0.0f, world);
			auto isEdge = [](const DebugSegment& s) { return s.kind == DebugKind::Edge || s.kind == DebugKind::Border; };
			size_t worldEdges = 0;
			for (const auto& segment : world)
				worldEdges += isEdge(segment) ? 1 : 0;

			size_t edgeSum = 0;
			for (size_t level = 0; level < chain.levels.size(); ++level)
			{
				const char* name = chain.levels[level].name.c_str();
				std::vector<DebugSegment> segments;
				CollectDebugSegments(nav, static_cast<int>(level), glm::vec3(0.0f), 0.0f, segments);
				glm::vec3 low, high;
				LevelBounds(chain.levels[level].nav, low, high);
				size_t edges = 0, borders = 0, starts = 0, ends = 0, outside = 0;
				for (const auto& segment : segments)
				{
					edges += segment.kind == DebugKind::Edge ? 1 : 0;
					borders += segment.kind == DebugKind::Border ? 1 : 0;
					starts += segment.kind == DebugKind::LinkStart ? 1 : 0;
					ends += segment.kind == DebugKind::LinkEnd ? 1 : 0;
					for (const glm::vec3& p : { segment.a, segment.b })
						if (p.x < low.x - 0.01f || p.x > high.x + 0.01f || p.z < low.z - 0.01f || p.z > high.z + 0.01f || p.y < low.y - 0.1f || p.y > high.y + 4.0f)
							++outside;
				}
				size_t expectedStarts = 0, expectedEnds = 0;
				for (const WorldLink& link : data.links)
				{
					expectedStarts += link.fromLevel == level ? 1 : 0;
					expectedEnds += link.toLevel == level && link.linked ? 1 : 0;
				}
				edgeSum += edges + borders;
				t.Check(edges > 0 && borders > 0, "%s: draws polygon edges (%zu) and surface borders (%zu)", name, edges, borders);
				t.Check(outside == 0, "%s: every point is in the level's own local bounds (%zu outside)", name, outside);
				t.Check(starts == expectedStarts * 5 && ends == expectedEnds * 5, "%s: one marker per portal end (%zu starts, %zu landings)", name, starts / 5, ends / 5);
			}
			t.Check(edgeSum == worldEdges, "the levels' edges add up to the whole world's (%zu of %zu)", edgeSum, worldEdges);

			std::vector<DebugSegment> none;
			CollectDebugSegments(nav, 99, glm::vec3(0.0f), 0.0f, none);
			t.Check(none.empty(), "an unknown level draws nothing");

			// Range limit: edges need an end inside the radius, markers always stay.
			std::vector<DebugSegment> all, near;
			const glm::vec3 centre = chain.east[0];
			CollectDebugSegments(nav, 0, centre, 0.0f, all);
			CollectDebugSegments(nav, 0, centre, 15.0f, near);
			size_t allEdges = 0, nearEdges = 0, allMarkers = 0, nearMarkers = 0, farEdges = 0;
			for (const auto& segment : all)
				(isEdge(segment) ? allEdges : allMarkers) += 1;
			for (const auto& segment : near)
			{
				(isEdge(segment) ? nearEdges : nearMarkers) += 1;
				if (isEdge(segment) && std::hypot(segment.a.x - centre.x, segment.a.z - centre.z) > 15.0f && std::hypot(segment.b.x - centre.x, segment.b.z - centre.z) > 15.0f)
					++farEdges;
			}
			t.Check(nearEdges > 0 && nearEdges < allEdges && farEdges == 0 && nearMarkers == allMarkers, "a 15 m range keeps %zu of %zu edges and every portal marker", nearEdges, allEdges);
		}
	}

	void RunNavTests(TestContext& context)
	{
		TestParseAndRewrite(context);
		TestTranslationRoundTrip(context);
		TestChainBake(context);
		TestLevelsDoNotStitch(context);
		TestBakeValidation(context);
		TestStaleCheck(context);
		TestDebugSegments(context);
	}
}

#endif

#include "WorldNavBake.h"

#include "NavTileData.h"
#include "WorldNav.h"

#include <Navigation/Detour/DetourAlloc.h>
#include <Navigation/Detour/DetourNavMeshQuery.h>

#include <entt/container/dense_map.hpp>

#include <glm/common.hpp>

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <memory>

namespace sim::nav
{
	namespace
	{
		using SlotKey = uint64_t;
		SlotKey Key(int x, int y) { return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) | static_cast<uint32_t>(y); }
		// Tile coordinates fit 28 bits by a wide margin (a tile is 6.4 m).
		SlotKey Key(int x, int y, int layer)
		{
			return (static_cast<uint64_t>(static_cast<uint32_t>(x) & 0x0FFFFFFFu) << 36) | (static_cast<uint64_t>(static_cast<uint32_t>(y) & 0x0FFFFFFFu) << 8) |
				static_cast<uint64_t>(layer & 0xff);
		}

		// Detour's neighbour directions (dtNavMesh::getNeighbourTilesAt).
		constexpr int DirX[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
		constexpr int DirY[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
		// Border sides that can carry tile-to-tile portal edges, with their neighbour offsets.
		constexpr int BorderSides[4] = { 0, 2, 4, 6 };

		struct MeshPoint
		{
			glm::vec3 local{ 0.0f };
			int tile = -1; // index into LevelWork::tiles
			int tileX = 0;
			int tileY = 0;
		};

		struct LevelWork
		{
			const LevelInput* input = nullptr;
			std::vector<TileData> tiles;
			entt::dense_map<SlotKey, int> tileIndex; // (x, y, layer) -> tiles index
			glm::vec3 orig{ 0.0f };
			dtNavMesh* mesh = nullptr;
			dtNavMeshQuery* query = nullptr;
			bool placed = false;
			int shiftX = 0;
			int shiftY = 0;

			LevelWork() = default;
			LevelWork(const LevelWork&) = delete;
			LevelWork& operator=(const LevelWork&) = delete;
			~LevelWork()
			{
				dtFreeNavMeshQuery(query);
				dtFreeNavMesh(mesh);
			}
		};

		struct LinkWork
		{
			std::string name;
			int from = 0;
			int to = 0;
			MeshPoint start; // in `from`
			MeshPoint end;   // in `to`
			bool satisfied = false;
			bool bidirectional = false;
		};

		std::string Fmt(const char* format, ...)
		{
			char buffer[1024];
			va_list args;
			va_start(args, format);
			std::vsnprintf(buffer, sizeof(buffer), format, args);
			va_end(args);
			return buffer;
		}

		bool BuildLevelMesh(LevelWork& level, std::string& error)
		{
			level.mesh = dtAllocNavMesh();
			dtNavMeshParams params = level.input->nav.params;
			params.maxTiles = std::max(params.maxTiles, static_cast<int>(level.tiles.size()));
			if (!level.mesh || dtStatusFailed(level.mesh->init(&params)))
			{
				error = "cannot build its mesh";
				return false;
			}
			for (const std::vector<uint8_t>& blob : level.input->nav.tiles)
			{
				unsigned char* copy = static_cast<unsigned char*>(dtAlloc(static_cast<int>(blob.size()), DT_ALLOC_PERM));
				std::memcpy(copy, blob.data(), blob.size());
				if (dtStatusFailed(level.mesh->addTile(copy, static_cast<int>(blob.size()), DT_TILE_FREE_DATA, 0, nullptr)))
				{
					dtFree(copy);
					error = "a tile was refused by Detour";
					return false;
				}
			}
			level.query = dtAllocNavMeshQuery();
			if (!level.query || dtStatusFailed(level.query->init(level.mesh, 2048)))
			{
				error = "cannot build its query";
				return false;
			}
			return true;
		}

		bool Snap(const LevelWork& level, const glm::vec3& position, const BakeSettings& settings, MeshPoint& out)
		{
			SimQueryFilter filter;
			dtPolyRef ref = 0;
			float nearest[3];
			if (dtStatusFailed(level.query->findNearestPoly(&position.x, &settings.projectExtents.x, &filter, &ref, nearest)) || !ref)
				return false;
			const dtMeshTile* tile = nullptr;
			const dtPoly* poly = nullptr;
			level.mesh->getTileAndPolyByRef(ref, &tile, &poly);
			const auto found = level.tileIndex.find(Key(tile->header->x, tile->header->y, tile->header->layer));
			if (found == level.tileIndex.end())
				return false;
			out.tile = found->second;
			out.tileX = tile->header->x;
			out.tileY = tile->header->y;

			// Keep the point strictly inside its tile slot, so Detour classifies it into the same slot.
			const float width = level.input->nav.params.tileWidth;
			const float minX = level.orig.x + out.tileX * width, minZ = level.orig.z + out.tileY * width;
			constexpr float inset = 0.01f;
			out.local = { std::clamp(nearest[0], minX + inset, minX + width - inset), nearest[1], std::clamp(nearest[2], minZ + inset, minZ + width - inset) };
			return true;
		}

		int WorldTileX(const LevelWork& level, const MeshPoint& point) { return point.tileX + level.shiftX; }
		int WorldTileY(const LevelWork& level, const MeshPoint& point) { return point.tileY + level.shiftY; }

		glm::vec3 LevelOffset(const LevelWork& level, float width)
		{
			return { level.shiftX * width - level.orig.x, 0.0f, level.shiftY * width - level.orig.z };
		}
	}

	std::string BakeReport::Text() const
	{
		std::string out = Fmt("world nav bake: %s, %zu errors, %zu warnings\n", Ok() ? "OK" : "FAILED", errors.size(), warnings.size());
		for (const std::string& line : errors)
			out += "  ERROR " + line + "\n";
		for (const std::string& line : warnings)
			out += "  WARNING " + line + "\n";
		for (const std::string& line : notes)
			out += "  " + line + "\n";
		return out;
	}

	LevelNavData CaptureLevelNav(const dtNavMesh& mesh, uint32_t sourceVersion)
	{
		LevelNavData result;
		result.params = *mesh.getParams();
		result.sourceVersion = sourceVersion;
		for (int i = 0; i < mesh.getMaxTiles(); ++i)
		{
			const dtMeshTile* tile = mesh.getTile(i);
			if (tile && tile->header && tile->data && tile->dataSize > 0)
				result.tiles.emplace_back(tile->data, tile->data + tile->dataSize);
		}
		return result;
	}

	bool BakeWorldNav(const std::vector<LevelInput>& inputs, const BakeSettings& settings, WorldNavData& out, BakeReport& report)
	{
		out = WorldNavData{};
		if (inputs.empty())
		{
			report.errors.push_back("the manifest lists no levels");
			return false;
		}

		// ---- 1. Parse tiles, check the shared profile.
		std::vector<std::unique_ptr<LevelWork>> levels;
		const float width = inputs[0].nav.params.tileWidth;
		float walkableHeight = 0.0f, walkableRadius = 0.0f, walkableClimb = 0.0f;
		for (const LevelInput& input : inputs)
		{
			auto level = std::make_unique<LevelWork>();
			level->input = &input;
			level->orig = { input.nav.params.orig[0], input.nav.params.orig[1], input.nav.params.orig[2] };
			if (input.nav.tiles.empty())
				report.errors.push_back(input.name + ": has no nav tiles");
			if (std::fabs(input.nav.params.tileWidth - width) > 1e-4f || std::fabs(input.nav.params.tileHeight - width) > 1e-4f)
				report.errors.push_back(Fmt("%s: tile size %.3f differs from %s's %.3f", input.name.c_str(), input.nav.params.tileWidth, inputs[0].name.c_str(), width));
			for (size_t i = 0; i < input.nav.tiles.size(); ++i)
			{
				TileData tile;
				std::string error;
				if (!TileData::Parse(input.nav.tiles[i].data(), input.nav.tiles[i].size(), tile, error))
				{
					report.errors.push_back(Fmt("%s: tile %zu: %s", input.name.c_str(), i, error.c_str()));
					continue;
				}
				if (walkableHeight == 0.0f)
				{
					walkableHeight = tile.header.walkableHeight;
					walkableRadius = tile.header.walkableRadius;
					walkableClimb = tile.header.walkableClimb;
				}
				else if (std::fabs(tile.header.walkableHeight - walkableHeight) > 1e-3f || std::fabs(tile.header.walkableRadius - walkableRadius) > 1e-3f ||
					std::fabs(tile.header.walkableClimb - walkableClimb) > 1e-3f)
					report.errors.push_back(input.name + ": agent profile (height, radius, climb) differs from the other levels (decision D4: one universal profile)");
				level->tileIndex[Key(tile.header.x, tile.header.y, tile.header.layer)] = static_cast<int>(level->tiles.size());
				level->tiles.push_back(std::move(tile));
			}
			for (const auto& other : levels)
				if (other->input->name == input.name)
					report.errors.push_back("level " + input.name + " is listed twice");
			std::string error;
			if (!level->tiles.empty() && !BuildLevelMesh(*level, error))
				report.errors.push_back(input.name + ": " + error);
			levels.push_back(std::move(level));
		}
		if (!report.Ok())
			return false;

		auto findLevel = [&](const std::string& name) -> int
		{
			for (size_t i = 0; i < inputs.size(); ++i)
				if (inputs[i].name == name)
					return static_cast<int>(i);
			return -1;
		};

		// ---- 2. Snap portals and spawn points.
		std::vector<LinkWork> links;
		for (size_t from = 0; from < inputs.size(); ++from)
			for (const PortalInput& portal : inputs[from].portals)
			{
				const std::string where = inputs[from].name + " portal '" + portal.name + "'";
				const int to = findLevel(portal.targetLevel);
				if (to < 0)
				{
					report.errors.push_back(where + " targets level '" + portal.targetLevel + "', which is not in the manifest");
					continue;
				}
				if (to == static_cast<int>(from))
				{
					report.errors.push_back(where + " targets its own level");
					continue;
				}
				const SpawnInput* spawn = nullptr;
				for (const SpawnInput& candidate : inputs[to].spawns)
					if (candidate.name == portal.targetSpawn)
						spawn = &candidate;
				if (!spawn)
				{
					report.errors.push_back(where + " names spawn point '" + portal.targetSpawn + "', which " + inputs[to].name + " does not have");
					continue;
				}
				LinkWork link;
				link.name = inputs[from].name + ":" + portal.name;
				link.from = static_cast<int>(from);
				link.to = to;
				if (!Snap(*levels[from], portal.position, settings, link.start))
				{
					report.errors.push_back(Fmt("%s at (%.1f %.1f %.1f) does not snap to the nav mesh", where.c_str(), portal.position.x, portal.position.y, portal.position.z));
					continue;
				}
				if (!Snap(*levels[to], spawn->position, settings, link.end))
				{
					report.errors.push_back(Fmt("%s spawn point '%s' at (%.1f %.1f %.1f) does not snap to the nav mesh", inputs[to].name.c_str(), spawn->name.c_str(),
						spawn->position.x, spawn->position.y, spawn->position.z));
					continue;
				}
				links.push_back(link);
			}
		if (!report.Ok())
			return false;

		// ---- 3. Layout. Each level's tile bounding box keeps at least one empty tile to every other level, so tiles of
		// different levels never touch. A level is placed as close as possible to the spot where its portal end would
		// sit on its partner's, which keeps cross-level searches short.
		struct Box
		{
			int minX = 0, minY = 0, maxX = -1, maxY = -1;
		};
		std::vector<Box> boxes(levels.size());
		for (size_t i = 0; i < levels.size(); ++i)
		{
			Box& box = boxes[i];
			bool first = true;
			for (const TileData& tile : levels[i]->tiles)
			{
				if (first || tile.header.x < box.minX) box.minX = tile.header.x;
				if (first || tile.header.y < box.minY) box.minY = tile.header.y;
				if (first || tile.header.x > box.maxX) box.maxX = tile.header.x;
				if (first || tile.header.y > box.maxY) box.maxY = tile.header.y;
				first = false;
			}
		}
		auto fits = [&](int index, int shiftX, int shiftY)
		{
			const Box& box = boxes[index];
			for (size_t other = 0; other < levels.size(); ++other)
			{
				if (!levels[other]->placed)
					continue;
				const Box& o = boxes[other];
				const int ox = levels[other]->shiftX, oy = levels[other]->shiftY;
				// One empty tile between the boxes.
				if (box.minX + shiftX <= o.maxX + ox + 1 && box.maxX + shiftX >= o.minX + ox - 1 && box.minY + shiftY <= o.maxY + oy + 1 &&
					box.maxY + shiftY >= o.minY + oy - 1)
					return false;
			}
			return true;
		};
		// Closest free shift to `ideal`, searched in growing square rings.
		auto placeNear = [&](int index, int idealX, int idealY)
		{
			for (int radius = 0; radius < 100000; ++radius)
				for (int dy = -radius; dy <= radius; ++dy)
					for (int dx = -radius; dx <= radius; ++dx)
					{
						if (std::max(std::abs(dx), std::abs(dy)) != radius)
							continue;
						if (fits(index, idealX + dx, idealY + dy))
						{
							levels[index]->placed = true;
							levels[index]->shiftX = idealX + dx;
							levels[index]->shiftY = idealY + dy;
							return;
						}
					}
		};

		levels[0]->placed = true;
		std::vector<int> frontier{ 0 };
		for (size_t cursor = 0; cursor < frontier.size(); ++cursor)
		{
			const int parent = frontier[cursor];
			const LevelWork& p = *levels[parent];
			for (int child = 0; child < static_cast<int>(levels.size()); ++child)
			{
				if (levels[child]->placed)
					continue;
				const LinkWork* anchor = nullptr;
				for (const LinkWork& link : links)
					if (!anchor && ((link.from == parent && link.to == child) || (link.from == child && link.to == parent)))
						anchor = &link;
				if (!anchor)
					continue;
				if (anchor->from == parent)
					placeNear(child, WorldTileX(p, anchor->start) - anchor->end.tileX, WorldTileY(p, anchor->start) - anchor->end.tileY);
				else
					placeNear(child, WorldTileX(p, anchor->end) - anchor->start.tileX, WorldTileY(p, anchor->end) - anchor->start.tileY);
				frontier.push_back(child);
			}
		}
		for (const auto& level : levels)
			if (!level->placed)
				report.errors.push_back(level->input->name + " has no portal path to " + inputs[0].name + " (not connected)");
		if (!report.Ok())
			return false;

		// ---- 4. Translate, link, mask.
		for (size_t index = 0; index < levels.size(); ++index)
		{
			LevelWork& level = *levels[index];
			const glm::vec3 offset = LevelOffset(level, width);
			for (TileData& tile : level.tiles)
			{
				tile.Translate(offset, tile.header.x + level.shiftX, tile.header.y + level.shiftY);
				tile.header.userId = static_cast<unsigned int>(index);
			}
		}

		// Every connected pair needs a way in both directions: reuse a portal backwards when only one direction exists.
		for (LinkWork& link : links)
		{
			bool reverse = false, alreadyBidirectional = false;
			for (const LinkWork& other : links)
			{
				reverse |= other.from == link.to && other.to == link.from;
				alreadyBidirectional |= other.bidirectional && other.from == link.from && other.to == link.to;
			}
			if (!reverse && !alreadyBidirectional)
			{
				link.bidirectional = true;
				report.warnings.push_back(Fmt("no portal leads from %s back to %s; %s is used in both directions by NPCs", inputs[link.to].name.c_str(),
					inputs[link.from].name.c_str(), link.name.c_str()));
			}
		}

		if (settings.createLinks)
			for (LinkWork& link : links)
			{
				LevelWork& a = *levels[link.from];
				LevelWork& b = *levels[link.to];
				const glm::vec3 start = link.start.local + LevelOffset(a, width);
				const glm::vec3 end = link.end.local + LevelOffset(b, width);
				a.tiles[link.start.tile].AddOffMeshConnection(start, end, settings.linkRadius, link.bidirectional, DT_OFFMESH_SIDE_REMOTE, settings.linkArea,
					settings.linkFlags, static_cast<uint32_t>(&link - links.data()));
				b.tiles[link.end.tile].extraLinks += 2; // the landing link's way back for bidirectional connections, plus spare
				link.satisfied = true;
			}

		// Levels never touch after the layout, but mask any border that faces another level anyway: cheap insurance
		// against a layout change silently stitching levels together.
		entt::dense_map<SlotKey, int> occupied;
		for (size_t index = 0; index < levels.size(); ++index)
			for (const TileData& tile : levels[index]->tiles)
				occupied[Key(tile.header.x, tile.header.y)] = static_cast<int>(index);
		int masked = 0;
		for (size_t index = 0; index < levels.size(); ++index)
			for (TileData& tile : levels[index]->tiles)
				for (const int side : BorderSides)
				{
					const auto neighbour = occupied.find(Key(tile.header.x + DirX[side], tile.header.y + DirY[side]));
					if (neighbour != occupied.end() && neighbour->second != static_cast<int>(index))
						masked += tile.MaskBorder(side);
				}

		// ---- 5. Assemble and validate on the real combined mesh.
		out.tileWidth = width;
		out.walkableHeight = walkableHeight;
		out.walkableRadius = walkableRadius;
		out.walkableClimb = walkableClimb;
		for (size_t index = 0; index < levels.size(); ++index)
		{
			const LevelWork& level = *levels[index];
			WorldLevel world;
			world.name = level.input->name;
			world.sourceVersion = level.input->nav.sourceVersion;
			world.offset = LevelOffset(level, width);
			world.tileShiftX = level.shiftX;
			world.tileShiftY = level.shiftY;
			world.tileCount = static_cast<int32_t>(level.tiles.size());
			out.levels.push_back(world);
			for (const TileData& tile : level.tiles)
			{
				out.tiles.push_back({ static_cast<uint16_t>(index), tile.WriteVector() });
				out.maxPolys = std::max(out.maxPolys, static_cast<int32_t>(tile.polys.size()));
			}
		}
		out.maxTiles = static_cast<int32_t>(out.tiles.size());
		for (const LinkWork& link : links)
		{
			WorldLink world;
			world.portal = link.name;
			world.fromLevel = static_cast<uint16_t>(link.from);
			world.toLevel = static_cast<uint16_t>(link.to);
			world.start = link.start.local + out.levels[link.from].offset;
			world.end = link.end.local + out.levels[link.to].offset;
			world.linked = link.satisfied;
			world.bidirectional = link.bidirectional;
			out.links.push_back(world);
		}

		WorldNav nav;
		std::string error;
		if (!nav.Load(out, error))
		{
			report.errors.push_back("combined mesh: " + error);
			return false;
		}

		const glm::vec3 snap(0.2f, 1.0f, 0.2f);
		for (const WorldLink& link : out.links)
		{
			if (!link.linked)
				continue;
			const NavPath path = nav.FindPath(nav.Project(link.start, snap), nav.Project(link.end, snap));
			if (!path.complete || path.portalCrossings < 1)
				report.errors.push_back(Fmt("%s: the off-mesh link does not carry a path (complete %d, %d polygons, %d crossings, %.1f m)", link.portal.c_str(),
					path.complete, path.polyCount, path.portalCrossings, path.length));
		}

		// Inside each level, every portal end must reach the others (addendum 4.3: doors that cannot reach each other).
		for (size_t index = 0; index < out.levels.size(); ++index)
		{
			std::vector<std::pair<std::string, glm::vec3>> ends;
			for (const WorldLink& link : out.links)
			{
				if (link.fromLevel == index)
					ends.emplace_back(link.portal + " trigger", link.start);
				if (link.toLevel == index)
					ends.emplace_back(link.portal + " arrival", link.end);
			}
			for (size_t i = 1; i < ends.size(); ++i)
			{
				const NavPath path = nav.FindPath(nav.Project(ends[0].second, snap), nav.Project(ends[i].second, snap));
				if (!path.complete)
					report.errors.push_back(out.levels[index].name + ": " + ends[i].first + " cannot be reached from " + ends[0].first);
			}
		}

		// Every level reaches every other level through the links.
		const size_t count = out.levels.size();
		for (size_t from = 0; from < count; ++from)
		{
			std::vector<bool> seen(count, false);
			std::vector<size_t> stack{ from };
			seen[from] = true;
			while (!stack.empty())
			{
				const size_t current = stack.back();
				stack.pop_back();
				for (const WorldLink& link : out.links)
				{
					if (!link.linked)
						continue;
					size_t next = count;
					if (link.fromLevel == current)
						next = link.toLevel;
					else if (link.bidirectional && link.toLevel == current)
						next = link.fromLevel;
					if (next < count && !seen[next])
					{
						seen[next] = true;
						stack.push_back(next);
					}
				}
			}
			for (size_t to = 0; to < count; ++to)
				if (!seen[to] && settings.createLinks)
					report.errors.push_back("no route from " + out.levels[from].name + " to " + out.levels[to].name);
		}

		for (const WorldLevel& level : out.levels)
			report.notes.push_back(Fmt("%-24s %4d tiles, tile shift (%d, %d), offset (%.1f, %.1f)", level.name.c_str(), level.tileCount, level.tileShiftX, level.tileShiftY,
				level.offset.x, level.offset.z));
		int linked = 0;
		for (const WorldLink& link : out.links)
			linked += link.linked ? 1 : 0;
		report.notes.push_back(Fmt("%d of %zu portals linked, %d border edges masked between levels, %zu tiles", linked, out.links.size(), masked, out.tiles.size()));
		return report.Ok();
	}
}

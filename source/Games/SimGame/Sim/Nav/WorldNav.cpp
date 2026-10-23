#include "WorldNav.h"

#include <Navigation/Detour/DetourAlloc.h>
#include <Navigation/Detour/DetourCommon.h>

#include <glm/geometric.hpp>

#include <algorithm>
#include <cstring>

namespace sim::nav
{
	namespace
	{
		int NextPow2(int value)
		{
			int result = 1;
			while (result < value)
				result <<= 1;
			return result;
		}
	}

	WorldNav::WorldNav() = default;

	WorldNav::~WorldNav()
	{
		dtFreeNavMeshQuery(query);
		dtFreeNavMesh(mesh);
	}

	bool WorldNav::Load(const WorldNavData& source, std::string& error)
	{
		dtFreeNavMeshQuery(query);
		dtFreeNavMesh(mesh);
		query = nullptr;
		mesh = nullptr;
		data = source;

		dtNavMeshParams params{};
		params.orig[0] = params.orig[1] = params.orig[2] = 0.0f;
		params.tileWidth = data.tileWidth;
		params.tileHeight = data.tileWidth;
		params.maxTiles = NextPow2(std::max(1, data.maxTiles));
		params.maxPolys = NextPow2(std::max(1, data.maxPolys));

		mesh = dtAllocNavMesh();
		if (!mesh || dtStatusFailed(mesh->init(&params)))
		{
			error = "cannot initialise the world nav mesh";
			return false;
		}
		for (size_t i = 0; i < data.tiles.size(); ++i)
		{
			const std::vector<uint8_t>& blob = data.tiles[i].data;
			unsigned char* copy = static_cast<unsigned char*>(dtAlloc(static_cast<int>(blob.size()), DT_ALLOC_PERM));
			std::memcpy(copy, blob.data(), blob.size());
			const dtStatus status = mesh->addTile(copy, static_cast<int>(blob.size()), DT_TILE_FREE_DATA, 0, nullptr);
			if (dtStatusFailed(status))
			{
				dtFree(copy);
				const dtMeshHeader* header = reinterpret_cast<const dtMeshHeader*>(blob.data());
				error = "tile " + std::to_string(i) + " of level " + data.levels[data.tiles[i].level].name + " at (" + std::to_string(header->x) + ", " +
					std::to_string(header->y) + ", layer " + std::to_string(header->layer) + ") was refused by Detour (status " + std::to_string(status) + ")";
				return false;
			}
		}

		int expectedLinks = 0;
		for (const WorldLink& link : data.links)
			expectedLinks += link.linked ? 1 : 0;
		const int landed = mesh->connectRemoteOffMeshLinks();
		if (landed != expectedLinks)
		{
			error = std::to_string(expectedLinks - landed) + " of " + std::to_string(expectedLinks) + " portal links found no landing polygon";
			return false;
		}

		query = dtAllocNavMeshQuery();
		if (!query || dtStatusFailed(query->init(mesh, QueryNodes)))
		{
			error = "cannot initialise the world nav query";
			return false;
		}
		return true;
	}

	int WorldNav::LevelOfPoly(dtPolyRef poly) const
	{
		const dtMeshTile* tile = nullptr;
		const dtPoly* polygon = nullptr;
		if (!mesh || dtStatusFailed(mesh->getTileAndPolyByRef(poly, &tile, &polygon)))
			return -1;
		return static_cast<int>(tile->header->userId);
	}

	NavPoint WorldNav::Project(const glm::vec3& navPosition, const glm::vec3& extents) const
	{
		NavPoint result;
		if (!query)
			return result;
		float nearest[3];
		dtPolyRef ref = 0;
		if (dtStatusSucceed(query->findNearestPoly(&navPosition.x, &extents.x, &filter, &ref, nearest)) && ref)
		{
			result.poly = ref;
			result.position = { nearest[0], nearest[1], nearest[2] };
		}
		return result;
	}

	NavPath WorldNav::FindPath(const NavPoint& start, const NavPoint& end, int maxPolys) const
	{
		NavPath empty;
		if (!query || !start.IsValid() || !end.IsValid())
			return empty;
		std::vector<dtPolyRef> corridor(maxPolys);
		int count = 0;
		const dtStatus status = query->findPath(start.poly, end.poly, &start.position.x, &end.position.x, &filter, corridor.data(), &count, maxPolys);
		if (dtStatusFailed(status) || count == 0)
			return empty;
		corridor.resize(count);
		const bool complete = corridor.back() == end.poly && !(status & DT_PARTIAL_RESULT);
		return MakePath(start.position, end.position, corridor, complete);
	}

	NavPath WorldNav::MakePath(const glm::vec3& start, const glm::vec3& end, const std::vector<dtPolyRef>& corridor, bool complete) const
	{
		NavPath path;
		path.complete = complete;
		path.polyCount = static_cast<int>(corridor.size());
		path.corridor = corridor;
		if (corridor.empty())
			return path;
		for (const dtPolyRef ref : corridor)
		{
			const dtMeshTile* tile = nullptr;
			const dtPoly* poly = nullptr;
			if (dtStatusSucceed(mesh->getTileAndPolyByRef(ref, &tile, &poly)) && poly->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)
				++path.portalCrossings;
		}

		glm::vec3 target = end;
		if (!complete)
		{
			// Aim for the closest point on the last reachable polygon.
			float closest[3];
			if (dtStatusSucceed(query->closestPointOnPoly(corridor.back(), &end.x, closest, nullptr)))
				target = { closest[0], closest[1], closest[2] };
		}

		const int maxCorners = static_cast<int>(corridor.size()) * 2 + 8;
		std::vector<float> corners(maxCorners * 3);
		std::vector<unsigned char> flags(maxCorners);
		std::vector<dtPolyRef> refs(maxCorners);
		int cornerCount = 0;
		query->findStraightPath(&start.x, &target.x, corridor.data(), static_cast<int>(corridor.size()), corners.data(), flags.data(), refs.data(),
			&cornerCount, maxCorners, 0);

		for (int i = 0; i < cornerCount; ++i)
		{
			const glm::vec3 point(corners[i * 3], corners[i * 3 + 1], corners[i * 3 + 2]);
			if (!path.points.empty())
				path.length += glm::distance(path.points.back(), point);
			path.points.push_back(point);
			// Detour gives the end corner no polygon; it lies on the corridor's last one.
			const dtPolyRef ref = refs[i] ? refs[i] : (i == cornerCount - 1 ? corridor.back() : 0);
			int level = ref ? LevelOfPoly(ref) : -1;
			if (level < 0)
				level = path.pointLevels.empty() ? LevelOfPoly(corridor.front()) : path.pointLevels.back();
			path.pointLevels.push_back(level);
		}
		return path;
	}

	// ---- PathService ------------------------------------------------------------------------------------------------

	PathService::PathService(WorldNav& nav) : nav(nav)
	{
		query = dtAllocNavMeshQuery();
		if (query && nav.Mesh())
			query->init(nav.Mesh(), WorldNav::QueryNodes);
	}

	PathService::~PathService()
	{
		dtFreeNavMeshQuery(query);
	}

	PathService::RequestId PathService::Request(const NavPoint& start, const NavPoint& end)
	{
		const RequestId id = nextId++;
		queue.push_back({ id, start, end });
		return id;
	}

	void PathService::Cancel(RequestId id)
	{
		for (auto it = queue.begin(); it != queue.end(); ++it)
			if (it->id == id)
			{
				if (it == queue.begin())
					active = false;
				queue.erase(it);
				return;
			}
	}

	void PathService::Update(int iterations)
	{
		while (iterations > 0 && !queue.empty())
		{
			QueuedRequest& request = queue.front();
			if (!active)
			{
				if (!request.start.IsValid() || !request.end.IsValid() ||
					dtStatusFailed(query->initSlicedFindPath(request.start.poly, request.end.poly, &request.start.position.x, &request.end.position.x, &filter)))
				{
					done.push_back({ request.id, {}, true });
					queue.pop_front();
					continue;
				}
				active = true;
			}

			int used = 0;
			const dtStatus status = query->updateSlicedFindPath(iterations, &used);
			iterations -= std::max(used, 1);
			if (dtStatusInProgress(status))
				continue;

			std::vector<dtPolyRef> corridor(4096);
			int count = 0;
			const dtStatus final = query->finalizeSlicedFindPath(corridor.data(), &count, static_cast<int>(corridor.size()));
			Result result;
			result.id = request.id;
			if (dtStatusFailed(status) || dtStatusFailed(final) || count == 0)
				result.failed = true;
			else
			{
				corridor.resize(count);
				const bool complete = corridor.back() == request.end.poly && !(final & DT_PARTIAL_RESULT);
				result.path = nav.MakePath(request.start.position, request.end.position, corridor, complete);
			}
			done.push_back(std::move(result));
			queue.pop_front();
			active = false;
		}
	}

	std::vector<PathService::Result> PathService::TakeResults()
	{
		std::vector<Result> results;
		results.swap(done);
		std::sort(results.begin(), results.end(), [](const Result& a, const Result& b) { return a.id < b.id; });
		return results;
	}
}

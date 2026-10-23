#pragma once

// The sim's own world navigation (addendum section 4.2): one combined dtNavMesh for all levels, owned and queried
// only by the sim thread. The engine's static NavigationSystem is never touched; physical bodies keep using it.
//
// Positions are in nav space. A level is derived from the tile a polygon belongs to (tile userId = level index).
// Level-local position = nav position - level offset.

#include "WorldNavData.h"

#include <Navigation/Detour/DetourNavMeshQuery.h>

#include <deque>
#include <memory>

namespace sim::nav
{
	// Query filter for sim agents. Off-mesh connections are level portals; their cost is the straight distance
	// between the two ends (Detour's default), which keeps the A* heuristic admissible across levels.
	class SimQueryFilter : public dtQueryFilter
	{
	public:
		SimQueryFilter() { setIncludeFlags(0xffff); setExcludeFlags(0); }
	};

	struct NavPoint
	{
		glm::vec3 position{ 0.0f };
		dtPolyRef poly = 0;
		bool IsValid() const { return poly != 0; }
	};

	struct NavPath
	{
		std::vector<glm::vec3> points; // straight path corners in nav space, start and end included
		std::vector<int> pointLevels;  // level of each corner
		std::vector<dtPolyRef> corridor; // polygons from start to end (a body's crowd corridor is built from these)
		bool complete = false;         // false: the end was not reachable, the path ends at the closest reachable point
		float length = 0.0f;           // metres along the corners
		int polyCount = 0;
		int portalCrossings = 0;       // off-mesh connections (level portals) in the corridor
	};

	class WorldNav
	{
	public:
		static constexpr int QueryNodes = 65535; // Detour's maximum; cross-level routes need far more than 2048

		WorldNav();
		~WorldNav();
		WorldNav(const WorldNav&) = delete;
		WorldNav& operator=(const WorldNav&) = delete;

		bool Load(const WorldNavData& data, std::string& error);
		bool IsLoaded() const { return mesh != nullptr; }

		const WorldNavData& Data() const { return data; }
		const dtNavMesh* Mesh() const { return mesh; }
		dtNavMeshQuery& Query() { return *query; }

		int LevelOfPoly(dtPolyRef poly) const;
		glm::vec3 ToNav(int level, const glm::vec3& local) const { return local + data.levels[level].offset; }
		glm::vec3 ToLocal(int level, const glm::vec3& nav) const { return nav - data.levels[level].offset; }

		// Nearest point on the mesh within `extents` (half size). Invalid NavPoint if nothing is in range.
		NavPoint Project(const glm::vec3& navPosition, const glm::vec3& extents = { 2.0f, 4.0f, 2.0f }) const;

		// One-shot path (bake validation, tests, tools). Agents use PathService.
		NavPath FindPath(const NavPoint& start, const NavPoint& end, int maxPolys = 4096) const;

		// Builds the corner path from a polygon corridor (used by FindPath and PathService).
		NavPath MakePath(const glm::vec3& start, const glm::vec3& end, const std::vector<dtPolyRef>& corridor, bool complete) const;

	private:
		WorldNavData data;
		dtNavMesh* mesh = nullptr;
		dtNavMeshQuery* query = nullptr;
		SimQueryFilter filter;
	};

	// Spreads path searches over ticks within an iteration budget (addendum section 4.2). Results are handed out in
	// request order at one point in the tick, so timing never changes outcomes.
	class PathService
	{
	public:
		using RequestId = uint32_t;

		explicit PathService(WorldNav& nav);
		~PathService();

		RequestId Request(const NavPoint& start, const NavPoint& end);
		void Cancel(RequestId id);

		// Runs up to `iterations` A* iterations across queued requests (oldest first).
		void Update(int iterations);

		struct Result
		{
			RequestId id = 0;
			NavPath path;
			bool failed = false;
		};
		// Finished results in request order.
		std::vector<Result> TakeResults();

		size_t Pending() const { return queue.size(); }

	private:
		struct QueuedRequest
		{
			RequestId id = 0;
			NavPoint start, end;
		};

		WorldNav& nav;
		dtNavMeshQuery* query = nullptr; // a dedicated query: sliced state lives in it
		SimQueryFilter filter;
		std::deque<QueuedRequest> queue;
		bool active = false;
		RequestId nextId = 1;
		std::vector<Result> done;
	};
}

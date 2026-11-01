#pragma once

#define RC_MAX_LAYERS_DEF 255

#include "Recast/Recast.h"
#include "Detour/DetourNavMesh.h"
#include "Detour/DetourNavMeshBuilder.h"
#include "Detour/DetourTileCache.h"
#include "Detour/DetourTileCacheBuilder.h"
#include "Detour/DetourCrowd.h"

#include "CrowdAgent.h"

#include <mutex>


#include <vector>

#include "Detour/DetourNavMeshQuery.h"

using namespace std;

class CustomFilter : public dtQueryFilter
{
    bool passFilter(const dtPolyRef /*ref*/, const dtMeshTile* /*tile*/, const dtPoly* poly) const override
    {
        return true;// poly->getArea() == DT_TILECACHE_WALKABLE_AREA; // Match area set during navmesh build
    }
};

// One navmesh polygon reached by NavigationSystem::BuildReachTree.
struct NavReachNode
{
    glm::vec3 center{};  // average of the polygon's vertices (the origin itself for the root)
    glm::vec3 portal{};  // midpoint of the edge shared with `parent`: where a path toward the origin leaves this polygon
    float cost = 0.0f;   // path length from the origin, along polygon edges
    int parent = -1;     // index of the next polygon toward the origin; -1 for the root
};

class NavigationSystem
{

private:


    static inline dtTileCache* tileCache = nullptr;
    static inline dtTileCacheAlloc* talloc = nullptr;
    static inline dtTileCacheCompressor* tcomp = nullptr;
    static inline dtTileCacheMeshProcess* tmproc = nullptr;

    static std::recursive_mutex mainLock;

    static vector<dtObstacleRef> obstacles;

    static bool HasLineOfSight(const vec3& pointA, const vec3& pointB);

public:

    static inline dtNavMesh* navMesh = nullptr;
    static inline dtCrowd* g_crowd = nullptr;

    static void DestroyNavData();

    static bool DebugDrawNavMeshEnabled;

    static void DestroyAllObstacles()
    {
        // RemoveObstacle erases from obstacles, so walk a copy
        std::vector<dtObstacleRef> toRemove;
        {
            std::lock_guard<std::recursive_mutex> lock(mainLock);
            toRemove = obstacles;
        }
        for (auto obstacle : toRemove)
        {
            RemoveObstacle(obstacle);
        }
    }

    static void Update();

    static void GenerateNavData();

    static void InitCrowd(int maxAgents = 128, float maxAgentRadius = 0.6f);

  
    // DrawNavmesh renders every edge in the navigation mesh using DebugDraw::Line.
// It uses the dtNavMesh's internal tile storage to iterate over all polygons.
    static void DrawNavmesh();

    static void RemoveObstacle(dtObstacleRef obstacleRef);

    static dtObstacleRef CreateObstacleBox(const glm::vec3& min, const glm::vec3& max);

    static std::vector<glm::vec3> FindSimplePath(glm::vec3 start, glm::vec3 target, float acceptanceRadius = 0.0f, bool* outReached = nullptr, bool allowPartialPath = false);

    static std::vector<glm::vec3> FindFleePath(
        const glm::vec3& start,
        const glm::vec3& threatPos,
        float desiredDist = 25,
        int maxCandidates = 20,
        float npcSpeed = 4.0f,
        float playerSpeed = 5.0f);

    static std::vector<glm::vec3> FindFleePathSimple(
        const glm::vec3& start,
        const glm::vec3& threatPos,
        float desiredDist = 25,
        int maxCandidates = 20,
        float npcSpeed = 4.0f,
        float playerSpeed = 5.0f);

    static bool ProjectPointToNavMesh(const glm::vec3& sample, const glm::vec3& navSnapTolerance, glm::vec3& navPos);

    // Shortest-path tree over the navmesh rooted at `origin` (Dijkstra,
    // dtNavMeshQuery::findPolysAroundCircle): every polygon reachable without
    // leaving a circle of `radius`, at most `maxNodes`. Following `parent` from
    // any node walks the shortest route to the origin, so the tree tells which
    // way (which doorway, which corridor) each place reaches the origin by.
    // out[0] is the root. Thread safe (same lock as the path queries).
    static bool BuildReachTree(const glm::vec3& origin, float radius, int maxNodes, std::vector<NavReachNode>& out);


};


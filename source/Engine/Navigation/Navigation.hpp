#pragma once

#define RC_MAX_LAYERS_DEF 255

#include "Recast/Recast.h"
#include "Detour/DetourNavMesh.h"
#include "Detour/DetourNavMeshBuilder.h"
#include "Detour/DetourTileCache.h"
#include "Detour/DetourTileCacheBuilder.h"

#include <mutex>




#include "Detour/DetourNavMeshQuery.h"


class NavigationSystem
{

private:

	static dtNavMesh* navMesh;
	static dtTileCache* tileCache;

    static std::recursive_mutex mainLock;

    static vector<dtObstacleRef> obstacles;

public:

    static void DestroyNavData();

    static bool DebugDrawNavMeshEnabled;

    static void DestroyAllObstacles()
    {
        for (auto obstacle : obstacles)
        {
            RemoveObstacle(obstacle);
        }
    }

    static void Update();

    static void GenerateNavData();

    // DrawNavmesh renders every edge in the navigation mesh using DebugDraw::Line.
// It uses the dtNavMesh's internal tile storage to iterate over all polygons.
    static void DrawNavmesh();

    static void RemoveObstacle(dtObstacleRef obstacleRef);

    static dtObstacleRef CreateObstacleBox(const glm::vec3& min, const glm::vec3& max);

    // =====================================================================
// New function: FindSimplePath
// Computes a simple path from a start to a target position. The path is
// returned as an array of 3D points (world coordinates) in outPath.
// Returns true if a valid path was found.
// =====================================================================
    static std::vector<glm::vec3> FindSimplePath(glm::vec3 start, glm::vec3 target);

};
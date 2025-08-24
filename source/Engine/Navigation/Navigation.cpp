#include <vector>
#include <mutex>
#include "../glm.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include "Navigation.hpp"

#include "../DebugDraw.hpp"

#include "../Level.hpp"

#include "../Physics.h"

#include "../Time.hpp"

#include "../LoadingScreen/LoadingScreenSystem.h"

// Recast and Detour includes

bool NavigationSystem::DebugDrawNavMeshEnabled = false;

// Static member initialization
std::recursive_mutex NavigationSystem::mainLock;
std::vector<dtObstacleRef> NavigationSystem::obstacles;


// Custom compressor (simplified, no real compression)
struct FastLZCompressor : public dtTileCacheCompressor {
    int maxCompressedSize(const int size) override { return static_cast<int>(size * 1.05f); }
    dtStatus compress(const unsigned char* buffer, const int bufferSize,
        unsigned char* compressed, const int maxCompressedSize, int* compressedSize) override {
        if (bufferSize > maxCompressedSize) return DT_BUFFER_TOO_SMALL;
        memcpy(compressed, buffer, bufferSize);
        *compressedSize = bufferSize;
        return DT_SUCCESS;
    }
    dtStatus decompress(const unsigned char* compressed, const int compressedSize,
        unsigned char* buffer, const int maxBufferSize, int* bufferSize) override {
        if (compressedSize > maxBufferSize) return DT_BUFFER_TOO_SMALL;
        memcpy(buffer, compressed, compressedSize);
        *bufferSize = compressedSize;
        return DT_SUCCESS;
    }
};

dtTileCacheAlloc* talloc = nullptr; // 1MB
FastLZCompressor* tcomp = nullptr;

void NavigationSystem::DestroyNavData()
{


	std::lock_guard<std::recursive_mutex> lock(mainLock);

    DestroyAllObstacles();

	if (tileCache)
		dtFreeTileCache(tileCache);

	if (navMesh)
		dtFreeNavMesh(navMesh);


	if (talloc)
		delete talloc;

	if (tcomp)
		delete tcomp;

    if (tmproc)
        delete tmproc;

    
}



void NavigationSystem::Update()
{
    if (tileCache == nullptr) return;

    std::lock_guard<std::recursive_mutex> lock(mainLock);

    bool upToDate = false;
    auto status = tileCache->update(Time::DeltaTimeF, navMesh, &upToDate);

    if (dtStatusFailed(status))
    {
        std::printf("Tile cache update failed. Status: %u\n", status);
    }
    else if (!upToDate)
    {
        //std::printf("Tile cache still processing updates.\n");
    }
}

// Set areas -> flags when the cache creates Detour data
struct MeshProcess : public dtTileCacheMeshProcess {
    unsigned short walkFlag = 1; // SAMPLE_POLYFLAGS_WALK
    void process(dtNavMeshCreateParams* params, unsigned char* polyAreas, unsigned short* polyFlags) override {
        for (int i = 0; i < params->polyCount; ++i) {
            if (polyAreas[i] == RC_WALKABLE_AREA) polyAreas[i] = 0; // SAMPLE_POLYAREA_GROUND
            polyFlags[i] = walkFlag;
        }
    }
};


void NavigationSystem::GenerateNavData()
{
    DestroyNavData();

    auto mesh = Level::Current->GetStaticNavObstaclesMesh();

    std::lock_guard<std::recursive_mutex> lock(mainLock);

    std::vector<glm::vec3> vertices = mesh.vertices;
    std::vector<uint32_t> indices = mesh.indices;
    if (vertices.size() < 3) return;

    glm::vec3 bmin = vertices[0], bmax = vertices[0];
    for (const auto& v : vertices) { bmin = glm::min(bmin, v); bmax = glm::max(bmax, v); }
    bmin -= glm::vec3(5); bmax += glm::vec3(5);

    // ---- Recast config (similar to yours)
    rcConfig cfg{};
    cfg.cs = 0.2f;
    cfg.ch = 0.3f;
    cfg.walkableSlopeAngle = 40.0f;
    cfg.walkableHeight = static_cast<int>(ceilf(2.0f / cfg.ch));
    cfg.walkableClimb = static_cast<int>(ceilf(0.6f / cfg.ch));
    cfg.walkableRadius = static_cast<int>(ceilf(0.2f / cfg.cs));
    cfg.maxEdgeLen = static_cast<int>(12 / cfg.cs);
    cfg.maxSimplificationError = 0.01f;
    cfg.minRegionArea = 0;
    cfg.mergeRegionArea = 200 * 200;
    cfg.maxVertsPerPoly = 6;
    cfg.tileSize = 64;
    cfg.borderSize = static_cast<int>(ceilf(0.5f / cfg.cs)) + 3;
    cfg.width = cfg.tileSize + cfg.borderSize * 2;
    cfg.height = cfg.tileSize + cfg.borderSize * 2;
    cfg.detailSampleDist = 0.1f;
    cfg.detailSampleMaxError = 0.1f;

    const float tileWorld = cfg.tileSize * cfg.cs;
    const int ntilesX = static_cast<int>(ceilf((bmax.x - bmin.x) / tileWorld));
    const int ntilesZ = static_cast<int>(ceilf((bmax.z - bmin.z) / tileWorld));
    const int expectedLayersPerTile = 4; // conservative upper bound; tune if you know your data
    const int maxTiles = ntilesX * ntilesZ * expectedLayersPerTile;
    const float borderWorld = cfg.borderSize * cfg.cs;

    // ---- Init Detour navmesh
    dtNavMeshParams navParams{};
    navParams.orig[0] = bmin.x; navParams.orig[1] = bmin.y; navParams.orig[2] = bmin.z;
    navParams.tileWidth = tileWorld;
    navParams.tileHeight = tileWorld;
    navParams.maxTiles = ntilesX * ntilesZ * expectedLayersPerTile; // Detour needs enough for all layers
    navParams.maxPolys = 1 << 14;

    navMesh = dtAllocNavMesh();
    if (!navMesh || dtStatusFailed(navMesh->init(&navParams))) {
        dtFreeNavMesh(navMesh); navMesh = nullptr;
        std::cerr << "Failed to initialize dtNavMesh\n";
        return;
    }

    // ---- Init Tile Cache
    talloc = new dtTileCacheAlloc();     // 4–16 MB temp buffer is typical
    tcomp = new FastLZCompressor();         // or your own compressor
    tmproc = new MeshProcess();

    dtTileCacheParams tcParams{};
    tcParams.orig[0] = bmin.x; tcParams.orig[1] = bmin.y; tcParams.orig[2] = bmin.z;
    tcParams.cs = cfg.cs; tcParams.ch = cfg.ch;
    tcParams.width = cfg.tileSize;          // in cells (not world units)
    tcParams.height = cfg.tileSize;          // in cells
    tcParams.walkableHeight = 2.0f;          // same as your dtCreateNavMeshData params
    tcParams.walkableRadius = 0.2f;
    tcParams.walkableClimb = 0.6f;
    tcParams.maxTiles = maxTiles;
    tcParams.maxObstacles = 2048;            // tune for your runtime needs

    tileCache = dtAllocTileCache();
    if (!tileCache || dtStatusFailed(tileCache->init(&tcParams, talloc, tcomp, tmproc))) {
        if (tileCache) dtFreeTileCache(tileCache); tileCache = nullptr;
        std::cerr << "Failed to initialize dtTileCache\n";
        return;
    }

    // ---- Flatten your geometry for Recast
    std::vector<float> vertFloats; vertFloats.reserve(vertices.size() * 3);
    for (auto& v : vertices) { vertFloats.push_back(v.x); vertFloats.push_back(v.y); vertFloats.push_back(v.z); }
    std::vector<int> triInts; triInts.reserve(indices.size());
    for (auto i : indices) triInts.push_back((int)i);

    const int nverts = (int)vertices.size();
    const int ntris = (int)indices.size() / 3;

    // ---- Precompute expanded tri bounds & tile bins (kept from your code)
    std::vector<std::array<float, 4>> triBounds(ntris);
    for (int i = 0; i < ntris; ++i) {
        const int* tri = &triInts[i * 3];
        const float* v0 = &vertFloats[tri[0] * 3];
        const float* v1 = &vertFloats[tri[1] * 3];
        const float* v2 = &vertFloats[tri[2] * 3];
        float minx = std::min(v0[0], std::min(v1[0], v2[0]));
        float minz = std::min(v0[2], std::min(v1[2], v2[2]));
        float maxx = std::max(v0[0], std::max(v1[0], v2[0]));
        float maxz = std::max(v0[2], std::max(v1[2], v2[2]));
        triBounds[i] = { minx - borderWorld, minz - borderWorld, maxx + borderWorld, maxz + borderWorld };
    }
    std::vector<std::vector<int>> tileTriangles(ntilesX * ntilesZ);
    for (int i = 0; i < ntris; ++i) {
        const float minx = triBounds[i][0], minz = triBounds[i][1];
        const float maxx = triBounds[i][2], maxz = triBounds[i][3];
        int tx0 = (int)((minx - bmin.x) / tileWorld);
        int tx1 = (int)((maxx - bmin.x) / tileWorld);
        int tz0 = (int)((minz - bmin.z) / tileWorld);
        int tz1 = (int)((maxz - bmin.z) / tileWorld);
        tx0 = std::max(0, tx0); tx1 = std::min(ntilesX - 1, tx1);
        tz0 = std::max(0, tz0); tz1 = std::min(ntilesZ - 1, tz1);
        for (int tz = tz0; tz <= tz1; ++tz)
            for (int tx = tx0; tx <= tx1; ++tx)
                tileTriangles[tz * ntilesX + tx].push_back(i);
    }

    rcContext* ctx = new rcContext();
    std::vector<unsigned char> triareas(ntris, 0);
    rcMarkWalkableTriangles(ctx, cfg.walkableSlopeAngle,
        vertFloats.data(), nverts, triInts.data(), ntris, triareas.data());

    // ---- Build cache tiles
    for (int tz = 0; tz < ntilesZ; ++tz) {
        float progress = (float)tz / (float)std::max(1, ntilesZ - 1);
        LoadingScreenSystem::Update(0.6f + progress * 0.3f);

        for (int tx = 0; tx < ntilesX; ++tx) {
            // Tight world bounds for this tile (no border)
            float tbmin[3] = { bmin.x + tx * tileWorld, bmin.y, bmin.z + tz * tileWorld };
            float tbmax[3] = { bmin.x + (tx + 1) * tileWorld, bmax.y, bmin.z + (tz + 1) * tileWorld };

            // Expanded bounds used for rasterization (add border)
            float ebmin[3] = { tbmin[0] - borderWorld, tbmin[1], tbmin[2] - borderWorld };
            float ebmax[3] = { tbmax[0] + borderWorld, tbmax[1], tbmax[2] + borderWorld };

            // Build heightfield for this tile
            rcHeightfield* solid = rcAllocHeightfield();
            if (!solid || !rcCreateHeightfield(ctx, *solid, cfg.width, cfg.height, ebmin, ebmax, cfg.cs, cfg.ch)) {
                if (solid) rcFreeHeightField(solid);
                continue;
            }

            const auto& triList = tileTriangles[tz * ntilesX + tx];
            if (triList.empty()) { rcFreeHeightField(solid); continue; }

            std::vector<int>    ttris;      ttris.reserve(triList.size() * 3);
            std::vector<unsigned char> ta;  ta.reserve(triList.size());
            for (int i : triList) {
                ttris.push_back(triInts[i * 3 + 0]);
                ttris.push_back(triInts[i * 3 + 1]);
                ttris.push_back(triInts[i * 3 + 2]);
                ta.push_back(triareas[i]);
            }

            if (!rcRasterizeTriangles(ctx, vertFloats.data(), nverts,
                ttris.data(), ta.data(), (int)triList.size(),
                *solid, cfg.walkableClimb)) {
                rcFreeHeightField(solid);
                continue;
            }

            rcFilterLowHangingWalkableObstacles(ctx, cfg.walkableClimb, *solid);
            rcFilterLedgeSpans(ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
            rcFilterWalkableLowHeightSpans(ctx, cfg.walkableHeight, *solid);

            rcCompactHeightfield* chf = rcAllocCompactHeightfield();
            if (!chf || !rcBuildCompactHeightfield(ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *chf)) {
                if (chf) rcFreeCompactHeightfield(chf);
                rcFreeHeightField(solid);
                continue;
            }
            rcFreeHeightField(solid);

            if (!rcErodeWalkableArea(ctx, cfg.walkableRadius, *chf)) {
                rcFreeCompactHeightfield(chf);
                continue;
            }

            // Build heightfield *layers* for tile cache
            rcHeightfieldLayerSet* lset = rcAllocHeightfieldLayerSet();
            if (!lset || !rcBuildHeightfieldLayers(ctx, *chf, cfg.borderSize, cfg.walkableHeight, *lset)) {
                if (lset) rcFreeHeightfieldLayerSet(lset);
                rcFreeCompactHeightfield(chf);
                continue;
            }
            rcFreeCompactHeightfield(chf);

            // For each layer, build compressed cache data and add to tile cache
            for (int li = 0; li < lset->nlayers; ++li) {
                const rcHeightfieldLayer* layer = &lset->layers[li];
                dtTileCacheLayerHeader header{};
                header.magic = DT_TILECACHE_MAGIC;
                header.version = DT_TILECACHE_VERSION;
                header.tx = tx; header.ty = tz; header.tlayer = (unsigned char)li;
                // Copy layer spatial info (cells)
                rcVcopy(header.bmin, layer->bmin);
                rcVcopy(header.bmax, layer->bmax);
                header.width = (unsigned char)layer->width;
                header.height = (unsigned char)layer->height;
                header.minx = (unsigned char)layer->minx;
                header.maxx = (unsigned char)layer->maxx;
                header.miny = (unsigned char)layer->miny;
                header.maxy = (unsigned char)layer->maxy;
                header.hmin = (unsigned short)layer->hmin;
                header.hmax = (unsigned short)layer->hmax;

                unsigned char* compressed = nullptr;
                int compressedSize = 0;
                if (dtStatusFailed(dtBuildTileCacheLayer(tcomp, &header,
                    layer->heights, layer->areas, layer->cons,
                    &compressed, &compressedSize))) {
                    continue;
                }

                dtCompressedTileRef cref = 0;
                // DT_COMPRESSEDTILE_FREE_DATA: cache will own & free the memory. :contentReference[oaicite:2]{index=2}
                if (dtStatusFailed(tileCache->addTile(compressed, compressedSize, DT_COMPRESSEDTILE_FREE_DATA, &cref))) {
                    // If add fails, free ourselves:
                    dtFree(compressed);
                }
            }

            rcFreeHeightfieldLayerSet(lset);

            // Build all detour tiles at this grid coordinate from the cache
            (void)tileCache->buildNavMeshTilesAt(tx, tz, navMesh); // builds one or more layers. :contentReference[oaicite:3]{index=3}
        }
    }

    delete ctx;
}


void NavigationSystem::DrawNavmesh()
{
    if (DebugDrawNavMeshEnabled == false) return;

    if (!navMesh)
        return;

    std::lock_guard<std::recursive_mutex> lock(mainLock);

    const int maxTiles = navMesh->getMaxTiles();
    for (int i = 0; i < maxTiles; ++i)
    {
        const dtMeshTile* tile = navMesh->getTile(i);
        if (!tile || !tile->header)
            continue;

        for (int j = 0; j < tile->header->polyCount; ++j)
        {
            const dtPoly* poly = &tile->polys[j];


            // Only draw standard ground polys (avoid off-mesh connections, etc.)
            if (poly->getType() != DT_POLYTYPE_GROUND)
                continue;

            const int vertCount = poly->vertCount;
            for (int k = 0; k < vertCount; ++k)
            {
                const int currentIdx = poly->verts[k];
                const int nextIdx = poly->verts[(k + 1) % vertCount];

                if (currentIdx >= tile->header->vertCount || nextIdx >= tile->header->vertCount)
                    continue; // Skip invalid indices

                const float* v0 = &tile->verts[currentIdx * 3];
                const float* v1 = &tile->verts[nextIdx * 3];

                glm::vec3 p0(v0[0], v0[1], v0[2]);
                glm::vec3 p1(v1[0], v1[1], v1[2]);

                // Skip degenerate edges
                if (glm::distance(p0, p1) < 0.001f)
                    continue;

                if (distance(Camera::position, p1) > 100)
                    continue;

                DebugDraw::Line(p0, p1, Time::DeltaTimeF / 2, 0.01); // longer duration for better visibility
            }
        }
    }
}

void NavigationSystem::RemoveObstacle(dtObstacleRef obstacleRef)
{
    if (!tileCache || obstacleRef == 0)
        return;

    std::lock_guard<std::recursive_mutex> lock(mainLock);



    // Remove the obstacle using its reference
    const dtStatus status = tileCache->removeObstacle(obstacleRef);
    if (dtStatusFailed(status))
    {
        std::printf("Failed to remove obstacle with ref: %u. Status: %u\n", obstacleRef, status);
    }
    else
    {
        //std::printf("Obstacle with ref: %u removed successfully.\n", obstacleRef);

    }

    // Remove from tracking vector if used
    auto it = std::find(obstacles.begin(), obstacles.end(), obstacleRef);
    if (it != obstacles.end())
        obstacles.erase(it);
}

dtObstacleRef NavigationSystem::CreateObstacleBox(const glm::vec3& min, const glm::vec3& max)
{
    if (!tileCache) // Ensure tileCache is valid
        return 0;

    glm::vec3 adjustedMin = min;
    glm::vec3 adjustedMax = max;

    // Ensure each axis (x, y, z) has a minimum length of 2.0 units
    for (int i = 0; i < 3; ++i)
    {
        const float currentLength = adjustedMax[i] - adjustedMin[i];
        if (currentLength < 0.2f)
        {
            const float delta = (0.2f - currentLength) * 0.5f;
            adjustedMin[i] -= delta;
            adjustedMax[i] += delta;
        }
    }

    dtObstacleRef obstacleRef = 0;

    // Add the adjusted box obstacle
    dtStatus status = tileCache->addBoxObstacle(
        &adjustedMin.x,     // Adjusted min coordinates
        &adjustedMax.x,     // Adjusted max coordinates
        &obstacleRef
    );

    if (dtStatusFailed(status))
    {
        std::printf("Failed to add box obstacle. Status: %u\n", status);
        return 0;
    }

    obstacles.push_back(obstacleRef);

    //std::printf("Box obstacle added. Ref: %u\n", obstacleRef);
    return obstacleRef;
}

bool HasLineOfSight(const vec3& pointA, const vec3& pointB)
{
    return Physics::SphereTrace(pointA, pointB, 0.4, BodyType::World).hasHit == false;
}

bool CollisionCheckPath(glm::vec3 start, std::vector<glm::vec3> path)
{

    if (path.size() == 0) return false;

    vec3 oldPos = start;


    for (auto pos : path)
    {

        auto hit = Physics::LineTrace(oldPos + vec3(0,1,0), pos + vec3(0, 1, 0), BodyType::World);

        if (hit.hasHit)
            return false;

        oldPos = pos;

    }
    return true;
}

// Custom filter to check polygon area instead of flags
class CustomFilter : public dtQueryFilter
{
    bool passFilter(const dtPolyRef /*ref*/, const dtMeshTile* /*tile*/, const dtPoly* poly) const override
    {
        return true;// poly->getArea() == DT_TILECACHE_WALKABLE_AREA; // Match area set during navmesh build
    }
};

// =====================================================================
    // FindSimplePath
    // Computes a simple path from a start to a target position. It returns the
    // computed path as a vector of 3D points (world coordinates). If no valid
    // path is found, an empty vector is returned.
    //
    // After computing the initial straight path, the function checks if the
    // second and second-from-last points are redundant. If there is a clear line
    // of sight skipping those points, then they are removed.
    // =====================================================================
static bool IsPointReallyOnPoly(dtNavMeshQuery* navQuery,
    dtPolyRef        polyRef,
    const float      pos[3],
    float            horizTolSq = 0.0025f)
{
    if (!polyRef) return false;
    float  closest[3];
    bool inside = 0;
    if (dtStatusFailed(navQuery->closestPointOnPoly(polyRef, pos, closest, &inside)))
        return false;
    if (!inside) return false;
    const float dx = pos[0] - closest[0];
    const float dz = pos[2] - closest[2];
    return (dx * dx + dz * dz) <= horizTolSq;
}

std::vector<glm::vec3> NavigationSystem::FindSimplePath(glm::vec3 start, glm::vec3 target)
{

    if (HasLineOfSight(start, target))
    {
        return { target };
    }

    std::vector<glm::vec3> outPath;
    if (!navMesh) return outPath;
    std::lock_guard<std::recursive_mutex> _lock(mainLock);

    // Allocate & init query
    dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
    if (!navQuery) return outPath;
    if (dtStatusFailed(navQuery->init(navMesh, 2048)))
    {
        dtFreeNavMeshQuery(navQuery);
        return outPath;
    }

    auto hit = Physics::LineTrace(start, start - vec3(0, 300, 0), BodyType::World);
    if (hit.hasHit)
        start = hit.position + vec3(0, 1, 0);

    hit = Physics::LineTrace(target, target - vec3(0, 30, 0), BodyType::World);
    if (hit.hasHit)
        target = hit.position + vec3(0, 1, 0);

    CustomFilter filter;
    filter.setIncludeFlags(0xffff);
    filter.setExcludeFlags(0);

    // --- 1) FindNearestPoly  (increment extents up to limit) ---
    const std::vector<glm::vec3> EXTENTS = {
        {0.50f, 1.5f, 0.50f},  // tight
        {0.60f, 1.5f, 0.60f},  // med
        {1.00f, 1.5f, 1.00f},  // wide
        {1.50f, 1.5f, 1.50f},  // wide
    };
    dtPolyRef sRef = 0, gRef = 0;
    float     sNearest[3], gNearest[3];
    float     sPos[3] = { start.x,  start.y,  start.z };
    float     gPos[3] = { target.x, target.y, target.z };

    auto findOnPoly = [&](const float pos[3], dtPolyRef& outRef, float outNearest[3]) {
        for (auto& e : EXTENTS)
        {
            float ext[3] = { e.x, e.y, e.z };
            if (dtStatusSucceed(navQuery->findNearestPoly(pos, ext, &filter, &outRef, outNearest)) && outRef>0)
            {
                if (IsPointReallyOnPoly(navQuery, outRef, pos))
                    return true;
            }
        }
        return false;
        };

    // attempt to pick start & goal polys
    if (!findOnPoly(sPos, sRef, sNearest) || 
        !findOnPoly(gPos, gRef, gNearest))
    {
        dtFreeNavMeshQuery(navQuery);
        return outPath; // failed to localize start or goal
    }

    // --- 2) FindPath (A*) across linked polys ---
    const int MAX_POLYS = 512;
    dtPolyRef polyPath[MAX_POLYS];
    int       polyCount = 0;
    if (dtStatusFailed(navQuery->findPath(sRef, gRef, sPos, gPos,
        &filter,
        polyPath, &polyCount, MAX_POLYS)) ||
        polyCount == 0)
    {
        dtFreeNavMeshQuery(navQuery);
        return outPath;
    }

    // --- 3) Clamp goal if partial ---
    bool reached = (polyPath[polyCount - 1] == gRef);
    if (!reached)
        navQuery->closestPointOnPoly(polyPath[polyCount - 1], gPos, gNearest, nullptr);

    // --- 4) StringPull / StraightPath ---
    const int MAX_STRAIGHT = 512;
    float          straight[3 * MAX_STRAIGHT];
    unsigned char  flags[MAX_STRAIGHT];
    dtPolyRef      strPolys[MAX_STRAIGHT];
    int            strCount = 0;

    if (dtStatusFailed(navQuery->findStraightPath(
        sNearest,  // Use closest point on navmesh for start
        reached ? gPos : gNearest,
        polyPath, polyCount,
        straight, flags, strPolys,
        &strCount, MAX_STRAIGHT, DT_STRAIGHTPATH_ALL_CROSSINGS)))
    {
        //dtFreeNavMeshQuery(navQuery);
        //return outPath;
    }

    // --- 5) Build glm path (skip the first point, it's the start) ---
    outPath.reserve(strCount);
    for (int i = 1; i < strCount; ++i)
    {
        outPath.emplace_back(
            straight[i * 3 + 0],
            straight[i * 3 + 1],
            straight[i * 3 + 2]
        );
    }
    // final target or clamped point
    outPath.emplace_back(
        reached ? target : glm::vec3(gNearest[0], gNearest[1], gNearest[2])
    );

    dtFreeNavMeshQuery(navQuery);

 //   // --- 6) Collision sanity check ---
	//if (!CollisionCheckPath(start, outPath))
	//{
	//	outPath.clear();

	//}
 //       

    return outPath;
}
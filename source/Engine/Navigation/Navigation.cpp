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

// Recast and Detour includes

bool NavigationSystem::DebugDrawNavMeshEnabled = false;

// Static member initialization
dtNavMesh* NavigationSystem::navMesh = nullptr;
dtTileCache* NavigationSystem::tileCache = nullptr;
std::recursive_mutex NavigationSystem::mainLock;
std::vector<dtObstacleRef> NavigationSystem::obstacles;

// Custom allocator for tile cache
struct LinearAllocator : public dtTileCacheAlloc {
    unsigned char* buffer;
    int capacity;
    int top;

    LinearAllocator(int cap) : buffer(new unsigned char[cap]), capacity(cap), top(0) {}
    ~LinearAllocator() { delete[] buffer; }
    void reset() override { top = 0; }
    void* alloc(size_t size) override {
        if (top + static_cast<int>(size) > capacity) return nullptr;
        void* ptr = buffer + top;
        top += static_cast<int>(size);
        return ptr;
    }
    void free(void*) override {} // No-op; memory managed by reset or destruction
};

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

LinearAllocator* talloc = nullptr; // 1MB
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



void NavigationSystem::GenerateNavData()
{
    DestroyNavData();

    auto mesh = Level::Current->GetStaticNavObstaclesMesh();
    //mesh = MeshUtils::RemoveDegenerates(mesh, 0.01f, 0.01f);
    //mesh = MeshUtils::MergeMeshes({ MeshUtils::MergeCoplanarRegions(mesh), mesh });

    std::lock_guard<std::recursive_mutex> lock(mainLock);

    std::vector<glm::vec3> vertices = mesh.vertices;
    std::vector<uint32_t> indices = mesh.indices;

    if (vertices.size() < 3) return;

    glm::vec3 bmin = vertices[0];
    glm::vec3 bmax = vertices[0];
    for (const auto& v : vertices) {
        bmin = glm::min(bmin, v);
        bmax = glm::max(bmax, v);
    }

    bmin -= vec3(5);
    bmax += vec3(5);

    // Recast configuration
    rcConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
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
    cfg.tileSize = 32;
    cfg.borderSize = static_cast<int>(ceilf(0.5f / cfg.cs)) + 3;
    cfg.width = cfg.tileSize + cfg.borderSize * 2;
    cfg.height = cfg.tileSize + cfg.borderSize * 2;
    cfg.detailSampleDist = 0.1f;
    cfg.detailSampleMaxError = 0.1f;

    // Calculate number of tiles
    const float tileWidth = cfg.tileSize * cfg.cs;
    const int ntilesX = static_cast<int>(ceilf((bmax.x - bmin.x) / tileWidth));
    const int ntilesZ = static_cast<int>(ceilf((bmax.z - bmin.z) / tileWidth));
    const int maxTiles = ntilesX * ntilesZ * 10;
    const float borderWorld = cfg.borderSize * cfg.cs;  // World-space border size

    // Initialize nav mesh
    dtNavMeshParams navParams;
    memset(&navParams, 0, sizeof(navParams));
    navParams.orig[0] = bmin.x;
    navParams.orig[1] = bmin.y;
    navParams.orig[2] = bmin.z;
    navParams.tileWidth = tileWidth;
    navParams.tileHeight = tileWidth;
    navParams.maxTiles = maxTiles;
    navParams.maxPolys = 16384;

    navMesh = dtAllocNavMesh();
    if (!navMesh || dtStatusFailed(navMesh->init(&navParams))) {
        std::cerr << "Failed to initialize navMesh" << std::endl;
        dtFreeNavMesh(navMesh);
        navMesh = nullptr;
        return;
    }

    // Convert geometry to Recast format
    std::vector<float> vertFloats;
    vertFloats.reserve(vertices.size() * 3);
    for (const auto& v : vertices) {
        vertFloats.push_back(v.x);
        vertFloats.push_back(v.y);
        vertFloats.push_back(v.z);
    }
    std::vector<int> triInts;
    triInts.reserve(indices.size());
    for (const auto& index : indices) {
        triInts.push_back(static_cast<int>(index));
    }
    const int nverts = vertices.size();
    const int ntris = indices.size() / 3;

    // Precompute triangle bounding boxes with border expansion
    std::vector<std::array<float, 4>> triBounds(ntris);
    for (int i = 0; i < ntris; i++) {
        const int* tri = &triInts[i * 3];
        const float* v0 = &vertFloats[tri[0] * 3];
        const float* v1 = &vertFloats[tri[1] * 3];
        const float* v2 = &vertFloats[tri[2] * 3];

        float minx = std::min(v0[0], std::min(v1[0], v2[0]));
        float minz = std::min(v0[2], std::min(v1[2], v2[2]));
        float maxx = std::max(v0[0], std::max(v1[0], v2[0]));
        float maxz = std::max(v0[2], std::max(v1[2], v2[2]));

        // Expand bounds by border size to include nearby tiles
        triBounds[i][0] = minx - borderWorld;
        triBounds[i][1] = minz - borderWorld;
        triBounds[i][2] = maxx + borderWorld;
        triBounds[i][3] = maxz + borderWorld;
    }

    // Precompute tile assignments for each triangle
    std::vector<std::vector<int>> tileTriangles(ntilesX * ntilesZ);
    for (int i = 0; i < ntris; i++) {
        const float minx = triBounds[i][0];
        const float minz = triBounds[i][1];
        const float maxx = triBounds[i][2];
        const float maxz = triBounds[i][3];

        // Calculate tile ranges for this triangle
        int tx0 = static_cast<int>((minx - bmin.x) / tileWidth);
        int tx1 = static_cast<int>((maxx - bmin.x) / tileWidth);
        int tz0 = static_cast<int>((minz - bmin.z) / tileWidth);
        int tz1 = static_cast<int>((maxz - bmin.z) / tileWidth);

        // Clamp to valid tile range
        tx0 = std::max(0, tx0);
        tx1 = std::min(ntilesX - 1, tx1);
        tz0 = std::max(0, tz0);
        tz1 = std::min(ntilesZ - 1, tz1);

        // Add triangle to relevant tiles
        for (int tz = tz0; tz <= tz1; tz++) {
            for (int tx = tx0; tx <= tx1; tx++) {
                tileTriangles[tz * ntilesX + tx].push_back(i);
            }
        }
    }

    // Context for Recast operations
    rcContext* ctx = new rcContext();
    const unsigned char SAMPLE_POLYAREA_GROUND = 0;
    const unsigned char SAMPLE_POLYFLAGS_WALK = 1;

    // Precompute walkable areas for all triangles
    std::vector<unsigned char> triareas(ntris);
    rcMarkWalkableTriangles(ctx, cfg.walkableSlopeAngle,
        vertFloats.data(), nverts,
        triInts.data(), ntris,
        triareas.data());

    // Process each tile
    for (int tz = 0; tz < ntilesZ; ++tz) {
        for (int tx = 0; tx < ntilesX; ++tx) {
            float tileBmin[3] = {
                bmin.x + tx * tileWidth,
                bmin.y,
                bmin.z + tz * tileWidth
            };
            float tileBmax[3] = {
                bmin.x + (tx + 1) * tileWidth,
                bmax.y,
                bmin.z + (tz + 1) * tileWidth
            };

            int dataSize = 0;
            unsigned char* data = [&]() -> unsigned char* {
                // Create expanded tile bounds (with border)
                rcConfig localCfg = cfg;
                float expandedBmin[3] = {
                    tileBmin[0] - borderWorld,
                    tileBmin[1],
                    tileBmin[2] - borderWorld
                };
                float expandedBmax[3] = {
                    tileBmax[0] + borderWorld,
                    tileBmax[1],
                    tileBmax[2] + borderWorld
                };
                rcVcopy(localCfg.bmin, expandedBmin);
                rcVcopy(localCfg.bmax, expandedBmax);

                // Allocate and initialize heightfield
                rcHeightfield* solid = rcAllocHeightfield();
                if (!solid || !rcCreateHeightfield(ctx, *solid, localCfg.width, localCfg.height,
                    localCfg.bmin, localCfg.bmax, localCfg.cs, localCfg.ch)) {
                    ctx->log(RC_LOG_ERROR, "Failed to create solid heightfield");
                    if (solid) rcFreeHeightField(solid);
                    return nullptr;
                }

                // Get precomputed triangles for this tile
                std::vector<int>& tileTriIndices = tileTriangles[tz * ntilesX + tx];
                if (tileTriIndices.empty()) {
                    rcFreeHeightField(solid);
                    return nullptr;
                }

                // Create arrays for this tile's triangles
                std::vector<int> tileTris;
                std::vector<unsigned char> tileTriareas;

                for (int i : tileTriIndices) {
                    tileTris.push_back(triInts[i * 3]);
                    tileTris.push_back(triInts[i * 3 + 1]);
                    tileTris.push_back(triInts[i * 3 + 2]);
                    tileTriareas.push_back(triareas[i]);
                }

                // Rasterize triangles
                if (!rcRasterizeTriangles(ctx, vertFloats.data(), nverts,
                    tileTris.data(), tileTriareas.data(), tileTris.size() / 3,
                    *solid, localCfg.walkableClimb)) {
                    rcFreeHeightField(solid);
                    return nullptr;
                }

                // Filter walkable surfaces
                rcFilterLowHangingWalkableObstacles(ctx, localCfg.walkableClimb, *solid);
                rcFilterLedgeSpans(ctx, localCfg.walkableHeight, localCfg.walkableClimb, *solid);
                rcFilterWalkableLowHeightSpans(ctx, localCfg.walkableHeight, *solid);

                // Build compact heightfield
                rcCompactHeightfield* chf = rcAllocCompactHeightfield();
                if (!chf || !rcBuildCompactHeightfield(ctx, localCfg.walkableHeight,
                    localCfg.walkableClimb, *solid, *chf)) {
                    ctx->log(RC_LOG_ERROR, "Failed to build compact heightfield");
                    rcFreeHeightField(solid);
                    if (chf) rcFreeCompactHeightfield(chf);
                    return nullptr;
                }
                rcFreeHeightField(solid);

                // Erode walkable area
                if (!rcErodeWalkableArea(ctx, localCfg.walkableRadius, *chf)) {
                    ctx->log(RC_LOG_ERROR, "Failed to erode walkable area");
                    rcFreeCompactHeightfield(chf);
                    return nullptr;
                }

                // Partition walkable surface (Watershed partitioning)
                if (!rcBuildDistanceField(ctx, *chf) ||
                    !rcBuildRegions(ctx, *chf, localCfg.borderSize,
                        localCfg.minRegionArea, localCfg.mergeRegionArea)) {
                    ctx->log(RC_LOG_ERROR, "Failed to build distance field or regions");
                    rcFreeCompactHeightfield(chf);
                    return nullptr;
                }

                // Build contours
                rcContourSet* cset = rcAllocContourSet();
                if (!cset || !rcBuildContours(ctx, *chf, localCfg.maxSimplificationError,
                    localCfg.maxEdgeLen, *cset)) {
                    ctx->log(RC_LOG_ERROR, "Failed to build contours");
                    rcFreeCompactHeightfield(chf);
                    if (cset) rcFreeContourSet(cset);
                    return nullptr;
                }
                if (cset->nconts == 0) {
                    rcFreeCompactHeightfield(chf);
                    rcFreeContourSet(cset);
                    return nullptr;
                }

                // Build poly mesh
                rcPolyMesh* pmesh = rcAllocPolyMesh();
                if (!pmesh || !rcBuildPolyMesh(ctx, *cset, localCfg.maxVertsPerPoly, *pmesh)) {
                    ctx->log(RC_LOG_ERROR, "Failed to build polygon mesh");
                    rcFreeCompactHeightfield(chf);
                    rcFreeContourSet(cset);
                    if (pmesh) rcFreePolyMesh(pmesh);
                    return nullptr;
                }

                // Build detail mesh
                rcPolyMeshDetail* dmesh = rcAllocPolyMeshDetail();
                if (!dmesh || !rcBuildPolyMeshDetail(ctx, *pmesh, *chf,
                    localCfg.detailSampleDist, localCfg.detailSampleMaxError, *dmesh)) {
                    ctx->log(RC_LOG_ERROR, "Failed to build detail mesh");
                    rcFreeCompactHeightfield(chf);
                    rcFreeContourSet(cset);
                    rcFreePolyMesh(pmesh);
                    if (dmesh) rcFreePolyMeshDetail(dmesh);
                    return nullptr;
                }

                // Clean up intermediate structures
                rcFreeCompactHeightfield(chf);
                rcFreeContourSet(cset);

                // Set polygon areas and flags
                for (int i = 0; i < pmesh->npolys; ++i) {
                    if (pmesh->areas[i] == RC_WALKABLE_AREA)
                        pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;
                    pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
                }

                // Generate Detour navmesh data
                dtNavMeshCreateParams params;
                memset(&params, 0, sizeof(params));
                params.verts = pmesh->verts;
                params.vertCount = pmesh->nverts;
                params.polys = pmesh->polys;
                params.polyAreas = pmesh->areas;
                params.polyFlags = pmesh->flags;
                params.polyCount = pmesh->npolys;
                params.nvp = pmesh->nvp;
                params.detailMeshes = dmesh->meshes;
                params.detailVerts = dmesh->verts;
                params.detailVertsCount = dmesh->nverts;
                params.detailTris = dmesh->tris;
                params.detailTriCount = dmesh->ntris;
                params.walkableHeight = 2.0f;
                params.walkableRadius = 0.2f;
                params.walkableClimb = 0.6f;
                params.tileX = tx;
                params.tileY = tz;
                params.tileLayer = 0;
                rcVcopy(params.bmin, pmesh->bmin);
                rcVcopy(params.bmax, pmesh->bmax);
                params.cs = localCfg.cs;
                params.ch = localCfg.ch;
                params.buildBvTree = true;

                unsigned char* navData = nullptr;
                int navDataSize = 0;
                if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
                    rcFreePolyMesh(pmesh);
                    rcFreePolyMeshDetail(dmesh);
                    return nullptr;
                }

                rcFreePolyMesh(pmesh);
                rcFreePolyMeshDetail(dmesh);
                return navData;
                }(); // End of tile processing lambda

            // Add tile to navmesh
            if (data) {
                // Remove existing tile
                dtTileRef existingTile = navMesh->getTileRefAt(tx, tz, 0);
                if (existingTile)
                    navMesh->removeTile(existingTile, nullptr, nullptr);

                // Add new tile
                if (dtStatusFailed(navMesh->addTile(data, dataSize, DT_TILE_FREE_DATA, 0, nullptr))) {
                    dtFree(data);
                }
            }
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

    // --- 6) Collision sanity check ---
	if (!CollisionCheckPath(start, outPath))
	{
		outPath.clear();

	}
        

    return outPath;
}
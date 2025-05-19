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
    mesh = MeshUtils::RemoveDegenerates(mesh, 0.1f, 0.00f);
    mesh = MeshUtils::MergeMeshes({ MeshUtils::MergeCoplanarRegions(mesh) , mesh}); // too dirty + results in false walkable areas if not surrounded by walls. FIX ME

    std::lock_guard<std::recursive_mutex> lock(mainLock);

    // Define sample geometry: a flat square
    std::vector<glm::vec3> vertices = mesh.vertices;
    std::vector<uint32_t> indices = mesh.indices;

    //DebugDraw::IndexedMesh(vertices, indices, 200);

    if (vertices.size() < 3) return;

    // Compute bounding box
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
    cfg.cs = 0.2f;                    // Cell size (voxel size in X/Z)
    cfg.ch = 0.3f;                    // Cell height
    cfg.walkableSlopeAngle = 45.0f;   // Max slope angle
    cfg.walkableHeight = static_cast<int>(ceilf(2.0f / cfg.ch)); 
    cfg.walkableClimb = static_cast<int>(ceilf(0.6f / cfg.ch));  
    cfg.walkableRadius = static_cast<int>(ceilf(0.35f / cfg.cs)); 
    cfg.maxEdgeLen = static_cast<int>(12 / cfg.cs);
    cfg.maxSimplificationError = 0.01f;
    cfg.minRegionArea = 1;      // Min region size
    cfg.mergeRegionArea = 200 * 200;  // Merge region size
    cfg.maxVertsPerPoly = 3;
    cfg.tileSize = 64;                // Tile size in cells
    cfg.borderSize = static_cast<int>(ceilf(0.5f / cfg.cs)) + 3;  // ~3-4 cells
    cfg.width = cfg.tileSize + cfg.borderSize * 2;
    cfg.height = cfg.tileSize + cfg.borderSize * 2;
    cfg.detailSampleDist = 0.2f;
    cfg.detailSampleMaxError = 0.1f;

    // Compute tile grid
    const float tileWidth = cfg.tileSize * cfg.cs;
    const int ntilesX = static_cast<int>(ceilf((bmax.x - bmin.x) / tileWidth));
    const int ntilesZ = static_cast<int>(ceilf((bmax.z - bmin.z) / tileWidth));
    const int maxTiles = ntilesX * ntilesZ * 10;

    // Initialize nav mesh
    dtNavMeshParams navParams;
    memset(&navParams, 0, sizeof(navParams));
    rcVcopy(navParams.orig, &bmin.x);
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

    // Initialize tile cache
    dtTileCacheParams tcParams;
    memset(&tcParams, 0, sizeof(tcParams));
    rcVcopy(tcParams.orig, &bmin.x);

    tcParams.cs = cfg.cs;
    tcParams.ch = cfg.ch;
    tcParams.width = cfg.tileSize;
    tcParams.height = cfg.tileSize;
    tcParams.walkableHeight = cfg.walkableHeight;
    tcParams.walkableRadius = cfg.walkableRadius;
    tcParams.walkableClimb = cfg.walkableClimb;
    tcParams.maxSimplificationError = cfg.maxSimplificationError;
    tcParams.maxTiles = maxTiles;
    tcParams.maxObstacles = 256;

    talloc = new LinearAllocator(1024 * 1024 * 5); // 1MB
    tcomp = new FastLZCompressor();

    tileCache = dtAllocTileCache();
    if (!tileCache || dtStatusFailed(tileCache->init(&tcParams, talloc, tcomp, nullptr))) {
        std::cerr << "Failed to initialize tileCache" << std::endl;
        dtFreeTileCache(tileCache);
        tileCache = nullptr;
        delete talloc;
        delete tcomp;
        return;
    }

    // Convert geometry to Recast format
    rcContext* ctx = new rcContext();
    std::vector<float> vertFloats(vertices.size() * 3);
    for (size_t i = 0; i < vertices.size(); ++i) {
        vertFloats[i * 3] = vertices[i].x;
        vertFloats[i * 3 + 1] = vertices[i].y;
        vertFloats[i * 3 + 2] = vertices[i].z;
    }
    std::vector<int> triInts(indices.begin(), indices.end());
    const int ntris = indices.size() / 3;

    // Tile generation loop
    for (int tz = 0; tz < ntilesZ; ++tz) {
        for (int tx = 0; tx < ntilesX; ++tx) {
            float tbmin[3] = {
                navParams.orig[0] + tx * tileWidth,
                bmin.y - 0.1f, // Extend below
                navParams.orig[2] + tz * tileWidth
            };
            float tbmax[3] = {
                navParams.orig[0] + (tx + 1) * tileWidth,
                bmax.y + 0.1f, // Extend above
                navParams.orig[2] + (tz + 1) * tileWidth
            };
            rcVcopy(cfg.bmin, tbmin);
            rcVcopy(cfg.bmax, tbmax);

            // Build heightfield
            rcHeightfield hf;
            if (!rcCreateHeightfield(ctx, hf, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch)) {
                std::cerr << "Failed to create heightfield for tile (" << tx << "," << tz << ")" << std::endl;
                continue;
            }

            // Rasterize triangles
            unsigned char* triareas = new unsigned char[ntris];
            rcMarkWalkableTriangles(ctx, cfg.walkableSlopeAngle, vertFloats.data(), vertices.size(),
                triInts.data(), ntris, triareas);
            rcRasterizeTriangles(ctx, vertFloats.data(), vertices.size(), triInts.data(), triareas, ntris, hf, cfg.walkableClimb);
            delete[] triareas;

            // Filter walkable surfaces
            rcFilterLowHangingWalkableObstacles(ctx, cfg.walkableClimb, hf);
            rcFilterLedgeSpans(ctx, cfg.walkableHeight, cfg.walkableClimb, hf);
            rcFilterWalkableLowHeightSpans(ctx, cfg.walkableHeight, hf);
            
            

            // Build compact heightfield
            rcCompactHeightfield chf;
            if (!rcBuildCompactHeightfield(ctx, cfg.walkableHeight, cfg.walkableClimb, hf, chf)) {
                std::cerr << "Failed to build compact heightfield for tile (" << tx << "," << tz << ")" << std::endl;
                continue;
            }

			// Erode walkable area to account for agent radius
			if (!rcErodeWalkableArea(ctx, cfg.walkableRadius, chf)) {
				std::cerr << "Failed to erode walkable area for tile (" << tx << "," << tz << ")" << std::endl;
				continue;
			}

			// Erode and build regions
			if (!rcBuildRegionsMonotone(ctx, chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea)) {
				std::cerr << "Failed to erode walkable area for tile (" << tx << "," << tz << ")" << std::endl;
				continue;
			}
            rcBuildDistanceField(ctx, chf);
            rcBuildRegions(ctx, chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea);

            // Build layers
            rcHeightfieldLayerSet* layers = rcAllocHeightfieldLayerSet();
            if (!layers || !rcBuildHeightfieldLayers(ctx, chf, cfg.borderSize, cfg.walkableHeight, *layers)) {
                std::cerr << "Failed to build heightfield layers for tile (" << tx << "," << tz << ")" << std::endl;
                rcFreeHeightfieldLayerSet(layers);
                continue;
            }


            // Process each layer
            for (int i = 0; i < layers->nlayers; ++i) {
                const rcHeightfieldLayer& layer = layers->layers[i];

                // Prepare layer header
                dtTileCacheLayerHeader header;
                header.magic = DT_TILECACHE_MAGIC;
                header.version = DT_TILECACHE_VERSION;
                header.tx = tx;
                header.ty = tz;
                header.tlayer = i;
                rcVcopy(header.bmin, layer.bmin);
                rcVcopy(header.bmax, layer.bmax);
                header.width = static_cast<unsigned char>(layer.width);
                header.height = static_cast<unsigned char>(layer.height);
                header.minx = static_cast<unsigned char>(layer.minx);
                header.maxx = static_cast<unsigned char>(layer.maxx);
                header.miny = static_cast<unsigned char>(layer.miny);
                header.maxy = static_cast<unsigned char>(layer.maxy);
                int lminh = static_cast<int>(floor((layer.hmin - cfg.bmin[1]) / cfg.ch));
                header.hmin = static_cast<unsigned short>(lminh);
                int lmaxh = static_cast<int>(ceil((layer.hmax - cfg.bmin[1]) / cfg.ch));
                header.hmax = static_cast<unsigned short>(lmaxh);

                // Allocate and copy layer data
                const int gridSize = layer.width * layer.height;


                // Debug layer data
                int walkableCount = 0;
                for (int j = 0; j < gridSize; ++j) {
                    if (layer.areas[j] == DT_TILECACHE_WALKABLE_AREA) walkableCount++;
                }

                // Build compressed layer
                unsigned char* outData = nullptr;
                int outDataSize = 0;
                dtStatus status = dtBuildTileCacheLayer(tcomp, &header,
                    layer.heights, layer.areas, layer.cons,
                    &outData, &outDataSize);
                talloc->free(layer.heights);
                talloc->free(layer.areas);
                talloc->free(layer.cons);

                if (dtStatusFailed(status)) {
                    std::cerr << "Failed to build tile cache layer for tile (" << tx << "," << tz << ") layer " << i
                        << " (status: " << status << ")" << std::endl;
                    if (outData) talloc->free(outData);
                    continue;
                }

                // Add to tile cache
                dtCompressedTileRef tileRef;

                status = tileCache->addTile(outData, outDataSize, DT_COMPRESSEDTILE_FREE_DATA, &tileRef);
                if (dtStatusFailed(status)) {
                    std::cerr << "Failed to add tile to cache for tile (" << tx << "," << tz << ") layer " << i
                        << " (status: " << status << ")" << std::endl;
                    talloc->free(outData);
                    continue;
                }

                // Build nav mesh tile
                status = tileCache->buildNavMeshTile(tileRef, navMesh);
                if (dtStatusFailed(status)) {
                    std::cerr << "Failed to build navmesh tile for tile (" << tx << "," << tz << ") layer " << i
                        << " (status: " << status << ")" << std::endl;
                }

            }
            rcFreeHeightfieldLayerSet(layers);
        }
    }

    delete ctx;
    // Note: talloc and tcomp are managed in DestroyNavData
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

    vec3 oldPos = start;

    for (auto pos : path)
    {

        auto hit = Physics::LineTrace(oldPos, pos, BodyType::World);

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
	/*if (!CollisionCheckPath(start, outPath))
	{
		DebugDraw::Path(outPath, 30, 0.1);
		outPath.clear();

	}*/
        

    return outPath;
}
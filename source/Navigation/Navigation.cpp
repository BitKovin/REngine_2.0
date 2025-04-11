#include <vector>
#include <mutex>
#include "../glm.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include "Navigation.hpp"

#include "../Level.hpp"

#include "../Physics.h"

// Recast and Detour includes



// Static member initialization
dtNavMesh* NavigationSystem::navMesh = nullptr;
dtTileCache* NavigationSystem::tileCache = nullptr;
std::mutex NavigationSystem::mainLock;
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


	std::lock_guard<std::mutex> lock(mainLock);

    for (auto obstacle : obstacles)
    {
        RemoveObstacle(obstacle);
    }

	if (tileCache)
		dtFreeTileCache(tileCache);

	if (navMesh)
		dtFreeNavMesh(navMesh);


	if (talloc)
		delete talloc;

	if (tcomp)
		delete tcomp;

    
}



void NavigationSystem::GenerateNavData()
{
    // Clear any existing navigation data
    DestroyNavData();

    // Retrieve the static obstacles mesh from the current level
    auto mesh = Level::Current->GetStaticNavObstaclesMesh();

    // Ensure thread safety during generation
    std::lock_guard<std::mutex> lock(mainLock);

    // Extract vertices and indices from the mesh
    std::vector<glm::vec3> vertices = mesh.vertices;
    std::vector<uint32_t> indices = mesh.indices;


    // Compute the bounding box of the level geometry
    glm::vec3 bmin = vertices[0];
    glm::vec3 bmax = vertices[0];
    for (const auto& v : vertices) {
        bmin = glm::min(bmin, v);
        bmax = glm::max(bmax, v);
    }
    bmin -= glm::vec3(5); // Expand by 5 units
    bmax += glm::vec3(5);

    // Set up Recast configuration
    rcConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.cs = 0.25f;                          // Cell size
    cfg.ch = 0.2f;                           // Cell height
    cfg.walkableSlopeAngle = 45.0f;          // Max slope angle
    cfg.walkableHeight = static_cast<int>(ceilf(2.0f / cfg.ch)); // Agent height in cells
    cfg.walkableClimb = static_cast<int>(ceilf(0.5f / cfg.ch));  // Max climb height in cells
    cfg.walkableRadius = static_cast<int>(ceilf(0.5f / cfg.cs)); // Agent radius in cells
    cfg.maxEdgeLen = static_cast<int>(12 / cfg.cs);              // Max edge length
    cfg.maxSimplificationError = 0.2f;      // Simplification error
    cfg.minRegionArea = 25;                  // Min region size
    cfg.mergeRegionArea = 100 * 100;         // Region merge size
    cfg.maxVertsPerPoly = 6;                 // Max vertices per polygon

    // Dynamically compute tile size to target ~16 tiles along the largest axis
    float span_x = bmax.x - bmin.x;
    float span_z = bmax.z - bmin.z;
    float max_span = std::max(span_x, span_z);
    int desired_ntiles = 16;
    float desired_tileWidth = max_span / desired_ntiles;
    cfg.tileSize = static_cast<int>(ceil(desired_tileWidth / cfg.cs));
    cfg.borderSize = static_cast<int>(ceilf(0.5f / cfg.cs)) + 5; // Border for tile overlap
    cfg.width = cfg.tileSize + cfg.borderSize * 2;
    cfg.height = cfg.tileSize + cfg.borderSize * 2;
    cfg.detailSampleDist = 6.0f;             // Detail mesh sampling distance
    cfg.detailSampleMaxError = 1.0f;         // Detail mesh max error

    // Calculate tile grid dimensions
    const float tileWidth = cfg.tileSize * cfg.cs;
    const int ntilesX = static_cast<int>(ceilf((bmax.x - bmin.x) / tileWidth));
    const int ntilesZ = static_cast<int>(ceilf((bmax.z - bmin.z) / tileWidth));
    const int maxTiles = ntilesX * ntilesZ * 10;

    // Initialize nav mesh parameters
    dtNavMeshParams navParams;
    memset(&navParams, 0, sizeof(navParams));
    rcVcopy(navParams.orig, &bmin.x);
    navParams.tileWidth = tileWidth;
    navParams.tileHeight = tileWidth;
    navParams.maxTiles = maxTiles;
    navParams.maxPolys = 65535;              // Max polygons per tile

    // Allocate and initialize the nav mesh
    navMesh = dtAllocNavMesh();
    if (!navMesh || dtStatusFailed(navMesh->init(&navParams))) {
        std::cerr << "Failed to initialize navMesh" << std::endl;
        dtFreeNavMesh(navMesh);
        navMesh = nullptr;
        return;
    }

    // Set up tile cache parameters
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
    tcParams.maxObstacles = 256;             // Max dynamic obstacles

    // Allocate memory management objects with increased size
    talloc = new LinearAllocator(1024 * 1024 * 100); // 100MB
    tcomp = new FastLZCompressor();

    // Initialize the tile cache
    tileCache = dtAllocTileCache();
    if (!tileCache || dtStatusFailed(tileCache->init(&tcParams, talloc, tcomp, nullptr))) {
        std::cerr << "Failed to initialize tileCache" << std::endl;
        dtFreeTileCache(tileCache);
        tileCache = nullptr;
        delete talloc;
        delete tcomp;
        return;
    }

    // Prepare geometry for Recast
    rcContext* ctx = new rcContext();
    std::vector<float> vertFloats(vertices.size() * 3);
    for (size_t i = 0; i < vertices.size(); ++i) {
        vertFloats[i * 3] = vertices[i].x;
        vertFloats[i * 3 + 1] = vertices[i].y;
        vertFloats[i * 3 + 2] = vertices[i].z;
    }
    std::vector<int> triInts(indices.begin(), indices.end());
    const int ntris = indices.size() / 3;

    // Process each tile
    for (int tz = 0; tz < ntilesZ; ++tz) {
        for (int tx = 0; tx < ntilesX; ++tx) {
            // Define tile bounding box
            float tbmin[3] = {
                navParams.orig[0] + tx * tileWidth,
                bmin.y - 0.1f, // Extend slightly below
                navParams.orig[2] + tz * tileWidth
            };
            float tbmax[3] = {
                navParams.orig[0] + (tx + 1) * tileWidth,
                bmax.y + 0.1f, // Extend slightly above
                navParams.orig[2] + (tz + 1) * tileWidth
            };
            rcVcopy(cfg.bmin, tbmin);
            rcVcopy(cfg.bmax, tbmax);

            // Create heightfield for the tile
            rcHeightfield hf;
            if (!rcCreateHeightfield(ctx, hf, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch)) {
                std::cerr << "Failed to create heightfield for tile (" << tx << "," << tz << ")" << std::endl;
                continue;
            }

            // Rasterize triangles into the heightfield
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

            // Erode walkable areas
            if (!rcErodeWalkableArea(ctx, cfg.walkableRadius, chf)) {
                std::cerr << "Failed to erode walkable area for tile (" << tx << "," << tz << ")" << std::endl;
                continue;
            }

            // Build regions using watershed partitioning
            if (!rcBuildDistanceField(ctx, chf)) {
                std::cerr << "Failed to build distance field for tile (" << tx << "," << tz << ")" << std::endl;
                continue;
            }
            if (!rcBuildRegions(ctx, chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea)) {
                std::cerr << "Failed to build regions for tile (" << tx << "," << tz << ")" << std::endl;
                continue;
            }

            // Build heightfield layers
            rcHeightfieldLayerSet* layers = rcAllocHeightfieldLayerSet();
            if (!layers || !rcBuildHeightfieldLayers(ctx, chf, cfg.borderSize, cfg.walkableHeight, *layers)) {
                std::cerr << "Failed to build heightfield layers for tile (" << tx << "," << tz << ")" << std::endl;
                rcFreeHeightfieldLayerSet(layers);
                continue;
            }

            // Process each layer in the tile
            for (int i = 0; i < layers->nlayers; ++i) {
                const rcHeightfieldLayer& layer = layers->layers[i];

                // Set up tile cache layer header
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

                // Allocate memory for layer data
                const int gridSize = layer.width * layer.height;



                // Build compressed tile data
                unsigned char* outData = nullptr;
                int outDataSize = 0;
                dtStatus status = dtBuildTileCacheLayer(tcomp, &header, layer.heights, layer.areas, layer.cons, &outData, &outDataSize);


                if (dtStatusFailed(status)) {
                    std::cerr << "Failed to build tile cache layer for tile (" << tx << "," << tz << ") layer " << i
                        << " (status: " << status << ")" << std::endl;
                    if (outData) talloc->free(outData);
                    continue;
                }

                // Add tile to the tile cache
                dtCompressedTileRef tileRef;
                status = tileCache->addTile(outData, outDataSize, DT_COMPRESSEDTILE_FREE_DATA, &tileRef);
                if (dtStatusFailed(status)) {
                    std::cerr << "Failed to add tile to cache for tile (" << tx << "," << tz << ") layer " << i
                        << " (status: " << status << ")" << std::endl;
                    talloc->free(outData);
                    continue;
                }

                // Build the nav mesh tile
                status = tileCache->buildNavMeshTile(tileRef, navMesh);
                if (dtStatusFailed(status)) {
                    std::cerr << "Failed to build navmesh tile for tile (" << tx << "," << tz << ") layer " << i
                        << " (status: " << status << ")" << std::endl;
                }
            }
            rcFreeHeightfieldLayerSet(layers);
        }
    }

    // Clean up
    delete ctx;
    // Note: talloc and tcomp are freed in DestroyNavData
}

bool HasLineOfSight(const vec3& pointA, const vec3& pointB)
{
    return Physics::SphereTrace(pointA, pointB, 0.4, BodyType::World).hasHit == false;
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
std::vector<glm::vec3> NavigationSystem::FindSimplePath(const glm::vec3& start, const glm::vec3& target)
{

    if (HasLineOfSight(start, target))
    {
        //return { target };
    }

    std::vector<glm::vec3> outPath;

    if (!navMesh)
        return outPath;

    std::lock_guard<std::mutex> lock(mainLock);

    // Create a navmesh query instance.
    dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
    if (!navQuery)
        return outPath;

    // Initialize the query. The 2048 value is the maximum number of nodes to use.
    dtStatus status = navQuery->init(navMesh, 2048);
    if (dtStatusFailed(status))
    {
        dtFreeNavMeshQuery(navQuery);
        return outPath;
    }

    // Setup a basic query filter.
    CustomFilter filter;
    filter.setIncludeFlags(0xffff);
    filter.setExcludeFlags(0);

    // Find the nearest polygon to the start and target positions.
    dtPolyRef startRef, endRef;
    float extents[3] = { 1, 4, 1 };  // search extents in each axis

    float startPos[3] = { start.x, start.y, start.z };
    float targetPos[3] = { target.x, target.y, target.z };

    status = navQuery->findNearestPoly(startPos, extents, &filter, &startRef, nullptr);
    if (dtStatusFailed(status) || !startRef)
    {
        dtFreeNavMeshQuery(navQuery);
        return outPath;
    }

    status = navQuery->findNearestPoly(targetPos, extents, &filter, &endRef, nullptr);
    if (dtStatusFailed(status) || !endRef)
    {
        dtFreeNavMeshQuery(navQuery);
        return outPath;
    }

    // Compute the polygon path.
    const int MAX_POLYS = 256;
    dtPolyRef polyPath[MAX_POLYS];
    int polyPathCount = 0;
    status = navQuery->findPath(startRef, endRef, startPos, targetPos, &filter,
        polyPath, &polyPathCount, MAX_POLYS);
    if (dtStatusFailed(status) || polyPathCount == 0)
    {
        dtFreeNavMeshQuery(navQuery);
        return outPath;
    }

    // Compute the straight path (a series of waypoints) from the polygon path.
    const int MAX_STRAIGHT_PATH = 256;
    float straightPath[MAX_STRAIGHT_PATH * 3];
    unsigned char straightPathFlags[MAX_STRAIGHT_PATH];
    dtPolyRef straightPathPolys[MAX_STRAIGHT_PATH];
    int straightPathCount = 0;
    status = navQuery->findStraightPath(startPos, targetPos, polyPath, polyPathCount,
        straightPath, straightPathFlags, straightPathPolys,
        &straightPathCount, MAX_STRAIGHT_PATH);
    if (dtStatusFailed(status))
    {
        dtFreeNavMeshQuery(navQuery);
        return outPath;
    }

    // Convert the computed straight path into a vector of glm::vec3 points.
    for (int i = 0; i < straightPathCount; ++i)
    {
        float* p = &straightPath[i * 3];
        outPath.emplace_back(p[0], p[1], p[2]);
    }

    dtFreeNavMeshQuery(navQuery);

    
    // ---------------------------------------------------------------------
    // Post-process the path by testing redundancy on the second and
    // second-from-last points based on line-of-sight.
    // ---------------------------------------------------------------------
    if (outPath.size() >= 2)
    {
        // Check the second point (index 1). If there is a clear line of sight
        // from the first point to the third point, then the second point is redundant.
        if (HasLineOfSight(outPath[1], start))
        {
            outPath.erase(outPath.begin());
        }
    }
    
    if (outPath.size() >= 1)
    {
        // Check the second-from-last point. If there is a clear line of sight from
        // the third-from-last point to the last point, then the second-from-last
        // point is redundant.
        size_t n = outPath.size();
        if (HasLineOfSight(outPath[n - 1], target))
        {
            outPath.erase(outPath.end()-1);
        }
    }
    
    outPath.push_back(target);

    return outPath;
}

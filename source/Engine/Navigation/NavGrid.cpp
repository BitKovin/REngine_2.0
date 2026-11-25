#include "NavGrid.h"
#include "Navigation.hpp"
#include "Detour/DetourCommon.h"

#include "../Camera.h"
#include "../DebugDraw.hpp"
#include "../Level.hpp"
#include "../FileSystem/FileSystem.h"
#include "../Helpers/ByteCompressor.h"

#include <Logger.hpp>

#include <algorithm>
#include <chrono>
#include <cfloat>
#include <cmath>
#include <cstring>

std::vector<NavGrid::Cell> NavGrid::cells;
std::unordered_map<uint64_t, std::vector<int>> NavGrid::columns;

namespace
{
    constexpr uint32_t CacheMagic = 'NGRD';
    constexpr uint32_t CacheFormat = 1;

    struct CacheHeader
    {
        uint32_t magic;
        uint32_t format;
        uint32_t version;   // modification time of the map file
        uint32_t polyCount; // ground polygons of the navmesh the grid was sampled from
        uint32_t cellCount;
        float cellSize;
    };

    // Two cells of one square are different floors when they are at least this far apart in height.
    constexpr float SameFloorHeight = 1.0f;

    // The most two linked cells may differ in height (stairs and ramps are within it, drops are not).
    constexpr float MaxStepHeight = 1.2f;

    int GridIndex(float coordinate)
    {
        return static_cast<int>(std::floor(coordinate / NavGrid::CellSize));
    }
}

glm::ivec2 NavGrid::Direction(int link)
{
    static const glm::ivec2 directions[LinkCount] = {
        { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 } };
    return directions[link & 7];
}

uint64_t NavGrid::ColumnKey(int ix, int iz)
{
    return (static_cast<uint64_t>(static_cast<uint32_t>(ix)) << 32) | static_cast<uint32_t>(iz);
}

void NavGrid::Clear()
{
    cells.clear();
    columns.clear();
}

void NavGrid::BuildColumns()
{
    columns.clear();
    for (int i = 0; i < static_cast<int>(cells.size()); ++i)
        columns[ColumnKey(GridIndex(cells[i].floor.x), GridIndex(cells[i].floor.z))].push_back(i);
}

void NavGrid::Build()
{
    Clear();

    if (NavigationSystem::navMesh == nullptr || Level::Current == nullptr)
        return;

    const auto startTime = std::chrono::steady_clock::now();

    // The grid is valid for a navmesh built from this version of the map.
    const std::string cachePath = Level::Current->filePath + ".navgrid";
    const uint32_t version = FileSystemEngine::GetFileModificationTime(Level::Current->filePath);

    uint32_t polyCount = 0;
    {
        const dtNavMesh* nav = NavigationSystem::navMesh;
        for (int t = 0; t < nav->getMaxTiles(); ++t)
        {
            const dtMeshTile* tile = nav->getTile(t);
            if (tile == nullptr || tile->header == nullptr) continue;
            polyCount += static_cast<uint32_t>(tile->header->polyCount);
        }
    }

    if (LoadCache(cachePath, version, polyCount))
    {
        BuildColumns();
        Logger::Info("nav grid loaded: %d cells", static_cast<int>(cells.size()));
        return;
    }

    if (BuildFromNavMesh() == false)
    {
        Clear();
        return;
    }

    SaveCache(cachePath, version, polyCount);

    const float ms = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - startTime).count();
    Logger::Info("nav grid built: %d cells in %.0f ms", static_cast<int>(cells.size()), ms);
}

bool NavGrid::BuildFromNavMesh()
{
    dtNavMesh* nav = NavigationSystem::navMesh;

    dtNavMeshQuery* query = dtAllocNavMeshQuery();
    if (query == nullptr)
        return false;

    if (dtStatusFailed(query->init(nav, 512)))
    {
        dtFreeNavMeshQuery(query);
        return false;
    }

    CustomFilter filter;
    filter.setIncludeFlags(0xffff);
    filter.setExcludeFlags(0);

    std::vector<dtPolyRef> polyOfCell; // the navmesh polygon each cell lies on, parallel to `cells`

    // ---- Cells: the centre of every grid square that lies on a ground polygon ----
    for (int t = 0; t < nav->getMaxTiles(); ++t)
    {
        const dtNavMesh* constNav = nav;
        const dtMeshTile* tile = constNav->getTile(t);
        if (tile == nullptr || tile->header == nullptr) continue;

        const dtPolyRef base = nav->getPolyRefBase(tile);

        for (int p = 0; p < tile->header->polyCount; ++p)
        {
            const dtPoly* poly = &tile->polys[p];
            if (poly->getType() != DT_POLYTYPE_GROUND) continue;

            float verts[DT_VERTS_PER_POLYGON * 3];
            glm::vec2 boundsMin(FLT_MAX), boundsMax(-FLT_MAX);
            for (int v = 0; v < poly->vertCount; ++v)
            {
                const float* src = &tile->verts[poly->verts[v] * 3];
                verts[v * 3 + 0] = src[0];
                verts[v * 3 + 1] = src[1];
                verts[v * 3 + 2] = src[2];
                boundsMin = glm::min(boundsMin, glm::vec2(src[0], src[2]));
                boundsMax = glm::max(boundsMax, glm::vec2(src[0], src[2]));
            }

            const dtPolyRef ref = base | static_cast<dtPolyRef>(p);

            for (int ix = GridIndex(boundsMin.x); ix <= GridIndex(boundsMax.x); ++ix)
            {
                for (int iz = GridIndex(boundsMin.y); iz <= GridIndex(boundsMax.y); ++iz)
                {
                    const float center[3] = { (ix + 0.5f) * CellSize, 0.0f, (iz + 0.5f) * CellSize };
                    if (dtPointInPolygon(center, verts, poly->vertCount) == false) continue;

                    float height = 0.0f;
                    if (dtStatusFailed(query->getPolyHeight(ref, center, &height))) continue;

                    // One cell per floor in the square.
                    std::vector<int>& column = columns[ColumnKey(ix, iz)];
                    bool sameFloor = false;
                    for (int other : column)
                    {
                        if (std::abs(cells[other].floor.y - height) < SameFloorHeight)
                        {
                            sameFloor = true;
                            break;
                        }
                    }
                    if (sameFloor) continue;

                    Cell cell;
                    cell.floor = glm::vec3(center[0], height, center[2]);
                    column.push_back(static_cast<int>(cells.size()));
                    cells.push_back(cell);
                    polyOfCell.push_back(ref);
                }
            }
        }
    }

    // ---- Links: neighbours that can be walked to along the navmesh ----
    for (int i = 0; i < static_cast<int>(cells.size()); ++i)
    {
        const glm::vec3 from = cells[i].floor;
        const int ix = GridIndex(from.x);
        const int iz = GridIndex(from.z);

        for (int d = 0; d < LinkCount / 2; ++d)
        {
            if (cells[i].link[d] >= 0) continue;

            const glm::ivec2 step = Direction(d);
            const auto column = columns.find(ColumnKey(ix + step.x, iz + step.y));
            if (column == columns.end()) continue;

            int best = -1;
            float bestStep = MaxStepHeight;
            for (int j : column->second)
            {
                const float dy = std::abs(cells[j].floor.y - from.y);
                if (dy <= bestStep)
                {
                    bestStep = dy;
                    best = j;
                }
            }
            if (best < 0) continue;

            // The navmesh decides whether the walk is free of walls and gaps.
            const glm::vec3 to = cells[best].floor;
            float t = 0.0f;
            float normal[3];
            dtPolyRef path[16];
            int pathCount = 0;
            if (dtStatusFailed(query->raycast(polyOfCell[i], &from.x, &to.x, &filter, &t, normal, path, &pathCount, 16)))
                continue;
            if (t < 1.0f) continue;

            cells[i].link[d] = best;
            cells[best].link[(d + LinkCount / 2) % LinkCount] = i;
        }
    }

    dtFreeNavMeshQuery(query);
    return cells.empty() == false;
}

bool NavGrid::LoadCache(const std::string& path, uint32_t version, uint32_t polyCount)
{
    const std::vector<uint8_t> compressed = FileSystemEngine::ReadFileBinary(path);
    if (compressed.empty())
        return false;

    const std::vector<uint8_t> buffer = ByteCompressor::DecompressData(compressed);
    if (buffer.size() < sizeof(CacheHeader))
        return false;

    CacheHeader header;
    std::memcpy(&header, buffer.data(), sizeof(header));

    if (header.magic != CacheMagic || header.format != CacheFormat || header.version != version
        || header.polyCount != polyCount || header.cellSize != CellSize)
        return false;

    if (buffer.size() != sizeof(CacheHeader) + static_cast<size_t>(header.cellCount) * sizeof(Cell))
        return false;

    cells.resize(header.cellCount);
    std::memcpy(cells.data(), buffer.data() + sizeof(CacheHeader), static_cast<size_t>(header.cellCount) * sizeof(Cell));
    return header.cellCount > 0;
}

void NavGrid::SaveCache(const std::string& path, uint32_t version, uint32_t polyCount)
{
#ifdef __EMSCRIPTEN__
    (void)path; (void)version; (void)polyCount;
    return;
#else
    CacheHeader header{};
    header.magic = CacheMagic;
    header.format = CacheFormat;
    header.version = version;
    header.polyCount = polyCount;
    header.cellCount = static_cast<uint32_t>(cells.size());
    header.cellSize = CellSize;

    std::vector<uint8_t> buffer(sizeof(CacheHeader) + cells.size() * sizeof(Cell));
    std::memcpy(buffer.data(), &header, sizeof(header));
    std::memcpy(buffer.data() + sizeof(CacheHeader), cells.data(), cells.size() * sizeof(Cell));

    FileSystemEngine::WriteFileBinary(path, ByteCompressor::CompressData(buffer));
#endif
}

int NavGrid::FindCell(const glm::vec3& position, float maxVertical)
{
    if (cells.empty()) return -1;

    const int ix = GridIndex(position.x);
    const int iz = GridIndex(position.z);

    int best = -1;
    float bestScore = FLT_MAX;

    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dz = -1; dz <= 1; ++dz)
        {
            const auto column = columns.find(ColumnKey(ix + dx, iz + dz));
            if (column == columns.end()) continue;

            for (int index : column->second)
            {
                const glm::vec3 delta = cells[index].floor - position;
                if (std::abs(delta.y) > maxVertical) continue;

                const float score = delta.x * delta.x + delta.z * delta.z + delta.y * delta.y;
                if (score < bestScore)
                {
                    bestScore = score;
                    best = index;
                }
            }
        }
    }

    return best;
}

void NavGrid::CellsInRadius(const glm::vec3& center, float radius, std::vector<int>& out, float maxVertical)
{
    if (cells.empty() || radius <= 0.0f) return;

    const float radiusSq = radius * radius;

    for (int ix = GridIndex(center.x - radius); ix <= GridIndex(center.x + radius); ++ix)
    {
        for (int iz = GridIndex(center.z - radius); iz <= GridIndex(center.z + radius); ++iz)
        {
            const auto column = columns.find(ColumnKey(ix, iz));
            if (column == columns.end()) continue;

            for (int index : column->second)
            {
                const glm::vec3 delta = cells[index].floor - center;
                if (delta.x * delta.x + delta.z * delta.z > radiusSq) continue;
                if (maxVertical > 0.0f && std::abs(delta.y) > maxVertical) continue;
                out.push_back(index);
            }
        }
    }
}

void NavGrid::DrawDebug()
{
    if (DebugDrawEnabled == false || cells.empty()) return;

    const float duration = 0.05f;
    const glm::vec3 lift(0.0f, 0.1f, 0.0f);

    std::vector<int> nearby;
    CellsInRadius(Camera::position, 25.0f, nearby);

    for (int index : nearby)
    {
        const Cell& cell = cells[index];
        DebugDraw::Line(cell.floor + lift, cell.floor + lift + glm::vec3(0.0f, 0.25f, 0.0f), duration, 0.02f, DebugColor::Green);

        // Each link once: the lower half of the directions.
        for (int d = 0; d < LinkCount / 2; ++d)
        {
            if (cell.link[d] < 0) continue;
            DebugDraw::Line(cell.floor + lift, cells[cell.link[d]].floor + lift, duration, 0.02f, DebugColor::Cyan);
        }
    }
}

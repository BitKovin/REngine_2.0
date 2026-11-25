#pragma once

#include "../glm.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// A sparse grid of walkable cells sampled from the navmesh when a level loads. Where the navmesh is a set of
// polygons for path queries, the grid is the shared canvas for spatial reasoning: posts and cover candidates,
// chase floods and search maps all work on its cells and links.
//
// Cells are CellSize metres across. A cell sits where the centre of a grid square lies on the navmesh, at the
// navmesh floor height there; a square with several floors (a bridge, the storeys of a building) has several
// cells. Each cell has up to 8 links to neighbouring cells that can be walked to directly (checked against the
// navmesh, so a thin wall or a drop between two cells breaks the link; stairs and ramps keep it).
//
// The grid is built once after the navmesh (NavGrid::Build) and cached next to the navmesh file. It is only read
// afterwards, so any thread may query it. Dynamic navmesh obstacles are not part of it.
class NavGrid
{
public:
    static constexpr float CellSize = 1.25f;
    static constexpr int LinkCount = 8;

    struct Cell
    {
        glm::vec3 floor{};                  // Centre of the cell, on the navmesh floor
        int32_t link[LinkCount] = { -1, -1, -1, -1, -1, -1, -1, -1 }; // Neighbour cell index per direction, -1 = none
    };

    // Link directions: 0 +x, 1 +x+z, 2 +z, 3 -x+z, 4 -x, 5 -x-z, 6 -z, 7 +x-z (the opposite of d is (d + 4) % 8).
    static glm::ivec2 Direction(int link);

    // Builds the grid from NavigationSystem::navMesh (or loads it from the cache file). Call after the navmesh
    // is generated or loaded. Leaves the grid empty when there is no navmesh.
    static void Build();

    static void Clear();

    static bool IsReady() { return !cells.empty(); }
    static int CellCount() { return static_cast<int>(cells.size()); }
    static const Cell& GetCell(int index) { return cells[index]; }

    // The cell under a position, the floor closest in height within maxVertical of it: -1 when there is none
    // (outside the navmesh). A position just over the border of a cell's square gets the nearest neighbour.
    static int FindCell(const glm::vec3& position, float maxVertical = 1.5f);

    // Appends every cell whose floor lies within radius of the centre (horizontal distance; vertical within
    // maxVertical of the centre when positive).
    static void CellsInRadius(const glm::vec3& center, float radius, std::vector<int>& out, float maxVertical = 0.0f);

    // Draws the cells and links near the camera (the world overlay for developers).
    static inline bool DebugDrawEnabled = false;
    static void DrawDebug();

private:
    static std::vector<Cell> cells;
    static std::unordered_map<uint64_t, std::vector<int>> columns; // grid square -> its cells

    static uint64_t ColumnKey(int ix, int iz);
    static void BuildColumns();
    static bool BuildFromNavMesh();
    static bool LoadCache(const std::string& path, uint32_t version, uint32_t polyCount);
    static void SaveCache(const std::string& path, uint32_t version, uint32_t polyCount);
};

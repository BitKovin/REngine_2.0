#pragma once

// Binary surgery on baked Detour tiles (addendum section 4.3, step 4 and 5).
//
// A tile blob is Detour's own layout (dtCreateNavMeshData): header, verts, polys, links, detail meshes, detail verts,
// detail tris, BV tree, off-mesh connections, each section 4-byte aligned. TileData splits a blob into those
// sections, edits them, and writes a new blob that dtNavMesh::addTile accepts.
//
// Edits the world bake needs:
//   * Translate: move the tile to another grid slot. Verts, detail verts, bounds and off-mesh positions move; the BV
//     tree is quantized relative to the tile bounds and needs no change.
//   * AddOffMeshConnection: append a portal link (an off-mesh poly, its two verts and the connection record).
//   * MaskBorder: turn the tile-border portal edges on one side into walls, so a neighbouring tile of another level
//     never stitches to this one.
//   * extraLinks: spare link capacity, needed by tiles where another tile's off-mesh connection lands (Detour
//     allocates the return link of a bidirectional connection in the landing tile).

#include <Navigation/Detour/DetourNavMesh.h>

#include <glm/vec3.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace sim::nav
{
	struct TileData
	{
		dtMeshHeader header{};
		std::vector<float> verts;              // 3 per vert; ground verts first, then 2 per off-mesh connection
		std::vector<dtPoly> polys;             // ground polys first, then one per off-mesh connection
		std::vector<dtPolyDetail> detailMeshes;
		std::vector<float> detailVerts;
		std::vector<uint8_t> detailTris;
		std::vector<dtBVNode> bvTree;
		std::vector<dtOffMeshConnection> offMeshCons;
		int extraLinks = 0;

		// Returns false (with a reason) for anything that is not a well-formed Detour tile of the expected version.
		static bool Parse(const uint8_t* data, size_t size, TileData& out, std::string& error);

		// A new dtAlloc'ed blob for dtNavMesh::addTile(..., DT_TILE_FREE_DATA, ...).
		uint8_t* Write(int& outSize) const;
		std::vector<uint8_t> WriteVector() const;

		int GroundPolyCount() const { return header.offMeshBase; }
		int GroundVertCount() const { return static_cast<int>(verts.size() / 3) - 2 * static_cast<int>(offMeshCons.size()); }

		// Moves the tile by `offset` (x and z should be whole tiles) and renames its grid slot.
		void Translate(const glm::vec3& offset, int newTileX, int newTileY);

		// Appends an off-mesh connection starting in this tile. `side` is the direction of the end point as Detour
		// classifies it (0xff inside this tile's slot, 0..7 for the neighbour slots, see dtNavMesh::getNeighbourTilesAt).
		void AddOffMeshConnection(const glm::vec3& start, const glm::vec3& end, float radius, bool bidirectional, uint8_t side,
			uint8_t area, uint16_t flags, uint32_t userId);

		// Removes the portal edges on `side` (0 = +x, 2 = +z, 4 = -x, 6 = -z). Returns how many edges were masked.
		int MaskBorder(int side);
	};

	// Detour's off-mesh end point classification for a point relative to a tile slot's bounds (x and z only).
	uint8_t ClassifySide(const glm::vec3& point, const float* slotMin, const float* slotMax);
}

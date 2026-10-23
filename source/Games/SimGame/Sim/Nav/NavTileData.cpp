#include "NavTileData.h"

#include <Navigation/Detour/DetourAlloc.h>
#include <Navigation/Detour/DetourCommon.h>

#include <cstring>

namespace sim::nav
{
	namespace
	{
		struct Sections
		{
			int header, verts, polys, links, detailMeshes, detailVerts, detailTris, bvTree, offMeshCons;
			int Total() const { return header + verts + polys + links + detailMeshes + detailVerts + detailTris + bvTree + offMeshCons; }
		};

		Sections SectionSizes(const dtMeshHeader& h)
		{
			Sections s;
			s.header = dtAlign4(sizeof(dtMeshHeader));
			s.verts = dtAlign4(sizeof(float) * 3 * h.vertCount);
			s.polys = dtAlign4(sizeof(dtPoly) * h.polyCount);
			s.links = dtAlign4(sizeof(dtLink) * h.maxLinkCount);
			s.detailMeshes = dtAlign4(sizeof(dtPolyDetail) * h.detailMeshCount);
			s.detailVerts = dtAlign4(sizeof(float) * 3 * h.detailVertCount);
			s.detailTris = dtAlign4(sizeof(unsigned char) * 4 * h.detailTriCount);
			s.bvTree = dtAlign4(sizeof(dtBVNode) * h.bvNodeCount);
			s.offMeshCons = dtAlign4(sizeof(dtOffMeshConnection) * h.offMeshConCount);
			return s;
		}

		template<class T>
		void CopyOut(const uint8_t*& cursor, int sectionSize, int count, std::vector<T>& out)
		{
			out.resize(count);
			if (count)
				std::memcpy(out.data(), cursor, sizeof(T) * count);
			cursor += sectionSize;
		}

		template<class T>
		void CopyIn(uint8_t*& cursor, int sectionSize, const std::vector<T>& in)
		{
			if (!in.empty())
				std::memcpy(cursor, in.data(), sizeof(T) * in.size());
			cursor += sectionSize;
		}

		void Offset(float* xyz, const glm::vec3& offset)
		{
			xyz[0] += offset.x;
			xyz[1] += offset.y;
			xyz[2] += offset.z;
		}
	}

	bool TileData::Parse(const uint8_t* data, size_t size, TileData& out, std::string& error)
	{
		out = TileData{};
		if (!data || size < sizeof(dtMeshHeader))
		{
			error = "tile blob is smaller than a header";
			return false;
		}
		std::memcpy(&out.header, data, sizeof(dtMeshHeader));
		const dtMeshHeader& h = out.header;
		if (h.magic != DT_NAVMESH_MAGIC || h.version != DT_NAVMESH_VERSION)
		{
			error = "not a Detour tile of version " + std::to_string(DT_NAVMESH_VERSION);
			return false;
		}
		if (h.vertCount < 0 || h.polyCount < 0 || h.maxLinkCount < 1 || h.detailMeshCount < 0 || h.detailVertCount < 0 || h.detailTriCount < 0 ||
			h.bvNodeCount < 0 || h.offMeshConCount < 0 || h.offMeshBase < 0 || h.offMeshBase + h.offMeshConCount != h.polyCount ||
			h.vertCount < 2 * h.offMeshConCount)
		{
			error = "tile header counts are inconsistent";
			return false;
		}
		const Sections s = SectionSizes(h);
		if (static_cast<size_t>(s.Total()) > size)
		{
			error = "tile blob is shorter than its header says";
			return false;
		}

		const uint8_t* cursor = data + s.header;
		CopyOut(cursor, s.verts, h.vertCount * 3, out.verts);
		CopyOut(cursor, s.polys, h.polyCount, out.polys);
		cursor += s.links; // runtime data, rebuilt by addTile
		CopyOut(cursor, s.detailMeshes, h.detailMeshCount, out.detailMeshes);
		CopyOut(cursor, s.detailVerts, h.detailVertCount * 3, out.detailVerts);
		CopyOut(cursor, s.detailTris, h.detailTriCount * 4, out.detailTris);
		CopyOut(cursor, s.bvTree, h.bvNodeCount, out.bvTree);
		CopyOut(cursor, s.offMeshCons, h.offMeshConCount, out.offMeshCons);

		for (const dtPoly& poly : out.polys)
			for (int i = 0; i < poly.vertCount && i < DT_VERTS_PER_POLYGON; ++i)
				if (poly.verts[i] >= h.vertCount)
				{
					error = "a polygon references a vertex outside the tile";
					return false;
				}
		return true;
	}

	uint8_t* TileData::Write(int& outSize) const
	{
		dtMeshHeader h = header;
		h.vertCount = static_cast<int>(verts.size() / 3);
		h.polyCount = static_cast<int>(polys.size());
		h.maxLinkCount = header.maxLinkCount + extraLinks;
		h.detailMeshCount = static_cast<int>(detailMeshes.size());
		h.detailVertCount = static_cast<int>(detailVerts.size() / 3);
		h.detailTriCount = static_cast<int>(detailTris.size() / 4);
		h.bvNodeCount = static_cast<int>(bvTree.size());
		h.offMeshConCount = static_cast<int>(offMeshCons.size());

		const Sections s = SectionSizes(h);
		outSize = s.Total();
		uint8_t* data = static_cast<uint8_t*>(dtAlloc(outSize, DT_ALLOC_PERM));
		if (!data)
			return nullptr;
		std::memset(data, 0, outSize);
		std::memcpy(data, &h, sizeof(h));

		uint8_t* cursor = data + s.header;
		CopyIn(cursor, s.verts, verts);
		CopyIn(cursor, s.polys, polys);
		cursor += s.links;
		CopyIn(cursor, s.detailMeshes, detailMeshes);
		CopyIn(cursor, s.detailVerts, detailVerts);
		CopyIn(cursor, s.detailTris, detailTris);
		CopyIn(cursor, s.bvTree, bvTree);
		CopyIn(cursor, s.offMeshCons, offMeshCons);
		return data;
	}

	std::vector<uint8_t> TileData::WriteVector() const
	{
		int size = 0;
		uint8_t* data = Write(size);
		std::vector<uint8_t> result;
		if (data)
		{
			result.assign(data, data + size);
			dtFree(data);
		}
		return result;
	}

	void TileData::Translate(const glm::vec3& offset, int newTileX, int newTileY)
	{
		for (size_t i = 0; i + 2 < verts.size(); i += 3)
			Offset(&verts[i], offset);
		for (size_t i = 0; i + 2 < detailVerts.size(); i += 3)
			Offset(&detailVerts[i], offset);
		for (dtOffMeshConnection& con : offMeshCons)
		{
			Offset(&con.pos[0], offset);
			Offset(&con.pos[3], offset);
		}
		Offset(header.bmin, offset);
		Offset(header.bmax, offset);
		header.x = newTileX;
		header.y = newTileY;
	}

	void TileData::AddOffMeshConnection(const glm::vec3& start, const glm::vec3& end, float radius, bool bidirectional, uint8_t side,
		uint8_t area, uint16_t flags, uint32_t userId)
	{
		const unsigned short base = static_cast<unsigned short>(verts.size() / 3);
		verts.insert(verts.end(), { start.x, start.y, start.z, end.x, end.y, end.z });

		dtPoly poly{};
		poly.verts[0] = base;
		poly.verts[1] = static_cast<unsigned short>(base + 1);
		poly.vertCount = 2;
		poly.flags = flags;
		poly.setArea(area);
		poly.setType(DT_POLYTYPE_OFFMESH_CONNECTION);
		polys.push_back(poly);

		dtOffMeshConnection con{};
		con.pos[0] = start.x;
		con.pos[1] = start.y;
		con.pos[2] = start.z;
		con.pos[3] = end.x;
		con.pos[4] = end.y;
		con.pos[5] = end.z;
		con.rad = radius;
		con.poly = static_cast<unsigned short>(polys.size() - 1);
		con.flags = bidirectional ? DT_OFFMESH_CON_BIDIR : 0;
		con.side = side;
		con.userId = userId;
		offMeshCons.push_back(con);

		// Base links (poly to start, start back to poly) and the landing link; spare one for safety.
		extraLinks += 4;
	}

	int TileData::MaskBorder(int side)
	{
		int masked = 0;
		const unsigned short portal = static_cast<unsigned short>(DT_EXT_LINK | side);
		for (int p = 0; p < header.offMeshBase; ++p)
		{
			dtPoly& poly = polys[p];
			for (int i = 0; i < poly.vertCount; ++i)
				if (poly.neis[i] == portal)
				{
					poly.neis[i] = 0;
					++masked;
				}
		}
		return masked;
	}

	uint8_t ClassifySide(const glm::vec3& point, const float* slotMin, const float* slotMax)
	{
		constexpr uint8_t XP = 1 << 0, ZP = 1 << 1, XM = 1 << 2, ZM = 1 << 3;
		uint8_t code = 0;
		code |= point.x >= slotMax[0] ? XP : 0;
		code |= point.z >= slotMax[2] ? ZP : 0;
		code |= point.x < slotMin[0] ? XM : 0;
		code |= point.z < slotMin[2] ? ZM : 0;
		switch (code)
		{
		case XP: return 0;
		case XP | ZP: return 1;
		case ZP: return 2;
		case XM | ZP: return 3;
		case XM: return 4;
		case XM | ZM: return 5;
		case ZM: return 6;
		case XP | ZM: return 7;
		default: return 0xff;
		}
	}
}

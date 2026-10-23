#include "WorldNavDebug.h"

#include <cmath>

namespace sim::nav
{
	namespace
	{
		constexpr float Lift = 0.05f;        // keeps the lines off the floor so they do not z-fight with it
		constexpr float MarkerRadius = 0.5f;
		constexpr float BeamHeight = 3.0f;

		void AddMarker(std::vector<DebugSegment>& out, const glm::vec3& position, DebugKind kind)
		{
			const glm::vec3 base = position + glm::vec3(0.0f, Lift, 0.0f);
			const glm::vec3 east = base + glm::vec3(MarkerRadius, 0.0f, 0.0f), west = base - glm::vec3(MarkerRadius, 0.0f, 0.0f);
			const glm::vec3 north = base + glm::vec3(0.0f, 0.0f, MarkerRadius), south = base - glm::vec3(0.0f, 0.0f, MarkerRadius);
			out.push_back({ east, north, kind });
			out.push_back({ north, west, kind });
			out.push_back({ west, south, kind });
			out.push_back({ south, east, kind });
			out.push_back({ base, base + glm::vec3(0.0f, BeamHeight, 0.0f), kind });
		}

		float HorizontalDistance(const glm::vec3& a, const glm::vec3& b)
		{
			const float dx = a.x - b.x, dz = a.z - b.z;
			return std::sqrt(dx * dx + dz * dz);
		}
	}

	void CollectDebugSegments(const WorldNav& nav, int level, const glm::vec3& centre, float radius, std::vector<DebugSegment>& out)
	{
		const dtNavMesh* mesh = nav.Mesh();
		if (!mesh)
			return;
		const WorldNavData& data = nav.Data();
		const bool wholeWorld = level < 0;
		if (!wholeWorld && level >= static_cast<int>(data.levels.size()))
			return;
		const glm::vec3 offset = wholeWorld ? glm::vec3(0.0f) : data.levels[level].offset;

		for (int t = 0; t < mesh->getMaxTiles(); ++t)
		{
			const dtMeshTile* tile = mesh->getTile(t);
			if (!tile || !tile->header || (!wholeWorld && static_cast<int>(tile->header->userId) != level))
				continue;
			for (int p = 0; p < tile->header->polyCount; ++p)
			{
				const dtPoly& poly = tile->polys[p];
				if (poly.getType() != DT_POLYTYPE_GROUND)
					continue;
				for (int k = 0; k < poly.vertCount; ++k)
				{
					const unsigned short neighbour = poly.neis[k];
					// A shared edge belongs to two polygons: draw it from the one with the lower index. Tile borders
					// (neighbour in another tile) are drawn from both sides, which is harmless.
					const bool border = neighbour == 0;
					if (!border && !(neighbour & DT_EXT_LINK) && static_cast<int>(neighbour) - 1 < p)
						continue;
					const float* v0 = &tile->verts[poly.verts[k] * 3];
					const float* v1 = &tile->verts[poly.verts[(k + 1) % poly.vertCount] * 3];
					const glm::vec3 a = glm::vec3(v0[0], v0[1], v0[2]) - offset + glm::vec3(0.0f, Lift, 0.0f);
					const glm::vec3 b = glm::vec3(v1[0], v1[1], v1[2]) - offset + glm::vec3(0.0f, Lift, 0.0f);
					if (radius > 0.0f && HorizontalDistance(a, centre) > radius && HorizontalDistance(b, centre) > radius)
						continue;
					out.push_back({ a, b, border ? DebugKind::Border : DebugKind::Edge });
				}
			}
		}

		for (const WorldLink& link : data.links)
		{
			if (wholeWorld || link.fromLevel == level)
				AddMarker(out, link.start - offset, link.linked ? DebugKind::LinkStart : DebugKind::LinkStartBroken);
			if (link.linked && (wholeWorld || link.toLevel == level))
				AddMarker(out, link.end - offset, DebugKind::LinkEnd);
		}
	}
}

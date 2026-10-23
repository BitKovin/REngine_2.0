#include "WorldNavReport.h"

#include <glm/geometric.hpp>

#include <cmath>
#include <cstdarg>
#include <cstdio>

namespace sim::nav
{
	namespace
	{
		std::string Fmt(const char* format, ...)
		{
			char buffer[512];
			va_list args;
			va_start(args, format);
			std::vsnprintf(buffer, sizeof(buffer), format, args);
			va_end(args);
			return buffer;
		}
	}

	std::string BuildNavReport(const WorldNav& nav)
	{
		const WorldNavData& data = nav.Data();
		std::string out = Fmt("world nav report: %zu levels, %zu spawn points, %zu portal links. Coordinates are level-local metres (x y z).\n", data.levels.size(), data.spawns.size(), data.links.size());

		for (size_t level = 0; level < data.levels.size(); ++level)
		{
			const WorldLevel& info = data.levels[level];
			glm::vec3 low(1e30f), high(-1e30f);
			for (const WorldTile& tile : data.tiles)
			{
				if (tile.level != level)
					continue;
				const dtMeshHeader* header = reinterpret_cast<const dtMeshHeader*>(tile.data.data());
				low = glm::min(low, glm::vec3(header->bmin[0], header->bmin[1], header->bmin[2]) - info.offset);
				high = glm::max(high, glm::vec3(header->bmax[0], header->bmax[1], header->bmax[2]) - info.offset);
			}
			out += Fmt("\nlevel %zu %s: %d tiles, walkable area x %.1f..%.1f, y %.1f..%.1f, z %.1f..%.1f\n", level, info.name.c_str(), info.tileCount, low.x, high.x, low.y, high.y, low.z, high.z);

			for (const WorldLink& link : data.links)
			{
				if (link.fromLevel == level)
					out += Fmt("  portal %-24s at (%.1f %.1f %.1f) -> %s (%.1f %.1f %.1f)%s\n", link.portal.c_str(), link.start.x - info.offset.x, link.start.y, link.start.z - info.offset.z,
						data.levels[link.toLevel].name.c_str(), link.end.x - data.levels[link.toLevel].offset.x, link.end.y, link.end.z - data.levels[link.toLevel].offset.z,
						link.linked ? "" : "  NOT LINKED");
			}

			for (const WorldSpawn& spawn : data.spawns)
			{
				if (spawn.level != level)
					continue;
				const glm::vec3 local = nav.ToLocal(static_cast<int>(level), spawn.position);
				out += Fmt("  spawn %-20s at (%.1f %.1f %.1f)%s\n", spawn.name.c_str(), local.x, local.y, local.z, spawn.playerStart ? "  [player start]" : "");

				// Walkable ground around it: rings of samples that the mesh holds and that can be walked to without a detour.
				const NavPoint origin = nav.Project(spawn.position);
				if (!origin.IsValid())
					continue;
				int shown = 0;
				for (const float radius : { 6.0f, 14.0f })
					for (int step = 0; step < 8; ++step)
					{
						const float angle = static_cast<float>(step) * 6.2831853f / 8.0f;
						const glm::vec3 wanted = spawn.position + glm::vec3(std::cos(angle) * radius, 0.0f, std::sin(angle) * radius);
						const NavPoint point = nav.Project(wanted, glm::vec3(0.4f, 3.0f, 0.4f));
						if (!point.IsValid())
							continue;
						const NavPath path = nav.FindPath(origin, point);
						if (!path.complete || path.length > radius * 1.4f || path.portalCrossings > 0)
							continue;
						const glm::vec3 spot = nav.ToLocal(static_cast<int>(level), point.position);
						out += Fmt("      near %-16s (%.1f %.1f %.1f)  %.1f m away, walk %.1f m\n", spawn.name.c_str(), spot.x, spot.y, spot.z, glm::distance(point.position, spawn.position), path.length);
						++shown;
					}
				if (shown == 0)
					out += "      (no walkable ring samples)\n";
			}
		}
		return out;
	}
}

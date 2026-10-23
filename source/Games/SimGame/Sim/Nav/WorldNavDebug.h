#pragma once

// Geometry for drawing the world mesh and its portal links (debug aid). Pure: it reads a WorldNav and returns line
// segments; the bridge hands them to the engine's DebugDraw (Sim/Bridge/SimNavDebugDraw).

#include "WorldNav.h"

namespace sim::nav
{
	enum class DebugKind : uint8_t
	{
		Edge,            // polygon edge shared with a neighbouring polygon
		Border,          // polygon edge at the boundary of the walkable surface
		LinkStart,       // portal trigger end of a link (level of origin)
		LinkStartBroken, // portal whose two levels could not be laid out as neighbours: it is not in the mesh
		LinkEnd,         // landing end of a link
	};

	struct DebugSegment
	{
		glm::vec3 a{ 0.0f };
		glm::vec3 b{ 0.0f };
		DebugKind kind = DebugKind::Edge;
	};

	// Appends the segments of one level in level-local coordinates (what the player sees in that level), or of the
	// whole world in nav coordinates when `level` is negative.
	// Polygon edges are kept when an end is within `radius` metres (horizontal distance) of `centre`; radius <= 0 keeps
	// all. Link markers (a diamond on the ground and a beam) are always kept so portals can be found from afar.
	void CollectDebugSegments(const WorldNav& nav, int level, const glm::vec3& centre, float radius, std::vector<DebugSegment>& out);
}

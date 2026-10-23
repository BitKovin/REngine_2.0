#pragma once

// A text report of the baked world for people who author content (NPC routes, spawn points): per level its local extent,
// spawn points and portals in level-local coordinates, and walkable sample points around every spawn point with the walking
// distance to them. Pure: reads a WorldNav, returns text.

#include "WorldNav.h"

#include <string>

namespace sim::nav
{
	std::string BuildNavReport(const WorldNav& nav);
}

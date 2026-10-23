#pragma once

// Walking along a path, as plain functions (addendum 7.1). No World, no events: the movement module wraps these.
//
// A path is a list of corners in nav space, each with the level it lies in. Corners i and i+1 in different levels
// are a portal: the agent walks to corner i, waits the portal delay, and continues from corner i+1. The straight line
// between the two corners is never walked, so it does not count towards distance or time.
//
// Progress is (leg, distance along the leg) in double precision, and the position is derived from it, so one big step
// and many small steps end at the same place.

#include <glm/vec3.hpp>

#include <cstdint>
#include <vector>

namespace sim::movement
{
	struct PathView
	{
		const std::vector<glm::vec3>* corners = nullptr;
		const std::vector<uint16_t>* levels = nullptr;

		size_t Count() const { return corners ? corners->size() : 0; }
	};

	struct Progress
	{
		uint32_t leg = 0;          // walking from corner `leg` towards corner `leg + 1`
		bool crossing = false;     // standing at corner `leg`, waiting to cross the portal to `leg + 1`
		double legDistance = 0.0;  // metres walked along the current leg
		double crossingLeft = 0.0; // seconds of the portal delay still to wait
		template<class Ar> void Visit(Ar& ar)
		{
			ar("leg", leg);
			ar("crossing", crossing);
			ar("legDistance", legDistance);
			ar("crossingLeft", crossingLeft);
		}
	};

	struct AdvanceResult
	{
		double timeUsed = 0.0;               // seconds of the budget spent (less than the budget when the path ended)
		double walked = 0.0;                 // metres walked (portal jumps excluded)
		bool arrived = false;                // progress is at the last corner
		std::vector<uint32_t> crossings;     // portals taken, as the index of the corner they started at
	};

	inline bool IsPortalLeg(const PathView& path, uint32_t leg) { return leg + 1 < path.Count() && (*path.levels)[leg] != (*path.levels)[leg + 1]; }

	// Spends up to `seconds` walking at `speed` metres per second; each portal costs `portalDelay` seconds.
	AdvanceResult Advance(const PathView& path, Progress& progress, double seconds, double speed, double portalDelay);

	glm::vec3 PositionOf(const PathView& path, const Progress& progress);

	// The level the agent is in (the level of the corner it is walking from); -1 for an empty path.
	int LevelOf(const PathView& path, const Progress& progress);

	// Metres of walking along the whole path (portal jumps excluded).
	double WalkLength(const PathView& path);
}

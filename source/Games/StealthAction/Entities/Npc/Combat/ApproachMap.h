#pragma once

#include <glm.h>

#include <Navigation/Navigation.hpp>

#include <vector>

// One way into the target's sight: a doorway, a corridor mouth, the open side
// of a yard. Every walkable route toward the target reaches it through exactly
// one lane (COMBAT_AI_DESIGN.md §8.7).
struct ApproachLane
{
	vec3 entry{};      // where the routes of this lane come into the target's sight
	vec3 stack{};      // hidden spot just short of the entry: where an assault waits
	vec3 direction{};  // from the target toward the entry (XZ, normalized)
	float entryCost = 0.0f;  // path metres from the target to the entry
	float stackCost = 0.0f;  // path metres from the target to the stack spot
	int hiddenNodes = 0;     // how much hidden ground lies behind it (a crate's shadow is not a lane)
};

// How the ground around one target is reached, built from a navmesh
// shortest-path tree rooted at the target (NavigationSystem::BuildReachTree):
// which places the target can see, and for each place it can't, through which
// lane a route from there comes into its sight. The squad plan sends flankers
// to lanes away from the one the squad fights through.
//
// Built and read on the main thread (CombatCoordinator::Step).
class ApproachMap
{
public:
	static constexpr int MaxNodes = 512;

	// eye: where the target looks from; `target` is its centre.
	bool Build(const vec3& target, const vec3& eye, double now);

	bool NeedsRebuild(const vec3& target, double now) const;

	bool Valid() const { return valid; }
	const std::vector<ApproachLane>& Lanes() const { return lanes; }

	// The lane a position reaches the target through, or the lane facing it for
	// a place inside the target's sight or off the map. -1 when there are no lanes.
	int LaneOf(const vec3& position) const;

	// Signed angle (degrees) between two lanes' directions round the target.
	float AngleBetween(int laneA, int laneB) const;

	// Entries, stack spots and lane lines (debug window open).
	void DebugDraw() const;

private:
	bool valid = false;
	vec3 builtFor{};
	double builtAt = -1000.0;

	std::vector<NavReachNode> nodes;
	std::vector<char> visible;
	std::vector<int> laneOfNode; // -1 for nodes in sight
	std::vector<ApproachLane> lanes;

	int LaneFacing(const vec3& position) const;
};

#pragma once

#include "CoverTypes.h"

#include <functional>
#include <vector>

class Entity;

struct CoverQuery
{
	const Entity* self = nullptr; // excluded from the ally spread penalty

	vec3 npcPosition{};           // NPC centre
	float npcCenterHeight = 0.9f; // centre above the ground

	vec3 threatEye{};
	vec3 threatPosition{};

	// Distance band to the threat the NPC wants to fight from.
	float minThreatDistance = 5.0f;
	float maxThreatDistance = 17.0f;
	float preferredThreatDistance = 10.0f;

	float searchRadius = 9.0f;

	// An NPC never runs far for cover, and never backs off much further from the
	// threat: in a corridor the nearest real cover can be round a distant corner,
	// and sprinting off to it looks like fleeing.
	float maxTravelDistance = 8.0f;  // straight-line metres from where it stands
	float maxRetreat = 3.0f;         // how much further from the threat the spot may be

	// Spots within avoidRadius of avoidPosition are skipped (the cover just left).
	vec3 avoidPosition{};
	float avoidRadius = 0.0f;

	// The squad fights the target through this entry (doorway, corridor mouth):
	// spots in its line of fire, out past the entry, score lower than spots to
	// the side of it, so a target coming out walks past guns, not into them.
	bool hasLaneEntry = false;
	vec3 laneEntry{};
	int traceBudget = 48; // cache probes; unknown sectors beyond it are skipped this query
};

// Where an NPC goes to get a line on the target from somewhere else
// (COMBAT_AI_DESIGN.md §6.7): a spot that sees the target (or the estimate of
// a hiding one) from minAngleDegrees..maxAngleDegrees round from a base
// direction, so it comes at the target from the side instead of straight on.
//
// Of the reachable candidates the one with the best route wins: short, and
// out of the target's sight for as much of the way as possible, never through
// the target's face.
struct FlankQuery
{
	const Entity* self = nullptr;

	vec3 npcPosition{};           // NPC centre
	float npcCenterHeight = 0.9f;

	vec3 estimatePosition{};      // centre of the target, or of the estimate of a hiding one
	vec3 threatEye{};             // where the target looks from; zero = estimate + 0.6 m

	// Angles are measured from baseDirection (from the target outward, XZ);
	// zero = from the target toward the NPC. side -1 / +1 keeps to one side
	// (the same convention as the squad plan), 0 = either.
	vec3 baseDirection{};
	float side = 0.0f;

	float minDistance = 5.0f;     // from the estimate
	float maxDistance = 14.0f;
	float minAngleDegrees = 60.0f;
	float maxAngleDegrees = 130.0f;
	float maxPathLength = 35.0f;

	// The route is kept this far from the target (no running past its nose).
	float routeClearance = 3.5f;

	// Extra metres of route a stretch in the target's sight counts as, per metre.
	float exposedRouteCost = 1.5f;
};

// Finds a spot that hides the NPC from a threat and still lets it shoot
// (COMBAT_AI_DESIGN.md §10.1): candidates on rings around the NPC plus the
// cover_point hints, scored on cached cover knowledge, the best few verified
// with real traces and a path check.
class CoverFinder
{
public:
	// Tries candidates best first; the first that verifies and that `claim`
	// accepts (the reservation) wins.
	using Claim = std::function<bool(const vec3& floor)>;

	static bool Find(const CoverQuery& query, const Claim& claim, CoverSpot& out);

	// Fills `outPosition` (NPC centre height) with a reachable spot that sees
	// the estimate from the side.
	static bool FindFlankPosition(const FlankQuery& query, vec3& outPosition);

	// True while the spot still hides a crouched (Low) or standing (Full)
	// NPC from the threat.
	static bool IsProtective(const CoverSpot& spot, const vec3& threatEye);

	// Cost of walking `path` toward a target at `targetPosition` seeing from
	// `targetEye`: its length plus `exposedRouteCost` per metre in the target's
	// sight. False when the route comes within `clearance` of the target.
	static bool RouteCost(const std::vector<vec3>& path, const vec3& targetPosition, const vec3& targetEye,
		float clearance, float exposedRouteCost, float& outCost);

	// cover_point hints
	static void AddHint(const CoverHint& hint);
	static void RemoveHint(const std::string& entityId);
};

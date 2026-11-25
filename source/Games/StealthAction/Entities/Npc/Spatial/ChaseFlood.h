#pragma once

// Where a lost target most likely went (NPC_AI_DESIGN.md §11.3, after the chase of Dishonored 2): heat is put on the cell
// where it was last known, a barrier on the cells behind the way it was going, and both spread over the NavGrid one
// step at a time. It ends when nothing heats any more, when too many cells heat in one step (an open room: the chase
// would just be a crowd) or after a number of steps. The chase destination is the centroid of what is warm.
//
// It reads only what the chasers know (a position and a velocity of a record), never the target itself.

#include "../Brain/NpcData.h"

#include <glm.h>

#include <unordered_map>
#include <unordered_set>
#include <vector>

class ChaseFlood
{
public:

	// Starts over from a known position and velocity.
	void Start(const glm::vec3& position, const glm::vec3& velocity, const NpcChaseTuning& tuning);

	// Runs up to `steps` steps; true once it has ended.
	bool Advance(int steps);

	bool Done() const { return done; }

	// The chase destination: where the flood began until it has ended, the centroid of the warm cells after.
	const glm::vec3& Destination() const { return destination; }

	// Where it started (and with what), to tell whether news is new enough to run it again.
	const glm::vec3& Origin() const { return origin; }

	int WarmCells() const { return static_cast<int>(heat.size()); }
	int Steps() const { return steps; }

private:

	NpcChaseTuning tuning;

	glm::vec3 origin = glm::vec3(0.0f);
	glm::vec3 behind = glm::vec3(0.0f);  // flat unit vector pointing back along the way it came; zero without a velocity
	glm::vec3 destination = glm::vec3(0.0f);
	bool done = true;
	int steps = 0;

	std::unordered_map<int, float> heat;     // warm cells and their heat
	std::unordered_set<int> barrier;         // cells the heat does not enter
	std::vector<int> heatFront;
	std::vector<int> barrierFront;

	bool IsBehind(int cell) const;
	void Finish();
};

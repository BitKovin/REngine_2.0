#pragma once

// The game side of materialization (SLICE_M2C_BODIES.md): answers the sim's body proposals, owns the SimNpc entities of the loaded
// level, forwards the sim's goals to them and reports where they stand. Engine glue, driven by SimBridge once per frame.
//
// Every proposal is accepted: the body appears where the agent is when it is spawned (the newest snapshot), not where it was when
// the sim asked. If the agent changed level meanwhile (it took a portal) there is no body.
//
// The rules a player can see, in one place (owner: NPCs fade in and out, also in sight and close by):
//   - after a level loads (InitialSpawnWindow seconds) bodies appear at once: the loading screen hid it
//   - later, a body appears at once when its spot is out of sight (outside a 70 degree half cone of the camera, behind world geometry, or
//     in a BSP cluster the camera cannot see), and fades in over SimNpc::FadeSeconds when the camera can see it
//   - a body goes when its agent reaches a portal, leaves the range, or the player changes level: at once when out of sight, faded out
//     over SimNpc::FadeSeconds when in sight (it keeps walking and reporting meanwhile)
//   - every appearance and disappearance is logged as "sim body: <id> ..." with the reason

#include <Sim/Bridge/SimClient.h>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace SimAgents
{
	constexpr double SnapshotWait = 1.0;          // seconds a proposal waits for the snapshot that shows its agent
	constexpr double ReportInterval = 0.2;        // seconds between position reports of a body
	constexpr double InitialSpawnWindow = 3.0;    // seconds after a level load in which bodies appear without a fade

	struct Counters
	{
		uint64_t proposals = 0;       // spawn proposals received
		uint64_t spawnedHidden = 0;   // bodies created at once (out of sight, or right after a load)
		uint64_t spawnedVisible = 0;  // bodies created in sight (faded in)
		uint64_t refused = 0;
		uint64_t despawned = 0;
		uint64_t reports = 0;
		uint64_t goals = 0;
	};

	// Call every frame. Handles level changes (bodies die with their level), the sim's output events, proposals and reports.
	void Update(sim::SimClient& client, const sim::nav::WorldNavData& world);

	// Forget every body and proposal (the sim restarted, or its world was replaced by a load).
	void Reset();

	size_t BodyCount();
	// Sim entity and level-local feet position of every body.
	std::vector<std::pair<sim::Entity, glm::vec3>> BodyPositions();
	size_t PendingCount();
	const Counters& GetCounters();
}

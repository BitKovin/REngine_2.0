#pragma once

// The game side of materialization (SLICE_M2C_BODIES.md): answers the sim's body proposals, owns the SimNpc entities of the loaded
// level, forwards the sim's goals to them and reports where they stand. Engine glue, driven by SimBridge once per frame.
//
// Every proposal is accepted: the body appears where the agent is when it is spawned (the newest snapshot), not where it was when
// the sim asked. If the agent changed level meanwhile (it took a portal) there is no body.
//
// The rules a player can see, in one place (owner: NPCs fade in and out, also in sight and close by; SLICE_M2F_CHARACTERS.md section 6):
//   - bodies are made farther out than they are shown: between VisibleRadius and the sim's spawn radius a body is Warming (it walks and animates,
//     it is not drawn and casts no shadow), so it is already at pace when it is shown; within the radius, once it has lived SimNpc::WarmupSeconds, it is
//     shown at once (faded in only if it was not warmed up and the camera sees it); BodyVisibility.h holds the decision, with a hysteresis when it walks away
//   - after a level loads (InitialSpawnWindow seconds) bodies within the visible radius appear at once: the loading screen hid it
//   - later, a body made within the visible radius appears at once when its spot is out of sight (outside a 70 degree half cone of the camera, behind world geometry, or
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
	constexpr float DefaultVisibleRadius = 45.0f; // metres from the player within which bodies are drawn (sim_body_radius changes it)
	constexpr float VisibleHysteresis = 5.0f;     // a shown body goes back to the ring only this far beyond the radius

	float VisibleRadius();
	void SetVisibleRadius(float metres);

	struct Counters
	{
		uint64_t proposals = 0;       // spawn proposals received
		uint64_t spawnedHidden = 0;   // bodies created at once (out of sight, or right after a load)
		uint64_t spawnedVisible = 0;  // bodies created in sight (faded in)
		uint64_t spawnedWarming = 0;  // bodies created in the ring (not drawn)
		uint64_t becameShown = 0;     // warming bodies that reached the visible radius
		uint64_t returnedToRing = 0;  // shown bodies that walked out of it
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
	size_t WarmingCount();

	struct BodyInfo
	{
		sim::Entity entity;
		glm::vec3 feet;
		float distance;     // from the player
		bool warming;
		bool drawn;         // the mesh was drawn in the last frame
		float age;
	};
	std::vector<BodyInfo> BodyInfos();
	const Counters& GetCounters();
}

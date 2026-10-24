#pragma once

// The player's level transition (SLICE_M2E_TRANSITION.md, addendum 5.2): a sim_portal the pawn walks into loads the level it names and puts
// the pawn at the named sim_spawn. This replaces LevelTraversalSystem::TravelToLevel, whose time skip re-simulates the level on entering: the
// sim clock never stopped, so nothing is re-simulated.
//
// Flow: SimPortal::OnBodyEntered -> Request (stores one pending transition, never loads inside the physics callback) -> SimBridge::Update ->
// Update (resolves the target against the baked world, saves the level the pawn leaves into LevelTraversalSystem::LevelMemory, which the game save
// already carries, restores the target's memory if it has one, loads) -> SimSpawn::FromData of the target level asks TakeArrival and creates the
// pawn at the named spawn (yaw from the spawn, pitch kept, no momentum).

#include <Sim/Nav/WorldNavData.h>

#include <glm/vec3.hpp>

#include <cstdint>
#include <string>

namespace SimTransition
{
	// ---- Pure: no engine state ------------------------------------------------------------------------------------------

	struct Target
	{
		bool ok = false;
		int level = -1;         // world level index
		int spawn = -1;         // index into WorldNavData::spawns
		glm::vec3 landing{ 0.0f }; // level-local position of the spawn
		std::string error;      // when !ok: what is wrong, naming the portal
	};

	// The portal's keys against the baked world: the level exists and has a spawn of that name. `portal` is only used in the message.
	Target Resolve(const sim::nav::WorldNavData& world, const std::string& portal, const std::string& targetLevel, const std::string& targetSpawn);

	// ---- Engine side ----------------------------------------------------------------------------------------------------

	// From a sim_portal when the pawn walked in. The first request wins; one that comes while a transition runs is ignored.
	void Request(const std::string& portal, const std::string& targetLevel, const std::string& targetSpawn);

	// Every frame (SimBridge::Update): performs the pending transition, and after a load that did not bring the pawn (the map lost the spawn)
	// puts it at the world file's position of that spawn or the level's default start.
	void Update(const sim::nav::WorldNavData& world);

	// True while a loaded level is expected to receive the pawn at a named spawn (the default player start must not create it then).
	bool Arriving();
	// For the sim_spawn named `spawnName` as it is created: true when the pawn arrives here; `pitch` is the view pitch to keep. Ends the arrival.
	bool TakeArrival(const std::string& spawnName, float& pitch);

	// Transitions performed since the game started (tests, sim_stats).
	uint32_t Performed();
}

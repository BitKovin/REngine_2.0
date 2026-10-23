#pragma once

// The engine glue of the sim (SLICE_M2B_BRIDGE.md): a static singleton started by GameStart. It loads the world file, builds the
// modules, runs the Host on its own thread through a SimClient, tells the sim where the pawn is, forwards the pause menu, registers
// the sim as a GameSaveSystem extension (one save file holds level and sim), and in development builds draws the agents as boxes
// (colour = tier) and offers the sim_* console commands.
//
// Launch arguments (development builds): -sim_demo [count] places commuting walkers (default 40);
// -sim_bridge_test runs the automated in-game check, writes SaveData/sim_bridge.log and quits.

#include <glm/vec3.hpp>

#include <string>

namespace SimBridge
{
	constexpr const char* TestReportPath = "SaveData/sim_bridge.log";

	// Loads GameData/sim/world.nav and starts the sim thread. False (logged loudly) when the world file is missing or unusable:
	// the game then runs without a sim. `demoCount` > 0 places that many demo walkers (development builds only).
	bool Start(int demoCount, bool selfTest);

	// Every frame, after the pause state is known.
	void Update();

	// Stops the sim thread. Called on SDL_QUIT and from atexit; safe to call again.
	void Stop();

	bool IsRunning();
}

#pragma once

// Draws the loaded level's part of the world nav mesh and the portal links with the engine's DebugDraw, so a
// designer can see what the sim walks on and compare it with the level (SIM_CONTRACT section 7).
//
//   console: sim_nav_draw [on|off]     launch: -sim_nav_draw
//
// Colours: polygon edges dim blue, surface borders light blue, portal triggers green (red: the link is not in the mesh),
// landing points yellow. It reads its own copy of GameData/sim/world.nav (game thread), never the sim's mesh.

#ifndef DISTRIBUTION

namespace SimNavDebugDraw
{
	// Enabling loads the world file (and runs the stale check); a missing or broken file leaves it disabled.
	void SetEnabled(bool enabled);
	bool IsEnabled();

	// Every frame from GameStart. Follows the loaded level.
	void Update();
}

#endif

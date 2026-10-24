#pragma once

#ifndef DISTRIBUTION

// The automated in-game check of the level transition (SLICE_M2E_TRANSITION.md section 5): launch -sim_transition_test from the game folder.
// The pawn is put into each portal of the test chain a - road - b - c and back; after every arrival the level, the pawn's place, the sim's idea of
// where the player is and the absence of a bounce back are checked; a game save made in level b is loaded from level a. Writes
// SaveData/sim_transition.log and quits.

namespace SimTransitionTest
{
	constexpr const char* ReportPath = "SaveData/sim_transition.log";

	void Start();

	// Every frame, after SimBridge::Update.
	void Update();
}

#endif

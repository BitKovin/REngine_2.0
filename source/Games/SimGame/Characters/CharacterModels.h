#pragma once

// The characters the game can show: a mesh, where its clips come from, and what the first-person body hides. Data, not code branches
// (SLICE_M2F_CHARACTERS.md section 4). The sim's `Appearance::model` indexes this table (0 and 1); the player's model is not selectable by the sim.
//
// Every model here shares one UE4-style skeleton, so any clip plays on any mesh: the mesh and the clips are separate sources.

#include <vector>

struct CharacterModel
{
	const char* name;
	const char* mesh;             // glb with the skin
	const char* clips;            // glb with the animations (often the same file)
	bool directional;             // clips walk_b/l/r, run_b/l/r and the crouch set exist, so strafing, backing up and crouching have animations
	bool tinted;                  // untextured: drawn white-textured with a per-person tint
	bool twoSided;                // drawn without back-face culling (cloth panels are single planes)
	std::vector<const char*> firstPersonHidden; // bones scaled to zero in the first-person body's visible mesh (the shadow keeps them)
};

namespace CharacterModels
{
	enum Id : int
	{
		Citizen = 0,     // base.glb
		Guard = 1,       // guard.glb
		PlayerCoat = 2,  // player_coat.glb, with the cloth of its .skmm; locomotion from base.glb
	};

	int Count();
	// Out-of-range ids give the citizen.
	const CharacterModel& Get(int id);
}

#pragma once

// The third-person look of a character (SLICE_M2F_CHARACTERS.md): a skeletal mesh, the locomotion animator, the paint of untextured models and, for
// the player's own body in first person, the second mesh that keeps the full-body shadow. Used by every character, NPC or player.
//
// It has no physics, no navigation and no sim knowledge: a driver creates it, calls Apply(state) every frame, and destroys it with itself. It can be
// spawned for a mirror, a cutscene or a test with a state made by hand.
//
// First person (CharacterState::Visibility::FirstPersonOwner): the pose is computed once. The full pose goes to the shadow mesh (drawn in the shadow
// pass only), which also runs the cloth; its result goes to the visible mesh with the model's first-person bones scaled to zero (no neck, no arms).

#include "CharacterAnimator.h"
#include "CharacterState.h"

#include <Entity.h>
#include <SkeletalMesh.hpp>

#include <memory>

class CharacterBody : public Entity
{
public:
	// Creates a body in the loaded level at the state's place, with its assets loading. Null when no level is loaded.
	static CharacterBody* Spawn(const CharacterState& state);

	CharacterBody();

	// Once per frame by the driver: places the mesh, runs the animator, pastes the pose.
	void Apply(const CharacterState& state);

	void LoadAssets() override;

	// What the last Apply did, for the tests and the stats.
	bool Drawn() const { return mesh && mesh->WasRended; }
	bool ShadowMeshActive() const { return shadowMesh && shadowMesh->Visible; }
	CharacterState::Visibility LastVisibility() const { return last.visibility; }

private:
	CharacterState last;
	SkeletalMesh* mesh = nullptr;       // what the camera sees
	SkeletalMesh* shadowMesh = nullptr; // first person only: the full pose, shadow pass only
	std::unique_ptr<CharacterAnimator> animator;
	bool paintApplied = false;
	bool clothNeedsReset = true;

	void ApplyPaint(const CharacterState& state);
	void SetFirstPerson(bool on);
	void PlaceMesh(SkeletalMesh& target, const CharacterState& state, bool firstPerson) const;
};

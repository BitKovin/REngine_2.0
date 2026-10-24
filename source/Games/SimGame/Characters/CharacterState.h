#pragma once

// Everything the look of a character needs, in one value (SLICE_M2F_CHARACTERS.md). A driver (SimNpc, SimPawn) fills it every frame from its
// own movement and decisions and hands it to its CharacterBody; the representation only reads it. Nothing in it points back at a driver.
//
// New things a character can show (a held item, hand targets, a death pose) become fields here, not extra arguments.

#include <glm/glm.hpp>

#include <cstdint>

struct CharacterState
{
	enum class Stance : uint8_t
	{
		Standing,
		Crouching,
	};

	// How the body is presented.
	enum class Visibility : uint8_t
	{
		Shown,            // drawn normally
		Warming,          // exists and animates, but is not drawn and casts no shadow (the ring beyond the visible distance)
		FirstPersonOwner, // the player's own body seen from its eyes: no neck, no arms, the full body in the shadow pass only
	};

	glm::vec3 feet{ 0.0f };      // level-local feet position (smoothed: what the eye sees)
	float bodyYaw = 0.0f;        // degrees, where the body faces (the engine's Rotation.y)
	glm::vec2 look{ 0.0f };      // degrees: x = pitch (positive looks down, the engine's convention), y = yaw; the head and spine will aim here
	glm::vec3 velocity{ 0.0f };  // world velocity; the animator turns it into movement relative to the body
	Stance stance = Stance::Standing;
	bool onGround = true;

	int model = 0;               // CharacterModels index
	uint32_t variant = 0;        // picks the tint of an untextured model, so a person always looks the same
	float alpha = 1.0f;          // below 1 while appearing or disappearing
	Visibility visibility = Visibility::Shown;
};

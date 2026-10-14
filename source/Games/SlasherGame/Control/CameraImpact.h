#pragma once

#include <Combat/Spring.h>
#include <Data/PlayerConfigData.h>

#include <glm.h>

// The camera's impact response (COMBAT_DESIGN.md §9.4) as view offsets in camera space. Pure math, no engine state:
// PlayerCamera feeds it events and applies the offsets, the self tests measure it.
//
// Comfort rules it is built on (a camera that blurs or disorients is a bug, not a tuning choice):
//   * every wave <= 12 Hz - at 60 fps faster motion is a frame-to-frame jump the eye smears into blur
//   * the view's angular speed is bounded - what disorients is the whole image swinging, not the displacement
//   * no roll - a tilting horizon is the strongest vestibular conflict
//   * translation and the vertical axis carry the impact: translation jolts the camera against a steady world,
//     a vertical jolt reads as weight
//   * events do not add up: a kick never exceeds the speed of the strongest allowed kick, and rumble takes the
//     strongest trauma instead of summing it, so combos and automatic fire never build a lasting shake
class CameraImpact
{
public:
	void Configure(const PlayerCameraImpactData& data) { settings = data; }
	void Reset();

	// A jolt that moves the image along `screenDirection` (x = right, y = up), size in impact units. It is biased
	// downward (kickVerticalBias): impacts read through their vertical part.
	void Kick(const glm::vec2& screenDirection, float impact);

	// Rumble with `amount` of trauma (0..1); the strongest current trauma wins.
	void Shake(float amount);

	// Unscaled seconds (the response keeps its pace in slow motion).
	void Update(float deltaTime);

	glm::vec2 GetRotation() const { return rotation; }   // degrees: x = pitch (+ = image up), y = yaw (+ = image right)
	glm::vec2 GetOffset() const { return offset; }       // m along the camera's right / up
	float GetTrauma() const { return trauma; }

private:
	PlayerCameraImpactData settings;
	DampedSpring kick;                 // x right, y up: image displacement in impact units
	float trauma = 0.0f;
	float time = 0.0f;
	glm::vec2 rotation = glm::vec2(0.0f);
	glm::vec2 offset = glm::vec2(0.0f);
};

#pragma once

#include <Control/CameraImpact.h>
#include <Data/PlayerConfigData.h>

#include <glm.h>

class CombatCharacter;

// Third-person orbit camera with collision, owned by the PlayerController. Writes the engine Camera.
class PlayerCamera
{
public:
	void Configure(const PlayerCameraData& data)
	{
		settings = data;
		impact.Configure(data.impact);
	}

	// Snap behind a body (spawn, possession).
	void Reset(const CombatCharacter& body);

	// Engine mouse delta (right stick is folded into it by the engine): x turns, y pitches.
	void AddLookInput(const glm::vec2& delta);

	// Main thread, LateUpdate (after the body moved this frame). deltaTime: unscaled seconds (impact response keeps its
	// pace in slow motion).
	void Update(const CombatCharacter& body, float deltaTime);

	// Impact response (COMBAT_DESIGN.md §9.4, CameraImpact), main thread. Kick: a jolt that moves the image along
	// `worldDirection` as seen on screen (impact units). Shake: rumble with `trauma` (0..1, the strongest wins).
	void AddKick(const glm::vec3& worldDirection, float size);
	void AddShake(float trauma);

	glm::vec3 GetForward() const;                 // full view direction
	glm::vec3 GetPlanarForward() const;           // horizontal
	glm::vec3 GetPlanarRight() const;
	float GetYaw() const { return yaw; }

private:
	PlayerCameraData settings;
	float yaw = 0.0f;
	float pitch = 10.0f;
	glm::vec3 pivot = glm::vec3(0.0f);
	bool initialized = false;

	CameraImpact impact;
};

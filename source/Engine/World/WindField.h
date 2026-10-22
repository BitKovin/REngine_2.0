#pragma once

#include "../glm.h"

// The world's wind: one base velocity for the whole level, with gusts rolling along it. Set it from the level or the
// game on the main thread (or the `wind` console command); Sample may run on any thread.
class WindField
{
public:
	// velocity in m/s. gustStrength: share of the base speed the gusts add and take away (0..1); gustFrequency: how
	// often a gust passes a point, Hz.
	static void Set(const glm::vec3& velocity, float gustStrength = 0.0f, float gustFrequency = 0.4f);

	static glm::vec3 GetBaseVelocity() { return baseVelocity; }

	// The wind at `position` at `time` (seconds of game time).
	static glm::vec3 Sample(const glm::vec3& position, double time);

private:
	static inline glm::vec3 baseVelocity = glm::vec3(0.0f);
	static inline float gustStrength = 0.0f;
	static inline float gustFrequency = 0.4f;
};

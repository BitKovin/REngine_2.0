#pragma once

#include <glm.h>

#include <functional>
#include <string>

namespace TacticHelpers
{
	// A value in [0, 1) that is always the same for the same id: lets NPCs pick
	// different peek points, ring angles and flanking sides without randomness
	// that would make them change their minds every frame.
	inline float StableRandom01(const std::string& id, unsigned salt = 0)
	{
		const size_t hash = std::hash<std::string>{}(id) + salt * 0x9E3779B97F4A7C15ull;
		return static_cast<float>((hash >> 8) % 10000) / 10000.0f;
	}

	inline float DistanceXZ(const vec3& a, const vec3& b)
	{
		return glm::distance(vec3(a.x, 0, a.z), vec3(b.x, 0, b.z));
	}
}

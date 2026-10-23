#pragma once

// The shared speed table (addendum 7.1): metres per second per mover class, gait and nav area. The abstract movement
// (Sim/Movement) and, later, the physical layer read the same numbers, so an agent arrives at the same time either way.
// Header only and free of sim types on purpose.
//
// The numbers are placeholders until the calibration slice measures a real body walking a route (addendum 7.1).

#include <array>
#include <cstdint>

namespace sim::movement
{
	enum class MoverClass : uint8_t
	{
		Person,
	};

	enum class Gait : uint8_t
	{
		Walk,
		Run,
	};

	// The engine bakes a single ground area (1) today; Detour allows 64 area ids.
	constexpr uint8_t GroundArea = 1;

	// Speed multiplier per area (roads, mud, stairs join here).
	inline constexpr std::array<float, 64> AreaMultipliers = []
	{
		std::array<float, 64> multipliers{};
		for (float& multiplier : multipliers)
			multiplier = 1.0f;
		return multipliers;
	}();

	constexpr float BaseSpeed(MoverClass, Gait gait) { return gait == Gait::Run ? 8.0f : 2.4f; }

	constexpr float Speed(MoverClass moverClass, Gait gait, uint8_t area) { return BaseSpeed(moverClass, gait) * AreaMultipliers[area & 63]; }

	// The fastest this class can move over ground, before the agent's own speed scale.
	constexpr float MaxSpeed(MoverClass moverClass)
	{
		float fastest = 0.0f;
		for (const Gait gait : { Gait::Walk, Gait::Run })
			for (const float multiplier : AreaMultipliers)
				fastest = fastest < BaseSpeed(moverClass, gait) * multiplier ? BaseSpeed(moverClass, gait) * multiplier : fastest;
		return fastest;
	}
}

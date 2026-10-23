#pragma once

// Basic types shared by every part of the simulation. See Docs/SIM_CONTRACT.md.

#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>

#include <cstdint>
#include <limits>

namespace sim
{
	// Simulation time in ticks. One tick is 0.1 s of world time (addendum section 6.1).
	using Tick = int64_t;
	constexpr Tick TicksPerSecond = 10;
	constexpr Tick TicksPerMinute = TicksPerSecond * 60;
	constexpr Tick TicksPerHour = TicksPerMinute * 60;
	constexpr Tick NeverTick = std::numeric_limits<Tick>::max();

	constexpr double TicksToSeconds(Tick ticks) { return static_cast<double>(ticks) / TicksPerSecond; }
	constexpr Tick SecondsToTicks(double seconds) { return static_cast<Tick>(seconds * TicksPerSecond + (seconds >= 0.0 ? 0.5 : -0.5)); }

	// Sim records are entities of the sim's own entt registry (never EcsWorld). Generational: a destroyed record's
	// handle stays invalid forever, even after its index is reused.
	using Entity = entt::entity;
	constexpr Entity NullEntity = entt::null;

	inline uint32_t EntityIndex(Entity entity) { return static_cast<uint32_t>(entt::to_entity(entity)); }
	inline uint32_t EntityVersion(Entity entity) { return static_cast<uint32_t>(entt::to_version(entity)); }

	// Sequential id of a logged event. 0 means "no event" (a root cause).
	using EventId = uint64_t;
	constexpr EventId NoEvent = 0;

	// Registration-order ids of registered types. Stable for a given module list, which saves record by name anyway.
	using TypeId = uint16_t;
	constexpr TypeId InvalidType = std::numeric_limits<TypeId>::max();

	using RuleId = uint16_t;
	constexpr RuleId NoRule = std::numeric_limits<RuleId>::max();
	// Rule id stamped on events that enter the world from outside a rule.
	constexpr RuleId InputRule = NoRule - 1;    // client input messages
	constexpr RuleId SystemRule = NoRule - 2;   // per-tick systems
	constexpr RuleId SetupRule = NoRule - 3;    // world setup and logged exceptions (RunSetup)
	// A scheduled event keeps the cause and rule of whoever scheduled it.

	// Upper bound for an event or command payload. Payloads are trivially copyable structs stored inline.
	constexpr size_t MaxPayloadSize = 48;
}

#pragma once

// Authored NPCs (SLICE_M2D_NPCS.md): named agents with a model, a speed and a routine of stops with waiting times. They are the
// first residents of the world and the pattern for later ones: content is data (GameData/sim/npcs.json, read by the bridge), the
// sim holds only records, and everything an NPC does is a walk request answered by an Arrived event and a timer.
//
//   a routine is a list of stops (level, level-local position, how long to stay); Loop goes round, PingPong goes there and back
//   Arrived at a stop -> wait a random time in [waitMin, waitMax] of that stop -> RoutineStep -> walk to the next stop
//   a stop may be in another level: the walk crosses the portals, the NPC is simply far away for a while

#include <Sim/Movement/SimMovement.h>

namespace sim::npcs
{
	using movement::Identity;

	struct RoutineStop
	{
		FixedString<40> level;     // map file name, e.g. "sim_test_a.bsp"
		glm::vec3 local{ 0.0f };   // level-local position (survives a rebake)
		float waitMin = 0.0f;      // seconds to stay after arriving
		float waitMax = 0.0f;
		template<class Ar> void Visit(Ar& ar) { ar("level", level); ar("local", local); ar("waitMin", waitMin); ar("waitMax", waitMax); }
	};

	struct Routine
	{
		enum class Mode : uint8_t { Loop, PingPong };

		static constexpr const char* TypeName = "Routine";
		Mode mode = Mode::Loop;
		movement::Gait gait = movement::Gait::Walk;
		std::vector<RoutineStop> stops;
		uint32_t current = 0;     // the stop the NPC is walking to or standing at
		int32_t direction = 1;    // PingPong: +1 towards the last stop, -1 back
		template<class Ar> void Visit(Ar& ar)
		{
			ar("mode", mode);
			ar("gait", gait);
			ar("stops", stops);
			ar("current", current);
			ar("direction", direction);
		}
	};

	// Time to go to the next stop.
	struct RoutineStep
	{
		static constexpr const char* TypeName = "RoutineStep";
		Entity npc = NullEntity;
		template<class Ar> void Visit(Ar& ar) { ar("npc", npc); }
	};

	struct RoutineAdvance
	{
		static constexpr const char* TypeName = "RoutineAdvance";
		Entity npc = NullEntity;
		template<class Ar> void Visit(Ar& ar) { ar("npc", npc); }
	};

	// The plain-data form of an NPC as content files describe it (no sim types, so a loader can fill it).
	struct StopDefinition
	{
		std::string level;
		glm::vec3 local{ 0.0f };
		float waitMin = 0.0f;
		float waitMax = 0.0f;
	};

	struct NpcDefinition
	{
		std::string id;                // unique, at most 23 characters
		std::string name;              // what people see
		int model = 0;                 // Appearance::model: 0 base.glb, 1 guard.glb
		float speed = 1.0f;            // multiplies the gait's table speed
		movement::Gait gait = movement::Gait::Walk;
		Routine::Mode mode = Routine::Mode::Loop;
		std::vector<StopDefinition> stops;
	};

	class NpcModule : public Module
	{
	public:
		// `movement` and the bodies module must be registered before this one (Appearance lives in bodies).
		NpcModule(const movement::MovementModule& movement, const nav::WorldNav& worldNav);

		const char* Name() const override { return "npcs"; }
		void Register(Registration& registration) override;

		// Everything wrong with a definition against the baked world, one message each, naming the NPC and the stop. Empty: fine.
		std::vector<std::string> Validate(const NpcDefinition& definition) const;

		// Setup helper (RunSetup): creates the NPC standing at its first stop and starts its routine after a short random delay.
		Entity Place(ApplyContext& context, const NpcDefinition& definition) const;

		TunableId retryDelay = 0, startDelayMax = 0;
		RandomStreamId starts = 0;

	private:
		const movement::MovementModule& movement;
		const nav::WorldNav& worldNav;

		// The stop's position on the world mesh; invalid when its level is gone or it is off the mesh.
		nav::NavPoint StopPoint(const RoutineStop& stop) const;
		void Advance(ApplyContext& context, Entity npc) const;
	};
}

#pragma once

// The demo population (development only, SLICE_M2B_BRIDGE.md section 5): walkers placed at the world's spawn points that
// walk from one spawn to another for ever. It stands in for real population until M4/M9, and it is the same trip rule the
// movement tests run, so what is tested is what is shipped to the console.
//
//   PopulateDemo {count}  input: places `count` walkers (the bridge sends it for -sim_demo and the sim_demo console command)
//   TripWanted {walker, destination}  input: send one walker to point `destination` (-1: a random one)
// After Arrived, PathFailed and Replaced a commuting walker picks a new random point.

#ifndef DISTRIBUTION

#include "SimMovement.h"

namespace sim::movement
{
	struct PopulateDemo
	{
		static constexpr const char* TypeName = "PopulateDemo";
		uint16_t count = 0;
		template<class Ar> void Visit(Ar& ar) { ar("count", count); }
	};

	struct TripWanted
	{
		static constexpr const char* TypeName = "TripWanted";
		Entity walker = NullEntity;
		int32_t destination = -1; // index into the module's points; -1 picks one at random
		template<class Ar> void Visit(Ar& ar) { ar("walker", walker); ar("destination", destination); }
	};

	struct PlaceDemoWalkers
	{
		static constexpr const char* TypeName = "PlaceDemoWalkers";
		uint16_t count = 0;
		template<class Ar> void Visit(Ar& ar) { ar("count", count); }
	};

	class DemoModule : public Module
	{
	public:
		// `points` are nav-space spots on the mesh (the world file's spawns). With commute false walkers only walk when asked.
		DemoModule(MovementModule& movement, std::vector<glm::vec3> points, bool commute = true);

		const char* Name() const override { return "demo"; }
		void Register(Registration& registration) override;

		const std::vector<glm::vec3>& Points() const { return points; }

	private:
		MovementModule& movement;
		std::vector<glm::vec3> points;
		bool commute = true;

		void ChooseTrip(RuleContext& context, Entity walker, int32_t index) const;
	};
}

#endif

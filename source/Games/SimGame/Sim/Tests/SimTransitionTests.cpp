// Level transition tests (SLICE_M2E_TRANSITION.md): the decision against a hand-made world, and every portal of the shipped
// GameData/sim/world.nav against its landing.

#include "SimTestFramework.h"

#ifndef DISTRIBUTION

#include <Sim/Bridge/SimTransition.h>
#include <Sim/Bridge/SimWorldNavFile.h>

#include <glm/geometric.hpp>

namespace sim::test
{
	namespace
	{
		using sim::nav::WorldNavData;

		WorldNavData SmallWorld()
		{
			WorldNavData world;
			world.levels.resize(2);
			world.levels[0].name = "a.bsp";
			world.levels[1].name = "b.bsp";
			world.levels[1].offset = glm::vec3(100.0f, 0.0f, 0.0f);
			sim::nav::WorldSpawn arrival;
			arrival.name = "from_a";
			arrival.level = 1;
			arrival.position = glm::vec3(103.0f, 0.5f, 4.0f);
			world.spawns.push_back(arrival);
			sim::nav::WorldSpawn start;
			start.name = "start";
			start.level = 0;
			start.playerStart = true;
			world.spawns.push_back(start);
			return world;
		}

		void TestResolve(TestContext& t)
		{
			t.Section("transition: the portal's keys against the world");
			const WorldNavData world = SmallWorld();

			const SimTransition::Target good = SimTransition::Resolve(world, "to_b", "b.bsp", "from_a");
			t.Check(good.ok && good.level == 1 && good.spawn == 0, "a known level and spawn resolve (level %d, spawn %d)", good.level, good.spawn);
			t.Check(glm::distance(good.landing, glm::vec3(3.0f, 0.5f, 4.0f)) < 1e-4f, "the landing is level-local (the level's offset is taken off)");

			const SimTransition::Target noLevel = SimTransition::Resolve(world, "to_x", "x.bsp", "from_a");
			t.Check(!noLevel.ok && noLevel.error.find("to_x") != std::string::npos && noLevel.error.find("x.bsp") != std::string::npos, "an unknown level is refused, naming the portal and the level: %s", noLevel.error.c_str());

			const SimTransition::Target noSpawn = SimTransition::Resolve(world, "to_b", "b.bsp", "from_c");
			t.Check(!noSpawn.ok && noSpawn.error.find("from_c") != std::string::npos, "an unknown spawn is refused, naming it: %s", noSpawn.error.c_str());

			// A spawn of the same name in another level is not the target's.
			const SimTransition::Target wrongLevel = SimTransition::Resolve(world, "to_a", "a.bsp", "from_a");
			t.Check(!wrongLevel.ok, "a spawn name that exists only in another level is refused");

			t.Check(!SimTransition::Resolve(world, "to_b", "", "from_a").ok && !SimTransition::Resolve(world, "to_b", "b.bsp", "").ok, "a portal without target keys is refused");
		}

		// Every link of the shipped world lands exactly on a spawn of the level it leads to: the pawn that arrives at that spawn stands where an
		// agent that walked through the link would.
		void TestShippedLinks(TestContext& t)
		{
			t.Section("transition: the shipped world's portals");
			WorldNavData world;
			std::string error;
			if (!t.Check(SimWorldNavFile::Load(world, error), "the shipped world file loads: %s", error.c_str()))
				return;
			int checked = 0;
			for (const sim::nav::WorldLink& link : world.links)
			{
				bool found = false;
				float best = 1e9f;
				for (const sim::nav::WorldSpawn& spawn : world.spawns)
					if (spawn.level == link.toLevel)
					{
						const float apart = glm::distance(spawn.position, link.end);
						best = apart < best ? apart : best;
						found = found || apart < 0.05f;
					}
				t.Check(found, "portal %s lands on a spawn of %s (nearest %.3f m)", link.portal.c_str(), world.levels[link.toLevel].name.c_str(), best);
				++checked;
			}
			t.Check(checked >= 6, "the shipped world has %d portal links", checked);
		}
	}

	void RunTransitionTests(TestContext& context)
	{
		TestResolve(context);
		TestShippedLinks(context);
	}
}

#endif

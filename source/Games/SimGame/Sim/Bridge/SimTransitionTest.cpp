// Time.hpp first: engine headers below use Time without including it.
#include <Time.hpp>

#include "SimTransitionTest.h"

#ifndef DISTRIBUTION

#include "SimBridge.h"
#include "SimClient.h"
#include "SimMapEntities.h"
#include "SimTransition.h"
#include "SimWorldNavFile.h"

#include <SimPawn.h>

#include <FileSystem/FileSystem.h>
#include <Level.hpp>
#include <Logger.hpp>
#include <SaveSystem/GameSaveSystem.h>

#include <SDL2/SDL.h>

#include <chrono>
#include <cstdarg>
#include <cstdio>

namespace SimTransitionTest
{
	namespace
	{
		using Clock = std::chrono::steady_clock;

		struct Step
		{
			const char* portal;  // sim_portal targetname in the level the pawn is in
			const char* level;   // level it must end up in
			const char* spawn;   // sim_spawn it must land at
		};

		// The chain a - road - b - c and back. The save is taken after step 1 (in b) and loaded at the end (in a).
		const Step Steps[] = {
			{ "to_road", "sim_test_road.bsp", "from_a" },
			{ "to_b", "sim_test_b.bsp", "from_road" },
			{ "to_c", "sim_test_c.bsp", "from_b" },
			{ "to_b", "sim_test_b.bsp", "from_c" },
			{ "to_road", "sim_test_road.bsp", "from_b" },
			{ "to_a", "sim_test_a.bsp", "from_road" },
		};
		constexpr int StepCount = sizeof(Steps) / sizeof(Steps[0]);
		constexpr int SaveAfterStep = 1;
		constexpr const char* SaveName = "sim_transition_test";

		enum class Stage { Ready, Enter, Arrive, Settle, Load, LoadSettle, Done };

		struct Test
		{
			bool enabled = false;
			Stage stage = Stage::Ready;
			int step = 0;
			Clock::time_point begin, stageStart;
			uint32_t performedBefore = 0;
			uint16_t changeBefore = 0;
			std::string report;
			int failures = 0;

			void Line(const char* format, ...)
			{
				char buffer[512];
				va_list args;
				va_start(args, format);
				std::vsnprintf(buffer, sizeof(buffer), format, args);
				va_end(args);
				report += buffer;
				report += "\n";
			}
			void Check(bool ok, const char* format, ...)
			{
				char buffer[512];
				va_list args;
				va_start(args, format);
				std::vsnprintf(buffer, sizeof(buffer), format, args);
				va_end(args);
				report += ok ? "PASS " : "FAIL ";
				report += buffer;
				report += "\n";
				failures += ok ? 0 : 1;
			}
			double Seconds(Clock::time_point since) const { return std::chrono::duration<double>(Clock::now() - since).count(); }
			void Next(Stage to)
			{
				stage = to;
				stageStart = Clock::now();
			}
		};

		Test test;

		void Finish()
		{
			test.Line("%s: %d failed checks, %u transitions", test.failures == 0 ? "RESULT PASSED" : "RESULT FAILED", test.failures, SimTransition::Performed());
			FileSystemEngine::WriteFile(ReportPath, test.report);
			Logger::Log("sim transition test %s (report %s)", test.failures == 0 ? "PASSED" : "FAILED", ReportPath);
			test.enabled = false;
			SDL_Event quit;
			quit.type = SDL_QUIT;
			SDL_PushEvent(&quit);
		}

		bool InLevel(const char* level)
		{
			return Level::Current && !Level::HasPendingLevelLoad() && SimWorldNavFile::LevelNameOfMapPath(Level::Current->filePath) == level;
		}

		// Level-local position of a spawn in the world file; false if the file does not have it.
		bool SpawnLocal(const sim::nav::WorldNavData& world, const char* level, const char* spawn, glm::vec3& out)
		{
			const int index = world.FindLevel(level);
			if (index < 0)
				return false;
			for (const sim::nav::WorldSpawn& candidate : world.spawns)
				if (candidate.level == index && candidate.name == spawn)
				{
					out = candidate.position - world.levels[index].offset;
					return true;
				}
			return false;
		}

		// The sim's idea of the player against the pawn's feet; the report is on a timer, so the caller waits a moment first.
		void CheckAnchor(const char* what, const char* level)
		{
			sim::SimClient* client = SimBridge::Client();
			const sim::nav::WorldNavData* world = SimBridge::World();
			SimPawn* pawn = SimPawn::Instance;
			if (!client || !world || !pawn)
			{
				test.Check(false, "%s: no sim or pawn to compare", what);
				return;
			}
			const int index = world->FindLevel(level);
			const auto& snapshot = client->Snapshot();
			const glm::vec3 expected = pawn->FeetPosition() + world->levels[index].offset;
			const float apart = snapshot.hasPlayer ? glm::distance(snapshot.playerNav, expected) : -1.0f;
			test.Check(snapshot.hasPlayer && snapshot.playerLevel == index && apart < 1.5f, "%s: the sim's player is in %s, %.2f m from the pawn", what, level, apart);
		}

		bool Fail(const char* format, ...)
		{
			char buffer[512];
			va_list args;
			va_start(args, format);
			std::vsnprintf(buffer, sizeof(buffer), format, args);
			va_end(args);
			test.Check(false, "%s", buffer);
			Finish();
			return false;
		}
	}

	void Start()
	{
		test.enabled = true;
		test.begin = test.stageStart = Clock::now();
	}

	void Update()
	{
		if (!test.enabled)
			return;
		Test& t = test;
		sim::SimClient* client = SimBridge::Client();
		const sim::nav::WorldNavData* world = SimBridge::World();
		if (!client || !world)
		{
			Fail("the sim is not running");
			return;
		}
		if (t.Seconds(t.begin) > 90.0)
		{
			Fail("the test did not finish within 90 s (stage %d, step %d)", static_cast<int>(t.stage), t.step);
			return;
		}
		SimPawn* pawn = SimPawn::Instance;

		switch (t.stage)
		{
		case Stage::Ready: // the pawn has stood in level a long enough for a portal to react, and the sim knows it
			if (pawn && InLevel("sim_test_a.bsp") && client->Snapshot().hasPlayer && Time::GameTime - pawn->SpawnTime > 2.5)
			{
				t.Line("ready after %.1f s in sim_test_a.bsp", t.Seconds(t.begin));
				t.Next(Stage::Enter);
			}
			break;

		case Stage::Enter:
		{
			const Step& step = Steps[t.step];
			SimPortal* portal = nullptr;
			for (LevelObject* object : Level::Current->LevelObjects)
				if (auto* candidate = dynamic_cast<SimPortal*>(object); candidate && candidate->Name == step.portal)
					portal = candidate;
			if (!portal)
			{
				Fail("step %d: the level has no sim_portal %s", t.step, step.portal);
				return;
			}
			t.performedBefore = SimTransition::Performed();
			t.changeBefore = Level::CurrentLevelChangeId;
			pawn->Teleport(portal->centre);
			t.Line("step %d: the pawn steps into %s", t.step, step.portal);
			t.Next(Stage::Arrive);
			break;
		}

		case Stage::Arrive:
		{
			const Step& step = Steps[t.step];
			if (Level::CurrentLevelChangeId != t.changeBefore && InLevel(step.level) && pawn)
			{
				t.Check(SimTransition::Performed() == t.performedBefore + 1, "step %d: one transition ran (%u)", t.step, SimTransition::Performed() - t.performedBefore);
				glm::vec3 landing;
				if (!SpawnLocal(*world, step.level, step.spawn, landing))
					Fail("step %d: the world file has no spawn %s in %s", t.step, step.spawn, step.level);
				else
				{
					const float apart = glm::distance(pawn->FeetPosition(), landing);
					t.Check(apart < 0.5f, "step %d: the pawn landed at %s of %s, %.2f m from it", t.step, step.spawn, step.level, apart);
				}
				t.Next(Stage::Settle);
			}
			else if (t.Seconds(t.stageStart) > 15.0)
				Fail("step %d: %s was not loaded within 15 s", t.step, step.level);
			break;
		}

		case Stage::Settle: // long enough for the pawn's report and for a bounce to happen if one would
		{
			if (t.Seconds(t.stageStart) < 3.5)
				break;
			const Step& step = Steps[t.step];
			t.Check(InLevel(step.level) && SimTransition::Performed() == t.performedBefore + 1, "step %d: the pawn stayed in %s (no bounce)", t.step, step.level);
			CheckAnchor(("step " + std::to_string(t.step)).c_str(), step.level);
			if (t.step == SaveAfterStep)
			{
				GameSaveSystem::SaveGameToFile(SaveName);
				t.Line("saved the game in %s", step.level);
			}
			++t.step;
			t.Next(t.step < StepCount ? Stage::Enter : Stage::Load);
			break;
		}

		case Stage::Load:
		{
			bool courier = false;
			int named = 0;
			for (const auto& agent : client->Agents())
				if (agent.id.c_str()[0] != '\0')
				{
					++named;
					courier = courier || std::string(agent.id.c_str()) == "courier";
				}
			t.Check(courier && named >= 10, "all %d authored NPCs are still in the sim after the chain (courier %s)", named, courier ? "yes" : "no");
			t.changeBefore = Level::CurrentLevelChangeId;
			GameSaveSystem::LoadGameFromFile(SaveName);
			t.Line("loaded the game saved in %s, from sim_test_a.bsp", Steps[SaveAfterStep].level);
			t.Next(Stage::LoadSettle);
			break;
		}

		case Stage::LoadSettle:
		{
			const Step& saved = Steps[SaveAfterStep];
			if (Level::CurrentLevelChangeId == t.changeBefore || !InLevel(saved.level) || !pawn)
			{
				if (t.Seconds(t.stageStart) > 15.0)
					Fail("the load did not bring back %s within 15 s", saved.level);
				break;
			}
			if (t.Seconds(t.stageStart) < 3.0)
				break;
			glm::vec3 landing;
			const bool known = SpawnLocal(*world, saved.level, saved.spawn, landing);
			const float apart = known ? glm::distance(pawn->FeetPosition(), landing) : -1.0f;
			t.Check(known && apart < 1.5f, "the load put the pawn back in %s, %.2f m from where it was saved", saved.level, apart);
			CheckAnchor("after the load", saved.level);
			Finish();
			break;
		}

		case Stage::Done:
			break;
		}
	}
}

#endif

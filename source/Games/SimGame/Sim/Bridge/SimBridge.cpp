// Time.hpp first: engine headers below use Time without including it.
#include <Time.hpp>

#include "SimBridge.h"

#include "SimClient.h"
#include "SimWorldNavFile.h"

#include <PauseGameManager.hpp>
#include <SimPawn.h>

#include <Console/ConsoleRegister.h>
#include <FileSystem/FileSystem.h>
#include <Level.hpp>
#include <Logger.hpp>
#include <SaveSystem/GameSaveSystem.h>

#include <SDL2/SDL.h>

#ifndef DISTRIBUTION
#include <Sim/Movement/SimDemoPopulation.h>

#include <DebugDraw.hpp>
#endif

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>

namespace SimBridge
{
	namespace
	{
		using Clock = std::chrono::steady_clock;

		struct State
		{
			std::unique_ptr<sim::nav::WorldNav> nav;
			std::unique_ptr<sim::movement::MovementModule> movement;
#ifndef DISTRIBUTION
			std::unique_ptr<sim::movement::DemoModule> demo;
#endif
			std::shared_ptr<const sim::TypeRegistry> types;
			int demoCount = 0;
			bool pausedByGame = false;

			// The last player report.
			bool reported = false;
			uint16_t reportedLevel = 0;
			glm::vec3 reportedLocal{ 0.0f };
			Clock::time_point reportedAt;
			std::string unknownLevel;

			bool drawAgents = true;

			// Last member: destroyed first, so the Host thread is gone before the world it reads.
			std::unique_ptr<sim::SimClient> client;
		};

		State state;
		bool exitHooksInstalled = false;

		bool Running() { return state.client && state.client->IsRunning(); }

		int SDLCALL OnSdlEvent(void*, SDL_Event* event)
		{
			if (event->type == SDL_QUIT)
				Stop();
			return 0;
		}

		void BuildClient()
		{
			sim::SimClient::Setup setup;
			setup.types = state.types;
			setup.worldNav = state.nav.get();
			setup.movement = state.movement.get();
			setup.seed = 1;
			state.client = std::make_unique<sim::SimClient>(setup);
			state.client->Start(sim::Host::Mode::Thread);
			state.reported = false;
			state.pausedByGame = false;
#ifndef DISTRIBUTION
			if (state.demoCount > 0)
				state.client->GetHost().SendEvent(sim::movement::PopulateDemo{ static_cast<uint16_t>(state.demoCount) });
#endif
		}

		// ---- Save extension -------------------------------------------------------------------------------------------

		void RegisterSaveExtension()
		{
			GameSaveExtension extension;
			extension.id = "sim";
			extension.save = [](std::vector<uint8_t>& out)
			{
				if (!Running())
					return true; // no sim, nothing to save
				std::string error;
				if (!state.client->SaveBlocking(out, error))
				{
					Logger::Error("sim: the save could not be taken: %s", error.c_str());
					return false;
				}
				return true;
			};
			extension.load = [](const std::vector<uint8_t>& bytes)
			{
				if (!state.client)
					return true;
				if (bytes.empty())
				{
					Logger::Warning("sim: this save has no sim entry (an older save): starting a fresh sim");
					state.client.reset();
					BuildClient();
					return true;
				}
				std::string error;
				if (!state.client->LoadBlocking(bytes, error))
				{
					Logger::Error("sim: the save's sim part could not be loaded: %s", error.c_str());
					return false;
				}
				state.reported = false; // the pawn reports again at once
				return true;
			};
			GameSaveSystem::RegisterExtension(std::move(extension));
		}

		// ---- Per frame ------------------------------------------------------------------------------------------------

		void ReportPlayer()
		{
			SimPawn* pawn = SimPawn::Instance;
			if (!pawn || !Level::Current)
				return;
			const int level = SimWorldNavFile::FindLevel(state.nav->Data(), Level::Current->filePath);
			if (level < 0)
			{
				if (state.unknownLevel != Level::Current->filePath)
				{
					state.unknownLevel = Level::Current->filePath;
					Logger::Warning("sim: %s is not a level of %s: the sim does not know where the player is", Level::Current->filePath.c_str(), SimWorldNavFile::Path);
				}
				return;
			}
			const glm::vec3 local = pawn->FeetPosition();
			const Clock::time_point now = Clock::now();
			if (state.reported && level == state.reportedLevel && glm::distance(local, state.reportedLocal) < 0.5f && std::chrono::duration<double>(now - state.reportedAt).count() < 0.5)
				return;
			state.client->ReportPlayer(static_cast<uint16_t>(level), local);
			state.reported = true;
			state.reportedLevel = static_cast<uint16_t>(level);
			state.reportedLocal = local;
			state.reportedAt = now;
		}

#ifndef DISTRIBUTION
		uint32_t TierColour(uint8_t tier)
		{
			return tier == 0 ? DebugColor::Green : tier == 1 ? DebugColor::Yellow : DebugColor::Gray;
		}

		void DrawAgents()
		{
			if (!state.drawAgents || !Level::Current)
				return;
			const int level = SimWorldNavFile::FindLevel(state.nav->Data(), Level::Current->filePath);
			if (level < 0)
				return;
			const glm::vec3 offset = state.nav->Data().levels[level].offset;
			const float duration = Time::DeltaTimeF / 2;
			for (const sim::movement::AgentView& agent : state.client->Agents())
			{
				if (agent.level != level)
					continue;
				const glm::vec3 local = agent.nav - offset;
				DebugDraw::Bounds(local - glm::vec3(0.25f, 0.0f, 0.25f), local + glm::vec3(0.25f, 1.8f, 0.25f), duration, 0.01f, TierColour(agent.tier));
			}
		}

		std::string Stats()
		{
			if (!Running())
				return "sim: not running";
			const auto& snapshot = state.client->Snapshot();
			int tiers[3] = { 0, 0, 0 };
			int walking = 0;
			for (const auto& agent : snapshot.agents)
			{
				++tiers[agent.tier % 3];
				walking += agent.state == static_cast<uint8_t>(sim::movement::Walker::State::Walking) ? 1 : 0;
			}
			char buffer[400];
			std::snprintf(buffer, sizeof(buffer), "sim: tick %lld%s, %llu snapshots (newest tick %lld), %zu agents (%d walking; tier 0/1/2 = %d/%d/%d), player %s", static_cast<long long>(state.client->CurrentTick()),
				state.client->IsPaused() ? " (paused)" : "", static_cast<unsigned long long>(state.client->SnapshotsReceived()), static_cast<long long>(state.client->SnapshotTick()),
				snapshot.agents.size(), walking, tiers[0], tiers[1], tiers[2], snapshot.hasPlayer ? "known" : "not reported yet");
			return buffer;
		}

		// ---- The automated in-game check (-sim_bridge_test) -----------------------------------------------------------

		struct SelfTest
		{
			bool enabled = false;
			int stage = 0;
			Clock::time_point begin, stageStart;
			std::vector<sim::movement::AgentView> startAgents;
			sim::Tick tickAtLoad = 0;
			std::string hashSaved, hashLater;
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
			void Next(int to)
			{
				stage = to;
				stageStart = Clock::now();
			}
		};

		SelfTest selfTest;

		void Finish()
		{
			selfTest.Line("%s: %d failed checks, %llu snapshots, tick %lld", selfTest.failures == 0 ? "RESULT PASSED" : "RESULT FAILED", selfTest.failures,
				state.client ? static_cast<unsigned long long>(state.client->SnapshotsReceived()) : 0ull, state.client ? static_cast<long long>(state.client->CurrentTick()) : 0ll);
			FileSystemEngine::WriteFile(TestReportPath, selfTest.report);
			Logger::Log("sim bridge test %s (report %s)", selfTest.failures == 0 ? "PASSED" : "FAILED", TestReportPath);
			selfTest.enabled = false;
			SDL_Event quit;
			quit.type = SDL_QUIT;
			SDL_PushEvent(&quit);
		}

		bool HashOf(std::string& out)
		{
			return state.client->TextBlocking("hash", out);
		}

		void UpdateSelfTest()
		{
			SelfTest& t = selfTest;
			SimPawn* pawn = SimPawn::Instance;
			const int level = Level::Current ? SimWorldNavFile::FindLevel(state.nav->Data(), Level::Current->filePath) : -1;
			if (t.stage < 8 && t.Seconds(t.begin) > 90.0)
			{
				t.Check(false, "the test did not finish within 90 s (stage %d)", t.stage);
				Finish();
				return;
			}

			switch (t.stage)
			{
			case 0: // ready: pawn exists, the sim knows the player and has its agents
			{
				const auto& snapshot = state.client->Snapshot();
				if (pawn && level >= 0 && snapshot.hasPlayer && static_cast<int>(snapshot.agents.size()) >= state.demoCount && state.demoCount > 0)
				{
					t.startAgents = snapshot.agents;
					t.Line("ready after %.1f s: %zu agents, player known in level %d", t.Seconds(t.begin), snapshot.agents.size(), snapshot.playerLevel);
					pawn->StartScriptedWalk(20.0f);
					t.Next(1);
				}
				else if (t.Seconds(t.begin) > 30.0)
				{
					t.Check(false, "not ready after 30 s (pawn %d, level %d, player known %d, agents %zu of %d)", pawn != nullptr, level, snapshot.hasPlayer, snapshot.agents.size(), state.demoCount);
					Finish();
				}
				break;
			}
			case 1: // the pawn walks
				if (!pawn || pawn->ScriptedWalkDone() || t.Seconds(t.stageStart) > 40.0)
					t.Next(2);
				break;
			case 2: // let the report and the snapshot catch up, then compare
			{
				if (t.Seconds(t.stageStart) < 1.5)
					break;
				const auto& snapshot = state.client->Snapshot();
				t.Check(pawn && pawn->ScriptedWalked() >= 5.0f, "the pawn walked %.1f m", pawn ? pawn->ScriptedWalked() : 0.0f);
				if (pawn && level >= 0)
				{
					const glm::vec3 expected = state.nav->ToNav(level, pawn->FeetPosition());
					t.Check(snapshot.hasPlayer && snapshot.playerLevel == level && glm::distance(snapshot.playerNav, expected) < 1.5f, "the sim's player anchor follows the pawn (%.2f m apart)",
						snapshot.hasPlayer ? glm::distance(snapshot.playerNav, expected) : -1.0f);
				}
				int moved = 0;
				for (const auto& now : snapshot.agents)
					for (const auto& before : t.startAgents)
						if (before.entity == now.entity && glm::distance(before.nav, now.nav) > 0.5f)
							++moved;
				t.Check(moved > 0, "%d of %zu demo agents have moved", moved, snapshot.agents.size());
				t.Check(state.client->CurrentTick() > 50 && state.client->SnapshotsReceived() > 5, "the sim ran %lld ticks and sent %llu snapshots", static_cast<long long>(state.client->CurrentTick()),
					static_cast<unsigned long long>(state.client->SnapshotsReceived()));
				t.Line("%s", Stats().c_str());
				state.client->Pause();
				t.Next(3);
				break;
			}
			case 3: // paused: save level and sim into one file
			{
				if (!state.client->IsPaused() && t.Seconds(t.stageStart) < 2.0)
					break;
				t.Check(state.client->IsPaused(), "the sim paused");
				const bool gotHash = HashOf(t.hashSaved); // before the Check: the arguments of a call are evaluated in no fixed order
				t.Check(gotHash, "state hash while paused: %s", t.hashSaved.c_str());
				GameSaveSystem::SaveGameToFile("sim_bridge_test");
				const std::string file = FileSystemEngine::ReadFile(GameSaveSystem::saveDataPath + "sim_bridge_test.savg");
				t.Check(file.find("\"sim\"") != std::string::npos, "the save file holds the sim entry (%zu bytes)", file.size());
				state.client->Resume();
				t.Next(4);
				break;
			}
			case 4: // run on, so that the load has something to undo
				if (t.Seconds(t.stageStart) > 1.5)
				{
					state.client->Pause();
					t.Next(5);
				}
				break;
			case 5:
			{
				if (!state.client->IsPaused() && t.Seconds(t.stageStart) < 2.0)
					break;
				const bool gotLater = HashOf(t.hashLater);
				t.Check(gotLater && t.hashLater != t.hashSaved, "the sim moved on after the save: %s", t.hashLater.c_str());
				GameSaveSystem::LoadGameFromFile("sim_bridge_test");
				std::string after;
				const bool gotAfter = HashOf(after);
				t.Check(gotAfter && after == t.hashSaved, "loading the save restored the saved sim state: %s", after.c_str());
				t.tickAtLoad = state.client->CurrentTick();
				state.client->Resume();
				t.Next(6);
				break;
			}
			case 6: // still alive after the load (level reload, pawn reports again)
				if (t.Seconds(t.stageStart) > 3.0)
				{
					t.Check(state.client->CurrentTick() > t.tickAtLoad, "the sim keeps running after the load (tick %lld)", static_cast<long long>(state.client->CurrentTick()));
					t.Check(state.client->Snapshot().hasPlayer, "and knows the player again");
					t.Next(7);
					Finish();
				}
				break;
			default:
				break;
			}
		}
#endif
	}

	bool Start(int demoCount, bool selfTestRun)
	{
		if (state.client)
			return true;

		sim::nav::WorldNavData data;
		std::string error;
		auto nav = std::make_unique<sim::nav::WorldNav>();
		if (!SimWorldNavFile::Load(data, error) || !nav->Load(data, error))
		{
			Logger::Error("==== SIM NOT STARTED: cannot load the world nav: %s ====", error.c_str());
			Logger::Error("  Bake it: SimGame.exe -headless -sim_bake_nav (from the game folder)");
			return false;
		}
#ifndef DISTRIBUTION
		SimWorldNavFile::CheckFresh(data);
#endif
		state.nav = std::move(nav);
		state.movement = std::make_unique<sim::movement::MovementModule>(*state.nav);

		sim::RegistryBuilder builder;
		builder.Add(*state.movement);
#ifndef DISTRIBUTION
		std::vector<glm::vec3> points;
		for (const sim::nav::WorldSpawn& spawn : data.spawns)
			points.push_back(spawn.position);
		state.demo = std::make_unique<sim::movement::DemoModule>(*state.movement, std::move(points), true);
		builder.Add(*state.demo);
#endif
		std::vector<std::string> errors;
		state.types = builder.Build(errors);
		if (!state.types)
		{
			for (const std::string& message : errors)
				Logger::Error("sim: %s", message.c_str());
			return false;
		}

		state.demoCount = demoCount;
		BuildClient();
		RegisterSaveExtension();

		if (!exitHooksInstalled)
		{
			exitHooksInstalled = true;
			SDL_AddEventWatch(OnSdlEvent, nullptr);
			std::atexit([] { Stop(); });
		}

#ifndef DISTRIBUTION
		if (selfTestRun)
		{
			selfTest.enabled = true;
			selfTest.begin = selfTest.stageStart = Clock::now();
		}
#else
		(void)selfTestRun;
#endif
		Logger::Log("sim: started (%zu levels, %zu spawn points, %d demo walkers)", data.levels.size(), data.spawns.size(), demoCount);
		return true;
	}

	void Stop()
	{
		if (state.client)
			state.client->Stop();
	}

	bool IsRunning() { return Running(); }

	void Update()
	{
		if (!Running())
			return;
		const bool paused = PauseGameManager::GetGamePaused();
		if (paused != state.pausedByGame)
		{
			state.pausedByGame = paused;
			if (paused)
				state.client->Pause();
			else
				state.client->Resume();
		}
		state.client->Update();
		ReportPlayer();
#ifndef DISTRIBUTION
		DrawAgents();
		if (selfTest.enabled)
			UpdateSelfTest();
#endif
	}

#ifndef DISTRIBUTION
	// ---- Console --------------------------------------------------------------------------------------------------------

	namespace
	{
		bool NeedsSim()
		{
			if (!Running())
				Logger::Log("sim: not running");
			return Running();
		}

		float Number(const std::vector<std::string>& args, size_t index, float fallback)
		{
			return index < args.size() ? static_cast<float>(std::atof(args[index].c_str())) : fallback;
		}
	}

	CONSOLE_FUNC("sim_pause", "Pause the sim thread")
	{
		if (NeedsSim())
			state.client->Pause();
	}

	CONSOLE_FUNC("sim_resume", "Resume the sim thread")
	{
		if (NeedsSim())
			state.client->Resume();
	}

	CONSOLE_FUNC("sim_step", "While paused, run n sim steps: sim_step [n]")
	{
		if (NeedsSim())
			state.client->Step(static_cast<int64_t>(Number(args, 0, 1.0f)));
	}

	CONSOLE_FUNC("sim_speed", "Sim seconds per real second and ticks per step (fast-forward): sim_speed <x> [stepTicks]")
	{
		if (NeedsSim())
			state.client->SetSpeed(Number(args, 0, 1.0f), static_cast<sim::Tick>(Number(args, 1, 1.0f)));
	}

	CONSOLE_FUNC("sim_text", "Run a sim text command (state, dump #i, why <event id>, timeline #i, log [n], scheduler, hash, types, tunables, set <name> <value>)")
	{
		if (!NeedsSim())
			return;
		std::string command;
		for (const std::string& word : args)
			command += (command.empty() ? "" : " ") + word;
		std::string reply;
		state.client->TextBlocking(command.empty() ? "state" : command, reply);
		Logger::Log("%s", reply.c_str());
	}

	CONSOLE_FUNC("sim_stats", "Tick, snapshot and agent counts of the sim")
	{
		Logger::Log("%s", Stats().c_str());
	}

	CONSOLE_FUNC("sim_demo", "Add commuting demo walkers at the world's spawn points: sim_demo [count] (default 40)")
	{
		if (!NeedsSim())
			return;
		const int count = static_cast<int>(Number(args, 0, 40.0f));
		state.demoCount += count;
		state.client->GetHost().SendEvent(sim::movement::PopulateDemo{ static_cast<uint16_t>(count) });
	}

	CONSOLE_FUNC("sim_agents", "Draw the sim's agents as boxes in the loaded level: sim_agents [on|off] (default toggle)")
	{
		state.drawAgents = args.empty() ? !state.drawAgents : args[0] != "off" && args[0] != "0";
		Logger::Log("sim_agents: %s", state.drawAgents ? "on" : "off");
	}

	CONSOLE_FUNC("sim_player", "Move the pawn to a level-local spot: sim_player <x> <y> <z> (feet)")
	{
		if (!SimPawn::Instance || args.size() < 3)
		{
			Logger::Log("sim_player: needs a pawn and three numbers");
			return;
		}
		SimPawn::Instance->Teleport(glm::vec3(Number(args, 0, 0.0f), Number(args, 1, 0.0f) + 0.9f, Number(args, 2, 0.0f)));
	}
#endif
}

// Time.hpp first: engine headers below use Time without including it.
#include <Time.hpp>

#include "SimBridge.h"

#include "SimAgents.h"
#include "SimClient.h"
#include "SimNpcFile.h"
#include "SimWorldNavFile.h"

#include <Sim/Bodies/SimBodies.h>
#include <Sim/Npcs/SimNpcs.h>

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
			std::unique_ptr<sim::bodies::BodiesModule> bodies;
			std::unique_ptr<sim::npcs::NpcModule> npcs;
			std::vector<sim::npcs::NpcDefinition> npcDefinitions; // the valid ones from npcs.json, placed in every fresh sim
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

			std::string lookId;      // the NPC the view follows
			bool lookPlaced = false; // the pawn was put next to it

			// Last member: destroyed first, so the Host thread is gone before the world it reads.
			std::unique_ptr<sim::SimClient> client;
		};

		State state;
		bool exitHooksInstalled = false;

		bool Running() { return state.client && state.client->IsRunning(); }

		// -sim_look / sim_look: the pawn stands 6 m from the NPC, once, and its view stays on the NPC.
		void FollowLook()
		{
			if (state.lookId.empty() || !SimPawn::Instance || !Level::Current)
				return;
			const auto& world = state.nav->Data();
			const int level = SimWorldNavFile::FindLevel(world, Level::Current->filePath);
			for (const sim::movement::AgentView& agent : state.client->Agents())
			{
				if (state.lookId != agent.id.c_str() || agent.level != level)
					continue;
				const glm::vec3 local = agent.nav - world.levels[level].offset;
				if (!state.lookPlaced)
				{
					SimPawn::Instance->Teleport(local + glm::vec3(-4.2f, 0.9f, -4.2f));
					state.lookPlaced = true;
				}
				SimPawn::Instance->LookAt(local + glm::vec3(0.0f, 1.2f, 0.0f));
				return;
			}
		}

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
			SimAgents::Reset();
			state.client = std::make_unique<sim::SimClient>(setup);
			if (!state.npcDefinitions.empty())
				state.client->ServerForSetup().RunSetup("authored npcs", [](sim::ApplyContext& ctx)
				{
					for (const sim::npcs::NpcDefinition& definition : state.npcDefinitions)
						state.npcs->Place(ctx, definition);
				});
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
		// The beam over a named NPC shows its body: grey none (abstract), yellow asked for, green has one, orange being taken back.
		uint32_t BeamColour(uint8_t body)
		{
			return body == 2 ? DebugColor::Green : body == 1 ? DebugColor::Yellow : body == 3 ? DebugColor::Orange : DebugColor::Gray;
		}

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
				if (agent.id.c_str()[0] != '\0') // a named NPC: a beam over its head, so it can be found
					DebugDraw::Line(local + glm::vec3(0.0f, 2.0f, 0.0f), local + glm::vec3(0.0f, 5.0f, 0.0f), duration, 0.01f, BeamColour(agent.body));
				if (agent.body == 2 || agent.body == 3) // a body draws itself
					continue;
				DebugDraw::Bounds(local - glm::vec3(0.25f, 0.0f, 0.25f), local + glm::vec3(0.25f, 1.8f, 0.25f), duration, 0.01f, TierColour(agent.tier));
			}
		}

		// One line per authored NPC the sim knows (from the newest snapshot).
		std::vector<std::string> NpcLines()
		{
			std::vector<std::string> lines;
			const auto& world = state.nav->Data();
			for (const sim::movement::AgentView& agent : state.client->Agents())
			{
				if (agent.id.c_str()[0] == '\0')
					continue;
				std::string name = agent.id.str();
				for (const sim::npcs::NpcDefinition& definition : state.npcDefinitions)
					if (definition.id == name)
						name += " (" + definition.name + ")";
				const glm::vec3 local = agent.nav - world.levels[agent.level].offset;
				static const char* states[] = { "standing", "waiting for a path", "walking" };
				static const char* bodies[] = { "no body", "body proposed", "body", "body leaving" };
				char line[300];
				std::snprintf(line, sizeof(line), "  %-34s %-20s (%6.1f %5.1f %6.1f) %-18s %s", name.c_str(), world.levels[agent.level].name.c_str(), local.x, local.y, local.z, states[agent.state % 3], bodies[agent.body % 4]);
				lines.push_back(line);
			}
			return lines;
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
			const SimAgents::Counters& agents = SimAgents::GetCounters();
			char buffer[600];
			std::snprintf(buffer, sizeof(buffer), "sim: tick %lld%s, %llu snapshots (newest tick %lld), %zu agents (%d walking; tier 0/1/2 = %d/%d/%d), player %s; bodies %zu (+%zu waiting): %llu proposals, %llu spawned hidden, %llu in view, %llu refused, %llu despawned, %llu goals, %llu reports",
				static_cast<long long>(state.client->CurrentTick()), state.client->IsPaused() ? " (paused)" : "", static_cast<unsigned long long>(state.client->SnapshotsReceived()),
				static_cast<long long>(state.client->SnapshotTick()), snapshot.agents.size(), walking, tiers[0], tiers[1], tiers[2], snapshot.hasPlayer ? "known" : "not reported yet",
				SimAgents::BodyCount(), SimAgents::PendingCount(), static_cast<unsigned long long>(agents.proposals), static_cast<unsigned long long>(agents.spawnedHidden),
				static_cast<unsigned long long>(agents.spawnedVisible), static_cast<unsigned long long>(agents.refused), static_cast<unsigned long long>(agents.despawned),
				static_cast<unsigned long long>(agents.goals), static_cast<unsigned long long>(agents.reports));
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
			std::vector<std::pair<sim::Entity, glm::vec3>> bodyStart;
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
				t.Check(state.client->CurrentTick() > 25 && state.client->SnapshotsReceived() > 5, "the sim ran %lld ticks and sent %llu snapshots", static_cast<long long>(state.client->CurrentTick()),
					static_cast<unsigned long long>(state.client->SnapshotsReceived()));
				t.Line("%s", Stats().c_str());
				t.Next(10);
				break;
			}
			case 10: // bodies: the sim proposes them for agents near the pawn, the game spawns them
			{
				if (SimAgents::BodyCount() == 0)
				{
					if (t.Seconds(t.stageStart) > 25.0)
					{
						t.Check(false, "no body appeared within 25 s. %s", Stats().c_str());
						t.Next(12);
					}
					break;
				}
				t.bodyStart = SimAgents::BodyPositions();
				t.Line("%zu bodies after %.1f s", t.bodyStart.size(), t.Seconds(t.stageStart));
				t.Next(11);
				break;
			}
			case 11: // and they walk the routes the sim dictates, the sim follows them
			{
				if (t.Seconds(t.stageStart) < 6.0)
					break;
				const auto positions = SimAgents::BodyPositions();
				const auto& snapshot = state.client->Snapshot();
				int moved = 0, followed = 0, driven = 0;
				for (const auto& [entity, feet] : positions)
				{
					for (const auto& before : t.bodyStart)
						if (before.first == entity && glm::distance(before.second, feet) > 1.0f)
							++moved;
					for (const auto& agent : snapshot.agents)
						if (agent.entity == entity && agent.body == 2)
						{
							++driven;
							if (level >= 0 && glm::distance(agent.nav - state.nav->Data().levels[level].offset, feet) < 3.0f)
								++followed;
						}
				}
				t.Check(!positions.empty(), "%zu bodies are still there", positions.size());
				t.Check(moved > 0, "%d of %zu bodies walked more than 1 m", moved, t.bodyStart.size());
				t.Check(driven > 0 && followed == driven, "the sim's agents follow their bodies (%d of %d within 3 m)", followed, driven);
				int named = 0, namedWithBody = 0;
				for (const auto& agent : snapshot.agents)
					if (agent.id.c_str()[0] != '\0')
					{
						++named;
						namedWithBody += agent.body == 2 ? 1 : 0;
					}
				t.Check(named == static_cast<int>(state.npcDefinitions.size()) && named > 0, "all %zu authored NPCs are in the sim (%d)", state.npcDefinitions.size(), named);
				t.Check(namedWithBody > 0, "authored NPCs near the player have bodies (%d of %d)", namedWithBody, named);
				t.Line("%s", Stats().c_str());
				for (const std::string& line : NpcLines())
					t.Line("%s", line.c_str());
				t.Next(12);
				break;
			}
			case 12:
				state.client->Pause();
				t.Next(3);
				break;
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
				// The hash differs by design when bodies were out at the save: a load drops them (the game has none). The tick is the same.
				auto tickOf = [](const std::string& text) { return text.substr(0, text.find(" hash")); };
				t.Check(gotAfter && tickOf(after) == tickOf(t.hashSaved) && tickOf(after) != tickOf(t.hashLater), "loading the save restored the saved sim state (%s, was %s)", tickOf(after).c_str(), tickOf(t.hashLater).c_str());
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
					t.Next(13);
				}
				break;
			case 13: // the bodies come back after the load: the level reloaded, the sim proposes them again
				if (SimAgents::BodyCount() > 0 || t.Seconds(t.stageStart) > 25.0)
				{
					t.Check(SimAgents::BodyCount() > 0, "bodies are back after the load (%zu)", SimAgents::BodyCount());
					t.Line("%s", Stats().c_str());
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

		state.bodies = std::make_unique<sim::bodies::BodiesModule>(*state.movement, *state.nav);
		state.npcs = std::make_unique<sim::npcs::NpcModule>(*state.movement, *state.nav);

		// The authored NPCs: what parses and fits the baked world is placed, the rest is reported loudly (one bad NPC costs only itself).
		{
			std::vector<sim::npcs::NpcDefinition> parsed;
			std::vector<std::string> problems;
			SimNpcFile::Load(data, parsed, problems);
			for (sim::npcs::NpcDefinition& definition : parsed)
			{
				const std::vector<std::string> invalid = state.npcs->Validate(definition);
				problems.insert(problems.end(), invalid.begin(), invalid.end());
				if (invalid.empty())
					state.npcDefinitions.push_back(std::move(definition));
			}
			if (!problems.empty())
			{
				Logger::Error("==== SIM NPC CONTENT: %zu problems in %s, %zu of %zu npcs will be placed ====", problems.size(), SimNpcFile::Path, state.npcDefinitions.size(), parsed.size());
				for (const std::string& problem : problems)
					Logger::Error("  %s", problem.c_str());
			}
		}

		sim::RegistryBuilder builder;
		builder.Add(*state.movement);
		builder.Add(*state.bodies);
		builder.Add(*state.npcs);
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
		Logger::Log("sim: started (%zu levels, %zu spawn points, %zu authored npcs, %d demo walkers)", data.levels.size(), data.spawns.size(), state.npcDefinitions.size(), demoCount);
		return true;
	}

	void Stop()
	{
		if (state.client)
			state.client->Stop();
	}

	bool IsRunning() { return Running(); }

	void LookAtNpc(const std::string& id)
	{
		state.lookId = id;
		state.lookPlaced = false;
	}

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
		FollowLook();
		ReportPlayer();
		SimAgents::Update(*state.client, state.nav->Data());
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

	CONSOLE_FUNC("sim_npcs", "List the authored NPCs: id, name, level, position, what they are doing, and whether they have a body")
	{
		if (!NeedsSim())
			return;
		for (const std::string& line : NpcLines())
			Logger::Log("%s", line.c_str());
	}

	CONSOLE_FUNC("sim_goto", "Move the pawn next to an authored NPC of the loaded level: sim_goto <id>")
	{
		if (!NeedsSim() || args.empty() || !SimPawn::Instance || !Level::Current)
		{
			Logger::Log("sim_goto <id>: needs the sim, a pawn and an NPC id (see sim_npcs)");
			return;
		}
		const auto& world = state.nav->Data();
		for (const sim::movement::AgentView& agent : state.client->Agents())
		{
			if (args[0] != agent.id.c_str())
				continue;
			const glm::vec3 local = agent.nav - world.levels[agent.level].offset;
			const int level = SimWorldNavFile::FindLevel(world, Level::Current->filePath);
			if (level != agent.level)
			{
				Logger::Log("sim_goto: %s is in %s at (%.1f %.1f %.1f); load that level first (map %s)", args[0].c_str(), world.levels[agent.level].name.c_str(), local.x, local.y, local.z,
					world.levels[agent.level].name.c_str());
				return;
			}
			SimPawn::Instance->Teleport(local + glm::vec3(0.0f, 0.9f, 3.0f));
			Logger::Log("sim_goto: next to %s", args[0].c_str());
			return;
		}
		Logger::Log("sim_goto: no NPC '%s'", args[0].c_str());
	}

	CONSOLE_FUNC("sim_look", "Stand 6 m from an authored NPC and keep the view on it: sim_look <id> (sim_look alone: stop)")
	{
		LookAtNpc(args.empty() ? std::string() : args[0]);
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

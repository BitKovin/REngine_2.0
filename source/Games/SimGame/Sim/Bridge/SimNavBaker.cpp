#include "SimNavBaker.h"

#ifndef DISTRIBUTION

#include "SimMapEntities.h"

#include <Sim/Nav/WorldNav.h>
#include <Sim/Nav/WorldNavBake.h>

#include <Console/ConsoleRegister.h>
#include <FileSystem/FileSystem.h>
#include <Level.hpp>
#include <Logger.hpp>
#include <Navigation/Navigation.hpp>

#include <SDL2/SDL.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <sstream>

namespace SimNavBaker
{
	namespace
	{
		struct State
		{
			bool running = false;
			bool quitWhenDone = false;
			std::string manifest;
			std::vector<std::string> maps;
			size_t index = 0;
			std::vector<sim::nav::LevelInput> levels;
		};

		State state;

		std::string MapPath(const std::string& map) { return "GameData/maps/" + map; }

		bool SamePath(std::string a, std::string b)
		{
			auto normalise = [](std::string& path)
			{
				std::replace(path.begin(), path.end(), '\\', '/');
				std::transform(path.begin(), path.end(), path.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			};
			normalise(a);
			normalise(b);
			return a == b;
		}

		void Finish(bool ok, const std::string& report)
		{
			FileSystemEngine::WriteFile(ReportPath, report);
			Logger::Log(report);
			Logger::Log(ok ? "sim_bake_nav: OK, wrote %s (report %s)" : "sim_bake_nav: FAILED (report %s%s)", ok ? OutputPath : ReportPath, ok ? ReportPath : "");
			state.running = false;
			state.levels.clear();
			if (state.quitWhenDone)
			{
				SDL_Event quit;
				quit.type = SDL_QUIT;
				SDL_PushEvent(&quit);
			}
		}

		// Routes from the root level's player start (or first spawn) to every spawn of every other level.
		std::string SmokeRoutes(const sim::nav::WorldNav& nav)
		{
			const auto& levels = state.levels;
			const sim::nav::SpawnInput* origin = nullptr;
			for (const auto& spawn : levels[0].spawns)
				if (!origin)
					origin = &spawn;
			std::string out = "routes from " + levels[0].name + (origin ? ":" + origin->name : std::string(" (no spawn)")) + "\n";
			if (!origin)
				return out;
			const sim::nav::NavPoint start = nav.Project(nav.ToNav(0, origin->position));
			char line[256];
			for (size_t level = 1; level < levels.size(); ++level)
				for (const auto& spawn : levels[level].spawns)
				{
					const sim::nav::NavPath path = nav.FindPath(start, nav.Project(nav.ToNav(static_cast<int>(level), spawn.position)));
					std::snprintf(line, sizeof(line), "  -> %s:%s  %s, %.1f m, %d portals, %zu corners\n", levels[level].name.c_str(), spawn.name.c_str(),
						path.complete ? "complete" : "NOT REACHED", path.length, path.portalCrossings, path.points.size());
					out += line;
				}
			return out;
		}

		void Bake()
		{
			sim::nav::WorldNavData data;
			sim::nav::BakeReport report;
			const bool ok = sim::nav::BakeWorldNav(state.levels, sim::nav::BakeSettings{}, data, report);
			std::string text = "manifest " + state.manifest + "\n" + report.Text();
			if (!ok)
			{
				Finish(false, text);
				return;
			}

			const std::vector<uint8_t> bytes = data.Serialize();
			if (!FileSystemEngine::WriteFileBinary(OutputPath, bytes))
			{
				Finish(false, text + "could not write " + std::string(OutputPath) + "\n");
				return;
			}
			text += "wrote " + std::string(OutputPath) + " (" + std::to_string(bytes.size()) + " bytes)\n";

			// Prove the written file loads and routes.
			sim::nav::WorldNavData loaded;
			sim::nav::WorldNav nav;
			std::string error;
			if (!sim::nav::WorldNavData::Deserialize(FileSystemEngine::ReadFileBinary(OutputPath), loaded, error) || !nav.Load(loaded, error))
			{
				Finish(false, text + "the written file does not load: " + error + "\n");
				return;
			}
			const std::string routes = SmokeRoutes(nav);
			text += routes;
			Finish(routes.find("NOT REACHED") == std::string::npos, text);
		}

		void CaptureCurrentLevel()
		{
			const std::string& map = state.maps[state.index];
			if (!Level::Current || !SamePath(Level::Current->filePath, MapPath(map)))
			{
				Finish(false, "sim_bake_nav: " + map + " did not load (current level: " + (Level::Current ? Level::Current->filePath : std::string("none")) + ")\n");
				return;
			}
			if (!NavigationSystem::navMesh)
			{
				Finish(false, "sim_bake_nav: " + map + " has no nav mesh after loading\n");
				return;
			}

			sim::nav::LevelInput input;
			input.name = map;
			input.nav = sim::nav::CaptureLevelNav(*NavigationSystem::navMesh, FileSystemEngine::GetFileModificationTime(MapPath(map)));
			for (LevelObject* object : Level::Current->LevelObjects)
			{
				if (auto* portal = dynamic_cast<SimPortal*>(object))
					input.portals.push_back({ portal->Name, portal->centre, portal->targetLevel, portal->targetSpawn });
				else if (auto* spawn = dynamic_cast<SimSpawn*>(object))
				{
					// The player start goes first: it is the origin of the smoke routes.
					const sim::nav::SpawnInput entry{ spawn->Name, spawn->Position, spawn->playerStart };
					if (spawn->playerStart)
						input.spawns.insert(input.spawns.begin(), entry);
					else
						input.spawns.push_back(entry);
				}
			}
			Logger::Log("sim_bake_nav: captured %s: %zu tiles, %zu portals, %zu spawns", map.c_str(), input.nav.tiles.size(), input.portals.size(), input.spawns.size());
			state.levels.push_back(std::move(input));

			if (++state.index < state.maps.size())
				Level::LoadLevelFromFile(MapPath(state.maps[state.index]));
			else
				Bake();
		}
	}

	bool Start(const std::string& manifestPath, bool quitWhenDone)
	{
		if (state.running)
		{
			Logger::Error("sim_bake_nav: a bake is already running");
			return false;
		}
		std::vector<std::string> maps = SimWorldNavFile::ReadManifest(manifestPath);
		if (maps.empty())
		{
			Logger::Error("sim_bake_nav: manifest %s is missing or lists no maps", manifestPath.c_str());
			if (quitWhenDone)
				Finish(false, "manifest " + manifestPath + " is missing or lists no maps\n");
			return false;
		}

		state = State{};
		state.running = true;
		state.quitWhenDone = quitWhenDone;
		state.manifest = manifestPath;
		state.maps = std::move(maps);
		Logger::Log("sim_bake_nav: baking %zu levels from %s", state.maps.size(), manifestPath.c_str());
		Level::LoadLevelFromFile(MapPath(state.maps[0]));
		return true;
	}

	void Update()
	{
		if (!state.running || Level::HasPendingLevelLoad())
			return;
		CaptureCurrentLevel();
	}

	bool IsRunning() { return state.running; }
}

CONSOLE_FUNC("sim_bake_nav", "Bake the world nav mesh: sim_bake_nav [manifest] (default GameData/sim/world.manifest)")
{
	SimNavBaker::Start(args.empty() ? SimNavBaker::DefaultManifest : args[0], false);
}

#endif

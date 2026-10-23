#include "SimNpcFile.h"

#include <FileSystem/FileSystem.h>
#include <json.hpp>

#include <algorithm>
#include <cstdarg>
#include <cstdio>

namespace SimNpcFile
{
	namespace
	{
		using nlohmann::json;
		using namespace sim::npcs;

		std::string Fmt(const char* format, ...)
		{
			char buffer[512];
			va_list args;
			va_start(args, format);
			std::vsnprintf(buffer, sizeof(buffer), format, args);
			va_end(args);
			return buffer;
		}

		bool ReadVector(const json& value, glm::vec3& out)
		{
			if (!value.is_array() || value.size() != 3)
				return false;
			for (int i = 0; i < 3; ++i)
				if (!value[i].is_number())
					return false;
			out = { value[0].get<float>(), value[1].get<float>(), value[2].get<float>() };
			return true;
		}

		std::string Lower(std::string text)
		{
			std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return text;
		}

		// One NPC; false (with problems appended) when it cannot be used.
		bool ParseNpc(const json& entry, size_t index, const sim::nav::WorldNavData& world, NpcDefinition& out, std::vector<std::string>& errors)
		{
			const size_t before = errors.size();
			std::string who = Fmt("npcs[%zu]", index);
			if (!entry.is_object())
			{
				errors.push_back(who + ": is not an object");
				return false;
			}
			out.id = entry.value("id", std::string());
			if (!out.id.empty())
				who = "npc '" + out.id + "'";
			out.name = entry.value("name", out.id);

			const json model = entry.contains("model") ? entry["model"] : json("base");
			if (model.is_string())
			{
				const std::string name = Lower(model.get<std::string>());
				if (name == "base")
					out.model = 0;
				else if (name == "guard")
					out.model = 1;
				else
					errors.push_back(who + ": model '" + name + "' is not \"base\" or \"guard\"");
			}
			else if (model.is_number_integer())
				out.model = model.get<int>();
			else
				errors.push_back(who + ": model must be \"base\", \"guard\" or a number");

			out.speed = entry.value("speed", 1.0f);

			const std::string gait = Lower(entry.value("gait", std::string("walk")));
			if (gait == "walk")
				out.gait = sim::movement::Gait::Walk;
			else if (gait == "run")
				out.gait = sim::movement::Gait::Run;
			else
				errors.push_back(who + ": gait '" + gait + "' is not \"walk\" or \"run\"");

			const std::string routine = Lower(entry.value("routine", std::string("loop")));
			if (routine == "loop")
				out.mode = Routine::Mode::Loop;
			else if (routine == "pingpong")
				out.mode = Routine::Mode::PingPong;
			else
				errors.push_back(who + ": routine '" + routine + "' is not \"loop\" or \"pingpong\"");

			const std::string defaultLevel = entry.value("level", std::string());
			if (!entry.contains("stops") || !entry["stops"].is_array() || entry["stops"].empty())
			{
				errors.push_back(who + ": needs a non-empty \"stops\" list");
				return false;
			}

			const json& stops = entry["stops"];
			for (size_t i = 0; i < stops.size(); ++i)
			{
				const json& stop = stops[i];
				const std::string where = Fmt("%s stop %zu", who.c_str(), i);
				StopDefinition definition;
				if (!stop.is_object())
				{
					errors.push_back(where + ": is not an object");
					continue;
				}
				definition.level = stop.value("level", defaultLevel);
				if (definition.level.empty())
				{
					errors.push_back(where + ": has no level (set \"level\" on the stop or on the npc)");
					continue;
				}

				if (stop.contains("at"))
				{
					if (!ReadVector(stop["at"], definition.local))
					{
						errors.push_back(where + ": \"at\" must be three numbers [x, y, z]");
						continue;
					}
				}
				else if (stop.contains("spawn") && stop["spawn"].is_string())
				{
					const std::string spawnName = stop["spawn"].get<std::string>();
					const int level = world.FindLevel(definition.level);
					bool found = false;
					for (const sim::nav::WorldSpawn& spawn : world.spawns)
						if (spawn.name == spawnName && level >= 0 && spawn.level == level)
						{
							definition.local = spawn.position - world.levels[level].offset;
							found = true;
						}
					if (!found)
					{
						errors.push_back(where + ": spawn point '" + spawnName + "' does not exist in " + definition.level);
						continue;
					}
				}
				else
				{
					errors.push_back(where + ": needs \"at\": [x, y, z] or \"spawn\": \"name\"");
					continue;
				}

				if (stop.contains("wait"))
				{
					const json& wait = stop["wait"];
					if (wait.is_number())
						definition.waitMin = definition.waitMax = wait.get<float>();
					else if (wait.is_array() && wait.size() == 2 && wait[0].is_number() && wait[1].is_number())
					{
						definition.waitMin = wait[0].get<float>();
						definition.waitMax = wait[1].get<float>();
					}
					else
					{
						errors.push_back(where + ": \"wait\" must be seconds or [min, max]");
						continue;
					}
				}
				out.stops.push_back(std::move(definition));
			}
			return errors.size() == before;
		}
	}

	void Parse(const std::string& text, const sim::nav::WorldNavData& world, std::vector<NpcDefinition>& npcs, std::vector<std::string>& errors)
	{
		json root;
		try
		{
			root = json::parse(text);
		}
		catch (const json::exception& exception)
		{
			errors.push_back(std::string(Path) + ": not valid JSON: " + exception.what());
			return;
		}
		if (!root.is_object() || !root.contains("npcs") || !root["npcs"].is_array())
		{
			errors.push_back(std::string(Path) + ": needs an object with an \"npcs\" list");
			return;
		}
		for (size_t i = 0; i < root["npcs"].size(); ++i)
		{
			NpcDefinition definition;
			if (!ParseNpc(root["npcs"][i], i, world, definition, errors))
				continue;
			const bool twice = std::any_of(npcs.begin(), npcs.end(), [&](const NpcDefinition& other) { return other.id == definition.id; });
			if (twice)
			{
				errors.push_back("npc '" + definition.id + "': the id is used twice, the second one is ignored");
				continue;
			}
			npcs.push_back(std::move(definition));
		}
	}

	void Load(const sim::nav::WorldNavData& world, std::vector<NpcDefinition>& npcs, std::vector<std::string>& errors, bool required)
	{
		const std::string text = FileSystemEngine::ReadFile(Path);
		if (text.empty())
		{
			if (required)
				errors.push_back(std::string(Path) + " is missing or empty");
			return;
		}
		Parse(text, world, npcs, errors);
	}
}

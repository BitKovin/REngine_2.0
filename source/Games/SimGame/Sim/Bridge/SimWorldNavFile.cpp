#include "SimWorldNavFile.h"

#include <FileSystem/FileSystem.h>
#include <Logger.hpp>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>

namespace SimWorldNavFile
{
	namespace
	{
		std::string Normalised(std::string path)
		{
			std::replace(path.begin(), path.end(), '\\', '/');
			return path;
		}

		bool SameText(const std::string& a, const std::string& b)
		{
			return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](unsigned char x, unsigned char y) { return std::tolower(x) == std::tolower(y); });
		}
	}

	std::vector<std::string> ReadManifest(const std::string& manifestPath)
	{
		std::istringstream lines(FileSystemEngine::ReadFile(manifestPath));
		std::vector<std::string> maps;
		std::string line;
		while (std::getline(lines, line))
		{
			line.erase(std::remove_if(line.begin(), line.end(), [](unsigned char c) { return std::isspace(c); }), line.end());
			if (!line.empty() && line[0] != '#')
				maps.push_back(line);
		}
		return maps;
	}

	std::string LevelNameOfMapPath(const std::string& mapPath)
	{
		const std::string path = Normalised(mapPath);
		const size_t folderLength = std::strlen(MapFolder);
		if (path.size() <= folderLength || !SameText(path.substr(0, folderLength), MapFolder))
			return {};
		return path.substr(folderLength);
	}

	int FindLevel(const sim::nav::WorldNavData& data, const std::string& mapPath)
	{
		const std::string name = LevelNameOfMapPath(mapPath);
		if (name.empty())
			return -1;
		for (size_t i = 0; i < data.levels.size(); ++i)
			if (SameText(data.levels[i].name, name))
				return static_cast<int>(i);
		return -1;
	}

	bool Load(sim::nav::WorldNavData& out, std::string& error)
	{
		const std::vector<uint8_t> bytes = FileSystemEngine::ReadFileBinary(Path);
		if (bytes.empty())
		{
			error = std::string(Path) + " is missing or empty";
			return false;
		}
		return sim::nav::WorldNavData::Deserialize(bytes, out, error);
	}

#ifndef DISTRIBUTION
	int CheckFresh(const sim::nav::WorldNavData& data)
	{
		const auto stale = sim::nav::FindStaleLevels(data, [](const std::string& level) { return FileSystemEngine::GetFileModificationTime(MapPath(level)); });
		if (stale.empty())
			return 0;
		Logger::Error("==== SIM WORLD NAV IS STALE: %zu of %zu levels changed since %s was baked ====", stale.size(), data.levels.size(), Path);
		for (const auto& level : stale)
			Logger::Error("  %s", level.Describe().c_str());
		Logger::Error("  Agents would walk the old layout. Rebake: SimGame.exe -headless -sim_bake_nav (from the game folder), or the console command sim_bake_nav");
		return static_cast<int>(stale.size());
	}

	bool CheckAtGameLoad()
	{
		sim::nav::WorldNavData data;
		std::string error;
		if (!Load(data, error))
		{
			Logger::Error("==== SIM WORLD NAV UNAVAILABLE: %s ====", error.c_str());
			Logger::Error("  Bake it: SimGame.exe -headless -sim_bake_nav (from the game folder), or the console command sim_bake_nav");
			return false;
		}
		return CheckFresh(data) == 0;
	}
#endif
}

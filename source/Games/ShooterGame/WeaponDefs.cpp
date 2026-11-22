#include "WeaponDefs.h"

#include <Helpers/CSVParser.h>
#include <FileSystem/FileSystem.h>
#include <Logger.hpp>

#include <algorithm>
#include <set>

namespace
{
	constexpr const char* kFolder = "GameData/tables/weapons/";

	float ParseFloat(const std::string& text)
	{
		if (text.empty())
			return 0.0f;
		try { return std::stof(text); }
		catch (...) { return 0.0f; }
	}

	int ParseInt(const std::string& text, int fallback)
	{
		if (text.empty())
			return fallback;
		try { return std::stoi(text); }
		catch (...) { return fallback; }
	}

	bool ParseAkimbo(const std::string& text, AkimboMode& out)
	{
		if (text.empty() || text == "None") { out = AkimboMode::None; return true; }
		if (text == "WithPerk") { out = AkimboMode::WithPerk; return true; }
		if (text == "Always") { out = AkimboMode::Always; return true; }
		if (text == "Only") { out = AkimboMode::Only; return true; }
		return false;
	}
}

void WeaponDefs::Load()
{
	defs.clear();

	for (const std::string& file : FileSystemEngine::GetFilesInPath(kFolder))
		LoadFile(kFolder + file);
}

void WeaponDefs::LoadFile(const std::string& path)
{
	std::string text = FileSystemEngine::ReadFile(path);

	text.erase(0, text.find_first_not_of("\r\n"));
	text.erase(text.find_last_not_of("\r\n") + 1);

	CSVParser parser(text);

	for (size_t row = 1; row < parser.rowCount(); ++row)
	{
		WeaponDef def;
		def.className = parser.at((int)row, 0).trimmed();
		if (def.className.empty())
			continue;

		def.slot = ParseInt(parser.at((int)row, 1).trimmed(), 0);
		def.slotOrder = ParseInt(parser.at((int)row, 2).trimmed(), 0);

		std::string akimbo = parser.at((int)row, 3).trimmed();
		if (!ParseAkimbo(akimbo, def.akimbo))
		{
			Logger::Error("WeaponDefs: %s: unknown akimbo mode '%s' for %s, using None", path.c_str(), akimbo.c_str(), def.className.c_str());
			def.akimbo = AkimboMode::None;
		}

		def.displayName = parser.at((int)row, 4).trimmed();
		def.iconModel = parser.at((int)row, 5).trimmed();
		def.iconRotation = glm::vec3(
			ParseFloat(parser.at((int)row, 6).trimmed()),
			ParseFloat(parser.at((int)row, 7).trimmed()),
			ParseFloat(parser.at((int)row, 8).trimmed()));

		if (def.slot < 0 || def.slot > 5)
		{
			Logger::Error("WeaponDefs: %s: slot of %s must be 0..5", path.c_str(), def.className.c_str());
			def.slot = std::max(0, std::min(5, def.slot));
		}

		defs[def.className] = def;
	}
}

const WeaponDef* WeaponDefs::Find(const std::string& className)
{
	auto it = defs.find(className);
	if (it != defs.end())
		return &it->second;

	static std::set<std::string> warned;
	if (!className.empty() && warned.insert(className).second)
		Logger::Warning("WeaponDefs: no row for weapon class %s", className.c_str());

	return nullptr;
}

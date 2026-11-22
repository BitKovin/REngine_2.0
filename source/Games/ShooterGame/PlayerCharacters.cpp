#include "PlayerCharacters.h"

#include <Helpers/CSVParser.h>
#include <FileSystem/FileSystem.h>
#include <Logger.hpp>

namespace
{
	constexpr const char* kFolder = "GameData/tables/characters/";

	bool ParseBool(const std::string& text)
	{
		return text == "TRUE" || text == "true" || text == "1" || text == "t";
	}
}

void PlayerCharacters::Load()
{
	defs.clear();

	for (const std::string& file : FileSystemEngine::GetFilesInPath(kFolder))
		LoadFile(kFolder + file);
}

void PlayerCharacters::LoadFile(const std::string& path)
{
	std::string text = FileSystemEngine::ReadFile(path);

	text.erase(0, text.find_first_not_of("\r\n"));
	text.erase(text.find_last_not_of("\r\n") + 1);

	CSVParser parser(text);

	for (size_t row = 1; row < parser.rowCount(); ++row)
	{
		PlayerCharacterDef def;
		def.id = parser.at((int)row, 0).trimmed();
		if (def.id.empty())
			continue;

		def.displayName = parser.at((int)row, 1).trimmed();
		def.akimboPerk = ParseBool(parser.at((int)row, 2).trimmed());
		def.hudSilhouette = parser.at((int)row, 3).trimmed();

		defs[def.id] = def;
	}
}

const PlayerCharacterDef* PlayerCharacters::Find(const std::string& id)
{
	auto it = defs.find(id);
	if (it != defs.end())
		return &it->second;

	Logger::Warning("PlayerCharacters: no character with id %s", id.c_str());
	return nullptr;
}

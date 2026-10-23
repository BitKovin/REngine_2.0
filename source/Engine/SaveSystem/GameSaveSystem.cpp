#include "GameSaveSystem.h"
#include "../Level.hpp"
#include <Helpers/Base64.h>
#include <Logger.hpp>

void GameSaveSystem::RegisterExtension(GameSaveExtension extension)
{
    for (GameSaveExtension& existing : extensions)
        if (existing.id == extension.id)
        {
            existing = std::move(extension);
            return;
        }
    extensions.push_back(std::move(extension));
}

GameSaveData GameSaveSystem::SaveGameToData()
{

    GameSaveData data;

    data.LevelData = LevelSaveSystem::SaveLevelToData();
    data.LevelMemory = LevelTraversalSystem::LevelMemory;

    ExtensionSaveFailed = false;
    for (const GameSaveExtension& extension : extensions)
    {
        std::vector<uint8_t> bytes;
        if (!extension.save || !extension.save(bytes))
        {
            Logger::Error("save extension '%s' failed to save", extension.id.c_str());
            ExtensionSaveFailed = true;
            continue;
        }
        data.Extensions[extension.id] = Base64::Encode(bytes);
    }

    return data;
}

void GameSaveSystem::LoadGameFromData(GameSaveData data)
{
    // Extensions go first and can cancel the load while nothing has changed.
    for (const GameSaveExtension& extension : extensions)
    {
        const auto entry = data.Extensions.find(extension.id);
        const std::vector<uint8_t> bytes = entry == data.Extensions.end() ? std::vector<uint8_t>() : Base64::Decode(entry->second);
        if (!extension.load || !extension.load(bytes))
        {
            Logger::Error("save extension '%s' refused the save; the load is cancelled", extension.id.c_str());
            return;
        }
    }

    LevelTraversalSystem::Reset();

    LevelTraversalSystem::LevelMemory = data.LevelMemory;
    LoadingGame = true;

    LevelSaveSystem::pendingSave = data.LevelData;
    LevelTraversalSystem::Traveling = false;
    Level::LoadLevelFromFile(data.LevelData.name);

}



void GameSaveSystem::SaveGameToFile(std::string saveName)
{

    GameSaveData data = SaveGameToData();
    if (ExtensionSaveFailed)
    {
        Logger::Error("save '%s' was not written: a save extension failed", saveName.c_str());
        return;
    }

    json j = json(data);

    FileSystemEngine::WriteFile(saveDataPath + saveName + ".savg", j.dump());

}

void GameSaveSystem::LoadGameFromFile(std::string saveName)
{

    string text = FileSystemEngine::ReadFile(saveDataPath + saveName + ".savg");

    if (text.size() < 1)
    {
        Logger::Log("failed to load save" + saveName);
        return;
    }


    json jsonData = json::parse(text);

    GameSaveData gameData = jsonData.get<GameSaveData>();

    LoadGameFromData(gameData);

}

std::vector<std::string> GameSaveSystem::GetAllSaves()
{
    std::vector<std::string> files = FileSystemEngine::GetFilesInPath(saveDataPath);

    for (auto& f : files)
    {
        constexpr std::string_view extension = ".savg";

        if (f.size() >= extension.size() &&
            f.compare(f.size() - extension.size(), extension.size(), extension) == 0)
        {
            f.erase(f.size() - extension.size());
        }
    }

    return files;
}
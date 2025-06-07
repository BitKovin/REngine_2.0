// LevelSaveSystem.cpp

#include "LevelSaveSystem.h"

#include <string>

#include "../Level.hpp"

#include "../Entity.h"

#include "../json.hpp"

#include "../LevelObjectFactory.h"

#include "../Physics.h"

using namespace nlohmann;

LevelSaveData LevelSaveSystem::pendingSave;

LevelSaveData LevelSaveSystem::SaveLevelToData()
{

    LevelSaveData levelData;

    Level* level = Level::Current;

    vector<EntitySaveData> entities;

    for (auto levelObject : level->LevelObjects)
    {

        Entity* entity = (Entity*)levelObject;

        if (entity == nullptr) continue;

        if (entity->SaveGame == false) continue;

        EntitySaveData data;
        data.className = entity->ClassName;
        data.id = entity->Id;
        
        json jsonData;

        entity->Serialize(jsonData);

        data.data = jsonData.dump();

        entities.push_back(data);

    }

    levelData.deletedIDs = level->deletedIDs;
    levelData.deletedNames = level->deletedNames;

    levelData.name = level->filePath;
    levelData.entities = entities;
    levelData.nextId = level->nextId;

    return levelData;
}

void LevelSaveSystem::LoadLevelFromData(LevelSaveData data)
{

    Physics::Simulate();

    Time::DeltaTime = 0.0001;
    Time::DeltaTimeF = 0.0001f;

    for (auto id : data.deletedIDs)
    {
        Entity* target = Level::Current->FindEntityWithId(id);

        if (target == nullptr) continue;

        if (target->SaveGame == false) continue;

        target->Destroy();
    }


    for (auto name : data.deletedNames)
    {
        Entity* target = Level::Current->FindEntityWithName(name);

        if (target == nullptr) continue;

        if (target->SaveGame == false) continue;

        target->Destroy();
    }

    vector<Entity*> createdEntities;
    unordered_map<Entity*, EntitySaveData> pendingLoadEntities;

    for (auto entitySaveData : data.entities)
    {
        Entity* targetEntity = Level::Current->FindEntityWithId(entitySaveData.id);

        if (targetEntity == nullptr)
        {
            targetEntity = LevelObjectFactory::instance().create(entitySaveData.className);
            Level::Current->AddEntity(targetEntity);
            createdEntities.push_back(targetEntity);
        }
        pendingLoadEntities[targetEntity] = entitySaveData;

    }

    Level::Current->AddPendingLevelObjects();

    Physics::Simulate();

    for (auto entity : createdEntities)
    {
        entity->Start();
        entity->LoadAssetsIfNeeded();
    }

    Physics::Simulate();

    Level::Current->AddPendingLevelObjects();

    for (auto entity : pendingLoadEntities)
    {

        json data = json::parse(entity.second.data);

        entity.first->Deserialize(data);

    }

    Physics::Simulate();
    Logger::Log("save loaded\n");
}

const string saveDataPath = "SaveData/";

void LevelSaveSystem::SaveLevelToFile(const string& saveName)
{
    auto levelSaveData = SaveLevelToData();

    json jsonData = json(levelSaveData);

    WriteSaveFile(saveDataPath + saveName + ".sav", jsonData.dump());

}

void LevelSaveSystem::LoadLevelFromFile(const string& saveName)
{

    string text;

    if (ReadSaveFile(saveDataPath + saveName + ".sav", text) == false)
    {
        Logger::Log("failed to load save" + saveName);
        return;
    }

    json jsonData = json::parse(text);

    LevelSaveData levelData = jsonData.get<LevelSaveData>();

    pendingSave = levelData;
    Level::LoadLevelFromFile(pendingSave.name);

}


#ifndef __EMSCRIPTEN__
// Desktop
#include <fstream>
#include <sstream>
#include <filesystem>  // C++17
#else
// Emscripten
#include <emscripten.h>
#include <emscripten/emscripten.h>

// --- Setup persistent IDBFS mount ---
// Call this once at app startup to mount /save and load persisted files
EM_JS(void, MountPersistentFS, (), {
    FS.mkdir('/save');
    FS.mount(IDBFS, {}, '/save');
    // pull persisted data into memory FS
    FS.syncfs(true, function(err) {
        if (err) console.error('IDBFS initial load failed:', err);
    });
    });

// JS helper to write a UTF‑8 string into /save and sync to IndexedDB
EM_JS(void, WriteFileJS, (const char* relPath, const char* content), {
    var path = '/save/' + UTF8ToString(relPath);
    var data = UTF8ToString(content);
    // ensure directory tree under /save exists
    (function ensureDir(fp) {
        var parts = fp.split('/');
        parts.pop();
        var cur = "";
        for (var i = 1; i < parts.length; i++) {
            cur += '/' + parts[i];
            try { FS.mkdir(cur); }
 catch (e) { /* exists */ }
}
})(path);
FS.writeFile(path, data);
// push changes to IndexedDB
FS.syncfs(false, function(err) {
    if (err) console.error('IDBFS write sync failed:', err);
});
    });

// JS helper to read a UTF‑8 string from /save
EM_JS(char*, ReadFileJS, (const char* relPath), {
    try {
        var path = '/save/' + UTF8ToString(relPath);
        var data = FS.readFile(path, { encoding: 'utf8' });
        var len = lengthBytesUTF8(data) + 1;
        var buf = _malloc(len);
        stringToUTF8(data, buf, len);
        return buf;
    }
 catch (e) {
  return 0;
}
    });
#endif

// Call at program initialization (before any save/load calls)
void LevelSaveSystem::InitPersistence()
{
#ifdef __EMSCRIPTEN__
    MountPersistentFS();
#endif
}

void LevelSaveSystem::WriteSaveFile(std::string filePath, std::string content)
{
#ifdef __EMSCRIPTEN__
    // Emscripten: save under /save and persist
    WriteFileJS(filePath.c_str(), content.c_str());
#else
    // Desktop: ensure directory exists
    try {
        auto parent = std::filesystem::path(filePath).parent_path();
        if (!parent.empty() && !std::filesystem::exists(parent)) {
            std::filesystem::create_directories(parent);
        }
    }
    catch (const std::exception& e) {
        // log or throw
    }
    std::ofstream ofs(filePath, std::ios::binary);
    if (!ofs) return;
    ofs.write(content.data(), content.size());
#endif
}

bool LevelSaveSystem::ReadSaveFile(std::string filePath, std::string& out)
{
#ifdef __EMSCRIPTEN__
    // Emscripten: read from /save
    char* buf = ReadFileJS(filePath.c_str());
    if (!buf) return false;
    out.assign(buf);
    free(buf);
    return true;
#else
    std::ifstream ifs(filePath, std::ios::binary);
    if (!ifs) return false;
    std::ostringstream ss;
    ss << ifs.rdbuf();
    out = ss.str();
    return true;
#endif
}

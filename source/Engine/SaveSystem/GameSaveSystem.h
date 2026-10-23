#pragma once
#include "LevelSaveSystem.h"
#include "../LevelTraversalSystem.h"
#include <cstdint>
#include <functional>
#include <vector>

struct GameSaveData
{
	LevelSaveData LevelData;
	std::unordered_map<std::string, LevelSaveData> LevelMemory;
	std::unordered_map<std::string, std::string> Extensions; // GameSaveExtension payloads, Base64; absent in older saves
	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(GameSaveData, LevelData, LevelMemory, Extensions)
};

// Game state that lives outside the level (a simulation, a quest log) and must be saved and loaded together with it.
// A game registers one at startup; games that register none are unaffected.
struct GameSaveExtension
{
	std::string id; // key inside the save file, e.g. "sim"
	// Called by SaveGameToData. Fill `out`; return false to fail the save (nothing is written, the reason is the extension's to log).
	std::function<bool(std::vector<uint8_t>& out)> save;
	// Called by LoadGameFromData before the level is touched; `bytes` is empty when the save has no entry (an older save).
	// Return false to cancel the load (nothing has changed yet).
	std::function<bool(const std::vector<uint8_t>& bytes)> load;
};

class GameSaveSystem
{
public:
	
	// Replaces an extension with the same id.
	static void RegisterExtension(GameSaveExtension extension);

	static GameSaveData SaveGameToData();
	static void LoadGameFromData(GameSaveData data);

	static void SaveGameToFile(std::string saveName);
	static void LoadGameFromFile(std::string saveName);

	static inline bool LoadingGame = false;

	// True when an extension failed in the last SaveGameToData; SaveGameToFile then writes nothing.
	static inline bool ExtensionSaveFailed = false;

	static std::vector<std::string> GetAllSaves();

	static inline string saveDataPath = "SaveData/saves/";

private:

	static inline std::vector<GameSaveExtension> extensions;

};


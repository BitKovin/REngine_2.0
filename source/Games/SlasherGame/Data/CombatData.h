#pragma once

#include <Data/CharacterData.h>
#include <Data/MoveSetData.h>
#include <Data/PlayerConfigData.h>
#include <Data/StatusEffectData.h>
#include <Data/WeaponData.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Thread-safe cache of combat data files, keyed by path (COMBAT_DESIGN.md §10.3).
//
// Load*() reads through FileSystemEngine, parses, validates and resolves the file once; every entity of a type
// shares the returned immutable data. Call Load*() only from LoadAssets paths (they may run on worker
// threads) - wrap them in a CombatData::LoadScope so dev builds can flag file reads made at runtime.
// A file that failed to load returns nullptr (errors are logged once) until the next dev reload.
class CombatData
{
public:
	static std::shared_ptr<const CharacterData> LoadCharacter(const std::string& path);
	static std::shared_ptr<const MoveSetData> LoadMoveSet(const std::string& path);
	static std::shared_ptr<const WeaponData> LoadWeapon(const std::string& path);
	static std::shared_ptr<const StatusEffectTable> LoadStatusEffects();
	static std::shared_ptr<const PlayerConfigData> LoadPlayerConfig();

	// Cached version without any file access (nullptr if never loaded). Used to adopt reloaded data.
	static std::shared_ptr<const CharacterData> FindCharacter(const std::string& path);
	static std::shared_ptr<const StatusEffectTable> FindStatusEffects();
	static std::shared_ptr<const PlayerConfigData> FindPlayerConfig();

	// Increments whenever a dev reload replaced data.
	static uint32_t GetGeneration();

	static constexpr const char* StatusEffectsPath = "GameData/combat/effects.json";
	static constexpr const char* PlayerConfigPath = "GameData/combat/player.json";

	// Marks the current thread as running LoadAssets (nesting allowed).
	class LoadScope
	{
	public:
		LoadScope();
		~LoadScope();
		LoadScope(const LoadScope&) = delete;
		LoadScope& operator=(const LoadScope&) = delete;
	};

#ifndef DISTRIBUTION
	// Re-reads every cached file. Files that fail keep their previous data. Returns a human-readable report.
	static std::string ReloadAll();

	// Writes the edits between `base` (the data the editor started from) and `edited` into the moveset's source file
	// (debug panel "Save", COMBAT_DESIGN.md §10.4): only the edited values change in the text - notes, layout and
	// whatever else changed in the file meanwhile stay. Returns false and fills error on failure (the file untouched).
	static bool SaveMoveSet(const MoveSetData& base, const MoveSetData& edited, std::string& error);

	// Replaces a cached moveset with an edited copy (live tuning) after validating it.
	static bool ReplaceMoveSet(const MoveSetData& edited, std::string& error);

	// Warnings of the last load of `path` (data self tests).
	static std::vector<std::string> GetWarnings(const std::string& path);
#endif
};

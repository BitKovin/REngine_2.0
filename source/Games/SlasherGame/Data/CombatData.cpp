#include "CombatData.h"

#include <FileSystem/FileSystem.h>
#include <Logger.hpp>

#include <atomic>
#include <map>
#include <mutex>

namespace
{
	template<typename T>
	using Cache = std::map<std::string, std::shared_ptr<const T>>;   // nullptr value = failed load

	struct Storage
	{
		std::recursive_mutex Mutex;
		Cache<CharacterData> Characters;
		Cache<MoveSetData> MoveSets;
		Cache<WeaponData> Weapons;
		std::shared_ptr<const StatusEffectTable> StatusEffects;
		bool StatusEffectsTried = false;
		std::shared_ptr<const PlayerConfigData> PlayerConfig;
		bool PlayerConfigTried = false;
		std::atomic<uint32_t> Generation{ 0 };
#ifndef DISTRIBUTION
		std::map<std::string, std::vector<std::string>> Warnings;   // per path, last load
#endif
	};

	Storage& Data()
	{
		static Storage storage;
		return storage;
	}

	thread_local int loadScopeDepth = 0;

	void WarnIfOutsideLoadScope(const std::string& path)
	{
#ifndef DISTRIBUTION
		if (loadScopeDepth == 0)
			Logger::Error("[CombatData] '%s' read outside LoadAssets - combat data must be loaded in LoadAssets", path.c_str());
#else
		(void)path;
#endif
	}

	// Reads + parses + converts + resolves one file. Returns nullptr on error (report is logged).
	template<typename T, typename ResolveFn>
	std::shared_ptr<T> ParseFile(const std::string& path, ResolveFn&& resolve)
	{
		DataLoadReport report;
		report.Source = path;

		nlohmann::json json;
		std::shared_ptr<T> data = std::make_shared<T>();

		const bool ok = ParseDataJson(FileSystemEngine::ReadFile(path), json, report)
			&& ConvertDataJson(json, *data, "", report)
			&& resolve(*data, report);

		report.Log();
#ifndef DISTRIBUTION
		{
			std::lock_guard lock(Data().Mutex);
			Data().Warnings[path] = report.Warnings;
		}
#endif
		if (!ok)
		{
			Logger::Error("[CombatData] failed to load '%s' (%d error(s))", path.c_str(), static_cast<int>(report.Errors.size()));
			return nullptr;
		}
		return data;
	}

	std::shared_ptr<MoveSetData> ParseMoveSet(const std::string& path)
	{
		return ParseFile<MoveSetData>(path, [&](MoveSetData& d, DataLoadReport& r) { d.sourcePath = path; return ResolveMoveSet(d, r); });
	}

	std::shared_ptr<WeaponData> ParseWeapon(const std::string& path)
	{
		return ParseFile<WeaponData>(path, [&](WeaponData& d, DataLoadReport& r) { d.sourcePath = path; return ResolveWeapon(d, r); });
	}

	std::shared_ptr<StatusEffectTable> ParseStatusEffects(const std::string& path)
	{
		return ParseFile<StatusEffectTable>(path, [&](StatusEffectTable& d, DataLoadReport& r) { d.sourcePath = path; return ResolveStatusEffects(d, r); });
	}

	std::shared_ptr<PlayerConfigData> ParsePlayerConfig(const std::string& path)
	{
		return ParseFile<PlayerConfigData>(path, [&](PlayerConfigData& d, DataLoadReport& r) { d.sourcePath = path; return ResolvePlayerConfig(d, r); });
	}

	// Character files reference movesets and weapons: those are loaded (or taken from the cache) first.
	std::shared_ptr<CharacterData> ParseCharacter(const std::string& path)
	{
		return ParseFile<CharacterData>(path, [&](CharacterData& d, DataLoadReport& r)
		{
			d.sourcePath = path;
			if (!ResolveCharacter(d, r))
				return false;

			d.moveSet = CombatData::LoadMoveSet(d.moveset);
			for (const auto& [slotName, files] : d.weapons)
			{
				const int slot = FindSlotIndex(WeaponSlotNames, slotName);
				if (slot < 0) continue;

				for (size_t i = 0; i < files.size(); ++i)
				{
					std::shared_ptr<const WeaponData> weapon = CombatData::LoadWeapon(files[i]);
					if (weapon)
						d.weaponData[static_cast<size_t>(slot)].push_back(std::move(weapon));
					else
						r.Error("weapons." + slotName + "[" + std::to_string(i) + "]", "failed to load '" + files[i] + "'");
				}
			}

			return ResolveCharacterReferences(d, r) && r.Ok();
		});
	}

	template<typename T, typename ParseFn>
	std::shared_ptr<const T> LoadCached(Cache<T>& cache, const std::string& path, ParseFn&& parse)
	{
		Storage& storage = Data();
		std::lock_guard lock(storage.Mutex);

		auto it = cache.find(path);
		if (it != cache.end())
			return it->second;

		WarnIfOutsideLoadScope(path);
		std::shared_ptr<const T> data = parse(path);
		cache[path] = data;
		return data;
	}
}

CombatData::LoadScope::LoadScope() { ++loadScopeDepth; }
CombatData::LoadScope::~LoadScope() { --loadScopeDepth; }

std::shared_ptr<const CharacterData> CombatData::LoadCharacter(const std::string& path)
{
	return LoadCached(Data().Characters, path, ParseCharacter);
}

std::shared_ptr<const MoveSetData> CombatData::LoadMoveSet(const std::string& path)
{
	return LoadCached(Data().MoveSets, path, ParseMoveSet);
}

std::shared_ptr<const WeaponData> CombatData::LoadWeapon(const std::string& path)
{
	return LoadCached(Data().Weapons, path, ParseWeapon);
}

std::shared_ptr<const StatusEffectTable> CombatData::LoadStatusEffects()
{
	Storage& storage = Data();
	std::lock_guard lock(storage.Mutex);

	if (!storage.StatusEffectsTried)
	{
		WarnIfOutsideLoadScope(StatusEffectsPath);
		storage.StatusEffects = ParseStatusEffects(StatusEffectsPath);
		storage.StatusEffectsTried = true;
	}
	return storage.StatusEffects;
}

std::shared_ptr<const PlayerConfigData> CombatData::LoadPlayerConfig()
{
	Storage& storage = Data();
	std::lock_guard lock(storage.Mutex);

	if (!storage.PlayerConfigTried)
	{
		WarnIfOutsideLoadScope(PlayerConfigPath);
		storage.PlayerConfig = ParsePlayerConfig(PlayerConfigPath);
		storage.PlayerConfigTried = true;
	}
	return storage.PlayerConfig;
}

std::shared_ptr<const PlayerConfigData> CombatData::FindPlayerConfig()
{
	Storage& storage = Data();
	std::lock_guard lock(storage.Mutex);
	return storage.PlayerConfig;
}

std::shared_ptr<const CharacterData> CombatData::FindCharacter(const std::string& path)
{
	Storage& storage = Data();
	std::lock_guard lock(storage.Mutex);
	auto it = storage.Characters.find(path);
	return it == storage.Characters.end() ? nullptr : it->second;
}

std::shared_ptr<const StatusEffectTable> CombatData::FindStatusEffects()
{
	Storage& storage = Data();
	std::lock_guard lock(storage.Mutex);
	return storage.StatusEffects;
}

uint32_t CombatData::GetGeneration()
{
	return Data().Generation.load(std::memory_order_acquire);
}

#ifndef DISTRIBUTION

std::string CombatData::ReloadAll()
{
	Storage& storage = Data();
	std::lock_guard lock(storage.Mutex);
	LoadScope scope;

	int reloaded = 0;
	int failed = 0;

	auto reloadCache = [&](auto& cache, auto parse)
	{
		for (auto& [path, data] : cache)
		{
			auto fresh = parse(path);
			if (fresh) { data = fresh; ++reloaded; }
			else ++failed;
		}
	};

	// Dependencies first: characters resolve against the freshly reloaded movesets / weapons.
	reloadCache(storage.Weapons, ParseWeapon);
	reloadCache(storage.MoveSets, ParseMoveSet);
	reloadCache(storage.Characters, ParseCharacter);

	if (storage.StatusEffectsTried)
	{
		if (auto fresh = ParseStatusEffects(StatusEffectsPath)) { storage.StatusEffects = fresh; ++reloaded; }
		else ++failed;
	}

	if (storage.PlayerConfigTried)
	{
		if (auto fresh = ParsePlayerConfig(PlayerConfigPath)) { storage.PlayerConfig = fresh; ++reloaded; }
		else ++failed;
	}

	storage.Generation.fetch_add(1, std::memory_order_acq_rel);

	const std::string summary = "combat data reloaded: " + std::to_string(reloaded) + " ok, " + std::to_string(failed) + " failed (kept previous)";
	Logger::Log(summary);
	return summary;
}

bool CombatData::SaveMoveSet(const MoveSetData& base, const MoveSetData& edited, std::string& error)
{
	if (edited.sourcePath.empty())
	{
		error = "moveset has no source path";
		return false;
	}

	const std::string text = FileSystemEngine::ReadFile(edited.sourcePath);
	if (text.empty())
	{
		error = "could not read '" + edited.sourcePath + "'";
		return false;
	}

	std::string patched;
	if (!PatchMoveSetText(text, base, edited, patched, error))
		return false;
	if (patched == text)
		return true;   // nothing edited

	if (!FileSystemEngine::WriteFile(edited.sourcePath, patched))
	{
		error = "could not write '" + edited.sourcePath + "'";
		return false;
	}
	return true;
}

bool CombatData::ReplaceMoveSet(const MoveSetData& edited, std::string& error)
{
	// Re-run the whole pipeline on the serialized form so the live data is exactly what a reload would produce.
	DataLoadReport report;
	report.Source = edited.sourcePath;

	auto fresh = std::make_shared<MoveSetData>();
	const nlohmann::json json = edited;
	bool ok = ConvertDataJson(json, *fresh, "", report);
	if (ok)
	{
		fresh->sourcePath = edited.sourcePath;
		ok = ResolveMoveSet(*fresh, report);
	}

	if (!ok)
	{
		report.Log();
		error = report.Errors.empty() ? "invalid moveset" : report.Errors.front();
		return false;
	}

	Storage& storage = Data();
	std::lock_guard lock(storage.Mutex);
	storage.MoveSets[edited.sourcePath] = fresh;

	// Characters hold their moveset pointer: rebuild those that use this file.
	LoadScope scope;
	for (auto& [path, character] : storage.Characters)
	{
		if (character && character->moveset == edited.sourcePath)
			if (auto rebuilt = ParseCharacter(path))
				character = rebuilt;
	}

	storage.Generation.fetch_add(1, std::memory_order_acq_rel);
	return true;
}

std::vector<std::string> CombatData::GetWarnings(const std::string& path)
{
	Storage& storage = Data();
	std::lock_guard lock(storage.Mutex);
	auto it = storage.Warnings.find(path);
	return it == storage.Warnings.end() ? std::vector<std::string>() : it->second;
}

#endif

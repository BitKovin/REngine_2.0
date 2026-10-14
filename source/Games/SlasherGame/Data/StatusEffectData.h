#pragma once

#include <Data/DataLoadReport.h>
#include <Data/JsonEnum.h>

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

// Data contract of GameData/combat/effects.json.

enum class EffectStacking : uint8_t { Invalid, Refresh, Stack, Ignore };
SLASHER_JSON_ENUM(EffectStacking, { { EffectStacking::Invalid, nullptr }, { EffectStacking::Refresh, "Refresh" },
	{ EffectStacking::Stack, "Stack" }, { EffectStacking::Ignore, "Ignore" } })

// Multipliers; combined by multiplication across active effects (and stacks).
struct StatModifiers
{
	float damageDealt = 1.0f;
	float damageTaken = 1.0f;
	float poiseDamageDealt = 1.0f;
	float poiseDamageTaken = 1.0f;
	float moveSpeed = 1.0f;
	float attackSpeed = 1.0f;

	void Combine(const StatModifiers& other)
	{
		damageDealt *= other.damageDealt;
		damageTaken *= other.damageTaken;
		poiseDamageDealt *= other.poiseDamageDealt;
		poiseDamageTaken *= other.poiseDamageTaken;
		moveSpeed *= other.moveSpeed;
		attackSpeed *= other.attackSpeed;
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(StatModifiers, damageDealt, damageTaken, poiseDamageDealt, poiseDamageTaken, moveSpeed, attackSpeed)
};

struct StatusEffectData
{
	float duration = 5.0f;           // <= 0: until removed
	EffectStacking stacking = EffectStacking::Refresh;
	int maxStacks = 1;
	StatModifiers modifiers;
	std::string hudLabel;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(StatusEffectData, duration, stacking, maxStacks, modifiers, hudLabel)

	// Resolved
	std::string id;
	int index = -1;
};

struct StatusEffectTable
{
	int version = 1;
	std::map<std::string, StatusEffectData> effects;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(StatusEffectTable, version, effects)

	// Resolved: effects in index order (pointers into `effects`, stable for the table's lifetime).
	std::vector<const StatusEffectData*> byIndex;
	std::string sourcePath;

	int Find(const std::string& id) const
	{
		auto it = effects.find(id);
		return it == effects.end() ? -1 : it->second.index;
	}
	const StatusEffectData* Get(int index) const
	{
		return (index >= 0 && index < static_cast<int>(byIndex.size())) ? byIndex[static_cast<size_t>(index)] : nullptr;
	}
};

bool ResolveStatusEffects(StatusEffectTable& table, DataLoadReport& report);

#include "StatusEffectData.h"

bool ResolveStatusEffects(StatusEffectTable& table, DataLoadReport& report)
{
	if (table.version != 1)
		report.Warning("version", "expected 1, got " + std::to_string(table.version));

	table.byIndex.clear();
	int index = 0;
	for (auto& [id, effect] : table.effects)
	{
		const std::string path = "effects." + id;
		effect.id = id;
		effect.index = index++;
		table.byIndex.push_back(&effect);

		if (effect.stacking == EffectStacking::Invalid) report.Error(path + ".stacking", "expected Refresh, Stack or Ignore");
		if (effect.maxStacks < 1) report.Error(path + ".maxStacks", "must be >= 1");

		const StatModifiers& m = effect.modifiers;
		const float values[] = { m.damageDealt, m.damageTaken, m.poiseDamageDealt, m.poiseDamageTaken, m.moveSpeed, m.attackSpeed };
		for (float value : values)
			if (value < 0.0f) report.Error(path + ".modifiers", "multipliers must be >= 0");
	}

	return report.Ok();
}

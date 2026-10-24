#include "CharacterModels.h"

namespace
{
	const std::vector<CharacterModel>& Table()
	{
		static const std::vector<CharacterModel> table = {
			{ "citizen", "GameData/models/npc/base.glb", "GameData/models/npc/base.glb", true, true, false, { "neck_01", "upperarm_l", "upperarm_r" } },
			{ "guard", "GameData/models/npc/guard.glb", "GameData/models/npc/guard.glb", false, false, false, { "neck_01", "upperarm_l", "upperarm_r" } },
			{ "player coat", "GameData/models/player/player_coat.glb", "GameData/models/npc/base.glb", true, false, true, { "neck_01", "upperarm_l", "upperarm_r" } },
		};
		return table;
	}
}

int CharacterModels::Count()
{
	return static_cast<int>(Table().size());
}

const CharacterModel& CharacterModels::Get(int id)
{
	const auto& table = Table();
	return table[id >= 0 && id < static_cast<int>(table.size()) ? id : 0];
}

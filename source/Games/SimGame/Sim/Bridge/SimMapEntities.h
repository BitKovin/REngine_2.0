#pragma once

// Map entities the sim reads from levels (GameData/entities.ent). Engine side: only the bridge and the bake use them.

#include <Entities/Brushes/TriggerBase.hpp>
#include <Entity.h>

#include <string>

// sim_portal: a level transition trigger. The world nav bake links its brush centre to `targetSpawn` in `targetLevel`.
// The player-side transition (addendum 5.2) is a later milestone; for now it is a sensor trigger without action.
class SimPortal : public TriggerBase
{
public:
	std::string targetLevel;
	std::string targetSpawn;
	glm::vec3 centre{ 0.0f }; // brush centre in world space, captured before the trigger drops its drawables

	void FromData(EntityData data) override;
	void Start() override;
};

// sim_spawn: a named arrival point for level transitions; spawnflags bit 0 marks the default player start.
class SimSpawn : public Entity
{
public:
	bool playerStart = false;

	void FromData(EntityData data) override;
};

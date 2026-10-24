#pragma once

// Map entities the sim reads from levels (GameData/entities.ent). Engine side: only the bridge and the bake use them.

#include <Entities/Brushes/TriggerBase.hpp>
#include <Entity.h>

#include <string>

// sim_portal: a level transition trigger. The world nav bake links its brush centre to `targetSpawn` in `targetLevel`, and the pawn
// walking into the volume asks SimTransition for the same transition (addendum 5.2, SLICE_M2E_TRANSITION.md).
class SimPortal : public TriggerBase
{
public:
	std::string targetLevel;
	std::string targetSpawn;
	glm::vec3 centre{ 0.0f }; // brush centre in world space, captured before the trigger drops its drawables

	void FromData(EntityData data) override;
	void Start() override;
	void OnBodyEntered(Body* body, Entity* entity) override;
};

// sim_spawn: a named arrival point for level transitions; spawnflags bit 0 marks the default player start. It creates the pawn: the arrival
// spawn of a level transition when one is under way, otherwise the default start.
class SimSpawn : public Entity
{
public:
	bool playerStart = false;

	void FromData(EntityData data) override;
};

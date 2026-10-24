#pragma once

// Map entities the sim reads from levels (GameData/entities.ent). Engine side: only the bridge and the bake use them.

#include <Entities/Brushes/AreaBase.hpp>
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

// sim_site and sim_settlement: footprints the sim reads (the baker slice's loader); in the level they are invisible sensor volumes without collision, the brush is only data.
class SimSite : public AreaBase
{
};

class SimSettlement : public AreaBase
{
};

// sim_spawn: a named arrival point for level transitions; spawnflags bit 0 marks the default player start. It creates the pawn: the arrival
// spawn of a level transition when one is under way, otherwise the default start.
class SimSpawn : public Entity
{
public:
	bool playerStart = false;
	// Development: the name of the sim_spawn the pawn starts at instead of the default player start (launch -sim_start <name>), for pictures from other places.
	static inline std::string StartOverride;

	void FromData(EntityData data) override;
};

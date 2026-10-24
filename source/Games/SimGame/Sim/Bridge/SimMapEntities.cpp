// Time.hpp first: engine headers below use Time without including it.
#include <Time.hpp>

#include "SimMapEntities.h"

#include "SimBridge.h"
#include "SimTransition.h"

#include <SimPawn.h>

#include <BSP/Quake3Bsp.h>
#include <RegisterLevelObject.h>

void SimPortal::FromData(EntityData data)
{
	TriggerBase::FromData(data);
	targetLevel = data.GetPropertyString("targetLevel");
	targetSpawn = data.GetPropertyString("targetSpawn");
}

void SimPortal::Start()
{
	BoundingBox bounds;
	bool found = false;
	for (IDrawMesh* drawable : Drawables)
		if (auto* model = dynamic_cast<BSPModelRef*>(drawable))
		{
			const BoundingBox modelBounds = model->GetTransformedBounds();
			bounds.Min = found ? glm::min(bounds.Min, modelBounds.Min) : modelBounds.Min;
			bounds.Max = found ? glm::max(bounds.Max, modelBounds.Max) : modelBounds.Max;
			found = true;
		}
	centre = found ? bounds.Center() : Position;
	TriggerBase::Start();
}

void SimPortal::OnBodyEntered(Body* body, Entity* entity)
{
	// Only the pawn, and not in its first seconds: a landing next to the return portal must not bounce it back.
	constexpr double ArrivalGrace = 2.0;
	if (!SimBridge::IsRunning() || !entity->HasTag("player") || Time::GameTime - entity->SpawnTime <= ArrivalGrace)
		return;
	SimTransition::Request(Name, targetLevel, targetSpawn);
}

void SimSpawn::FromData(EntityData data)
{
	Entity::FromData(data);
	playerStart = (static_cast<int>(data.GetPropertyFloat("spawnflags")) & 1) != 0;
	const float yaw = data.GetPropertyFloat("angle") + 90.0f;
	float pitch = 0.0f;
	if (SimTransition::Arriving())
	{
		if (SimTransition::TakeArrival(Name, pitch))
			SimPawn::SpawnAt(Position, yaw, pitch); // the pawn arrives where the portal sent it
	}
	else if (StartOverride.empty() ? playerStart : Name == StartOverride)
		SimPawn::SpawnAt(Position, yaw); // the pawn the sim follows (SLICE_M2B_BRIDGE 3.4)
}

REGISTER_ENTITY(SimPortal, "sim_portal")
REGISTER_ENTITY(SimSpawn, "sim_spawn")
REGISTER_ENTITY(SimSite, "sim_site")
REGISTER_ENTITY(SimSettlement, "sim_settlement")

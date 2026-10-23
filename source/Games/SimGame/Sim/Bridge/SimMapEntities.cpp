#include "SimMapEntities.h"

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

void SimSpawn::FromData(EntityData data)
{
	Entity::FromData(data);
	playerStart = (static_cast<int>(data.GetPropertyFloat("spawnflags")) & 1) != 0;
}

REGISTER_ENTITY(SimPortal, "sim_portal")
REGISTER_ENTITY(SimSpawn, "sim_spawn")

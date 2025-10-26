#include "nav_point.h"

void nav_point::FromData(EntityData data)
{

	Entity::FromData(data);

	NextPoint = data.GetPropertyString("target");
	WaitTimeAfterReach = data.GetPropertyFloat("waitTime");
	Rotation.y = data.GetPropertyFloat("angle") + 90;
	acceptanceRadius = data.GetPropertyFloat("acceptanceRadius", acceptanceRadius);
}

REGISTER_ENTITY(nav_point, "nav_point")
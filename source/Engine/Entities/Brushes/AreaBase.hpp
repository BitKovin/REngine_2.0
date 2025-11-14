#pragma once

#include "../../Entity.h"
#include "../../Physics.h"

class AreaBase : public Entity
{
public:


	AreaBase()
	{
		ClassName = "area";
		SaveGame = true;
		ConvexCollision = true;

		DefaultBrushGroup = BodyType::Area1;
		DefaultBrushCollisionMask = BodyType::None;

	}

	void Start()
	{

		Entity::Start();

		LeadBody->SetIsSensor(true);
		DestroyDrawables();
	}


	void OnBodyEntered(Body* body, Entity* entity){}
	void OnBodyExited(Body* body, Entity* entity){}

private:

};
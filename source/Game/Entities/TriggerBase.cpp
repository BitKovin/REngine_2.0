#include "TriggerBase.hpp"

void TriggerBase::Start()
{
	LeadBody->SetIsSensor(true);
}

void TriggerBase::OnBodyEntered(Body* body, Entity* entity)
{

	if (entity->HasTag("player"))
	{
		printf("player entered \n");
	}

}

void TriggerBase::OnBodyExited(Body* body, Entity* entity)
{

	if (entity->HasTag("player"))
	{
		printf("player exited \n");
	}

}

REGISTER_ENTITY(TriggerBase, "trigger")

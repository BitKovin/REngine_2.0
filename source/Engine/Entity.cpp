#include "Entity.h"
#include "LevelObjectFactory.h"

Entity* Entity::Spawn(std::string technicalName)
{
	auto entity = LevelObjectFactory::instance().create(technicalName);

	if (entity)
	{
		Level::Current->AddEntity(entity);
	}

	return entity;
}

void Entity::FinalLevelRemove()
{
	DestroyDrawables();
	Logger::Log("FinalLevelRemove: " + Id);
}

void Entity::Destroy()
{

	if (Destroyed)
	{
		return;
	}

	Destroyed = true;

	if (OwnerId != "")
	{
		Entity* ownerRef = Level::Current->FindEntityWithId(OwnerId);
		if (ownerRef)
		{
			ownerRef->OnAction("despawned");
		}
	}

	Physics::DestroyBody(LeadBody);
	LeadBody = nullptr;
	for (Body* body : Bodies)
	{
		Physics::DestroyBody(body);
	}
	Bodies.clear();

	Level::Current->RemoveEntity(this);
}
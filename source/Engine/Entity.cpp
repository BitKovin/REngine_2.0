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
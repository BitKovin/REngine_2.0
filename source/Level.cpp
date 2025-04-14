#include "Level.hpp"

#include "MapData.h"
#include "MapParser.h"

#include "Physics.h"



Level* Level::Current = nullptr;

void Level::CloseLevel()
{
	for (LevelObject* obj : Current->LevelObjects)
	{
		obj->Dispose();
		delete(obj);
	}
	Current->LevelObjects.clear();

	Current->RemovePendingEntities();

	Physics::DestroyAllBodies();

	Current->RemovePendingEntities();
}

Level* Level::OpenLevel(string filePath)
{
	if (Current)
	{

		CloseLevel();

		Current->Dispose();
		delete(Current);
	}

	Level* newLevel = new Level();

	Current = newLevel;

	MapData mapData = MapParser::ParseMap(filePath);

	mapData.LoadToLevel();

	for (LevelObject* obj : Current->LevelObjects)
	{
		obj->Start();
	}

	printf("generating nav mesh");

	NavigationSystem::GenerateNavData();

	printf("generated nav mesh");

	return newLevel;
}

void Level::AsyncUpdate()
{

	RemovePendingEntities();

	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
	for (auto var : LevelObjects)
	{
		asyncUpdateThreadPool->QueueJob([var]() {var->AsyncUpdate(); });
	}

	asyncUpdateThreadPool->WaitForFinish();

	RemovePendingEntities();
}

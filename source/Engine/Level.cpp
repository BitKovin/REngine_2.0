#include "Level.hpp"

#include "MapData.h"
#include "MapParser.h"

#include "Physics.h"

#include "EngineMain.h"

Level* Level::Current = nullptr;

void Level::CloseLevel()
{

	EngineMain::MainInstance->MainThreadPool.Stop();

	for (LevelObject* obj : Current->LevelObjects)
	{
		obj->Dispose();
	}
	

	Current->RemovePendingEntities();
	Current->MemoryCleanPendingEntities();

	Physics::DestroyAllBodies();

	Current->RemovePendingEntities();

	Current->MemoryCleanPendingEntities();
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

	EngineMain::MainInstance->MainThreadPool.Start();

	MapData mapData = MapParser::ParseMap(filePath);

	mapData.LoadToLevel();

	for (LevelObject* obj : Current->LevelObjects)
	{
		obj->Start();
	}

	printf("generating nav mesh\n");

	NavigationSystem::GenerateNavData();

	printf("generated nav mesh\n");


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

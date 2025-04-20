#include "Level.hpp"

#include "MapData.h"
#include "MapParser.h"

#include "Entity.hpp"

#include "Physics.h"

#include "EngineMain.h"

#include "LightSystem/LightManager.h"

#include "SaveSystem/LevelSaveSystem.h"

Level* Level::Current = nullptr;

string Level::pendingLoadLevelPath = "";

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

	NavigationSystem::DestroyAllObstacles();

}

///UNSAFE. Loads level as soon as gets called
Level* Level::OpenLevel(string filePath)
{

	bool isNewLevel = true;

	if (Current)
	{

		if (Current->filePath == filePath)
			isNewLevel = false;

		CloseLevel();

		Current->Dispose();
		delete(Current);
	}

	Level* newLevel = new Level();

	newLevel->filePath = filePath;

	Current = newLevel;

	EngineMain::MainInstance->MainThreadPool.Start();

	MapData mapData = MapParser::ParseMap(filePath);

	mapData.LoadToLevel();

	for (LevelObject* obj : Current->LevelObjects)
	{
		obj->Start();
	}

	if (isNewLevel) 
	{
		printf("generating nav mesh\n");

		NavigationSystem::GenerateNavData();

		printf("generated nav mesh\n");
	}

	if (LevelSaveSystem::pendingSave.name == Current->filePath)
	{
		LevelSaveSystem::LoadLevelFromData(LevelSaveSystem::pendingSave);

		LevelSaveSystem::pendingSave = LevelSaveData();

	}

	return newLevel;
}

void Level::AddEntity(LevelObject* obj)
{

	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

	Entity* entity = (Entity*)obj;

	if (entity)
	{
		string classname = entity->ClassName;

		auto nId = nextId.find(classname);

		if (nId == nextId.end())
		{
			nextId[classname] = 0;
		}

		int id = nextId[classname];

		nextId[classname]++;

		string entId = classname + "_" + to_string(id);

		entity->Id = entId;

	}

	LevelObjects.push_back(obj);

	
}

void Level::RemoveEntity(LevelObject* obj)
{
	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

	Entity* entity = (Entity*)obj;

	if (entity)
	{

		if (entity->Unique && entity->Name != "")
		{
			deletedNames.push_back(entity->Name);
		}
		else
		{
			deletedIDs.push_back(entity->Id);
		}

	}

	PendingRemoveLevelObjects.push_back(obj);
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

vector<Entity*> Level::FindAllEntitiesWithName(const std::string& name)
{
	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

	vector<Entity*> result;

	for (auto var : LevelObjects)
	{
		Entity* entity = (Entity*)var;

		if (entity && entity->Name == name)
		{
			result.push_back(entity);
		}

	}

	return result;
}

Entity* Level::FindEntityWithName(const std::string& name)
{
	auto result = FindAllEntitiesWithName(name);

	if (result.size() == 1)
	{
		return result[0];
	}

	return nullptr;

}

Entity* Level::FindEntityWithId(const std::string& id)
{
	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

	for (auto var : LevelObjects)
	{
		Entity* entity = (Entity*)var;

		if (entity && entity->Id == id)
		{
			return entity;
		}

	}
	return nullptr;
}

void Level::FinalizeFrame()
{

	VissibleRenderList.clear();

	vector<IDrawMesh*> opaque;
	vector<IDrawMesh*> transparent;

	vector<IDrawMesh*> allShadowCasters;

	{

		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

		for (auto var : LevelObjects)
		{

			for (IDrawMesh* mesh : var->GetDrawMeshes())
			{

				mesh->FinalizeFrameData();

				if (mesh->IsCameraVisible())
				{
					if (mesh->Transparent)
					{
						transparent.push_back(mesh);
					}
					else
					{
						opaque.push_back(mesh);
					}

					mesh->LastRenderedTime = Time::GameTime;
					mesh->WasRended = true;
				}
				else
				{
					mesh->WasRended = false;
				}

				allShadowCasters.push_back(mesh);

			}

		}
	}



	// Sort opaque objects from closest to farthest (ascending order by distance).
	std::sort(opaque.begin(), opaque.end(),
		[](IDrawMesh* a, IDrawMesh* b) {
			return a->GetDistanceToCamera() < b->GetDistanceToCamera();
		});

	// Sort transparent objects from farthest to closest (descending order by distance).
	std::sort(transparent.begin(), transparent.end(),
		[](IDrawMesh* a, IDrawMesh* b) {
			return a->GetDistanceToCamera() > b->GetDistanceToCamera();
		});

	// Append sorted opaque objects first.
	for (auto mesh : opaque)
	{
		VissibleRenderList.push_back(mesh);
	}

	// Append sorted transparent objects second.
	for (auto mesh : transparent)
	{
		VissibleRenderList.push_back(mesh);
	}

	ShadowRenderList = allShadowCasters;

	LightManager::Update();

}

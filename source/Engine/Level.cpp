#include "Level.hpp"

#include "MapData.h"
#include "MapParser.h"

#include "Entity.h"

#include "Physics.h"

#include "EngineMain.h"

#include "LightSystem/LightManager.h"

#include "SaveSystem/LevelSaveSystem.h"

Level* Level::Current = nullptr;

string Level::pendingLoadLevelPath = "";

void Level::CloseLevel()
{

	EngineMain::MainInstance->MainThreadPool->Stop();
	delete(EngineMain::MainInstance->MainThreadPool);

	Current->AddPendingLevelObjects();

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

inline bool endsWith(const std::string& str, const std::string& suffix) {
	if (suffix.size() > str.size()) return false;
	return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
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

	Time::Update();
	Time::DeltaTime = 0;
	Time::DeltaTimeF = 0;

	Level* newLevel = new Level();

	newLevel->filePath = filePath;

	Current = newLevel;

	

	EngineMain::MainInstance->MainThreadPool = new ThreadPool();

	EngineMain::MainInstance->MainThreadPool->Start();

	if (endsWith(filePath, ".bsp"))
	{
		Current->BspData.LoadBSP(filePath.c_str());
		Current->BspData.BuildVBO();
		Current->BspData.GenerateTexture();
		Current->BspData.GenerateLightmap();
		Current->BspData.LoadToLevel();
		Current->BspData.PreloadFaces();
	}
	else
	{
		MapData mapData = MapParser::ParseMap(filePath);

		mapData.LoadToLevel();
	}

	

	Current->AddPendingLevelObjects();
	Current->RemovePendingEntities();
	Current->MemoryCleanPendingEntities();

	Current->LoadAssets();

	for (LevelObject* obj : Current->LevelObjects)
	{
		obj->Start();
	}

	Current->AddPendingLevelObjects();
	Current->RemovePendingEntities();
	Current->MemoryCleanPendingEntities();

	if (isNewLevel) 
	{
		printf("generating nav mesh\n");

		NavigationSystem::GenerateNavData();

		printf("generated nav mesh\n");
	}

	Time::Update();
	Time::DeltaTime = 0;
	Time::DeltaTimeF = 0;
	Time::GameTime = 0;
	Time::GameTimeNoPause = 0;

	if (LevelSaveSystem::pendingSave.name == Current->filePath)
	{
		LevelSaveSystem::LoadLevelFromData(LevelSaveSystem::pendingSave);

		LevelSaveSystem::pendingSave = LevelSaveData();

	}


	Current->LoadAssets();

	Current->AddPendingLevelObjects();
	Current->RemovePendingEntities();
	Current->MemoryCleanPendingEntities();

	Time::Update();
	Time::Update();

	return newLevel;
}

void Level::AddEntity(LevelObject* obj)
{

	std::lock_guard<std::recursive_mutex> lock(pendingEntityArrayLock);

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

	pendingAddLevelObjects.push_back(obj);

	
}

void Level::RemoveEntity(LevelObject* obj)
{
	std::lock_guard<std::recursive_mutex> lock(pendingEntityArrayLock);

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

void Level::AsyncUpdate(bool paused)
{
	AddPendingLevelObjects();
	RemovePendingEntities();

	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
	for (auto var : LevelObjects)
	{

		if (var->UpdateWhenPaused || paused == false)
		{
			asyncUpdateThreadPool->QueueJob([var]() {var->AsyncUpdate(); });
		}
	}

	asyncUpdateThreadPool->WaitForFinish();
	AddPendingLevelObjects();
	RemovePendingEntities();
}

void Level::RemovePendingEntities()
{

	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
	std::lock_guard<std::recursive_mutex> lockP(pendingEntityArrayLock);

	for (auto& entity : PendingRemoveLevelObjects)
	{

		if (entity == nullptr) continue;

		auto it = std::find(LevelObjects.begin(), LevelObjects.end(), entity);
		if (it != LevelObjects.end())
		{
			LevelObjects.erase(it);
		}

		PendingMemoryCleanObjects.push_back(entity);

		entity = nullptr;

	}

	PendingRemoveLevelObjects.clear();

}

void Level::MemoryCleanPendingEntities()
{

	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
	std::lock_guard<std::recursive_mutex> lockP(pendingEntityArrayLock);

	for (auto& entity : PendingMemoryCleanObjects)
	{

		if (entity == nullptr) continue;

		entity->FinalLevelRemove();
		delete(entity);

		entity = nullptr;
	}

	PendingMemoryCleanObjects.clear();

}

void Level::Update(bool paused)
{
	AddPendingLevelObjects();
	RemovePendingEntities();

	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
	for (auto var : LevelObjects)
	{
		if (var->UpdateWhenPaused || paused == false)
		{
			var->Update();
		}

	}

	AddPendingLevelObjects();
	RemovePendingEntities();
}

void Level::LateUpdate(bool paused)
{
	AddPendingLevelObjects();
	RemovePendingEntities();

	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
	for (auto var : LevelObjects)
	{
		if (var->LateUpdateWhenPaused || paused == false)
		{
			var->LateUpdate();
		}
	}
	AddPendingLevelObjects();
	RemovePendingEntities();
}

vector<Entity*> Level::FindAllEntitiesWithName(const std::string& name)
{
	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

	vector<Entity*> result;

	auto curLevelObjects = LevelObjects;

	for (auto var : curLevelObjects)
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
	//std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

	auto curLevelObjects = LevelObjects;

	for (auto var : curLevelObjects)
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
	AddPendingLevelObjects();

	VissibleRenderList.clear();

	vector<IDrawMesh*> opaque;
	vector<IDrawMesh*> transparent;

	vector<IDrawMesh*> ShadowCasters;
	vector<IDrawMesh*> DetailShadowCasters;

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

				if (mesh->IsShadowVisible())
				{

					if (mesh->IsDetailShadow())
					{
						DetailShadowCasters.push_back(mesh);
					}
					else
					{
						ShadowCasters.push_back(mesh);
					}

					
				}
				

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

	ShadowRenderList = ShadowCasters;
	DetailShadowRenderList = DetailShadowCasters;

	LightManager::Update();

}

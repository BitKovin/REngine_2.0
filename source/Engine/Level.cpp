#include "Level.hpp"

#include "MapData.h"
#include "MapParser.h"

#include "Physics.h"

#include "EngineMain.h"

#include "LightSystem/LightManager.h"

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

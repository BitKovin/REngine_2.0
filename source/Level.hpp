#pragma once
#include <vector>

#include "LevelObject.hpp"
#include "EObject.hpp"

#include "IDrawMesh.h"

#include "mutex"

#include "Navigation/Navigation.hpp"

#include "MeshUtils.hpp"

#include "ThreadPool.h"

using namespace std;

class Level : EObject
{

private:
	vector<LevelObject*> LevelObjects = vector<LevelObject*>();
	vector<LevelObject*> PendingRemoveLevelObjects = vector<LevelObject*>();
	vector<LevelObject*> PendingMemoryCleanObjects = vector<LevelObject*>();

	std::recursive_mutex entityArrayLock = std::recursive_mutex();

	ThreadPool* asyncUpdateThreadPool;

public:

	static Level* Current;

	vector<IDrawMesh*> VissibleRenderList = vector<IDrawMesh*>();



	Level()
	{
		asyncUpdateThreadPool = new ThreadPool();
		asyncUpdateThreadPool->Start();
	}

	~Level()
	{
		asyncUpdateThreadPool->Stop();
		delete(asyncUpdateThreadPool);
	}

	static void CloseLevel();

	static Level* OpenLevel(string filePath);

	MeshUtils::PositionVerticesIndices GetStaticNavObstaclesMesh()
	{
		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

		vector<MeshUtils::PositionVerticesIndices> meshes;

		for (auto obj : LevelObjects)
		{
			if(obj->Static)
			for (auto mesh : obj->GetDrawMeshes()) 
			{

				if (mesh->StaticNavigation == false) continue;

				auto obstacles = mesh->GetNavObstacleMeshes();

				meshes.insert(meshes.end(), obstacles.begin(), obstacles.end());

			}
		}



		MeshUtils::PositionVerticesIndices resultMesh = MeshUtils::MergeMeshes(meshes);

		return resultMesh;

	}
	
	void AddEntity(LevelObject* entity)
	{
		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

		LevelObjects.push_back(entity);

	}

	void RemoveEntity(LevelObject* entity)
	{
		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

		PendingRemoveLevelObjects.push_back(entity);

		

	}

	void RemovePendingEntities()
	{

		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

		for (auto entity : PendingRemoveLevelObjects)
		{
			auto it = std::find(LevelObjects.begin(), LevelObjects.end(), entity);
			if (it != LevelObjects.end())
			{
				LevelObjects.erase(it);
			}

			PendingMemoryCleanObjects.push_back(entity);

		}

		PendingRemoveLevelObjects.clear();
		
	}

	void MemoryCleanPendingEntities()
	{

		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

		for (auto entity : PendingMemoryCleanObjects)
		{

			entity->FinalLevelRemove();

		}

		PendingMemoryCleanObjects.clear();

	}

	void UpdatePhysics()
	{
		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

		for (auto var : LevelObjects)
		{
			var->UpdatePhysics();
		}

	}

	void Update()
	{

		RemovePendingEntities();

		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
		for (auto var : LevelObjects)
		{
			var->Update();
		}

		RemovePendingEntities();

	}

	void AsyncUpdate();

	void DevUiUpdate()
	{
		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
		for (auto var : LevelObjects)
		{
			var->UpdateDebugUI();
		}

		RemovePendingEntities();

	}

	void FinalizeFrame()
	{

		VissibleRenderList.clear();

		vector<IDrawMesh*> opaque;
		vector<IDrawMesh*> transparent;

		{

			std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

			for (auto var : LevelObjects)
			{

				for (IDrawMesh* mesh : var->GetDrawMeshes())
				{
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

						mesh->FinalizeFrameData();

						mesh->LastRenderedTime = Time::GameTime;
						mesh->WasRended = true;
					}
					else
					{
						mesh->WasRended = false;
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

	}

protected:

	void OnDispose()
	{
		for (auto var : LevelObjects)
		{
			var->Dispose();
		}
	}

};
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

class Entity;

class Level : EObject
{

private:
	
	vector<LevelObject*> PendingRemoveLevelObjects = vector<LevelObject*>();
	vector<LevelObject*> PendingMemoryCleanObjects = vector<LevelObject*>();





	std::recursive_mutex entityArrayLock = std::recursive_mutex();

	ThreadPool* asyncUpdateThreadPool;

	static string pendingLoadLevelPath;

	vector<LevelObject*> pendingAddLevelObjects;

public:

	static Level* Current;

	vector<LevelObject*> LevelObjects = vector<LevelObject*>(); //not safe to use outside of class
	std::unordered_map<std::string, int> nextId;
	std::vector<std::string> deletedNames;
	std::vector<std::string> deletedIDs;

	vector<IDrawMesh*> VissibleRenderList = vector<IDrawMesh*>();
	vector<IDrawMesh*> ShadowRenderList = vector<IDrawMesh*>();
	vector<IDrawMesh*> DetailShadowRenderList = vector<IDrawMesh*>();

	string filePath;

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

	static void LoadLevelFromFile(string filePath)
	{
		pendingLoadLevelPath = filePath;
	}

	static bool LoadPendingLevel()
	{
		if (pendingLoadLevelPath != "")
		{
			OpenLevel(pendingLoadLevelPath);
			pendingLoadLevelPath = "";

			return true;
		}

		
		return false;
	}

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

	void LoadAssets()
	{
		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

		AddPendingLevelObjects();

		for (auto obj : LevelObjects)
		{
			obj->LoadAssetsIfNeeded();
		}
	}
	
	void AddEntity(LevelObject* obj);

	void RemoveEntity(LevelObject* obj);

	void AddPendingLevelObjects()
	{
		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
		for (auto entity : pendingAddLevelObjects)
		{
			LevelObjects.push_back(entity);
		}
		pendingAddLevelObjects.clear();
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
			delete(entity);

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
		AddPendingLevelObjects();
	}

	void Update()
	{
		AddPendingLevelObjects();
		RemovePendingEntities();

		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
		for (auto var : LevelObjects)
		{
			var->Update();
		}

		AddPendingLevelObjects();
		RemovePendingEntities();

	}

	void LateUpdate()
	{
		AddPendingLevelObjects();
		RemovePendingEntities();

		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
		for (auto var : LevelObjects)
		{
			var->LateUpdate();
		}
		AddPendingLevelObjects();
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

	vector<Entity*> FindAllEntitiesWithName(const std::string& name);
	Entity* FindEntityWithName(const std::string& name);
	Entity* FindEntityWithId(const std::string& id);

	void FinalizeFrame();

protected:

	void OnDispose()
	{
		for (auto var : LevelObjects)
		{
			var->Dispose();
		}
	}

};
#include <Entity.hpp>

class Spawner : public Entity
{
public:

	string spawnClassName = "testnpc";
	string onSpawnedAction = "spawned";
	string onDespawnedAction = "despawned";
	string actionTarget = "";


	Spawner()
	{

	}

	void FromData(EntityData data)
	{
		Entity::FromData(data);

		Rotation.y = data.GetPropertyFloat("angle") + 90;
		spawnClassName = data.GetPropertyString("className", spawnClassName);
		onSpawnedAction = data.GetPropertyString("onSpawned", onSpawnedAction);
		onSpawnedAction = data.GetPropertyString("onDespawned", onDespawnedAction);
		actionTarget = data.GetPropertyString("target", actionTarget);
	}

	void LoadAssets()
	{
		PreloadEntityType(spawnClassName);
	}

	void PerformSpawn()
	{
		Entity* entity = Spawn(spawnClassName);
		Logger::Log("1");
		if (entity == nullptr)
		{
			Logger::Log("failed to spawn entity of type :" + spawnClassName);
			return;
		}
		Logger::Log("2");
		entity->Position = Position;
		entity->Rotation = Rotation;
		Logger::Log("3");
		entity->Start();
		
		Logger::Log("calling action");

		CallActionOnEveryEntityWithName(actionTarget, onSpawnedAction);

	}

	void OnAction(string action)
	{

		Logger::Log("spawner received action " + action);

		if (action == "despawned")
		{
			CallActionOnEveryEntityWithName(actionTarget, onDespawnedAction);
			return;
		}

		PerformSpawn();

	}


private:

};

REGISTER_ENTITY(Spawner, "spawner")
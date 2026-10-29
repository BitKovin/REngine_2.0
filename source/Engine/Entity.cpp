#include "Entity.h"
#include "LevelObjectFactory.h"

#include <EngineMain.h>
#include <mutex>
#include <unordered_set>
#include <tracy/tracy/Tracy.hpp>

void Entity::FromData(EntityData data)
{

	Name = data.GetPropertyString("targetname");

	Position = data.GetPropertyVectorPosition("origin");

	Unique = data.GetPropertyBool("unique", Unique);

	if (data.HasPropery("angles"))
	{
		vec3 angles = data.GetPropertyVectorRotation("angles");

		Rotation = EntityData::ConvertRotation(angles, true);
	}
	else if (data.HasPropery("angle"))
	{
		Rotation.y = data.GetPropertyFloat("angle") + 90;
	}
	else
	{
		Rotation.y = 90;
	}



}

Entity* Entity::Spawn(std::string technicalName, bool imOwner)
{
	auto entity = LevelObjectFactory::instance().create(technicalName);

	if (entity)
	{
		Level::Current->AddEntity(entity, imOwner);
	}


	return entity;
}

void Entity::PreloadEntityType(std::string technicalName)
{

	if (Level::Current->IsEntityTypeLoaded(technicalName)) return; //already preloaded

	auto entity = LevelObjectFactory::instance().create(technicalName);

	if (entity)
	{
		//entity->Start();
		entity->LoadAssets();
		entity->SaveGame = false;
		entity->DestroyOnPlace();

	}
	Level::Current->AddLoadedEntityType(technicalName);
}

namespace
{
	// Types currently being preloaded by an async job. A batch that names the
	// same type repeatedly (e.g. many spawners of one class) would otherwise
	// queue - and fully construct - one throwaway Entity per request.
	std::mutex g_preloadInFlightLock;
	std::unordered_set<std::string> g_preloadInFlight;
}

void Entity::ClearPreloadInFlight()
{
	std::lock_guard<std::mutex> lk(g_preloadInFlightLock);
	g_preloadInFlight.clear();
}

void Entity::PreloadEntityTypeAsync(std::string technicalName)
{

	if (Level::Current->IsEntityTypeLoaded(technicalName)) return; //already preloaded

	//we don't want to finish preloading when LoadingConstantAssets changes to false
	if (AssetRegistry::LoadingConstantAssets)
	{
		PreloadEntityType(technicalName);
		return;
	}

	{
		std::lock_guard<std::mutex> lk(g_preloadInFlightLock);
		if (!g_preloadInFlight.insert(technicalName).second)
			return; //another job is already preloading this type
	}

	EngineMain::MainInstance->MainThreadPool->QueueJob([technicalName]()
		{
			// always clear the in-flight marker, even if creation fails
			struct InFlightRelease
			{
				const std::string& name;
				~InFlightRelease()
				{
					std::lock_guard<std::mutex> lk(g_preloadInFlightLock);
					g_preloadInFlight.erase(name);
				}
			} release{ technicalName };

			ZoneScoped;
			std::string zoneName = "PreloadEntityTypeAsync Job for" + technicalName;

			ZoneName(zoneName.c_str(), zoneName.size());

			auto entity = LevelObjectFactory::instance().create(technicalName);

			if (entity)
			{
				//entity->Start();
				{
					ZoneScopedN("LoadAssets");
					entity->LoadAssets();
				}

				entity->SaveGame = false;
				entity->DestroyOnPlace();
			}
			Level::Current->AddLoadedEntityType(technicalName);

		});



}

void Entity::DestroyPhysics()
{

	Physics::DestroyBody(LeadBody);
	LeadBody = nullptr;
	for (Body* body : Bodies)
	{
		Physics::DestroyBody(body);
	}
	Bodies.clear();

}

void Entity::OnLevelRemoved()
{

	EcsWorld::DestroyOwned(m_Handle.entity(), this);

}

void Entity::FinalLevelRemove()
{
	DestroyDrawables();
	//Logger::Log("FinalLevelRemove: " + Id);
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

	DestroyPhysics();


	//if (m_Handle.valid())
		//m_Handle.destroy();

	Level::Current->RemoveEntity(this);
}

void Entity::DestroyOnPlace()
{

	if (Level::Current->FindEntityWithId(Id) != nullptr)
	{
		Destroy();
		return;
	}

	Level::Current->AddPendingMemoryRemoveObject(this);

}

void Entity::Serialize(json& target)
{
	SERIALIZE_FIELD(target, Name);
	SERIALIZE_FIELD(target, Position);
	SERIALIZE_FIELD(target, Rotation);
	SERIALIZE_FIELD(target, Scale);
	SERIALIZE_FIELD(target, Health);
	SERIALIZE_FIELD(target, destroyDelay);
	SERIALIZE_FIELD(target, Tags);
	SERIALIZE_FIELD(target, OwnerId);

}

LightVolPointData Entity::GetLightVolData(bool wallCheck)
{

	if (UseBoundsCenterForLightVol)
	{
		auto bounds = GetBoundingBox();
		auto center = bounds.Center();
		auto light = Level::Current->BspData.GetLightvolColorPoint(center * MAP_SCALE, wallCheck);
		return light;
	}

	auto light = Level::Current->BspData.GetLightvolColorPoint(Position * MAP_SCALE, wallCheck);

	return light;
}

void Entity::Teleport(vec3 position)
{

	Position = position;

	if (LeadBody)
		Physics::SetBodyPosition(LeadBody, position);

}

void Entity::Deserialize(json& source)
{
	DESERIALIZE_FIELD(source, Name);
	DESERIALIZE_FIELD(source, Position);
	DESERIALIZE_FIELD(source, Rotation);
	DESERIALIZE_FIELD(source, Scale);
	DESERIALIZE_FIELD(source, Health);
	DESERIALIZE_FIELD(source, destroyDelay);
	DESERIALIZE_FIELD(source, Tags);
	DESERIALIZE_FIELD(source, OwnerId);
}
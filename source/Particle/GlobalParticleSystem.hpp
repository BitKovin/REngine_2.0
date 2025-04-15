#pragma once

#include "ParticleSystem.hpp"

#include "../LevelObjectFactory.h"

#include <unordered_map>

class GlobalParticleSystem: public ParticleSystem
{
public:
	
	string technicalName = "";

	static void SpawnParticleAt(string name,vec3 position, vec3 rotation, vec3 scale)
	{

		GlobalParticleSystem* instance = nullptr;

		auto it = Instances.find(name);
		if (it != Instances.end()) 
		{
			instance = it->second;
		}

		if (instance == nullptr)
		{

			Entity* ent = LevelObjectFactory::instance().create(name);

			instance = (GlobalParticleSystem*)(ent);

			if (ent == nullptr || instance == nullptr)
			{
				Logger::Log("wasn't able to create global system with name" + name);
				return;
			}

			instance->technicalName = name;
			Instances[name] = instance;


			instance->Position = position;
			instance->Rotation = rotation;
			instance->Scale = scale;
			instance->Start();
			Level::Current->AddEntity(instance);

		}

		instance->SpawnParticleAtInst(position, rotation, scale);

	}

		
	

	void Destroy()
	{
		ParticleSystem::Destroy();

		Instances.erase(technicalName);

	}

private:

	void SpawnParticleAtInst(vec3 position, vec3 rotation, vec3 scale)
	{
		Position = position;
		Rotation = rotation;
		Scale = scale;

		for (auto emitter : emitters)
		{

			emitter->Position = position;
			emitter->Rotation = rotation;
			emitter->Scale = scale;

			emitter->AddParticle(emitter->GetNewParticle());
		}
	}

	static std::unordered_map<string, GlobalParticleSystem*> Instances;

};


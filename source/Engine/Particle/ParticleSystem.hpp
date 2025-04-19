#pragma once

#include "ParticleEmitter.h"
#include "../Entity.hpp"

#include "../Time.hpp"

class ParticleSystem : public Entity
{
public:
	
	std::vector<ParticleEmitter*> emitters;

	void Start()
	{
		for (auto emitter : emitters)
		{


			emitter->Position = Position;
			emitter->Rotation = Rotation;


			Drawables.push_back(emitter);
			emitter->Start();
		}
	}

	void AsyncUpdate()
	{
		for (auto emitter : emitters)
		{
			emitter->Position = Position;
			emitter->Rotation = Rotation;
			emitter->Scale = Scale;
			emitter->Update(Time::DeltaTimeF);
		}
	}

	static void PreloadSystemAssets(string name)
	{
		Entity* ent = LevelObjectFactory::instance().create(name);

		auto system = (ParticleSystem*)(ent);
		if (system)
		{
			system->PreloadAssets();
			delete(system);
		}
	}

	void PreloadAssets()
	{
		for (auto emitter : emitters)
		{
			emitter->PreloadAssets();
		}
	}

private:

};

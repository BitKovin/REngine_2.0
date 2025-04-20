#pragma once

#include "ParticleEmitter.h"
#include "../Entity.hpp"

#include "../Time.hpp"

class ParticleSystem : public Entity
{
public:
	
	std::vector<ParticleEmitter*> emitters;

	void Start() override
	{
		for (auto emitter : emitters)
		{


			emitter->Position = Position;
			emitter->Rotation = Rotation;


			Drawables.push_back(emitter);
			emitter->Start();
		}
	}

	void AsyncUpdate() override
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
			system->LoadAssetsIfNeeded();
			delete(system);
		}
	}

protected:

	void LoadAssets() override
	{
		for (auto emitter : emitters)
		{
			emitter->PreloadAssets();
		}
	}


private:

};

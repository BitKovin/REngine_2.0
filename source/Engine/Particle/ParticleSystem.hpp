#pragma once

#include "ParticleEmitter.h"
#include "../Entity.h"

#include "../Time.hpp"

class ParticleSystem : public Entity
{
public:

	std::vector<ParticleEmitter*> emitters;

	Delay StopDelay = Delay(1000000000);

	~ParticleSystem()
	{

	}

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

	void StopAll(float delay = 0)
	{

		if (delay <= 0)
		{
			for (auto emitter : emitters)
			{
				emitter->Emitting = false;
			}
		}
		else
		{
			StopDelay.AddDelay(delay);
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

		if (StopDelay.Wait() == false)
		{
			StopAll();
			StopDelay.AddDelay(1000000);
		}

		for (auto emitter : emitters)
		{
			if (emitter->destroyed == false) return;
		}

		Destroy();

		UpdateDestroyDelay();

	}

	static void PreloadSystemAssets(string name)
	{
		Entity* ent = Spawn(name);

		auto system = (ParticleSystem*)(ent);
		if (system)
		{
			system->LoadAssetsIfNeeded();
			ent->Destroy();
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

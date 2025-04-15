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


private:

};

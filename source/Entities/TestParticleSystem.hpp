#pragma once

#include "../Particle/ParticleSystem.hpp"

class TestParticleEmitter : public ParticleEmitter
{
public:
	
	TestParticleEmitter()
	{
		texture = AssetRegistry::GetTextureFromFile("GameData/cat.png");
		InitialSpawnCount = 0;
		SpawnRate = 0;
		Duration = 100000000;
	}
	
	Particle GetNewParticle() 
	{
		Particle p = ParticleEmitter::GetNewParticle();

		p.HasCollision = true;

		p.velocity = Camera::Forward() * -3.0f;

		return p;
	}

private:

};


class TestParticleSystem : public ParticleSystem
{

private:

	static TestParticleSystem* Instance;

public:

	TestParticleSystem()
	{
		emitters.push_back(new TestParticleEmitter());
	}

	static void SpawnParticleAt(vec3 position, vec3 rotation, vec3 scale);

	void Destroy()
	{
		ParticleSystem::Destroy();

		Instance = nullptr;

	}

	void SpawnParticleAtInst(vec3 position, vec3 rotation, vec3 scale);


};


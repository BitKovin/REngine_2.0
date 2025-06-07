#pragma once

#include <Entity.h>

#include <Particle/ParticleSystem.hpp>

class Bullet : public Entity
{

private:

	vec3 oldPos;
	float traveledDistance = 0;

public:




	float Damage = 15;
	float Speed = 100;
	float MaxDistance = 100;

	ParticleSystem* trail;

	Bullet();
	~Bullet();

	void LoadAssets()
	{

		PreloadEntityType("bullet_trail");
	}

	void Start()
	{
		oldPos = Position;

		trail = (ParticleSystem*)Spawn("bullet_trail");
		trail->Position = Position;
		trail->Rotation = Rotation;
		trail->Start();

	}

	void Update();



};
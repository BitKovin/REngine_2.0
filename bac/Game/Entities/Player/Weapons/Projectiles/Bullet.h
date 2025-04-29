#pragma once

#include <Entity.hpp>

#include <Particle/ParticleSystem.hpp>

class Bullet : public Entity
{

private:

	vec3 oldPos;
	float traveledDistance = 0;

public:

	StaticMesh* mesh = nullptr;



	float Damage = 15;
	float Speed = 40;
	float MaxDistance = 100;

	ParticleSystem* trail;

	Bullet();
	~Bullet();

	void LoadAssets()
	{
		mesh->Scale = vec3(0.2f,0.2f,0.8f);
		mesh->LoadFromFile("GameData/Models/Weapons/Bullet/bullet.obj");
		PreloadEntityType("bullet_trail");
	}

	void Start() 
	{
		oldPos = Position;

		trail = (ParticleSystem*)Spawn("bullet_trail");
		trail->Position = Position;
		trail->Rotation = Rotation;

	}

	void Update();



};
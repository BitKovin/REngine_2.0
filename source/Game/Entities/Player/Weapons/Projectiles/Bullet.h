#pragma once

#include <Entity.hpp>

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

	Bullet();
	~Bullet();

	void LoadAssets()
	{
		mesh->Scale = vec3(0.2f,0.2f,0.8f);
		mesh->LoadFromFile("GameData/Models/Weapons/Bullet/bullet.obj");
	}

	void Start() 
	{
		oldPos = Position;
	}

	void Update();



};
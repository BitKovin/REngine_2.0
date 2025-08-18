#pragma once

#include <Entity.h>

#include <Particle/ParticleSystem.hpp>

class CaneProjectile : public Entity
{

private:

	vec3 oldPos;
	float traveledDistance = 0;

public:


	float Damage = 15;
	float Speed = 100;
	float MaxDistance = 100;

	bool hited = false;

	ParticleSystem* trail = nullptr;

	StaticMesh* mesh = nullptr;

	CaneProjectile()
	{

		SaveGame = true;
		ClassName = "caneProjectile";

		mesh = new StaticMesh(this);
		Drawables.push_back(mesh);

		Name = "caneProjectile";

	}
	~CaneProjectile(){}

	void LoadAssets()
	{

		PreloadEntityType("bullet_trail");
		mesh->LoadFromFile("GameData/models/player/weapons/cane/cane.obj");
		mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/models/player/weapons/cane/cane.png");

	}

	void Start()
	{
		oldPos = Position;

		trail = (ParticleSystem*)Spawn("bullet_trail");
		trail->Position = Position;
		trail->Rotation = Rotation;
		trail->Start();

	}

	void Destroy()
	{

		Entity::Destroy();

		if (trail != nullptr)
		{
			trail->StopAll();
			trail = nullptr;
		}


	}

	void Serialize(json& target)
	{

		Entity::Serialize(target);

		SERIALIZE_FIELD(target, oldPos)
		SERIALIZE_FIELD(target, traveledDistance)

	}

	void Deserialize(json& source)
	{

		Entity::Deserialize(source);

		DESERIALIZE_FIELD(source, oldPos)
			DESERIALIZE_FIELD(source, traveledDistance)

	}

	void Update();



};
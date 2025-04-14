#pragma once

#include "../Entity.hpp"

#include "../Input.h"

#include "../MathHelper.hpp"

#include "../Camera.h"

#include "../SkeletalMesh.hpp"
#include "../AssetRegisty.h"

class TestCube : public Entity
{
public:

	SkeletalMesh* mesh;

	TestCube()
	{
		mesh = new SkeletalMesh();
	}

	TestCube(vec3 pos)
	{
		Position = pos;

		mesh = new SkeletalMesh();

		Start();

	}

	~TestCube()
	{

	}



	void Start()
	{
		mesh->LoadFromFile("GameData/dog.glb");
		mesh->PlayAnimation("run");
		mesh->SetLooped(true);
		mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");

		Drawables.push_back(mesh);

		LeadBody = Physics::CreateBoxBody(this, Position, vec3(1), 10, false);

	}

	void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr)
	{
		Destroy();
	}

	void AsyncUpdate()
	{

		mesh->UpdatePose = mesh->WasRended;

		mesh->Update();

		mesh->Position = Position - MathHelper::GetUpVector(Rotation)*0.5f;
		mesh->Rotation = Rotation;
	}


private:

};
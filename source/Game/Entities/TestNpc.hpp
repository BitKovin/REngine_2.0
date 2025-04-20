#pragma once

#include <Entity.hpp>

#include <Input.h>

#include <MathHelper.hpp>

#include <Camera.h>

#include <SkeletalMesh.hpp>
#include <AssetRegisty.h>

#include <Navigation/PathFollowQuery.h>

class TestNpc : public Entity
{

private:

	vec3 desiredDirection;
	vec3 movingDirection;

	PathFollowQuery pathFollow;

public:

	SkeletalMesh* mesh;

	TestNpc()
	{
		mesh = new SkeletalMesh();

		ClassName = "testnpc";
		SaveGame = true;

	}


	~TestNpc()
	{

	}



	void Start()
	{
		

		mesh->Position = Position;
		mesh->Rotation = Rotation;

		Drawables.push_back(mesh);

		LeadBody = Physics::CreateCharacterBody(this, Position, 0.5, 2, 50);

		Physics::SetGravityFactor(LeadBody, 4);

		desiredDirection = MathHelper::XZ(MathHelper::GetForwardVector(Rotation));
		movingDirection = desiredDirection;

		pathFollow.CalculatePathOnThread();

	}

	void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr)
	{
		Destroy();
	}

	void AsyncUpdate();

	void Serialize(json& target);
	void Deserialize(json& source);

protected:

	void LoadAssets();


};
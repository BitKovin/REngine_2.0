#pragma once

#include <Entity.hpp>

#include <Input.h>

#include <MathHelper.hpp>

#include <Camera.h>

#include <SkeletalMesh.hpp>
#include <AssetRegisty.h>

#include <Navigation/PathFollowQuery.h>

#include "SoundSystem/SoundManager.hpp"
#include "Entities/SoundPlayer.h"

class TestNpc : public Entity
{

private:

	vec3 desiredDirection;
	vec3 movingDirection;

	PathFollowQuery pathFollow;

	SoundPlayer* DeathSoundPlayer = nullptr;
	SoundPlayer* HurtSoundPlayer = nullptr;
	SoundPlayer* StunSoundPlayer = nullptr;
	SoundPlayer* AttackSoundPlayer = nullptr;

	bool dead = false;

	AnimationState animationStateSaveData;

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

	void SetupSoundPlayer(SoundPlayer* soundPlayer)
	{
		soundPlayer->MaxDistance = 100;
		soundPlayer->Volume = 0.3;
		soundPlayer->MinDistance = 0.5;
	}

	void Start()
	{
		

		mesh->Position = Position - vec3(0, 1, 0);
		mesh->Rotation = Rotation;

		Drawables.push_back(mesh);

		LeadBody = Physics::CreateCharacterBody(this, Position, 0.5, 2, 50);

		Physics::SetGravityFactor(LeadBody, 4);

		desiredDirection = MathHelper::XZ(MathHelper::GetForwardVector(Rotation));
		movingDirection = desiredDirection;

		pathFollow.CalculatePathOnThread();

		DeathSoundPlayer = SoundPlayer::Create();
		HurtSoundPlayer = SoundPlayer::Create();
		StunSoundPlayer = SoundPlayer::Create();
		AttackSoundPlayer = SoundPlayer::Create();

		SetupSoundPlayer(DeathSoundPlayer);
		SetupSoundPlayer(HurtSoundPlayer);
		SetupSoundPlayer(StunSoundPlayer);
		SetupSoundPlayer(AttackSoundPlayer);

	}

	void Death();

	void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone = "", Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

	void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

	void Destroy()
	{
		Entity::Destroy();

		DeathSoundPlayer->DestroyWithDelay(3);
		HurtSoundPlayer->DestroyWithDelay(3);
		StunSoundPlayer->DestroyWithDelay(3);
		AttackSoundPlayer->DestroyWithDelay(3);

	}

	void AsyncUpdate();

	void Serialize(json& target);
	void Deserialize(json& source);

protected:

	void LoadAssets();


};
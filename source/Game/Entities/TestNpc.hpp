#pragma once

#include <Entity.h>

#include <Input.h>

#include <MathHelper.hpp>

#include <Camera.h>

#include <SkeletalMesh.hpp>
#include <AssetRegistry.h>

#include <Navigation/PathFollowQuery.h>

#include <SoundSystem/SoundManager.hpp>
#include <Entities/SoundPlayer.h>
#include <Delay.hpp>

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
	SoundPlayer* AttackHitSoundPlayer = nullptr;

	float maxSpeed = 8;
	float speed = 4;

	bool dead = false;

	AnimationState animationStateSaveData;

	Delay inAttackDelay;

	bool attacking = false;
	bool stuned = false;

	bool attackingDamage = false;

public:

	SkeletalMesh* mesh;



	TestNpc()
	{
		mesh = new SkeletalMesh(this);
		Drawables.push_back(mesh);

		ClassName = "testnpc";
		SaveGame = true;

		Health = 50;

		mesh->UpdatePoseOnlyWhenRendered = true;

	}


	~TestNpc()
	{

	}

	void SetupSoundPlayer(SoundPlayer* soundPlayer)
	{
		soundPlayer->MaxDistance = 100;
		soundPlayer->Volume = 1.0;
		soundPlayer->MinDistance = 0.5;
	}

	void ProcessAnimationEvent(AnimationEvent& event);

	void Start();

	void Stun(Entity* DamageCauser, Entity* Weapon);

	void Attack();

	void Death();

	void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone = "", Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

	void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

	void UpdateAttackDamage();

	void Destroy()
	{
		Entity::Destroy();

		mesh->ClearHitboxes();


	}

	void AsyncUpdate();

	void Serialize(json& target);
	void Deserialize(json& source);

protected:

	void LoadAssets();


};
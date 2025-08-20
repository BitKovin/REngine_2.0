#pragma once

#include <Entity.h>
#include <MathHelper.hpp>
#include <Camera.h>
#include <SkeletalMesh.hpp>
#include <AssetRegistry.h>

#include <Navigation/PathFollowQuery.h>

#include <SoundSystem/SoundManager.hpp>
#include <Entities/SoundPlayer.h>
#include <Delay.hpp>

#include "Animators/NpcAnimatorBase.h"

class NpcBase : public Entity
{

protected:

	vec3 desiredDirection = vec3();
	vec3 movingDirection = vec3();

	PathFollowQuery pathFollow;

	SoundPlayer* DeathSoundPlayer = nullptr;
	SoundPlayer* HurtSoundPlayer = nullptr;
	SoundPlayer* StunSoundPlayer = nullptr;
	SoundPlayer* AttackSoundPlayer = nullptr;
	SoundPlayer* AttackHitSoundPlayer = nullptr;

	float maxSpeed = 5;
	float speed = 4;

	bool dead = false;

	AnimationState animationStateSaveData;

	NpcAnimatorBase animator = 0;

	Animation* getFromRagdollAnimation = nullptr;
	AnimationPose ragdollPose;

	AnimationState getFromRagdollAnimationSaveState;


public:

	SkeletalMesh* mesh;



	NpcBase();


	~NpcBase();

	void SetupSoundPlayer(SoundPlayer* soundPlayer)
	{
		soundPlayer->MaxDistance = 100;
		soundPlayer->Volume = 1.0;
		soundPlayer->MinDistance = 0.5;
	}

	void ProcessAnimationEvent(AnimationEvent& event);

	void Start();

	void Death();

	void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone = "", Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

	void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

	void StartReturnFromRagdoll();
	void UpdateReturnFromRagdoll();

	void Destroy()
	{
		Entity::Destroy();

		mesh->ClearHitboxes();


	}

	void AsyncUpdate();

	void Serialize(json& target);
	void Deserialize(json& source);

	void UpdateDebugUI();

protected:

	void UpdateAnimations();

	void LoadAssets();


private:

};


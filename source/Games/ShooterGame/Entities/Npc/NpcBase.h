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

#include <BehaviourTree/BehaviorTree.h>
#include <BehaviorTreeEditor.h>

class NpcBase : public Entity
{

protected:

	vec3 desiredDirection = vec3();
	vec3 movingDirection = vec3();

	SoundPlayer* DeathSoundPlayer = nullptr;
	SoundPlayer* HurtSoundPlayer = nullptr;
	SoundPlayer* StunSoundPlayer = nullptr;
	SoundPlayer* AttackSoundPlayer = nullptr;
	SoundPlayer* AttackHitSoundPlayer = nullptr;

	float maxSpeed = 4;
	float speed = 2.0;

	bool dead = false;

	AnimationState animationStateSaveData;

	NpcAnimatorBase animator = 0;

	Animation* getFromRagdollAnimation = nullptr;
	AnimationPose ragdollPose;

	AnimationState getFromRagdollAnimationSaveState;

	vec3 desiredTargetLocation;

	BehaviorTree behaviorTree;
	BehaviorTreeEditor editor;

	std::string btSaveState;

	bool btEditorEnabled = false;

	Delay tickIntervalDelay;


public:

	SkeletalMesh* mesh;

	std::string CurrentTargetNavPoint = "np_0_0";

	PathFollowQuery pathFollow;

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

	void UpdateBT();

	void Serialize(json& target);
	void Deserialize(json& source);

	void UpdateDebugUI();

	void MoveTo(const vec3& target, float acceptanceRadius);

protected:

	void UpdateAnimations();

	void LoadAssets();


private:

};


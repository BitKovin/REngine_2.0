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
#include <BehaviourTree/BehaviorTreeEditor.h>

#include <AiPerception/AiPerceptionSystem.h>
#include <AiPerception/Observer.h>
#include <AiPerception/ObservationTarget.h>

enum class InvestigationReason
{

	NpcInTrouble,
	WeaponFire,
	Body,
	Explosion,
	LoudNoise,
	Noise,
	None
};



class NpcBase : public Entity
{

protected:

	vec3 desiredDirection = vec3();
	vec3 movingDirection = vec3();

	SoundPlayer* VoiceSoundPlayer = nullptr;


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

	std::shared_ptr<Observer> observer;

	bool target_follow;
	std::string target_id;
	vec3 target_lastSeenPosition;
	Delay target_stopUpdateLastSeenPositionDelay;
	float target_lastSeenTime;
	bool target_sees;
	bool target_underArrest;
	bool target_attack;
	float target_underArrestExpire = 5.0f;
	bool target_attackInRange;


	InvestigationReason currentInvestigation = InvestigationReason::None;
	vec3 investigation_target;
	std::string investigation_targetId;

	bool investigation_changed;

	bool needToInvestigateBody = false;

	std::shared_ptr<ObservationTarget> observationTarget;

	const std::string vo_base_event_path = "event:/Character/Guards/Guard1/";

public:

	SkeletalMesh* mesh;

	std::string CurrentTargetNavPoint = "np_0_0";

	vec3 desiredLookVector = vec3(1);

	PathFollowQuery pathFollow;

	NpcBase();


	~NpcBase();

	void SetupSoundPlayer(SoundPlayer* soundPlayer)
	{
		soundPlayer->MaxDistance = 40;
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

	void PlayPhrace(std::string name);

	void AsyncUpdate();
	void LateUpdate();

	void UpdateBT();
	void UpdateObserver();
	void UpdateObservationTarget();

	void UpdateTargetFollow();

	void Serialize(json& target);
	void Deserialize(json& source);

	void UpdateDebugUI();

	void TryStartInvestigation(InvestigationReason reason, vec3 target, string causer);

	void FinishInvestigation();

	void PrepareToStartMovement();
	void StopMovement();
	void MoveTo(const vec3& target, float acceptanceRadius);
	void StopTargetFollow();

	void BodyInvestigated();

protected:

	void UpdateAnimations();

	void LoadAssets();

	void ShareTargetKnowlageWith(NpcBase* anotherNpc);


private:

};


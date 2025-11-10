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

#include "../../Ai/NpcTasks/TaskState.h"

enum class InvestigationReason
{
	TargetSeen,
	NpcInTrouble,
	WeaponFire,
	Body,
	Explosion,
	LoudNoise,
	Noise,
	None
};

enum class Crime
{

	WeaponFire, //aka violentCrime tag
	Group_Attack,
	WeaponFireSound,
	NearBody,
	WeaponHolding,
	Group_Arrest,
	Trespassing,
	Group_Follow,
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

	Animation* getFromRagdollAnimation = nullptr;
	AnimationPose ragdollPose;

	AnimationState getFromRagdollAnimationSaveState;

	vec3 desiredTargetLocation = vec3();

	BehaviorTree behaviorTree;
	BehaviorTreeEditor editor;

	std::string btSaveState = "";

	bool btEditorEnabled = false;

	Delay tickIntervalDelay;

	float attackRange = 20;
	float attackDesiredRange = 10;

	std::shared_ptr<Observer> observer;

	bool target_follow = false;
	std::string target_id = "";
	vec3 target_lastSeenPosition = vec3();
	Delay target_stopUpdateLastSeenPositionDelay = Delay();
	float target_lastSeenTime = -1;
	bool target_sees = false;
	bool target_underArrest = false;
	bool target_attack = false;
	float target_underArrestExpire = 5.0f;
	bool target_attackInRange = false;

	bool report_to_guard = false;
	bool found_guard = false;
	std::string closestGuard = "";

	Delay findGuardCooldown = Delay();

	InvestigationReason currentInvestigation = InvestigationReason::None;
	vec3 investigation_target = vec3();
	std::string investigation_targetId = "";

	bool investigation_changed;

	bool needToInvestigateBody = false;

	std::shared_ptr<ObservationTarget> observationTarget;

	const std::string vo_base_event_path = "event:/Character/Guards/Guard1/";

	Crime currentCrime = Crime::None;

	vec3 flee_target;

	StaticMesh* weaponMesh = nullptr;

	float detection_progress = 0.0f;

	bool stunnedRagdoll;
	Delay stunnedRagdollDelay;
	bool returningFromRagdoll;

	Delay attackDelay;
	Delay attackPositionUpdateDelay;

	vec3 attackPosition;

	vec3 spineRotation = vec3();

	std::string modelPath = "GameData/models/npc/base.glb";

	std::string defaultTask = "np_0_0";

private:

	bool has_observed_crime = false;

	float GetDetectionSpeed(Crime crime) const;

	std::vector<NpcBase*> shareKnowlageWith;

	short knowlageSharedThisFrame = 0;

public:

	bool DoingTask = true;
	bool DoingTaskOld = true;

	NpcAnimatorBase animator = 0;

	bool isGuard = true;

	static inline Delay globalPhraceDelay;

	SkeletalMesh* mesh;

	TaskState taskState;

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

	bool isStunned();

	void StartStunnedRagdoll();
	void UpdateStunnedReturn();

	void StartReturnFromRagdoll();
	void UpdateReturnFromRagdoll();

	void Destroy()
	{
		Entity::Destroy();

		mesh->ClearHitboxes();


	}

	void PlayPhrace(std::string name);

	void Update();

	void AsyncUpdate();
	void UpdateWeaponMesh();

	void LateUpdate();

	void UpdateBT();
	void UpdateObserver();
	void UpdateObservationTarget();

	void UpdateTargetFollow();

	void UpdateTargetAttack();

	bool CheckAttackLOS(vec3 location, vec3 targetLocation);

	//input are positions of attacker and target without vertical offset for fire point
	bool CheckAttackLocation(vec3 location, vec3 targetLocation);

	vec3 FindAttackLocation();

	void Serialize(json& target);
	void Deserialize(json& source);

	void UpdateDebugUI();

	void FindClosestGuard();

	void TryStartInvestigation(InvestigationReason reason, vec3 target, string causer, bool sharedByNpc = false);

	bool TryCommitCrime(Crime crime, std::string offender, vec3 pos);

	void FinishInvestigation();

	void PrepareToStartMovement();
	void StopMovement();
	void MoveTo(const vec3& target, float acceptanceRadius);
	void StopTargetFollow();

	void BodyInvestigated();

	void Task_TargetReached();
	void Task_DoStationaryJob();

	void Task_Doint();
	void Task_NotDoing();

	TaskState& GetTaskStateRef();

	void StartTask(const std::string& taskName);
	void StopTask();
	void UpdateTask();

protected:

	void UpdateAnimations();

	void LoadAssets();

	void ShareTargetKnowlageWith(NpcBase* anotherNpc);

	void ShareTargetKnowlageWithFinal(NpcBase* anotherNpc);


private:

};
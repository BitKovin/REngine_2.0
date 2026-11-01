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

#include <Character/CharacterController.h>

#include "../../Ai/NpcTasks/TaskState.h"

#include "Investigations/InvestigationBase.h"

#include <memory>
#include <unordered_map>
#include <set>
#include <limits>



class CombatBehaviorBase;

enum class NpcStance
{
	Standing,
	Crouched
};

// How a gun is carried in combat. Default = the legacy rules (aimed only while
// the target is in sight); the combat brain always sets one of the others.
enum class NpcWeaponPosture
{
	Default,
	Aimed,   // up and pointed: seeing the target, watching a point, walking in a fight
	Ready,   // held at the chest: sprinting
	Lowered  // reloading
};

// What the NPC should face while it moves.
enum class NpcFacing
{
	Movement, // turn toward the walking direction
	Target,   // keep facing the combat target
	Point     // keep facing CombatMoveOrder::lookAt
};

// Written by the combat brain every tick while it has a target; read by the
// movement code in AsyncUpdate. Where to go stays in attackPosition, which
// the behavior tree walks to. When no order is active the legacy rules apply
// (investigation speeds, patrol facing).
struct CombatMoveOrder
{
	bool active = false;
	float speed = 0.0f;
	NpcStance stance = NpcStance::Standing;
	NpcFacing facing = NpcFacing::Movement;
	vec3 lookAt{};
	NpcWeaponPosture posture = NpcWeaponPosture::Default;

	// The tactic wants this move slow on purpose (creeping, crouching in,
	// stepping through a door): the brain's "not shooting means running" rule
	// leaves it alone.
	bool careful = false;
};

enum class Crime
{

	WeaponFire, //aka violentCrime tag
	WeaponFireAttack,
	Group_Attack,
	WeaponFireSound,
	NearBody,
	WeaponHolding,
	Group_Arrest,
	Trespassing,
	Group_Follow,
	None
};

enum class NpcType
{
	None,
	Civilian,
	Guard,
	VIP
};

// Per-target memory structure used to track multiple targets
struct TargetInfo
{
	std::string id;
	bool follow = false;
	vec3 lastSeenPosition = vec3(); // keeps tracking ~1 s after sight is lost: for searching
	Delay stopUpdateLastSeenPositionDelay = Delay();
	float lastSeenTime = -1;
	bool sees = false;

	// Where the target stood the last moment anyone (this NPC, or whoever told
	// it) actually had a line of sight on it. Combat aims and plans with this.
	vec3 lastLOSPosition = vec3();
	float lastLOSTime = -1;

	bool seesAndDetected = false;

	bool underArrest = false;
	bool attack = false;
	float underArrestExpire = 5.0f;
	bool attackInRange = false;
	Crime currentCrime = Crime::None;
	float detection_progress = 0.0f;

	Crime lastMinCrime = Crime::None;
	double lastTimeSpotedCrime = -100000;

	void Serialize(json& target) const
	{
		SERIALIZE_FIELD(target, id);
		SERIALIZE_FIELD(target, follow);
		SERIALIZE_FIELD(target, lastSeenPosition);
		SERIALIZE_FIELD(target, stopUpdateLastSeenPositionDelay);
		SERIALIZE_FIELD(target, lastSeenTime);
		SERIALIZE_FIELD(target, sees);
		SERIALIZE_FIELD(target, lastLOSPosition);
		SERIALIZE_FIELD(target, lastLOSTime);
		SERIALIZE_FIELD(target, seesAndDetected);
		SERIALIZE_FIELD(target, underArrest);
		SERIALIZE_FIELD(target, attack);
		SERIALIZE_FIELD(target, underArrestExpire);
		SERIALIZE_FIELD(target, attackInRange);
		SERIALIZE_FIELD(target, currentCrime);
		SERIALIZE_FIELD(target, detection_progress);
		SERIALIZE_FIELD(target, lastMinCrime);
		SERIALIZE_FIELD(target, lastTimeSpotedCrime);
	}

	void Deserialize(const json& source)
	{
		DESERIALIZE_FIELD(source, id);
		DESERIALIZE_FIELD(source, follow);
		DESERIALIZE_FIELD(source, lastSeenPosition);
		DESERIALIZE_FIELD(source, stopUpdateLastSeenPositionDelay);
		DESERIALIZE_FIELD(source, lastSeenTime);
		DESERIALIZE_FIELD(source, sees);
		DESERIALIZE_FIELD(source, lastLOSPosition);
		DESERIALIZE_FIELD(source, lastLOSTime);
		DESERIALIZE_FIELD(source, seesAndDetected);
		DESERIALIZE_FIELD(source, underArrest);
		DESERIALIZE_FIELD(source, attack);
		DESERIALIZE_FIELD(source, underArrestExpire);
		DESERIALIZE_FIELD(source, attackInRange);
		DESERIALIZE_FIELD(source, currentCrime);
		DESERIALIZE_FIELD(source, detection_progress);
		DESERIALIZE_FIELD(source, lastMinCrime);
		DESERIALIZE_FIELD(source, lastTimeSpotedCrime);
	}
};


class NpcBase : public Entity
{

protected:

	CharacterController* controller = nullptr;

	SoundPlayer* VoiceSoundPlayer = nullptr;

	struct TargetLostSpreadData
	{
		std::string targetId;
		float lastSeenTime;
		vec3 position;
	};
	std::vector<TargetLostSpreadData> pendingTargetLostSpreads;

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

	std::string fractionTag = "citizen";

	float attackRange = 20; //enemy will perform attack in this range
	float attackDesiredRange = 10; //enemy will try to keep this distance

	// Primary-target copies (kept for compatibility with existing code)
	bool target_follow = false;
	std::string target_id = "";
	vec3 target_lastSeenPosition = vec3();
	Delay target_stopUpdateLastSeenPositionDelay = Delay();
	float target_lastSeenTime = -1;
	vec3 target_lastLOSPosition = vec3();
	float target_lastLOSTime = -1;
	bool target_sees = false;
	bool target_underArrest = false;
	bool target_attack = false;
	float target_underArrestExpire = 5.0f;
	bool target_attackInRange = false;

	// Multi-target memory and tag lists
	mutable std::recursive_mutex targetsMutex;
	std::unordered_map<std::string, TargetInfo> knownTargets;
	std::set<hashed_string> neutralTags = { "player" }; // default neutral tags
	std::set<hashed_string> hostileTags = {}; // tags that are always considered hostile
	std::set<hashed_string> friendlyTags = {}; // tags that are always considered friendly
	float forgetTime = 6000000.0f; // seconds to forget unseen targets


	Delay findGuardCooldown = Delay();



	//InvestigationReason currentInvestigation = InvestigationReason::None;
	//vec3 investigation_target = vec3();
	//std::string investigation_targetId = "";

	int investigation_changed = 0;

	bool needToInvestigateBody = false;// this body needs to be investigated

	std::shared_ptr<ObservationTarget> observationTarget;

	const std::string vo_base_event_path = "event:/Character/Guards/Guard1/";

	Crime currentCrime = Crime::None;

	vec3 flee_target = vec3();

	StaticMesh* weaponMesh = nullptr;

	float detection_progress = 0.0f;

	bool stunnedRagdoll = false;
	Delay stunnedRagdollDelay;
	bool returningFromRagdoll = false;

	Delay attackDelay;
	Delay attackPositionUpdateDelay;

	vec3 attackPosition = vec3();

	vec3 spineRotation = vec3();

	std::string modelPath = "GameData/models/npc/base.glb";

	// Decides how this NPC fights (null for NPCs that never fight, like civilians).
	std::shared_ptr<CombatBehaviorBase> combatBehavior;

	CombatMoveOrder combatMoveOrder;

	bool weaponLowered = false; // reloading: weapon held, not aimed
	bool weaponForcedAim = false;
	bool aimOverrideActive = false;
	vec3 aimOverridePoint{};

	std::string lastAttackerId;
	double lastAttackedTime = -100000;

	// Melee effects on other NPCs, queued from AsyncUpdate and applied in
	// LateUpdate (main thread) so no worker thread writes another NPC.
	struct PendingNpcMeleeEvent
	{
		std::string targetId;
		bool isHit = false; // false = warn the target about the incoming attack
		float damage = 0.0f;
		vec3 point{};
		vec3 direction{};
		std::string bone;
		vec3 knockback{}; // velocity given to a player target (shove)
	};
	std::vector<PendingNpcMeleeEvent> pendingNpcMeleeEvents;

	double nextShoveTime = 0.0;

	// Contact frame of the shove (the "attack_kick" event of sword_kick).
	void DoShoveHit();

	std::string defaultTask = "np_0_0";

	std::string scheduledTask = "";
	std::string DesiredTask = "defaultTask";

private:


	bool has_observed_crime = false;

	float GetDetectionSpeed(Crime crime) const;

	std::vector<NpcBase*> shareKnowlageWith;

	short knowlageSharedThisFrame = 0;

	std::shared_ptr<InvestigationBase> CreateInvestigationFromReason(InvestigationReason reason);

	vec3 ragdollPelvisWorldPos = vec3();
	float pelvisBlendTimer = 0.0f;

public:


	std::shared_ptr<Observer> observer;

	vec3 desiredDirection = vec3();
	vec3 movingDirection = vec3();

	bool needHelpStunned = false;
	bool found_guard = false;
	bool report_to_guard = false;
	std::string closestGuard = "";
	Delay movementLockDelay = Delay();
	std::shared_ptr<InvestigationBase> currentInvestigation;

	float Height = 1.8f;

	bool DoingTask = true;
	bool DoingTaskOld = true;

	std::unique_ptr<NpcAnimatorBase> animator = 0;

	NpcType npcType = NpcType::Guard;


	static inline Delay globalPhraceDelay;

	SkeletalMesh* mesh = nullptr;

	TaskState taskState;

	vec3 desiredLookVector = vec3(1);

	PathFollowQuery pathFollow;

	std::string SimulationId;

	NpcBase();


	~NpcBase();

	void SetupSoundPlayer(SoundPlayer* soundPlayer)
	{
		soundPlayer->MaxDistance = 40;
		soundPlayer->Volume = 1.0;
		soundPlayer->MinDistance = 0.5;
	}

	void DoInterpolatedAnimationUpdate();

	void ProcessAnimationEvent(AnimationEvent& event);

	void FromData(EntityData data);

	void Start();

	void Death();

	void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone = "", Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

	void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

	virtual bool isStunned();

	void StartStunnedRagdoll();
	void UpdateStunnedReturn();

	void StartReturnFromRagdoll();
	void UpdateReturnFromRagdoll();


	void Destroy();

	void PlayPhrace(std::string name);

	void Update();

	void AsyncUpdate();
	virtual void UpdateWeaponMesh();

	void LateUpdate();

	void UpdateDoorUpdate();

	void UpdateBT();
	void UpdateObserver();
	void UpdateObservationTarget();

	void UpdateTargetFollow();

	virtual bool IsPlayingRootMotionAnimation();
	virtual bool IsRotationAllowedDuringRootMotion();

	virtual void UpdateTargetAttack();//updates all attack behavior by running the combat behavior (or the plain ranged attack when the NPC has none)

	// The ranged attack body: strafing, aiming and firing at the target.
	void RunRangedAttack();

	CombatBehaviorBase* GetCombatBehavior() const { return combatBehavior.get(); }

	// What the combat brain needs to know about this NPC when it is somebody's target.
	const std::string& GetTargetId() const { return target_id; }
	bool IsFollowingTarget() const { return target_follow; }
	bool SeesTarget() const { return target_sees; }

	// Where the target was the last moment this NPC (or whoever told it) saw it;
	// zero time = never.
	vec3 GetTargetLastLOSPosition() const { return target_lastLOSPosition; }
	float GetTargetLastLOSTime() const { return target_lastLOSTime; }
	vec3 GetTargetLastSeenPosition() const { return target_lastSeenPosition; }

	// What the body faces and the spine aims at in a fight: the combat brain's
	// believed target position, never the target's live position through a wall.
	vec3 GetCombatLookPoint() const;

	// A point the NPC is told to face or aim at, swapped for the corner it would
	// come round when a wall stands between (CombatBehaviorBase::LookPointFor).
	vec3 ResolveCombatLook(const vec3& point) const;

	const CombatMoveOrder& GetCombatMoveOrder() const { return combatMoveOrder; }
	vec3 GetControllerVelocity() const { return controller ? controller->GetVelocity() : vec3(0); }
	bool IsCrouched() const { return controller && controller->isCrouched; }
	virtual bool IsBlockingStance() const { return false; }

	// ── Combat movement ───────────────────────────────────────────────────────

	// True while the NPC stands at a firing position with the target in sight.
	bool IsInAttackPosition() const { return npcType == NpcType::Guard && target_sees && target_attackInRange && target_attack; }

	// Whether the NPC should keep facing its target right now (strafing at a
	// firing position, standing still while the target sees it, or up close).
	bool ShouldFaceTargetInCombat() const;

	// Standing order for the current situation: strafe speed at a firing
	// position, run speed otherwise; facing per ShouldFaceTargetInCombat().
	CombatMoveOrder MakeDefaultCombatMoveOrder() const;

	void SetCombatMoveOrder(const CombatMoveOrder& order);

	// ── Ranged attack pieces (COMBAT_AI_DESIGN.md §9) ─────────────────────────

	// Spine toward the target and, unless the caller owns the position (cover
	// tactics), the periodic attack position refresh.
	void UpdateAimAndAttackPosition(bool refreshAttackPosition = true);

	// Attack conditions, cooldown, speed and line-of-sight gate for a shot.
	bool ReadyToFireRanged();

	// Where a perfect shot would go (target body, led by its velocity).
	vec3 ComputeRangedAimPoint() const;

	// Fires one bullet at aimPoint with the fire animation and cooldown.
	void FireShot(const vec3& aimPoint);

	// Drops the weapon out of the aim pose for this frame (reloading).
	void SetWeaponLowered(bool lowered) { weaponLowered = lowered; }

	// Holds the weapon aimed for this frame even without a target in sight
	// (covering a hiding target's cover).
	void SetWeaponAimed() { weaponForcedAim = true; }

	// Aims the spine at a point instead of the target for this frame: the squad's
	// estimate while the target is hidden.
	void SetAimPoint(const vec3& point) { aimOverrideActive = true; aimOverridePoint = point; }

	// Gate for shots at a point (suppressing a hidden target): cooldown, standing
	// still enough, and the world must not swallow the shot far short of the point.
	bool ReadyToFireAtPoint(const vec3& point);

	// True while the combat brain deliberately holds contact with this id
	// (hiding target, cover being checked), so detection must not decay.
	bool HoldsCombatContactWith(const std::string& id) const;

	// Where the behaviour tree walks the NPC while it fights.
	void SetAttackPosition(const vec3& position) { attackPosition = position; }
	vec3 GetAttackPosition() const { return attackPosition; }

	// The NPC is in a fight it means to take part in (not just following or arresting).
	bool WantsToAttackTarget() const { return target_follow && target_attack; }

	// The behaviour tree keeps the NPC in combat while this holds: it sees the
	// target, or the brain deliberately holds its ground against an unseen one.
	bool IsTargetInContact() const;

	// The combat brain decides where the NPC goes (its tactics set attackPosition):
	// the behaviour tree only walks it there.
	bool CombatBrainSteers() const;

	float GetCenterHeight() const { return controller ? controller->height * 0.5f : Height * 0.5f; }
	float GetAttackRange() const { return attackRange; }
	float GetAttackDesiredRange() const { return attackDesiredRange; }
	const std::string& GetFractionTag() const { return fractionTag; }

	// Incoming fire pinned this NPC down (near miss).
	void AddSuppression(float amount);

	// Pushes the next shot back by `seconds`.
	void DelayAttack(float seconds) { attackDelay.AddDelay(seconds); }

	// Who last hurt this NPC, for the combat brain's threat ranking.
	const std::string& GetLastAttackerId() const { return lastAttackerId; }
	double GetLastAttackedTime() const { return lastAttackedTime; }

	// Melee against other NPCs (see COMBAT_AI_DESIGN.md §4.1).
	bool IsHostileEntity(const Entity* entity) const;
	void QueueMeleeWarning(const std::string& targetId);
	void QueueMeleeHit(const std::string& targetId, float damage, vec3 point, vec3 direction, const std::string& bone, vec3 knockback = vec3(0));

	// ── Shove (COMBAT_AI_DESIGN.md §6.11) ─────────────────────────────────────

	bool CanShove() const;
	void PerformShove();

	// Animation events of every clip the NPC plays; melee NPCs handle their attacks here.
	virtual void HandleAnimationEvent(const AnimationEvent& event);

	class NpcSimulationState* GetSimulationStateRef();

	bool CheckAttackLOS(vec3 location, vec3 targetLocation);

	//input are positions of attacker and target without vertical offset for fire point
	bool CheckAttackLocation(vec3 location, vec3 targetLocation);

	virtual vec3 FindAttackLocation(); //calculates attack location. By default for strafing

	
	void Serialize(json& target);
	void Deserialize(json& source);

	void UpdateDebugUI();

	void FindClosestGuard();

	bool InvestigationCanBeReTriggered(InvestigationReason reason, string causer);

	void TryStartInvestigation(InvestigationReason reason, vec3 target, string causer, bool sharedByNpc = false);

	bool TryCommitCrime(Crime crime, std::string offender, vec3 pos);

	void InvestigationReachedDestination();

	void PrepareToStartMovement();
	void StopMovement();
	void MoveTo(const vec3& target, float acceptanceRadius);

	// Overload: stop follow for a specific target id (new)
	void StopTargetFollow();
	void StopTargetFollow(const std::string id);

	void TargetLost();

	void StopTargetFollowFinal(const std::string& id, float originatorLastSeenTime, const vec3& originatorPosition);


	void BodyInvestigated();

	void Task_TargetReached();
	void Task_DoStationaryJob();

	void Task_Doint();
	void Task_NotDoing();

	TaskState& GetTaskStateRef();

	void StartTask(const std::string& taskName);
	void StopTask();
	void UpdateTask();

	virtual void WarnAboutAttack(Entity* from){}

	// ── Action animation ──────────────────────────────────────────────────────
	// Thin wrappers over NpcAnimatorBase so any NPC subclass (guard, civilian,
	// melee, hybrid…) can trigger stun / special-move animations without casting
	// the animator directly.  Implemented in NpcBase.cpp.
	void  PlayActionAnimation(const std::string& name, bool loop = false,
	                          float blendIn = 0.3f, float blendOut = 0.3f);
	bool  IsPlayingActionAnimation() const;
	void  StopActionAnimation();
	float GetActionAnimationRemainingTime() const;

	void ShareTargetKnowlageWith(NpcBase* anotherNpc);

	void UpdateTargetLocation(std::string target, vec3 location);

	// The squad (CombatCoordinator, main thread) passes on what one of its
	// members saw or heard of a shared target, whether or not this NPC can see
	// that member: lineOfSight = somebody actually saw it there.
	void ReceiveSquadSighting(const std::string& targetId, const vec3& position, bool lineOfSight);

	// A sound the target made was heard at `position`. ownSenses: this NPC heard
	// it (or a shot whizzed past it) itself, so its squad hears of it too; false
	// when the squad is passing it on.
	void ReceiveHeardTarget(const std::string& targetId, const vec3& position, bool ownSenses);

	// A bullet passed close (Bullet): pinned down, and if it came from the target
	// it is after, it now knows where that is.
	void OnNearMiss(Entity* shooter);

protected:

	virtual void UpdateAnimations(bool forceFullUpdate = false);

	void LoadAssets();

	// Creates the combat brain for this kind of NPC. Called once, from Start or
	// Deserialize, whichever comes first. Guards fight with the ranged brain by default.
	virtual std::shared_ptr<CombatBehaviorBase> CreateCombatBehavior();



	void ShareTargetKnowlageWithFinal(NpcBase* anotherNpc);

	void ApplyCombatStance();
	void ApplyPendingNpcMeleeEvents();

	// helper to select primary (existing) target from knownTargets
	void SelectPrimaryAndCopy();

	bool IsNeutral(const std::shared_ptr<ObservationTarget>& target) const;
	bool IsHostile(const std::shared_ptr<ObservationTarget>& target) const;

	void MoveToScheduledTask();

	void UpdateScheduledTask();

private:

};
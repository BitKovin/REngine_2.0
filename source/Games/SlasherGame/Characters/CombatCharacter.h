#pragma once

#include <Actions/MoveRunner.h>
#include <Characters/CharacterAnimator.h>
#include <Characters/CharacterInput.h>
#include <Characters/WeaponSet.h>
#include <Combat/AirJuggle.h>
#include <Combat/CombatComponents.h>
#include <Combat/CombatEvents.h>
#include <Combat/Spring.h>
#include <Data/CharacterData.h>
#include <Data/StatusEffectData.h>

#include <Character/CharacterController.h>
#include <Delay.hpp>
#include <Entity.h>
#include <SkeletalMesh.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

class AIController;
class Controller;
struct HitResult;

// Every fighting body: hero, enemies, allies, possessed enemies (COMBAT_DESIGN.md §3). Configured entirely by
// a character JSON; concrete types are thin subclasses that pass their data path (see Characters.cpp).
//
// Per frame (COMBAT_DESIGN.md §2.2):
//   BeginCombatFrame / UpdateCombat  (PostUpdate, CombatBodyUpdateSystem, parallel across bodies)
//                         controller input -> MoveRunner (moves, combos, hit sweeps) -> facing -> aim. Anything that
//                         touches other entities or engine-global state is queued.
//   FlushCombatFrame      (same system, main thread, in body order) queued hits, cues, shots, cross-body actions
//   UpdatePose   (PostAsyncUpdate, CombatBodyPoseSystem, parallel across bodies, after the hits resolved)
//                         animator (clips at the move clock, rate by distance) -> movement (root motion /
//                         locomotion / behaviour overrides, in local time) -> mesh pose + hitboxes -> weapon posing
//   LateUpdate   (main)   weapon trails
class CombatCharacter : public Entity
{
	ECS_COMPONENT(CombatBodyComponent, CombatBody, this);
	ECS_COMPONENT(TeamComponent, Team);
	ECS_COMPONENT(CombatStateComponent, CombatState);
	ECS_COMPONENT(PoiseComponent, Poise);
	ECS_COMPONENT(WeakPointsComponent, WeakPoints);
	ECS_COMPONENT(StatusEffectsComponent, StatusEffects);
	ECS_COMPONENT(AttackTokensComponent, AttackTokens);

public:
	explicit CombatCharacter(std::string characterDataPath = {});
	~CombatCharacter() override;

	// ---- Entity ------------------------------------------------------------------------------------------
	void FromData(EntityData data) override;
	void Start() override;
	void LateUpdate() override;
	void Destroy() override;
	void Teleport(vec3 position) override;
	void OnDamage(float damage, Entity* causer = nullptr, Entity* weapon = nullptr) override;
	void OnPointDamage(float damage, vec3 point, vec3 direction, string bone = "", Entity* causer = nullptr, Entity* weapon = nullptr) override;

	// ---- Combat frame (CombatBodyUpdateSystem, COMBAT_DESIGN.md §2.2) --------------------------------------
	bool IsCombatReady() const { return started && data && !Destroyed; }
	void BeginCombatFrame();                  // worker thread: own state only (position, grounded, input edges)
	void UpdateCombat();                      // worker thread: controller, moves, facing, aim; side effects queued
	void FlushCombatFrame();                  // main thread, in body order: queued hits, cues, shots, actions
	void UpdatePose();                        // worker thread (CombatBodyPoseSystem): animation, movement, hitboxes
	// Runs fn on the main thread: at once when called there, otherwise in this body's FlushCombatFrame.
	void RunOnMainThread(std::function<void()> fn);
	void QueueHit(const HitRequestEvent& hit);   // emitted into the funnel by FlushCombatFrame, in order

	// ---- Data and parts ----------------------------------------------------------------------------------
	const CharacterData* GetData() const { return data.get(); }
	const std::string& GetDataPath() const { return dataPath; }
	MoveRunner& GetMoves() { return moves; }
	const MoveRunner& GetMoves() const { return moves; }
	WeaponSet& GetWeapons() { return weapons; }
	const WeaponSet& GetWeapons() const { return weapons; }
	SkeletalMesh* GetMesh() const { return mesh; }
	CharacterInput& GetInput() { return input; }
	const CharacterInput& GetInput() const { return input; }
	float GetClipDuration(const MoveData& move) const { return animator.GetClipDuration(move.animationIndex, move.clipHash); }

	// ---- Control -----------------------------------------------------------------------------------------
	void SetController(Controller* newController);
	Controller* GetController() const { return controller; }
	AIController* GetAIController() const { return aiController.get(); }
	bool IsPlayerControlled() const;

	// ---- State -------------------------------------------------------------------------------------------
	bool IsDead() const;
	bool IsBroken() const;                    // poise broken: finishable, only forced moves
	bool IsGrounded() const { return grounded; }
	Team GetTeamId() const;
	void SetTeamId(Team team);
	float GetLocalDeltaTime() const;          // Time::DeltaTimeF * hit-stop dilation
	float GetTimeDilation() const;
	float GetAttackSpeedMultiplier() const;
	float GetMoveSpeedMultiplier() const;
	// A weapon in the hands aims (its stance has a sweep) and nothing overrides it: no primary move runs, or the
	// running one allows overlays (jumping, gliding). A dash or an attack faces its own direction instead.
	const WeaponData* GetAimingWeapon() const;
	// Equips weapon `index` of `slot` (the player's switch, COMBAT_DESIGN.md §9.3): the outgoing weapon's moves stop and
	// the move tables follow the new one.
	void EquipWeapon(WeaponSlot slot, size_t index);

	// ---- Combat callbacks (main thread, from the hit funnel) ---------------------------------------------
	virtual void OnHitResolved(const HitResult& result);
	virtual void OnHitDealt(const HitResult& result);
	virtual void OnDeath(Entity* killer);

	// ---- MoveRunner hooks --------------------------------------------------------------------------------
	void OnMoveStarted(const MoveSlot& slot, bool primary);
	void OnMoveEnded(const MoveSlot& slot, MoveEndReason reason, bool primary);
	void PlayCue(const CueData& cue);         // queued while the bodies update in parallel

	// ---- Movement (requests last one frame; behaviours set them from OnTick) ------------------------------
	glm::vec3 GetForward() const;
	glm::vec3 GetRight() const;
	float GetYaw() const { return yaw; }
	glm::vec3 GetVelocity() const { return velocity; }
	float GetCapsuleHeight() const;
	glm::vec3 GetFeetPosition() const;

	// Turn toward `direction` this frame at most turnRateDegrees deg/s, easing with `sharpness` (1/s; <= 0: the body's
	// movement.turnSharpness).
	void RequestFacing(const glm::vec3& direction, float turnRateDegrees, float sharpness = 0.0f);
	void SnapFacing(const glm::vec3& direction);
	void RequestVelocity(const glm::vec3& worldVelocity, bool includeVertical);  // replaces root motion/locomotion
	void RequestLocomotion(float speedScale);                                   // let input move the body during a move
	void RequestSpeedScale(float scale);                                        // scales locomotion speed (firing)
	void RequestGravityScale(float scale);
	void RequestMaxFallSpeed(float speed);
	void RequestRootMotionScale(float scale);
	void RequestGliding();
	// Melee target assist (COMBAT_DESIGN.md §5.7): pulls toward `target` until idealDistance on top of the root
	// motion, at most maxSpeed, arriving within arrivalTime local seconds. With closeGap false it only keeps root
	// motion from carrying the body closer than idealDistance.
	void RequestMagnet(const glm::vec3& target, float idealDistance, float arrivalTime, float maxSpeed, bool closeGap);
	void RequestAdditiveVelocity(const glm::vec3& worldVelocity);               // horizontal, on top of the rest
	void Launch(float upwardSpeed);                                            // sets vertical velocity (>= 0)
	void SetVerticalSpeed(float speed);                                        // sets vertical velocity (any sign)
	void LimitHorizontalSpeed(float maxSpeed);
	// Air combat (COMBAT_DESIGN.md §5.8, rules in Combat/AirJuggle.h): for `seconds` (at least) the body hovers - it
	// rises on its normal arc and, falling, has almost no gravity (CharacterData::airCombat). Ends on landing.
	void EnterAirCombat(float seconds);
	void EndAirCombat();                                                       // gravity at once (a slam, a corpse); no pull either
	bool IsInAirCombat() const { return airBody.remaining > 0.0f && !grounded; }
	bool IsAirHovering() const { return data && airBody.IsHovering(data->airCombat); }   // hover that can still be felt
	void LiftTo(float upwardSpeed);                                            // vertical speed at least this (m/s)

	// A melee hit between two airborne fighters settles the victim once (AirJuggle::PlanPull): it covers
	// `displacement` within airCombat.pullTime, on top of nothing else - the destination is fixed now, whatever the
	// attacker does next. Called on the main thread by the attacker's hit; never for player-controlled bodies.
	void SetAirPull(const glm::vec3& displacement, bool horizontal);

	// The launcher in the air is weaker with every use (airCombat.launch*). PeekAirLaunchScale: the strength of one
	// started now (1 on the ground). AddAirLaunchStrain: one connected. RecoverAirLaunch: a melee hit of the combo.
	float PeekAirLaunchScale() const;
	void AddAirLaunchStrain();
	void RecoverAirLaunch();
	void SetLaunchScale(float scale) { launchScale = scale; }                  // strength of the running attack's launching hits
	float GetLaunchScale() const { return launchScale; }

	// Upward speed (local units) after last frame's physics step: a rise the capsule could not make was blocked (a
	// ceiling) - adopt what it made. `appliedDilation`: the time dilation the speed was handed to the physics with (a
	// hit-stop changes it between frames); `physicsY`: the vertical speed the physics ended the step with. Rises too
	// slow for the controller to leave the ground (it zeroes them itself) are not judged.
	static float ResolveRise(float velocityY, float appliedDilation, float physicsY);

	struct MovementResources
	{
		int airJumpsUsed = 0;
		int airDashesUsed = 0;
		float glideTimeUsed = 0.0f;
		float airLaunchStrain = 0.0f;   // air launchers that connected since landing, less what combo hits healed
	};
	MovementResources& GetMovementResources() { return resources; }
	const MovementResources& GetMovementResources() const { return resources; }

	// Knockback / launch handed from the hit funnel to the reaction move.
	glm::vec3 TakePendingKnockback();

	// ---- Targeting ---------------------------------------------------------------------------------------
	// Best hostile, alive combatant within range and the cone around direction (closest + most centered).
	// requireBroken: only poise-broken (finishable) bodies. prefer: returned whenever it qualifies.
	Entity* FindSoftTarget(const glm::vec3& direction, float range, float halfAngleDegrees, bool requireBroken = false,
		const Entity* prefer = nullptr) const;
	// Target of an attack starting now (COMBAT_DESIGN.md §5.7): every hostile, alive combatant within range all around
	// the body and in sight, the one closest to the move input direction (the facing without input) - distance breaks
	// near ties, the current soft target is favoured (CharacterData::targeting). maxHeight: only bodies at most this far
	// above / below (< 0: any height); airborneOnly: only bodies in the air. Safe from the parallel update.
	Entity* FindAttackTarget(float range, float maxHeight = -1.0f, bool airborneOnly = false) const;
	// The maxHeight of this body's attack searches now: targeting.airHeightRange in the air, any height on the ground.
	float GetAttackHeightRange() const { return grounded || !data ? -1.0f : data->targeting.airHeightRange; }
	// What the attack opener would pick now (HUD), or nullptr.
	Entity* PredictAttackTarget() const;
	// Finishable target for this body's finisher, using its data's finisher range / angle around input or facing.
	Entity* FindFinisherTarget() const;
	Entity* GetSoftTarget() { return ResolveAlive(softTarget); }
	void SetSoftTarget(Entity* target);
	glm::vec3 GetAimPoint() const;            // where firearms aim: input aim ray hit (or far along it)
	glm::vec3 GetTargetPoint() const;         // where others aim at this body (chest)

	// ---- Debug -------------------------------------------------------------------------------------------
	void DrawDebugUI();

protected:
	void LoadAssets() override;

	std::string dataPath;
	std::shared_ptr<const CharacterData> data;
	std::shared_ptr<const StatusEffectTable> statusEffects;

	CharacterController movement;
	SkeletalMesh* mesh = nullptr;
	CharacterAnimator animator;
	MoveRunner moves;
	WeaponSet weapons;
	CharacterInput input;

	Controller* controller = nullptr;
	std::unique_ptr<AIController> aiController;
	Body* fallbackHitbox = nullptr;
	EntityHandle softTarget;

private:
	struct FrameRequests
	{
		bool hasVelocity = false;
		glm::vec3 velocity = glm::vec3(0.0f);
		bool velocityIncludesVertical = false;
		float locomotionScale = -1.0f;        // < 0: not requested
		float speedScale = 1.0f;
		float gravityScale = 1.0f;
		float maxFallSpeed = -1.0f;
		float rootMotionScale = 1.0f;
		bool hasFacing = false;
		glm::vec3 facing = glm::vec3(0.0f, 0.0f, 1.0f);
		float facingRate = 0.0f;
		float facingSharpness = 0.0f;         // <= 0: movement.turnSharpness
		bool gliding = false;
		bool hasMagnet = false;
		glm::vec3 magnetTarget = glm::vec3(0.0f);
		float magnetIdealDistance = 0.0f;
		float magnetArrivalTime = 0.1f;
		float magnetMaxSpeed = 0.0f;
		bool magnetCloseGap = false;
		glm::vec3 additiveVelocity = glm::vec3(0.0f);
	};

	FrameRequests requests;
	MovementResources resources;
	glm::vec3 velocity = glm::vec3(0.0f);     // logical velocity in local time (m/s)
	glm::vec3 acceleration = glm::vec3(0.0f); // horizontal, local time (m/s^2), for the animator's lean
	glm::vec3 pendingKnockback = glm::vec3(0.0f);
	bool hasPendingVerticalSpeed = false;
	float pendingVerticalSpeed = 0.0f;
	float pendingLift = 0.0f;                 // LiftTo this frame (m/s, 0 = none)
	float pendingHorizontalLimit = -1.0f;
	AirJuggle::Body airBody;                  // hover timers and the hang clock of this airtime (local seconds)
	glm::vec3 pullDestination = glm::vec3(0.0f);   // SetAirPull: where the victim settles
	float pullRemaining = 0.0f;               // local seconds of the settle left
	bool pullHorizontal = false;
	float launchScale = 1.0f;                 // strength of the launching hits of the running attack
	float appliedDilation = 1.0f;             // time dilation of the velocity handed to the physics last frame
	float yaw = 0.0f;
	bool grounded = false;
	bool started = false;
	uint32_t adoptedDataGeneration = 0;
	Delay corpseDelay;                        // started on death; the corpse is removed when it runs out
	glm::vec3 aimPoint = glm::vec3(0.0f);     // where the weapon in the hands aims this frame (its sweep)
	bool hasAimPoint = false;
	bool reloadPlayed = false;                // the running reload of the Secondary weapon started its Reload move
	bool fireAfterReload = false;             // the reload cut a fire move short: a held trigger fires again after it
	float secondaryLinger = -1.0f;            // holsterAfterFire: local s until it goes away (< 0: not counting)
	bool recoveryWasHeld = false;             // last frame an attack played out unreleased (the body stood in its stance)
	std::array<uint32_t, static_cast<size_t>(WeaponSlot::Count)> switchPressSeen{};   // SwitchMain / SwitchSecondary
	std::array<uint32_t, static_cast<size_t>(WeaponSlot::Count)> pendingSwitches{};   // presses not applied yet
	DampedSpring hitRecoil;                   // world-space rotation vector (radians) of the upper body (§9.1)

	// Side effects of the parallel update, run by FlushCombatFrame on the main thread.
	std::vector<HitRequestEvent> queuedHits;
	std::vector<const CueData*> queuedCues;   // point into the moveset, alive until the flush
	std::vector<std::function<void()>> queuedActions;

	// Pose level of detail (CharacterData::animationLod, UpdatePoseDetail).
	int animationInterval = 1;                // evaluate the pose every n-th frame
	int animationFramesSinceEvaluate = 0;
	float animationPendingTime = 0.0f;        // local seconds since the last evaluation
	glm::vec3 rootMotionVelocity = glm::vec3(0.0f);   // model space, m/s, from the last evaluation

	void ConfigureComponents();
	void CreateHitboxes();
	void UpdateFacing(float deltaTime);
	void ApplyAirHit(const HitResult& result);
	void ApplyMovement(float deltaTime);
	void BuildAnimatorInput(AnimatorInput& out) const;
	void UpdatePoseDetail();
	void UpdateWeaponPosing();
	void UpdateWeaponState(float deltaTime);
	void UpdateWeaponSwitch();
	void SyncEquippedMoves();
	void UpdateRecoveryRelease();
	void UpdateAim();
	void AddHitRecoil(const glm::vec3& direction, float strength);
	void PlayCueNow(const CueData& cue);
	void AdoptReloadedData();
	glm::vec3 BonePositionOrOrigin(const std::string& bone) const;
};

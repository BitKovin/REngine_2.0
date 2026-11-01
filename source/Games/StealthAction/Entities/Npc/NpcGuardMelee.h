#pragma once

#include "NpcBase.h"
#include "Animators/NpcMeleeAnimator.h"

#include <RandomHelper.h>
#include <Entities/Player/Player.hpp>
#include <Entities/Enemy/Enemies/NpcHelper.h>

// ─────────────────────────────────────────────────────────────────────────────
//  NpcGuardMelee
//
//  Melee combat state machine overview
//
//   IDLE        – disengaged; waits until target enters attackRange
//   APPROACHING – closing the distance to enter melee range
//   PRESSURE    – tight-circling the target, probing for an opening
//   WIND_UP     – committed telegraph before a swing (can't cancel)
//   RECOVERING  – post-attack vulnerability window
//   RETREATING  – resetting spacing after an exchange
//   BLOCKING    – active parry raised in response to WarnAboutAttack
//   PUNISHING   – rapid counter immediately after a successful block
//   BAITING     – deliberately inviting an attack in order to punish it
// ─────────────────────────────────────────────────────────────────────────────

class NpcGuardMelee : public NpcBase
{
protected:

	enum class MeleeState
	{
		IDLE, APPROACHING, PRESSURE,
		WIND_UP, RECOVERING, RETREATING,
		BLOCKING, PUNISHING, BAITING
	};

	// ── State machine fields ──────────────────────────────────────────────────
	Delay      stunDelay;
	MeleeState meleeState = MeleeState::IDLE;
	Delay      stateTimer;
	Delay      retreatPosTimer;

	bool  pendingCounterAttack  = false;
	bool  isCurrentlyBlocking   = false;
	int   playerBlockCount      = 0;
	Delay blockCountDecayTimer;

	bool  playerWasBlockingPrev = false;
	Delay continuousBlockTimer;
	bool  nextAttackIsBreak     = false;

	int   timesParriedThisFight = 0;

	// Against NPC defenders: a swing within WarnRange tells the defender to
	// consider blocking; a blocking defender facing the attacker (dot product
	// above BlockFacingDot) takes no damage.
	static constexpr float WarnRange = 3.0f;
	static constexpr float BlockFacingDot = 0.5f;

	// Against a target that cannot block or parry (empty hands, a gun) there is nothing to be
	// careful about: the NPC closes in straight, attacks as soon as it is in range
	// (after MinPressureBeforeAttack) and recovers quickly. The probing, baiting and
	// retreating only happen while the target can actually counter.
	static constexpr double MinPressureBeforeAttack = 0.15; // seconds in range before an open target is attacked
	static constexpr float OpenRecoveryMin = 0.35f;         // recovery after a swing against an open target
	static constexpr float OpenRecoveryJitter = 0.25f;
	static constexpr float OpenApproachRadius = 1.4f;       // metres from the target when closing in straight

	static constexpr float GuardedApproachRadius = 1.7f;    // where a straight run stops against a target that can block

	double stateEnteredTime = 0.0;

	// ── State transition helpers ──────────────────────────────────────────────
	inline void EnterState(MeleeState next, float duration = 0.0f)
	{
		stateEnteredTime = Time::GameTime;
		meleeState = next;
		if (duration > 0.0f) stateTimer.AddDelay(duration);
	}

	inline float RandomPressureDuration() const
	{
		float base = 0.9f + RandomHelper::RandomFloat() * 1.6f;
		base -= std::min(timesParriedThisFight * 0.12f, 0.55f);
		return std::max(base, 0.35f);
	}

	// ── State machine ─────────────────────────────────────────────────────────
	void CommitAttack();
	void UpdateMeleeStateMachine();

	// ── Actions ───────────────────────────────────────────────────────────────
	void Stun();
	void DoNormalAttackDamage();
	void DoBreakAttackDamage();
	void HitNpcTarget(const Physics::HitResult& hit, float damage);
	void WarnTargetAboutAttack();
	void PerformMeleeAttack();
	void PerformBaitingMeleeAttack();
	void PerformBreakingAttack();
	void OnParried();
	void StartBlock();
	void StopBlock();

	// ── Queries ───────────────────────────────────────────────────────────────
	inline bool InMeleeAttackRange() const
	{
		Entity* t = Level::Current->FindEntityWithId(target_id);
		return t && glm::distance(Position, t->Position) < 2.0f;
	}

	inline bool CanAttackOrBlock() { return !isStunned(); }

	bool IsTargetBlocking() const;

	// The target holds something that can block or parry (a blade, a cane, a melee
	// NPC): the only reason to hold back. Whether it is doing so right now doesn't
	// matter; IsTargetBlocking covers that.
	bool IsTargetCountering() const;

	// ── Animation ─────────────────────────────────────────────────────────────
	void HandleAnimationEvent(const AnimationEvent& event) override;
	void UpdateAnimations(bool forceFullUpdate = false) override;

	// Convenience accessor – safe to call from derived classes that also use
	// a NpcMeleeAnimator (or subclass thereof).
	inline NpcMeleeAnimator* GetMeleeAnimator() const
	{
		return static_cast<NpcMeleeAnimator*>(animator.get());
	}

public:

	// Further than this from the target the NPC is not in the exchange yet: it runs
	// straight at the target. Inside it the footwork (circling, flanking) starts.
	static constexpr float EngageDistance = 3.5f;

	NpcGuardMelee();

	bool isStunned()                        override;
	bool IsPlayingRootMotionAnimation()     override;
	bool IsRotationAllowedDuringRootMotion() override;

	void WarnAboutAttack(Entity* from)      override;
	vec3 FindAttackLocation()               override;
	bool IsBlockingStance() const           override { return isCurrentlyBlocking; }

	// The melee attack body: the duel state machine and spine tracking. Run by
	// the melee duel tactic every frame.
	void RunMeleeAttack();

protected:

	std::shared_ptr<CombatBehaviorBase> CreateCombatBehavior() override;
};

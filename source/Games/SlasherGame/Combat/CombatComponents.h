#pragma once

#include <Combat/CombatTypes.h>
#include <Data/CharacterData.h>
#include <Data/MoveSetData.h>
#include <Data/StatusEffectData.h>

#include <Ecs/Ecs.h>
#include <EntityHandle.h>

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

// Combat data blocks (engine ECS components). Any entity can carry them - CombatCharacter declares them with
// ECS_COMPONENT, a destructible prop could carry only TeamComponent - and any code reads them through
// entity->GetComponent<T>(). Health is the engine's Entity::Health / MaxHealth, not a component.
// Per-frame data updates are TICK_COMPONENT systems in CombatComponents.cpp (PreUpdate).

class CombatCharacter;

// Marks a combat body. CombatBodyUpdateSystem (CombatBodySystems.cpp) runs the frame of every body from it, in
// parallel (COMBAT_DESIGN.md §2.2).
struct CombatBodyComponent
{
	CombatCharacter* body = nullptr;
};
REGISTER_COMPONENT(CombatBodyComponent)

struct TeamComponent
{
	Team team = Team::Neutral;
	Team originalTeam = Team::Neutral;   // team before possession / recruitment

	bool IsHostileTo(Team other) const { return GetAttitude(team, other) == Attitude::Hostile; }
};
REGISTER_COMPONENT(TeamComponent)

struct CombatStateComponent
{
	bool dead = false;
	bool invulnerable = false;           // move i-frame windows, finishers (written by the move runner)
	bool godMode = false;                // debug
	bool inFinisher = false;             // attacker or victim of a finisher
	TagMask activeMoveTags = 0;          // tags of the running primary move
	EntityHandle lastAttacker;
	double lastHitTime = -1000.0;

	// Per-entity hit-stop (local time dilation), on global game time (COMBAT_DESIGN.md §6.3). The body stops dead at
	// the contact (hitStopScale, a near-freeze), holds, and eases back to normal speed over the last share of the
	// duration: the contact reads as a hard impact, the release never pops.
	float hitStopDuration = 0.0f;        // 0 = none
	float hitStopElapsed = 0.0f;
	float hitStopScale = 1.0f;

	// Victim shake during its hit-stop: an offset of the mesh only (the capsule does not move). It starts displaced
	// along the hit direction, so the contact frame already shows the blow.
	glm::vec3 shakeDirection = glm::vec3(0.0f);
	float shakeAmplitude = 0.0f;
	float shakeDuration = 0.0f;
	float shakeElapsed = 0.0f;

	static constexpr float HitStopEaseOutShare = 0.3f; // share of the duration spent easing back to normal speed
	static constexpr float ShakeFrequency = 26.0f;     // Hz

	float GetHitStopDepth() const
	{
		if (hitStopDuration <= 0.0f)
			return 0.0f;
		const float easeOut = hitStopDuration * HitStopEaseOutShare;
		const float remaining = hitStopDuration - hitStopElapsed;
		return remaining < easeOut ? glm::smoothstep(0.0f, 1.0f, remaining / easeOut) : 1.0f;
	}

	float GetTimeDilation() const { return glm::mix(1.0f, hitStopScale, GetHitStopDepth()); }
	bool CanBeHit() const { return !dead && !invulnerable && !godMode; }

	// A hit during a hit-stop stops the body again (full depth) for whichever is longer: the rest of the current one
	// or the new one, at the deeper scale.
	void ApplyHitStop(float duration, float scale)
	{
		if (duration <= 0.0f)
			return;

		if (hitStopDuration > 0.0f)
		{
			hitStopScale = std::min(hitStopScale, scale);
			duration = std::max(duration, hitStopDuration - hitStopElapsed);
		}
		else
		{
			hitStopScale = scale;
		}
		hitStopDuration = duration;
		hitStopElapsed = 0.0f;
	}

	void ApplyShake(const glm::vec3& direction, float amplitude, float duration)
	{
		if (amplitude <= 0.0f || duration <= 0.0f)
			return;
		const bool shaking = shakeElapsed < shakeDuration;
		shakeDirection = direction;
		shakeAmplitude = shaking ? std::max(amplitude, shakeAmplitude) : amplitude;
		shakeDuration = duration;
		shakeElapsed = 0.0f;
	}

	glm::vec3 GetShakeOffset() const
	{
		if (shakeElapsed >= shakeDuration)
			return glm::vec3(0.0f);
		const float decay = 1.0f - shakeElapsed / shakeDuration;
		return shakeDirection * (shakeAmplitude * decay * std::cos(shakeElapsed * ShakeFrequency * glm::two_pi<float>()));
	}

	void Tick(float deltaTime)
	{
		if (hitStopDuration > 0.0f)
		{
			hitStopElapsed += deltaTime;
			if (hitStopElapsed >= hitStopDuration)
			{
				hitStopDuration = 0.0f;
				hitStopElapsed = 0.0f;
				hitStopScale = 1.0f;
			}
		}
		if (shakeElapsed < shakeDuration)
			shakeElapsed += deltaTime;
	}
};
REGISTER_COMPONENT(CombatStateComponent)

struct PoiseComponent
{
	bool enabled = true;
	float current = 40.0f;
	float max = 40.0f;
	float regenDelay = 2.0f;
	float regenRate = 15.0f;
	float breakDuration = 3.0f;
	float brokenDamageTaken = 1.5f;
	float firearmScale = 0.3f;

	float armorResist = 0.0f;            // current move's armor window (written by the move runner)
	float regenCooldown = 0.0f;
	float brokenRemaining = 0.0f;        // > 0: Broken (finishable)

	void Configure(const PoiseData& data);
	bool IsBroken() const { return brokenRemaining > 0.0f; }
	float GetFraction() const { return max > 0.0f ? current / max : 0.0f; }

	// Applies poise damage (armor considered unless ignoreArmor). Returns true if this broke poise.
	bool ApplyDamage(float amount, bool ignoreArmor);
	void Break();
	void EndBroken();                    // finisher consumed the Broken state, or it timed out
	void Tick(float deltaTime);
};
REGISTER_COMPONENT(PoiseComponent)

struct WeakPointState
{
	float health = 0.0f;
	float regrowRemaining = 0.0f;
	bool broken = false;
};

struct WeakPointsComponent
{
	std::shared_ptr<const CharacterData> definitions;  // CharacterData::weakPoints
	std::vector<WeakPointState> points;

	void Configure(const std::shared_ptr<const CharacterData>& data);
	int FindByHitbox(const hashed_string& hitbox) const;
	const WeakPointData* GetDefinition(int index) const;

	// Body damage multiplier from broken points with the DamageBonus effect.
	float GetBrokenDamageBonus() const;
	void Tick(float deltaTime);
};
REGISTER_COMPONENT(WeakPointsComponent)

struct ActiveStatusEffect
{
	int index = -1;
	float remaining = 0.0f;              // <= 0 with duration <= 0: infinite
	int stacks = 1;
};

struct StatusEffectsComponent
{
	std::shared_ptr<const StatusEffectTable> table;
	std::vector<ActiveStatusEffect> active;
	StatModifiers modifiers;             // aggregate of all active effects

	bool Add(const std::string& id);
	bool Add(int index);
	void Remove(int index);
	bool Has(int index) const;
	void Clear();
	void Tick(float deltaTime);
	void Recompute();
};
REGISTER_COMPONENT(StatusEffectsComponent)

// How many attackers of each kind may engage this entity at once. AI asks the *target* for a token before an
// attack and releases it after; leases expire so a holder that dies or forgets cannot block others.
struct AttackTokensComponent
{
	struct Holder
	{
		EntityHandle entity;
		AttackToken kind = AttackToken::None;
		double expireTime = 0.0;
	};

	std::array<int, static_cast<size_t>(AttackToken::Count)> capacity = { 0, 2, 2, 1 };
	std::vector<Holder> holders;

	// Thread-safe: bodies take and give back tokens on their targets from the parallel body update.
	void Configure(const AttackTokenCapacityData& data);
	bool TryAcquire(const EntityHandle& requester, AttackToken kind, double now, float lease);
	void Release(const EntityHandle& requester);
	bool IsHeldBy(const EntityHandle& requester) const;
	int CountHeld(AttackToken kind) const;
	void Tick(double now);
};
REGISTER_COMPONENT(AttackTokensComponent)

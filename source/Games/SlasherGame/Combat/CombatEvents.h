#pragma once

#include <Combat/CombatTypes.h>

#include <Ecs/Ecs.h>
#include <EntityHandle.h>
#include <glm.h>

#include <cstdint>
#include <string>

namespace HitFlags
{
	inline constexpr uint8_t IgnoreArmor        = 1 << 0;
	inline constexpr uint8_t IgnoreInvulnerable = 1 << 1;
	inline constexpr uint8_t FriendlyFire       = 1 << 2;
}

// THE combat funnel (COMBAT_DESIGN.md §6.1). Emitted on the main thread with EcsScheduler::Emit (bodies queue their
// hits during the parallel update and emit them in body order); resolved by the single handler ResolveHitRequest
// (CombatRules.cpp) when the PostUpdate tick group flushes its events, i.e. before the bodies' pose (PostAsyncUpdate).
struct HitRequestEvent
{
	EntityHandle attacker;
	EntityHandle victim;
	DamageSource source = DamageSource::Melee;
	float damage = 0.0f;
	float poiseDamage = 0.0f;
	float weakPointDamage = 0.0f;
	HitReaction reaction = HitReaction::Flinch;
	float knockback = 0.0f;              // horizontal push speed along direction (m/s)
	float launch = 0.0f;                 // upward speed (m/s)
	float hitStop = 0.0f;                // seconds attacker and victim stop at the contact
	float hitStopScale = 0.03f;          // time scale while stopped
	float shake = 0.0f;                  // m the victim's mesh shakes during its hit-stop
	float camera = 0.0f;                 // view kick if the player lands it (impact units)
	float impactScale = 1.0f;            // multiplies the victim's hit effect
	float airLift = 0.0f;                // in the air: the victim (and an airborne attacker) rise at least this fast (m/s)
	float airStall = 0.0f;               // s of air combat for a victim hit in the air or launched
	bool heavy = false;
	uint8_t flags = 0;                   // HitFlags
	int moveIndex = -1;                  // attacker's move (-1 = none)
	glm::vec3 point = glm::vec3(0.0f);
	glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);   // horizontal push, attacker -> victim
	// The hit effect sprays and the player's view kicks along it: the blade's sweep for melee, back out of the entry
	// wound for projectiles.
	glm::vec3 spray = glm::vec3(0.0f, 0.0f, 1.0f);
	std::string hitbox;                  // victim hitbox bone, empty if unknown
	std::string hitSound;                // feedback overrides from the move / projectile (empty: the victim's hitEffect)
	std::string hitParticle;
};
REGISTER_EVENT(HitRequestEvent)

// ---- Feedback notifications (emitted by ResolveHitRequest). Rules never depend on these. --------------

struct HitLandedEvent
{
	EntityHandle attacker;
	EntityHandle victim;
	DamageSource source = DamageSource::Melee;
	float damage = 0.0f;
	float poiseDamage = 0.0f;
	HitReaction reaction = HitReaction::None;   // what was actually played
	bool heavy = false;
	bool weakPointHit = false;
	bool weakPointBroken = false;
	bool poiseBroken = false;
	bool killed = false;
	float camera = 0.0f;
	float impactScale = 1.0f;
	glm::vec3 point = glm::vec3(0.0f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 spray = glm::vec3(0.0f, 0.0f, 1.0f);
	std::string hitSound;
	std::string hitParticle;
};
REGISTER_EVENT(HitLandedEvent)

struct DeathEvent
{
	EntityHandle victim;
	EntityHandle killer;
	glm::vec3 position = glm::vec3(0.0f);
};
REGISTER_EVENT(DeathEvent)

struct PoiseBrokenEvent
{
	EntityHandle victim;
	EntityHandle attacker;
	glm::vec3 position = glm::vec3(0.0f);
};
REGISTER_EVENT(PoiseBrokenEvent)

struct WeakPointBrokenEvent
{
	EntityHandle victim;
	EntityHandle attacker;
	std::string weakPoint;
	glm::vec3 point = glm::vec3(0.0f);
};
REGISTER_EVENT(WeakPointBrokenEvent)

struct FinisherEvent
{
	EntityHandle attacker;
	EntityHandle victim;
	bool completed = false;              // false = started
};
REGISTER_EVENT(FinisherEvent)

// Call once at startup, outside any tick group (GameStart). See CombatEvents.cpp for why.
void RegisterCombatEventTypes();

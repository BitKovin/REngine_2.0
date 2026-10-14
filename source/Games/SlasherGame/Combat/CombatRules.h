#pragma once

#include <Combat/CombatEvents.h>
#include <Combat/CombatTypes.h>
#include <Data/CharacterData.h>

#include <glm.h>

class Entity;

// Outcome of one resolved HitRequestEvent, handed to CombatCharacter::OnHitResolved / OnHitDealt.
struct HitResult
{
	const HitRequestEvent* request = nullptr;
	Entity* attacker = nullptr;          // may be null (environment) or already dead
	Entity* victim = nullptr;
	float healthDamage = 0.0f;
	float poiseDamage = 0.0f;
	bool poiseBroken = false;
	bool killed = false;
	bool weakPointHit = false;
	bool weakPointBroken = false;
	int weakPointIndex = -1;
	HitReaction reaction = HitReaction::None;   // reaction the victim should play
	glm::vec3 knockback = glm::vec3(0.0f);      // velocity: horizontal push + upward launch
};

// Combat rules (COMBAT_DESIGN.md §6). ResolveHit is the only code that changes a victim's combat state; it is
// invoked by the single EVENT_HANDLER for HitRequestEvent. The small pure functions are exposed for tests.
namespace CombatRules
{
	void ResolveHit(const HitRequestEvent& request);

	// Armor absorbs or downgrades reactions; heavy attacks upgrade Flinch to Stagger against unarmored bodies;
	// a Broken (finishable) body only reacts to Knockdown / Launch.
	HitReaction SelectReaction(HitReaction requested, bool heavy, float armorResist, bool victimBroken);

	float ScalePoiseDamage(float amount, DamageSource source, float firearmScale);

	// The reaction of a hit that breaks poise: a stagger - unless it knocks down or launches, which still happen (a
	// broken body reacts to those anyway). The PoiseBreak reaction move plays either way.
	HitReaction PoiseBreakReaction(HitReaction requested);

	// Melee target score (COMBAT_DESIGN.md §5.7), lower is better: the angle off the reference direction (0..1 for
	// 0..180 degrees) plus distanceWeight * distance / range, minus keepBonus for the previous attack's target.
	float ScoreAttackTarget(float angleDegrees, float distance, float range, bool previousTarget, const TargetingData& settings);
}

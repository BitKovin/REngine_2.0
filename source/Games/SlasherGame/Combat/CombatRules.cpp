#include "CombatRules.h"

#include <Characters/CombatCharacter.h>
#include <Combat/CombatComponents.h>

#include <Time.hpp>

#include <algorithm>

namespace
{
	constexpr float MinShakeDuration = 0.12f;   // s; short hit-stops still show a visible shake
}

namespace CombatRules
{
	HitReaction SelectReaction(HitReaction requested, bool heavy, float armorResist, bool victimBroken)
	{
		if (victimBroken)
			return (requested == HitReaction::Knockdown || requested == HitReaction::Launch) ? requested : HitReaction::None;

		HitReaction reaction = requested;
		if (heavy && armorResist <= 0.0f && reaction == HitReaction::Flinch)
			reaction = HitReaction::Stagger;

		if (armorResist >= 1.0f)
			return HitReaction::None;

		if (armorResist > 0.0f)
		{
			switch (reaction)
			{
			case HitReaction::Launch:
			case HitReaction::Knockdown: return HitReaction::Stagger;
			case HitReaction::Stagger:   return HitReaction::Flinch;
			default:                     return HitReaction::None;
			}
		}

		return reaction;
	}

	float ScalePoiseDamage(float amount, DamageSource source, float firearmScale)
	{
		return source == DamageSource::Firearm ? amount * firearmScale : amount;
	}

	HitReaction PoiseBreakReaction(HitReaction requested)
	{
		return (requested == HitReaction::Knockdown || requested == HitReaction::Launch) ? requested : HitReaction::Stagger;
	}

	float ScoreAttackTarget(float angleDegrees, float distance, float range, bool previousTarget, const TargetingData& settings)
	{
		const float angle = std::clamp(std::abs(angleDegrees), 0.0f, 180.0f) / 180.0f;
		const float near = range > 0.0f ? std::clamp(distance / range, 0.0f, 1.0f) : 0.0f;
		return angle + settings.distanceWeight * near - (previousTarget ? settings.keepBonus : 0.0f);
	}

	void ResolveHit(const HitRequestEvent& request)
	{
		EntityHandle victimHandle = request.victim;
		EntityHandle attackerHandle = request.attacker;
		Entity* victim = ResolveAlive(victimHandle);
		Entity* attacker = ResolveAlive(attackerHandle);
		if (victim == nullptr)
			return;

		auto state = victim->GetComponent<CombatStateComponent>();
		if (!state)
		{
			// Not a combatant (props, destructible brushes): the engine's own damage path.
			victim->OnPointDamage(request.damage, request.point, request.direction, request.hitbox, attacker, nullptr);
			return;
		}

		if (state->dead)
			return;

		if (!(request.flags & HitFlags::FriendlyFire) && attacker != nullptr && attacker != victim)
		{
			auto attackerTeam = attacker->GetComponent<TeamComponent>();
			auto victimTeam = victim->GetComponent<TeamComponent>();
			if (attackerTeam && victimTeam && GetAttitude(attackerTeam->team, victimTeam->team) == Attitude::Friendly)
				return;
		}

		if (!(request.flags & HitFlags::IgnoreInvulnerable) && (state->invulnerable || state->godMode))
			return;

		StatModifiers attackerModifiers;
		StatModifiers victimModifiers;
		if (attacker != nullptr)
			if (auto effects = attacker->GetComponent<StatusEffectsComponent>())
				attackerModifiers = effects->modifiers;
		if (auto effects = victim->GetComponent<StatusEffectsComponent>())
			victimModifiers = effects->modifiers;

		HitResult result;
		result.request = &request;
		result.attacker = attacker;
		result.victim = victim;

		float damage = request.damage * attackerModifiers.damageDealt * victimModifiers.damageTaken;
		const float poiseDamage = request.poiseDamage * attackerModifiers.poiseDamageDealt * victimModifiers.poiseDamageTaken;

		auto poise = victim->GetComponent<PoiseComponent>();
		const bool wasBroken = poise && poise->IsBroken();
		if (wasBroken)
			damage *= poise->brokenDamageTaken;

		// ---- Weak points ----
		bool breakPoiseFromWeakPoint = false;
		bool killFromWeakPoint = false;
		if (auto weakPoints = victim->GetComponent<WeakPointsComponent>())
		{
			if (!request.hitbox.empty())
			{
				const int index = weakPoints->FindByHitbox(hashed_string(request.hitbox));
				const WeakPointData* definition = weakPoints->GetDefinition(index);
				if (definition != nullptr)
				{
					WeakPointState& point = weakPoints->points[static_cast<size_t>(index)];
					result.weakPointHit = true;
					result.weakPointIndex = index;

					if (!point.broken)
					{
						damage *= definition->multiplier;
						if (definition->sourceMask & SourceBit(request.source))
						{
							point.health -= request.weakPointDamage * attackerModifiers.damageDealt;
							if (point.health <= 0.0f)
							{
								point.health = 0.0f;
								point.broken = true;
								point.regrowRemaining = definition->regrow;
								result.weakPointBroken = true;
								breakPoiseFromWeakPoint = definition->breakPoise;
								killFromWeakPoint = definition->kill;
							}
						}
					}
				}
			}
			damage *= weakPoints->GetBrokenDamageBonus();
		}

		// ---- Poise ----
		if (poise && poise->enabled && !wasBroken)
		{
			if (breakPoiseFromWeakPoint)
			{
				poise->Break();
				result.poiseBroken = true;
			}
			else
			{
				result.poiseDamage = ScalePoiseDamage(poiseDamage, request.source, poise->firearmScale);
				result.poiseBroken = poise->ApplyDamage(result.poiseDamage, (request.flags & HitFlags::IgnoreArmor) != 0);
			}
		}

		// ---- Health (engine Entity::Health) ----
		if (killFromWeakPoint)
			damage = std::max(damage, victim->Health);
		result.healthDamage = damage;
		victim->Health -= damage;
		if (victim->Health <= 0.0f)
		{
			victim->Health = 0.0f;
			result.killed = true;
		}

		// ---- Reaction + knockback ----
		const float armor = (poise && !(request.flags & HitFlags::IgnoreArmor)) ? poise->armorResist : 0.0f;
		if (result.killed)
			result.reaction = HitReaction::None;
		else if (result.poiseBroken)
			result.reaction = PoiseBreakReaction(request.reaction);
		else
			result.reaction = SelectReaction(request.reaction, request.heavy, armor, wasBroken);

		if (result.reaction != HitReaction::None || result.killed)
		{
			glm::vec3 direction = MathHelper::XZ(request.direction);
			direction = glm::dot(direction, direction) > 1e-6f ? glm::normalize(direction) : glm::vec3(0.0f);
			result.knockback = direction * request.knockback;
			if (result.reaction == HitReaction::Launch)
				result.knockback.y = request.launch;
		}

		// ---- Hit-stop for both fighters (a stop at the contact, eased release), the victim shakes through it ----
		state->ApplyHitStop(request.hitStop, request.hitStopScale);
		if (attacker != nullptr)
			if (auto attackerState = attacker->GetComponent<CombatStateComponent>())
				attackerState->ApplyHitStop(request.hitStop, request.hitStopScale);
		if (!result.killed)
		{
			glm::vec3 shakeDirection = MathHelper::XZ(request.direction);
			shakeDirection = glm::dot(shakeDirection, shakeDirection) > 1e-6f ? glm::normalize(shakeDirection) : glm::vec3(1.0f, 0.0f, 0.0f);
			state->ApplyShake(shakeDirection, request.shake, std::max(request.hitStop, MinShakeDuration));
		}

		// ---- Notify bodies (reactions, death, AI awareness) ----
		if (auto* character = dynamic_cast<CombatCharacter*>(victim))
			character->OnHitResolved(result);
		if (auto* attackerCharacter = dynamic_cast<CombatCharacter*>(attacker))
			attackerCharacter->OnHitDealt(result);

		// ---- Feedback notifications ----
		HitLandedEvent landed;
		landed.attacker = request.attacker;
		landed.victim = request.victim;
		landed.source = request.source;
		landed.damage = result.healthDamage;
		landed.poiseDamage = result.poiseDamage;
		landed.reaction = result.reaction;
		landed.heavy = request.heavy;
		landed.weakPointHit = result.weakPointHit;
		landed.weakPointBroken = result.weakPointBroken;
		landed.poiseBroken = result.poiseBroken;
		landed.killed = result.killed;
		landed.camera = request.camera;
		landed.impactScale = request.impactScale;
		landed.point = request.point;
		landed.direction = request.direction;
		landed.spray = request.spray;
		landed.hitSound = request.hitSound;
		landed.hitParticle = request.hitParticle;
		EcsScheduler::Emit(landed);

		if (result.poiseBroken)
			EcsScheduler::Emit(PoiseBrokenEvent{ request.victim, request.attacker, victim->Position });

		if (result.weakPointBroken)
		{
			const WeakPointData* definition = victim->GetComponent<WeakPointsComponent>()->GetDefinition(result.weakPointIndex);
			EcsScheduler::Emit(WeakPointBrokenEvent{ request.victim, request.attacker, definition ? definition->name : std::string(), request.point });
		}

		if (result.killed)
			EcsScheduler::Emit(DeathEvent{ request.victim, request.attacker, victim->Position });
	}
}

// The single handler of the combat funnel (COMBAT_DESIGN.md §6.1).
EVENT_HANDLER(ResolveHitRequest, HitRequestEvent)
{
	CombatRules::ResolveHit(e);
}

#include "CombatComponents.h"

#include <Time.hpp>

#include <algorithm>
#include <mutex>

namespace
{
	// Tokens live on the target and are taken by many attackers at once (parallel body update); one lock for all
	// token tables keeps them consistent. Contention is negligible: a few calls per attack decision.
	std::mutex& TokenMutex()
	{
		static std::mutex mutex;
		return mutex;
	}
}

// ---- PoiseComponent ---------------------------------------------------------------------------------

void PoiseComponent::Configure(const PoiseData& data)
{
	enabled = data.enabled;
	max = data.max;
	current = data.max;
	regenDelay = data.regenDelay;
	regenRate = data.regenRate;
	breakDuration = data.breakDuration;
	brokenDamageTaken = data.brokenDamageTaken;
	firearmScale = data.firearmScale;
	armorResist = 0.0f;
	regenCooldown = 0.0f;
	brokenRemaining = 0.0f;
}

bool PoiseComponent::ApplyDamage(float amount, bool ignoreArmor)
{
	if (!enabled || IsBroken() || amount <= 0.0f)
		return false;

	const float resist = ignoreArmor ? 0.0f : std::clamp(armorResist, 0.0f, 1.0f);
	current -= amount * (1.0f - resist);
	regenCooldown = regenDelay;

	if (current > 0.0f)
		return false;

	Break();
	return true;
}

void PoiseComponent::Break()
{
	if (!enabled)
		return;
	current = 0.0f;
	brokenRemaining = breakDuration;
	regenCooldown = 0.0f;
}

void PoiseComponent::EndBroken()
{
	brokenRemaining = 0.0f;
	current = max;
}

void PoiseComponent::Tick(float deltaTime)
{
	if (!enabled)
		return;

	if (brokenRemaining > 0.0f)
	{
		brokenRemaining -= deltaTime;
		if (brokenRemaining <= 0.0f)
			EndBroken();
		return;
	}

	if (regenCooldown > 0.0f)
	{
		regenCooldown -= deltaTime;
		return;
	}

	current = std::min(max, current + regenRate * deltaTime);
}

// ---- WeakPointsComponent ----------------------------------------------------------------------------

void WeakPointsComponent::Configure(const std::shared_ptr<const CharacterData>& data)
{
	definitions = data;
	points.clear();
	if (!data)
		return;

	points.resize(data->weakPoints.size());
	for (size_t i = 0; i < points.size(); ++i)
		points[i].health = data->weakPoints[i].health;
}

int WeakPointsComponent::FindByHitbox(const hashed_string& hitbox) const
{
	return definitions ? definitions->FindWeakPointByHitbox(hitbox) : -1;
}

const WeakPointData* WeakPointsComponent::GetDefinition(int index) const
{
	if (!definitions || index < 0 || index >= static_cast<int>(definitions->weakPoints.size()))
		return nullptr;
	return &definitions->weakPoints[static_cast<size_t>(index)];
}

float WeakPointsComponent::GetBrokenDamageBonus() const
{
	float bonus = 1.0f;
	for (size_t i = 0; i < points.size(); ++i)
	{
		const WeakPointData* def = GetDefinition(static_cast<int>(i));
		if (def && def->damageBonus && points[i].broken)
			bonus *= def->bonusDamageTaken;
	}
	return bonus;
}

void WeakPointsComponent::Tick(float deltaTime)
{
	for (size_t i = 0; i < points.size(); ++i)
	{
		WeakPointState& point = points[i];
		if (!point.broken || point.regrowRemaining <= 0.0f)
			continue;

		point.regrowRemaining -= deltaTime;
		if (point.regrowRemaining <= 0.0f)
		{
			const WeakPointData* def = GetDefinition(static_cast<int>(i));
			point.broken = false;
			point.regrowRemaining = 0.0f;
			point.health = def ? def->health : 0.0f;
		}
	}
}

// ---- StatusEffectsComponent -------------------------------------------------------------------------

bool StatusEffectsComponent::Add(const std::string& id)
{
	if (!table)
		return false;
	return Add(table->Find(id));
}

bool StatusEffectsComponent::Add(int index)
{
	const StatusEffectData* data = table ? table->Get(index) : nullptr;
	if (!data)
		return false;

	for (ActiveStatusEffect& effect : active)
	{
		if (effect.index != index)
			continue;

		switch (data->stacking)
		{
		case EffectStacking::Ignore:
			return false;
		case EffectStacking::Stack:
			effect.stacks = std::min(effect.stacks + 1, data->maxStacks);
			effect.remaining = data->duration;
			break;
		default: // Refresh
			effect.remaining = data->duration;
			break;
		}
		Recompute();
		return true;
	}

	active.push_back({ index, data->duration, 1 });
	Recompute();
	return true;
}

void StatusEffectsComponent::Remove(int index)
{
	const auto before = active.size();
	active.erase(std::remove_if(active.begin(), active.end(), [index](const ActiveStatusEffect& e) { return e.index == index; }), active.end());
	if (active.size() != before)
		Recompute();
}

bool StatusEffectsComponent::Has(int index) const
{
	return std::any_of(active.begin(), active.end(), [index](const ActiveStatusEffect& e) { return e.index == index; });
}

void StatusEffectsComponent::Clear()
{
	active.clear();
	Recompute();
}

void StatusEffectsComponent::Tick(float deltaTime)
{
	bool changed = false;
	for (size_t i = 0; i < active.size();)
	{
		const StatusEffectData* data = table ? table->Get(active[i].index) : nullptr;
		if (data && data->duration > 0.0f)
		{
			active[i].remaining -= deltaTime;
			if (active[i].remaining <= 0.0f)
			{
				active.erase(active.begin() + static_cast<std::ptrdiff_t>(i));
				changed = true;
				continue;
			}
		}
		++i;
	}

	if (changed)
		Recompute();
}

void StatusEffectsComponent::Recompute()
{
	modifiers = StatModifiers();
	for (const ActiveStatusEffect& effect : active)
	{
		const StatusEffectData* data = table ? table->Get(effect.index) : nullptr;
		if (!data)
			continue;
		for (int stack = 0; stack < effect.stacks; ++stack)
			modifiers.Combine(data->modifiers);
	}
}

// ---- AttackTokensComponent --------------------------------------------------------------------------

void AttackTokensComponent::Configure(const AttackTokenCapacityData& data)
{
	std::lock_guard lock(TokenMutex());
	capacity[static_cast<size_t>(AttackToken::None)] = 0;
	capacity[static_cast<size_t>(AttackToken::Melee)] = data.melee;
	capacity[static_cast<size_t>(AttackToken::Ranged)] = data.ranged;
	capacity[static_cast<size_t>(AttackToken::Heavy)] = data.heavy;
	holders.clear();
}

bool AttackTokensComponent::TryAcquire(const EntityHandle& requester, AttackToken kind, double now, float lease)
{
	if (kind == AttackToken::None)
		return true;

	std::lock_guard lock(TokenMutex());

	for (Holder& holder : holders)
	{
		if (holder.entity == requester)
		{
			if (holder.kind == kind)
			{
				holder.expireTime = now + lease;   // renew
				return true;
			}
			holder.kind = AttackToken::None;       // switching kind: give the old token back first
		}
	}
	holders.erase(std::remove_if(holders.begin(), holders.end(), [](const Holder& h) { return h.kind == AttackToken::None; }), holders.end());

	const auto held = std::count_if(holders.begin(), holders.end(), [kind](const Holder& h) { return h.kind == kind; });
	if (held >= capacity[static_cast<size_t>(kind)])
		return false;

	holders.push_back({ requester, kind, now + lease });
	return true;
}

void AttackTokensComponent::Release(const EntityHandle& requester)
{
	std::lock_guard lock(TokenMutex());
	holders.erase(std::remove_if(holders.begin(), holders.end(), [&](const Holder& h) { return h.entity == requester; }), holders.end());
}

bool AttackTokensComponent::IsHeldBy(const EntityHandle& requester) const
{
	std::lock_guard lock(TokenMutex());
	return std::any_of(holders.begin(), holders.end(), [&](const Holder& h) { return h.entity == requester; });
}

int AttackTokensComponent::CountHeld(AttackToken kind) const
{
	std::lock_guard lock(TokenMutex());
	return static_cast<int>(std::count_if(holders.begin(), holders.end(), [kind](const Holder& h) { return h.kind == kind; }));
}

void AttackTokensComponent::Tick(double now)
{
	std::lock_guard lock(TokenMutex());
	holders.erase(std::remove_if(holders.begin(), holders.end(), [now](Holder& h)
	{
		if (h.expireTime < now)
			return true;
		Entity* entity = ResolveAlive(h.entity);
		if (!entity)
			return true;
		auto state = entity->GetComponent<CombatStateComponent>();
		return state && state->dead;
	}), holders.end());
}

// ---- Systems ----------------------------------------------------------------------------------------
// PreUpdate: data is current before entities run their Update.

TICK_COMPONENT(HitStopSystem, PreUpdate, CombatStateComponent, state)
{
	state.Tick(ctx.DeltaTime);
}
REGISTER_TICK(HitStopSystem)

TICK_COMPONENT(PoiseSystem, PreUpdate, PoiseComponent, poise)
{
	poise.Tick(ctx.DeltaTime);
}
REGISTER_TICK(PoiseSystem)

TICK_COMPONENT(WeakPointRegrowSystem, PreUpdate, WeakPointsComponent, weakPoints)
{
	weakPoints.Tick(ctx.DeltaTime);
}
REGISTER_TICK(WeakPointRegrowSystem)

TICK_COMPONENT(StatusEffectSystem, PreUpdate, StatusEffectsComponent, effects)
{
	effects.Tick(ctx.DeltaTime);
}
REGISTER_TICK(StatusEffectSystem)

TICK_COMPONENT(AttackTokenSystem, PreUpdate, AttackTokensComponent, tokens)
{
	tokens.Tick(Time::GameTime);
}
REGISTER_TICK(AttackTokenSystem)

#include "RangedFireControl.h"

#include "CombatCoordinator.h"
#include "../NpcBase.h"
#include "CombatTuning.h"

#include <RandomHelper.h>

void RangedFireControl::Tick(const CombatContext& ctx, float deltaTime)
{
	const double now = Time::GameTime;

	if (magazine < 0)
		magazine = CombatTuning::Get().fire.magazineSize;

	const bool engaged = ctx.target.Valid();
	const bool sees = engaged && ctx.owner->SeesTarget();

	if (engaged) lastEngagedTime = now;

	// Line of sight timer. A short loss keeps the ramp; a long one is a new acquisition.
	if (sees)
	{
		losTime += deltaTime;
		unseenTime = 0.0f;
	}
	else
	{
		unseenTime += deltaTime;

		if (unseenTime > CombatTuning::Get().fire.losResetTime)
		{
			losTime = 0.0f;
			shotsSinceAcquire = 0;
		}
	}

	// Reload
	if (reloading && now >= reloadEnd)
	{
		reloading = false;
		magazine = CombatTuning::Get().fire.magazineSize;
	}
	else if (!reloading && magazine <= 0)
	{
		StartReload(ctx);
	}
	else if (!engaged && !reloading && magazine < CombatTuning::Get().fire.magazineSize && now - lastEngagedTime > CombatTuning::Get().fire.idleRefillDelay)
	{
		magazine = CombatTuning::Get().fire.magazineSize;
	}

	wantsToken = engaged && now - lastExposedTime < CombatTuning::Get().fire.tokenExposureGrace && !ctx.meleeMode && ctx.target.isPlayer && !reloading && !ctx.target.isStunned && !ctx.owner->isStunned();
}

void RangedFireControl::StartReload(const CombatContext& ctx)
{
	reloading = true;
	reloadEnd = Time::GameTime + CombatTuning::Get().fire.reloadTime;

	ctx.owner->PlayPhrace("combat_reloading");
}

bool RangedFireControl::TryShoot(const CombatContext& ctx)
{
	if (reloading || magazine <= 0 || !ctx.target.Valid()) return false;

	const double now = Time::GameTime;

	// A new burst starts with the weapon coming up: a readable pause before the first shot.
	if (now - lastShotTime > CombatTuning::Get().fire.burstGap)
	{
		if (!aimingUp)
		{
			aimingUp = true;
			aimUpUntil = now + CombatTuning::Get().fire.aimUpTime;
		}

		if (now < aimUpUntil) return false;
		aimingUp = false;
	}

	NpcBase* body = ctx.owner;
	const bool isPlayer = ctx.target.isPlayer;
	const bool offscreen = isPlayer && !Camera::frustum.IsSphereVisible(body->Position, 1.0f);

	float chance = HitChance(ctx, offscreen);

	// Only attack token holders shoot to hit the player; the rest keep him honest.
	if (isPlayer && !hasToken)
		chance = std::min(chance, CombatTuning::Get().fire.suppressedAccuracy);

	bool hit = RandomHelper::RandomFloat() < chance;

	// The first shot after (re)acquiring the player always misses: the warning shot.
	if (isPlayer && shotsSinceAcquire == 0)
		hit = false;

	// A hit that would exceed the player's damage budget this second is a near miss.
	if (hit && isPlayer && !CombatCoordinator::TryConsumeDamageBudget(CombatTuning::Get().shot.bulletDamage))
		hit = false;

	const vec3 aimPoint = body->ComputeRangedAimPoint();
	const vec3 scatter = hit
		? RandomHelper::RandomPosition(CombatTuning::Get().fire.hitSpread)
		: MissOffset(body->Position, aimPoint);

	body->FireShot(aimPoint + scatter);

	magazine--;
	shotsSinceAcquire++;
	lastShotTime = now;

	return true;
}

float RangedFireControl::HitChance(const CombatContext& ctx, bool offscreen) const
{
	float chance = ctx.target.isPlayer ? CombatTuning::Get().fire.playerAccuracy : CombatTuning::Get().fire.npcAccuracy;

	// Incoming fire spoils the aim.
	chance *= 1.0f - glm::clamp(ctx.suppression, 0.0f, 1.0f);

	// Accuracy builds up the longer the shooter keeps line of sight.
	const float rampTime = CombatTuning::Get().fire.rampTime * (offscreen ? CombatTuning::Get().fire.offscreenRampScale : 1.0f);
	const float ramp = glm::clamp(losTime / rampTime, 0.0f, 1.0f);
	chance *= glm::mix(CombatTuning::Get().fire.rampStart, 1.0f, ramp);

	if (glm::length(MathHelper::XZ(ctx.owner->GetControllerVelocity())) > 1.0f)
		chance *= CombatTuning::Get().fire.movingFactor;

	if (offscreen)
		chance *= CombatTuning::Get().fire.offscreenFactor;

	if (Time::GameTime < alertUntil)
		chance *= CombatTuning::Get().fire.holderAccuracyFactor;

	const float distance = glm::distance(ctx.owner->Position, ctx.target.position);
	const float falloff = glm::clamp((distance - CombatTuning::Get().fire.falloffNear) / (CombatTuning::Get().fire.falloffFar - CombatTuning::Get().fire.falloffNear), 0.0f, 1.0f);
	chance *= glm::mix(1.0f, CombatTuning::Get().fire.falloffMin, falloff);

	return glm::clamp(chance, 0.0f, 1.0f);
}

// A miss lands beside the target, not through it: sideways first, a little up or down.
vec3 RangedFireControl::MissOffset(const vec3& from, const vec3& aimPoint) const
{
	const vec3 toAim = MathHelper::Normalized(aimPoint - from);
	vec3 side = glm::cross(toAim, vec3(0, 1, 0));

	if (glm::length(side) < 0.001f) side = vec3(1, 0, 0);
	side = MathHelper::Normalized(side);

	const float magnitude = CombatTuning::Get().fire.missMin + RandomHelper::RandomFloat() * (CombatTuning::Get().fire.missMax - CombatTuning::Get().fire.missMin);
	const float direction = RandomHelper::RandomFloat() < 0.5f ? -1.0f : 1.0f;
	const float vertical = (RandomHelper::RandomFloat() - 0.5f) * 0.6f;

	return side * magnitude * direction + vec3(0, vertical, 0);
}

void RangedFireControl::NoteExposed()
{
	lastExposedTime = Time::GameTime;
}

bool RangedFireControl::ExposedWithin(double seconds) const
{
	return Time::GameTime - lastExposedTime < seconds;
}

void RangedFireControl::OnHoldReaction()
{
	alertUntil = Time::GameTime + CombatTuning::Get().fire.holderAlertTime;

	if (shotsSinceAcquire < 1) shotsSinceAcquire = 1;
	losTime = std::max(losTime, CombatTuning::Get().fire.rampTime * 0.5f);
}

bool RangedFireControl::TrySuppressShot(const CombatContext& ctx, const vec3& point)
{
	if (reloading || magazine <= 0) return false;

	NpcBase* body = ctx.owner;
	body->FireShot(point + MissOffset(body->Position, point));

	magazine--;
	lastShotTime = Time::GameTime;

	return true;
}

void RangedFireControl::OnDamaged()
{
	losTime *= CombatTuning::Get().fire.damagedRampFactor;
}

float RangedFireControl::TokenBid(double now) const
{
	// Shooters that have waited longer win the next free token.
	const double waited = hasToken ? 0.0 : std::max(0.0, now - tokenLostSince);
	float bid = 1.0f + 0.2f * static_cast<float>(waited);

	// A holder that has stepped back into hiding is the first to be replaced.
	if (hasToken && now - lastExposedTime > 0.3)
		bid *= 0.5f;

	return bid;
}

void RangedFireControl::SetToken(bool granted, double now)
{
	if (granted == hasToken) return;

	hasToken = granted;

	if (granted) tokenHeldSince = now;
	else tokenLostSince = now;
}

nlohmann::json RangedFireControl::Serialize() const
{
	nlohmann::json data = nlohmann::json::object();

	data["magazine"] = magazine;
	data["reloadRemaining"] = reloading ? std::max(0.0, reloadEnd - Time::GameTime) : -1.0;
	data["losTime"] = losTime;
	data["shotsSinceAcquire"] = shotsSinceAcquire;

	return data;
}

void RangedFireControl::Deserialize(const nlohmann::json& data)
{
	magazine = data.value("magazine", -1);
	losTime = data.value("losTime", 0.0f);
	shotsSinceAcquire = data.value("shotsSinceAcquire", 0);

	const double remaining = data.value("reloadRemaining", -1.0);
	reloading = remaining >= 0.0;
	reloadEnd = Time::GameTime + std::max(0.0, remaining);
}

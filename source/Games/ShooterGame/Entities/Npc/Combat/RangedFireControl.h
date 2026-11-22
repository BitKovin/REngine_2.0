#pragma once

#include "CombatTacticBase.h"

#include <json.hpp>

// Everything that decides whether, when and how accurately a gun NPC shoots:
// magazine and reload, the accuracy ramp while it keeps line of sight, the
// aim-up pause before a burst, and the fairness rules against the player
// (attack tokens, first shot misses, damage budget). Owned by the brain so
// the state survives tactic switches. See COMBAT_AI_DESIGN.md §9.
//
// Accuracy terms still to come with their milestones: the
// target's speed and cover exposure.
class RangedFireControl
{
public:
	// The tactic reports every frame the NPC is in a position to shoot.
	void NoteExposed();

	// Was the NPC in a position to shoot within the last `seconds`?
	bool ExposedWithin(double seconds) const;


	// Per tick: line-of-sight timer, reload progress, silent refill.
	void Tick(const CombatContext& ctx, float deltaTime);

	// Fires one shot if the aim-up pause is over and the weapon is loaded.
	// The caller has already passed NpcBase::ReadyToFireRanged().
	bool TryShoot(const CombatContext& ctx);

	void OnDamaged();

	// The target showed up at the angle this NPC was holding: it shoots at once,
	// without the usual first-shot miss, and a bit better for a while.
	void OnHoldReaction();

	// A suppressing shot at a point (a hidden target's cover): no token, no
	// damage budget, no hit roll; it lands beside the point like a miss.
	bool TrySuppressShot(const CombatContext& ctx, const vec3& point);

	bool IsReloading() const { return reloading; }

	// Seconds since line of sight to the target was last present.
	float UnseenTime() const { return unseenTime; }
	int Magazine() const { return magazine; }

	// ── Attack tokens (granted by CombatCoordinator) ──────────────────────────
	bool WantsToken() const { return wantsToken; }
	bool HasToken() const { return hasToken; }
	float TokenBid(double now) const;
	double TokenHeldSince() const { return tokenHeldSince; }
	void SetToken(bool granted, double now);

	nlohmann::json Serialize() const;
	void Deserialize(const nlohmann::json& data);

private:
	int magazine = -1; // -1 until the first tick fills it
	bool reloading = false;
	double reloadEnd = 0.0;

	double lastShotTime = -100000.0;
	double lastEngagedTime = -100000.0;
	bool aimingUp = false;
	double aimUpUntil = 0.0;

	float losTime = 0.0f;    // continuous line of sight, seconds
	float unseenTime = 0.0f; // time since line of sight was last present
	int shotsSinceAcquire = 0;
	double alertUntil = 0.0;
	double lastExposedTime = -100000.0;

	bool wantsToken = false;
	bool hasToken = false;
	double tokenHeldSince = 0.0;
	double tokenLostSince = 0.0;

	void StartReload(const CombatContext& ctx);
	float HitChance(const CombatContext& ctx, bool offscreen) const;
	vec3 MissOffset(const vec3& from, const vec3& aimPoint) const;
};

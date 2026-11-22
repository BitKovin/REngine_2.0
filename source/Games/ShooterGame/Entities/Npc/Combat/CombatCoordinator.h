#pragma once

#include "ApproachMap.h"
#include "TargetBelief.h"

#include <glm.h>

#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class CombatBehaviorBase;

// Cross-NPC combat rules (attack tokens, melee slots, roles, cover
// reservations, squad belief, voice line arbitration). See
// COMBAT_AI_DESIGN.md §8.
//
// Mutated on the main thread only. Step() runs once per frame from the first
// NpcBase::Update; brains running in AsyncUpdate read the state it published
// and never write it. The one exception is the damage budget, a lock-free
// leaky bucket the brains draw from when they decide a shot hits the player.
//
// Implemented: attack tokens, the player damage budget, squad belief, the
// squad plan (flank / suppress / hold / push / check roles), melee slots and
// cover reservations.
class CombatCoordinator
{
public:

	// Called by CombatBehaviorBase's constructor / destructor.
	static void Register(CombatBehaviorBase* brain);
	static void Unregister(CombatBehaviorBase* brain);

	// Safe to call from every NPC; resolves at most once per engine frame.
	static void Step();

	// Worker-thread safe. Returns false when this much more damage would
	// exceed the budget; the caller turns the shot into a near miss.
	static bool TryConsumeDamageBudget(float damage);

	// ── Squad belief (§5) ────────────────────────────────────────────────────
	// One belief per target, shared by every NPC engaged on it (the split into
	// separate squads per target of §8.1 is not done yet). Null when no NPC is
	// engaged on that target.
	static const TargetBelief* FindBelief(const std::string& targetId);

	// ── Cover reservations ────────────────────────────────────────────────────
	// One NPC per spot (spots closer than tokens.coverSpacing count as one).
	// Reserving replaces the brain's previous reservation. Worker-thread safe.
	static bool ReserveCover(const CombatBehaviorBase* brain, const vec3& floor);
	static void ReleaseCover(const CombatBehaviorBase* brain);

	// Draws the target's approach map (lanes, entries, stack spots). Main thread.
	static void DrawApproachMap(const std::string& targetId);

	static int BrainCount() { return (int)brains.size(); }
	static int EngagedCount() { return engagedCount; }
	static float DamageBudgetUsed() { return damageBudgetUsed.load(); }

private:
	static inline std::vector<CombatBehaviorBase*> brains;
	static inline int engagedCount = 0;
	static inline unsigned long lastStepFrame = ~0ul;
	static inline std::atomic<float> damageBudgetUsed{ 0.0f };

	struct CoverReservation
	{
		const CombatBehaviorBase* brain;
		vec3 floor;
	};
	static inline std::vector<CoverReservation> coverReservations;
	static inline std::mutex coverMutex;

	static inline std::unordered_map<std::string, TargetBelief> beliefs;
	static inline std::unordered_map<std::string, ApproachMap> approachMaps;

	static void UpdateBeliefs();
	static void ResolveAttackTokens();
	static void ResolveMeleeSlots();
};

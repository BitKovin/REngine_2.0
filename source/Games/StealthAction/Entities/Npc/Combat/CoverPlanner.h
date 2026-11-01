#pragma once

#include "CombatTacticBase.h"
#include "CoverTypes.h"

class CombatBehaviorBase;

// Keeps one cover spot per NPC: searches for it, reserves it, notices when the
// target has flanked it, and tells the tactics whether the NPC is standing in
// it. Owned by the brain so the assignment survives tactic switches.
// See COMBAT_AI_DESIGN.md §10.
class CoverPlanner
{
public:

	explicit CoverPlanner(CombatBehaviorBase* brain) : brain(brain) {}

	// wantCover false drops the current spot and its reservation.
	void Update(const CombatContext& ctx, bool wantCover);

	void Release();

	// Drops the spot and never picks it again soon: peeking from it doesn't see the target.
	void Abandon();

	// The squad plan's "move up while he reloads" (CombatCoordinator, main thread).
	void RequestAdvance() { if (spot.Valid()) advanceRequested = true; }

	bool HasCover() const { return spot.Valid(); }
	bool Arrived() const { return spot.Valid() && arrived; }
	const CoverSpot& Spot() const { return spot; }

private:
	CombatBehaviorBase* brain = nullptr;

	CoverSpot spot;
	bool arrived = false;
	double arrivedSince = 0.0;
	double stayTime = 0.0;

	bool hasPrevious = false;
	vec3 previousFloor{};

	bool advanceRequested = false;
	double lastAdvanceTime = -1000.0;

	bool InCoverArea(const vec3& position) const;
	bool Search(const CombatContext& ctx, bool advance, CoverSpot& found) const;
	double nextSearch = 0.0;
	double nextValidate = 0.0;
};

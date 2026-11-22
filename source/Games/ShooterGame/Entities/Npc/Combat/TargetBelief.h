#pragma once

#include <glm.h>

#include <string>

class CombatBehaviorBase;

// What the squad believes about one target (COMBAT_AI_DESIGN.md §5).
//
//   Visible    someone sees the target
//   Concealed  just lost: "he's behind that cover", no evidence it left
//   Lost       evidence it left, or concealment timed out: check the spot, then search
//   None       nothing known (never seen yet)
enum class TargetContact
{
	None,
	Visible,
	Concealed,
	Lost
};

// What the squad plan (CombatCoordinator, COMBAT_AI_DESIGN.md §8.3) wants one
// gun NPC to do. None = fight normally (cover fight, strafe, kite): the base of
// fire that keeps the target busy.
//
//   Flank     go round to the side of the squad axis given by `side`
//   Suppress  target hidden: keep shooting at where it was last seen
//   Hold      target hidden: keep the gun on the spot, shoot when it shows
//   Push      target hidden: move up on the spot for a line of sight
//   Check     target lost: walk up to the spot
//   Watch     target lost: cover the checker
enum class SquadRole
{
	None,
	Flank,
	Suppress,
	Hold,
	Push,
	Check,
	Watch
};

const char* SquadRoleName(SquadRole role);

// Where an NPC with a lane order is in it (§8.7), reported by its tactic.
enum class LanePhase
{
	Moving,    // on the way to the stack spot
	Stacked,   // waiting at it, hidden, gun on the entry
	Breaching, // going in through the entry
	Done       // in (or gave up): back to the base of fire
};

const char* LanePhaseName(LanePhase phase);

struct SquadOrder
{
	SquadRole role = SquadRole::None;

	// Flank: -1 or +1, which side of the squad axis to come round on.
	float side = 0.0f;

	// From the target toward the squad's gunners (XZ, normalized): the line the
	// squad attacks along. Flankers swing away from it.
	vec3 axis{ 1, 0, 0 };

	double since = 0.0;
	double until = 0.0; // Flank: given up after this

	// Lane orders (§8.7): come at the target through a doorway or corridor of the
	// approach map. Flankers and pushers stack hidden short of the entry and go
	// in together when the squad breaches; the checker of a lost target goes in
	// through a lane other than the front one.
	bool hasLane = false;
	vec3 entry{};          // where the lane comes into the target's sight
	vec3 stack{};          // hidden spot short of it (this NPC's place in the stack)
	bool breach = false;   // go in now
	bool waitTurn = false; // pushers: hold, the one ahead is moving up
};

struct TargetBelief
{
	static constexpr int PeekPointCount = 3;

	std::string targetId;
	TargetContact contact = TargetContact::None;

	// Where the target was when someone last saw it.
	vec3 lastVisiblePosition{};
	vec3 lastVisibleVelocity{};
	double lastVisibleTime = 0.0;

	// Where the squad thinks it is hiding while Concealed or Lost, and how
	// unsure it is of that (grows over time without evidence).
	vec3 estimatePosition{};
	float estimateRadius = 0.0f;

	// Eye-height points the target can reappear at (over the cover, either side).
	vec3 peekPoints[PeekPointCount]{};

	double concealedSince = 0.0;
	double concealedUntil = 0.0;
	double nextTrackTime = 0.0;   // combat awareness refresh (§5.7)
	double seekUntil = 0.0;       // the target turned up somewhere else: the squad goes to find it until then
	double lostSince = 0.0;

	// Seconds somebody has had a clear view of the estimate without finding the target.
	float emptyTime = 0.0f;

	// Visible only: seconds NPCs that were out in the open to shoot (or in melee)
	// have looked without seeing the target. NPCs hiding by choice don't count.
	float blindExposedTime = 0.0f;

	int memberCount = 0;

	// Gun NPCs out in the open shooting right now (peeking, strafing, kiting).
	int exposedCount = 0;
	bool targetIsPlayer = false;

	// ── Squad plan state (§8.3); the orders themselves live in the brains ─────
	double fightSince = 0.0;      // the squad first saw the target this fight

	// Settling (§8.8): is the target holding its ground or running?
	vec3 anchorPosition{};        // where the target has been staying
	double anchorSince = 0.0;
	bool targetHolding = false;   // within squad.anchorRadius of the anchor for squad.holdTime
	bool targetRetreating = false; // its last move took it away from the squad
	bool squadSettled = false;    // the base of fire has taken its positions
	bool flankUnderWay = false;   // flankers out, breach not called yet: the base of fire is patient
	double nextFlankTime = 0.0;   // no new flanker before this
	double suppressUntil = 0.0;   // Concealed: the suppressors pin the spot until then
	double nextPlanTime = 0.0;    // roles are re-planned a few times a second
	bool targetWasReloading = false;
	vec3 squadAxis{ 1, 0, 0 };    // from the target toward the squad's gunners

	// ── Lanes and the breach (§8.7) ──────────────────────────────────────────
	bool hasFrontLane = false;     // the way the squad fights the target through
	vec3 frontEntry{};
	vec3 frontStack{};
	vec3 frontDirection{};
	double firstLaneOrderAt = 0.0; // 0 = no lane assault under way
	double firstStackedAt = 0.0;
	bool breachCalled = false;
	double assaultUntil = 0.0;     // breach: the base of fire is all out shooting until then

	double ConcealedFor(double now) const { return now - concealedSince; }
};

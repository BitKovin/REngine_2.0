#pragma once

#include "CombatTargetView.h"
#include "TargetBelief.h"

#include <json.hpp>

class NpcBase;
class RangedFireControl;
class CoverPlanner;
class CombatBehaviorBase;

// Inputs a tactic scores itself against. Built once per brain tick.
struct CombatContext
{
	NpcBase* owner = nullptr;
	CombatTargetView target;
	bool meleeMode = false; // owner currently fights with its melee weapon
	RangedFireControl* fire = nullptr; // the brain's gun state (magazine, accuracy, tokens)
	CoverPlanner* cover = nullptr;     // the brain's cover assignment

	float suppression = 0.0f; // 0..1+: how much incoming fire has pinned this NPC down
	bool flinched = false;    // took a hit moments ago

	// ── What the squad believes (§5) ──────────────────────────────────────────
	const CombatBehaviorBase* brain = nullptr;
	const TargetBelief* belief = nullptr; // null when the coordinator has none for this target
	TargetContact contact = TargetContact::None;

	// Where the threat is believed to be: the target itself while visible, the
	// squad's estimate while concealed or lost. Cover and aiming use these, never
	// the target's real position, so a hidden player can sneak away.
	vec3 threatPosition{};
	vec3 threatEye{};

	// This NPC's part in the squad plan (§8.3), and shorthands for it.
	SquadOrder squad;
	bool isSuppressor = false;
	bool isFlanker = false;
	bool isPusher = false;
	bool isChecker = false;
	bool isWatcher = false;
	bool isHolder = false;

	// Squadmates (not counting this NPC) out in the open shooting right now.
	int squadExposed = 0;

	// Melee slots against the player (§8.2).
	bool wantsMeleeSlot = false;
	bool hasMeleeSlot = false;

	bool Concealed() const { return contact == TargetContact::Concealed; }
	bool Lost() const { return contact == TargetContact::Lost; }
	bool Hunting() const { return contact == TargetContact::Concealed || contact == TargetContact::Lost; }
};

// One thing an NPC can be doing in a fight (strafe, take cover, duel, ...).
// The brain scores all of its tactics every tick and runs the best one.
class CombatTacticBase
{
public:
	virtual ~CombatTacticBase() = default;

	// Stable id used for save/load and the debug UI.
	virtual const char* Id() const = 0;

	// 0 = not applicable. The brain multiplies this by the archetype weight.
	virtual float Score(const CombatContext& ctx) const = 0;

	virtual void Start(const CombatContext& ctx) {}
	virtual void Update(const CombatContext& ctx, float deltaTime) = 0;
	virtual void Stop() {}

	// A finished tactic is replaced as soon as any other tactic scores.
	virtual bool IsDone() const { return false; }

	// Seconds the brain keeps this tactic once started, unless interrupted.
	virtual float MinCommitTime() const { return 0.0f; }

	virtual nlohmann::json Serialize() const { return nlohmann::json::object(); }
	virtual void Deserialize(const nlohmann::json& data) {}
};

#pragma once

#include "CombatTacticBase.h"
#include "RangedFireControl.h"
#include "CoverPlanner.h"

#include <memory>
#include <string>
#include <vector>

class NpcBase;

// The combat brain of one NPC: owns its tactics, scores them every tick and
// runs the best one. The base class holds everything archetypes share; the
// children (Ranged / Melee / Hybrid in CombatBehaviors.h) only choose which
// tactics exist and how they are weighted. Mirrors InvestigationBase.
// See COMBAT_AI_DESIGN.md §3.
class CombatBehaviorBase
{
public:


	explicit CombatBehaviorBase(NpcBase* owner);
	virtual ~CombatBehaviorBase();

	CombatBehaviorBase(const CombatBehaviorBase&) = delete;
	CombatBehaviorBase& operator=(const CombatBehaviorBase&) = delete;

	// Called once per frame from NpcBase::UpdateTargetAttack (worker thread).
	void Update(float deltaTime);

	// True while the NPC fights with a melee weapon (always for melee
	// archetypes, mode dependent for hybrids).
	virtual bool IsMeleeMode() const { return false; }

	// True while the brain has a live target to fight.
	bool IsEngaged() const { return targetView.Valid(); }

	const CombatTargetView& GetTargetView() const { return targetView; }
	const CombatTacticBase* GetCurrentTactic() const { return currentTactic; }

	NpcBase* GetOwner() const { return owner; }

	// Does this NPC itself see the target right now?
	bool SeesTargetNow() const;

	// Where this NPC believes the target is (§11.3): the target while it sees it,
	// the squad's last line-of-sight position while a squadmate does, the
	// estimate while it hides, its own last line-of-sight position otherwise.
	// What the body faces and the spine aims at.
	vec3 BelievedTargetPosition() const { return believedPosition; }

	// A point to face or aim at near the believed target position, unless a
	// wall stands between: then the first bend of the path there, where the
	// target would come round (§11.3). Other points pass through unchanged.
	vec3 LookPointFor(const vec3& point) const;

	// ── Melee slots (granted by CombatCoordinator) ────────────────────────────
	bool WantsMeleeSlot() const { return meleeSlotWanted; }
	bool HasMeleeSlot() const { return meleeSlotHeld; }
	double MeleeSlotHeldSince() const { return meleeSlotSince; }
	void SetMeleeSlot(bool granted, double now)
	{
		if (granted && !meleeSlotHeld) meleeSlotSince = now;
		meleeSlotHeld = granted;
	}

	// ── Squad plan (written by CombatCoordinator on the main thread) ──────────
	const SquadOrder& GetSquadOrder() const { return squadOrder; }
	void SetSquadOrder(const SquadOrder& order);

	// When this NPC last stopped being a flanker: it isn't sent round again at once.
	double FlankEndedAt() const { return flankEndedAt; }

	bool IsPressingAttack() const;
	void ReportFlankEngaged();

	// The flanker got where it was going (or found no way round): the plan
	// hands it back to the base of fire.
	void ReportFlankFinished() { flankFinished = true; lanePhase = LanePhase::Done; }
	bool FlankFinished() const { return flankFinished; }

	// Lane orders: the tactic reports how far along the NPC is.
	void ReportLanePhase(LanePhase phase);
	LanePhase GetLanePhase() const { return lanePhase; }

	RangedFireControl& Fire() { return fire; }
	const RangedFireControl& Fire() const { return fire; }
	bool IsReloading() const { return fire.IsReloading(); }

	CoverPlanner& Cover() { return cover; }
	const CoverPlanner& Cover() const { return cover; }

	// The NPC took `damage`: shaken aim, pinned down, maybe a flinch.
	void OnDamaged(float damage);

	// The hit came from its own target: whatever it was waiting for (a lane
	// stack, a doorway) is off, it turns and fights (main thread, between ticks).
	void OnHitByTarget();

	// It heard (or felt a shot from) its target at `position` itself; the
	// coordinator passes it on to the squad at the next step.
	void NoteTargetHeard(const vec3& position);
	bool TakeTargetHeard(vec3& outPosition)
	{
		if (!heardPending) return false;
		heardPending = false;
		outPosition = heardPosition;
		return true;
	}

	// The lane it was flanking through no longer leads to the target (it moved):
	// the flank ends without the usual rest, so the plan can send it round
	// another way at once.
	void ReportLaneStale();

	// Incoming fire that missed (or hit) pins the NPC down: it ducks and aims
	// worse. Decays over time.
	void AddSuppression(float amount);
	float GetSuppression() const { return suppression; }

	// True while the brain deliberately holds its ground against a target it
	// cannot see (in cover, waiting). Feeds the behaviour tree's "in contact"
	// check so it doesn't send the NPC running at the target's last position.
	bool HoldsContact() const;

	nlohmann::json Serialize() const;
	void Deserialize(const nlohmann::json& data);

	void DrawDebugUI();

protected:
	// Tactics are owned by the brain. `weight` scales the tactic's own score,
	// which is how an archetype prefers one tactic over another.
	void AddTactic(std::unique_ptr<CombatTacticBase> tactic, float weight = 1.0f);

	// Archetype hook that runs before tactics are scored (hybrids refresh the
	// weapon mode here so scoring sees the up-to-date mode).
	virtual void PreSelect() {}

	// Archetypes that react to a hit with a short stagger.
	virtual bool CanFlinch() const { return false; }

	// A tactic keeps this score bonus while inside its MinCommitTime, and a
	// challenger must beat the current tactic by SwitchMargin to replace it.
	virtual float CommitBonus() const { return 0.15f; }
	virtual float SwitchMargin() const { return 0.1f; }

	NpcBase* owner = nullptr;

private:
	struct TacticEntry
	{
		std::unique_ptr<CombatTacticBase> tactic;
		float weight = 1.0f;
		float lastScore = 0.0f; // for the debug UI
	};

	std::vector<TacticEntry> tactics;

	RangedFireControl fire;
	CoverPlanner cover{ this };

	SquadOrder squadOrder;
	bool flankFinished = false;
	LanePhase lanePhase = LanePhase::Moving;
	double flankEndedAt = -1000.0;
	double pressUntil = 0.0;

	bool meleeSlotWanted = false;
	bool meleeSlotHeld = false;
	double meleeSlotSince = 0.0;

	float suppression = 0.0f;
	double lastFlinchTime = -100000.0;
	double flinchStopTime = 0.0; // when the flinch animation is cut short; 0 = none

	void TryFlinch(float damage);
	void FireIfTargetVisible();
	void ApplyBodyRules();

	vec3 believedPosition{};
	bool heardPending = false;
	vec3 heardPosition{};
	double heardTurnUntil = 0.0;
	bool skipFlankRest = false;
	vec3 watchPoint{};
	bool threatOccluded = false;
	double nextWatchUpdate = 0.0;

	void UpdateWatchPoint(bool sees);
	bool ShouldUseCover(const CombatContext& ctx) const;

	CombatTacticBase* currentTactic = nullptr;
	bool currentStarted = false; // false after Deserialize until the next tick starts it
	double currentStartTime = 0.0;

	CombatTargetView targetView;
	CombatContext context;

	void RefreshContext();
	void SelectTactic();
	void SwitchTo(CombatTacticBase* next);
};

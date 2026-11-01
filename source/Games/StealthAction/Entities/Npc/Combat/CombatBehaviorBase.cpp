#include "CombatBehaviorBase.h"

#include "CombatCoordinator.h"
#include "Tactics/TacticHelpers.h"
#include "../NpcBase.h"
#include "CombatTuning.h"

#include "CoverMap.h"

#include <Navigation/Navigation.hpp>
#include <Physics.h>
#include <RandomHelper.h>
#include <imgui/imgui.h>

#include <algorithm>
#include <cmath>

CombatBehaviorBase::CombatBehaviorBase(NpcBase* owner) : owner(owner)
{
	CombatCoordinator::Register(this);
}

CombatBehaviorBase::~CombatBehaviorBase()
{
	CombatCoordinator::Unregister(this);
}

void CombatBehaviorBase::SetSquadOrder(const SquadOrder& order)
{
	const bool newLane = order.hasLane != squadOrder.hasLane
		|| (order.hasLane && glm::distance(order.entry, squadOrder.entry) > 1.0f);

	if (squadOrder.role == SquadRole::Flank && order.role != SquadRole::Flank)
	{
		flankEndedAt = skipFlankRest ? -1000.0 : Time::GameTime;
		skipFlankRest = false;
	}

	if (order.role != squadOrder.role || newLane)
	{
		flankFinished = false;
		lanePhase = LanePhase::Moving;
	}
	squadOrder = order;
}

bool CombatBehaviorBase::IsPressingAttack() const
{
	return Time::GameTime < pressUntil;
}

void CombatBehaviorBase::ReportFlankEngaged()
{
	pressUntil = Time::GameTime + CombatTuning::Get().squad.postFlankPressTime;
	lanePhase = LanePhase::Done;
	flankFinished = true;
}

void CombatBehaviorBase::ReportLanePhase(LanePhase phase)
{
	if (phase == LanePhase::Done && lanePhase == LanePhase::Breaching)
		pressUntil = Time::GameTime + CombatTuning::Get().squad.postFlankPressTime;

	lanePhase = phase;
	if (phase == LanePhase::Done) flankFinished = true;
}

void CombatBehaviorBase::AddTactic(std::unique_ptr<CombatTacticBase> tactic, float weight)
{
	tactics.push_back({ std::move(tactic), weight, 0.0f });
}

void CombatBehaviorBase::Update(float deltaTime)
{
	RefreshContext();
	PreSelect();
	context.meleeMode = IsMeleeMode();

	meleeSlotWanted = context.meleeMode && targetView.Valid() && targetView.isPlayer
		&& glm::distance(owner->Position, targetView.position) < CombatTuning::Get().brain.meleeSlotRange;
	context.wantsMeleeSlot = meleeSlotWanted;
	context.hasMeleeSlot = meleeSlotHeld || !targetView.isPlayer;

	context.fire = &fire;
	context.cover = &cover;
	context.suppression = suppression;
	context.flinched = Time::GameTime - lastFlinchTime < CombatTuning::Get().brain.flinchDuckTime;

	suppression = std::max(0.0f, suppression - CombatTuning::Get().brain.suppressionDecayPerSecond * deltaTime);

	if (flinchStopTime > 0.0 && Time::GameTime >= flinchStopTime)
	{
		flinchStopTime = 0.0;
		owner->StopActionAnimation();
	}

	fire.Tick(context, deltaTime);
	cover.Update(context, ShouldUseCover(context));

	SelectTactic();

	if (currentTactic)
	{
		if (!currentStarted)
		{
			currentStarted = true;
			currentStartTime = Time::GameTime;
			currentTactic->Start(context);
		}

		currentTactic->Update(context, deltaTime);
	}

	ApplyBodyRules();
	FireIfTargetVisible();
}

// How the body carries out whatever the tactic decided (COMBAT_AI_DESIGN.md
// §11.3). Tactics choose where to go and what to face; these rules hold for
// every gun NPC whatever the tactic, so no tactic can forget them:
//
//   posture  the gun is up (aimed) unless the NPC sprints (ready, at the
//            chest) or reloads (lowered): never down mid-fight otherwise
//   pace     an NPC that can't shoot right now has no reason to walk: more
//            than RunWhenIdleDistance to go and it runs, facing its path.
//            Tactics mark deliberate slow moves (creeping, crouching in) careful
//   stance   pinned down out in the open and not going anywhere: crouch
void CombatBehaviorBase::ApplyBodyRules()
{
	if (context.meleeMode || !targetView.Valid()) return;

	CombatMoveOrder order = owner->GetCombatMoveOrder();

	if (!order.active)
	{
		// No tactic this tick: stand facing the threat, gun up.
		owner->UpdateAimAndAttackPosition(false);

		order.active = true;
		order.speed = CombatTuning::Get().movement.strafeSpeed;
		order.facing = NpcFacing::Point;
		order.lookAt = context.threatPosition;
	}

	const float toGo = TacticHelpers::DistanceXZ(owner->Position, owner->GetAttackPosition());
	const bool reloading = fire.IsReloading();
	const bool canShoot = owner->SeesTarget() && !reloading;

	if (!canShoot && !order.careful && toGo > CombatTuning::Get().brain.runWhenIdleDistance && order.stance == NpcStance::Standing)
	{
		order.speed = CombatTuning::Get().movement.runSpeed;
		order.facing = NpcFacing::Movement;
	}

	// Heard its target without seeing it: head round to the sound at once.
	if (!owner->SeesTarget() && Time::GameTime < heardTurnUntil && order.speed <= CombatTuning::Get().movement.strafeSpeed)
	{
		order.facing = NpcFacing::Point;
		order.lookAt = heardPosition + vec3(0, 0.6f, 0);
	}

	const bool inCover = cover.Arrived();
	if (suppression >= CombatTuning::Get().brain.duckSuppression && !inCover && toGo < CombatTuning::Get().brain.pinnedCrouchMoveDistance)
	{
		order.stance = NpcStance::Crouched;
		order.speed = std::min(order.speed, CombatTuning::Get().movement.crouchWalkSpeed);
	}

	if (reloading)
		order.posture = NpcWeaponPosture::Lowered;
	else if (order.speed > CombatTuning::Get().shot.maxFireSpeed && toGo > CombatTuning::Get().brain.runWhenIdleDistance)
		order.posture = NpcWeaponPosture::Ready;
	else
		order.posture = NpcWeaponPosture::Aimed;

	owner->SetCombatMoveOrder(order);
}

// The rule every gun NPC follows, whatever its tactic: if it can see its target
// it shoots. Hiding is only ever "can't see it from here"; an NPC never stands
// in view deciding not to fire. Accuracy comes from the attack token (only two
// NPCs at a time aim to hit the player); the gate (cooldown, standing still
// enough, line of sight) is NpcBase::ReadyToFireRanged.
void CombatBehaviorBase::FireIfTargetVisible()
{
	if (context.meleeMode || !targetView.Valid() || !owner->SeesTarget()) return;

	fire.NoteExposed();

	if (owner->ReadyToFireRanged())
		fire.TryShoot(context);
}

void CombatBehaviorBase::OnHitByTarget()
{
	nextWatchUpdate = 0.0;

	// A flanker on its way or waiting at its stack has been found: fight from here.
	if (squadOrder.role == SquadRole::Flank && squadOrder.hasLane
		&& (lanePhase == LanePhase::Moving || lanePhase == LanePhase::Stacked))
		ReportFlankEngaged();
}

void CombatBehaviorBase::NoteTargetHeard(const vec3& position)
{
	heardPending = true;
	heardPosition = position;
	heardTurnUntil = Time::GameTime + CombatTuning::Get().brain.heardTurnTime;
	nextWatchUpdate = 0.0;
}

void CombatBehaviorBase::ReportLaneStale()
{
	lanePhase = LanePhase::Done;
	flankFinished = true;
	skipFlankRest = true;
}

void CombatBehaviorBase::AddSuppression(float amount)
{
	suppression = std::min(CombatTuning::Get().brain.maxSuppression, suppression + amount);
}

void CombatBehaviorBase::OnDamaged(float damage)
{
	fire.OnDamaged();
	AddSuppression(CombatTuning::Get().brain.hitSuppression);
	TryFlinch(damage);
}

// A hit staggers the NPC for a moment: the short `stun` clip, no shooting, and
// the current cover cycle ducks. Rate limited so one gun can't stun-lock it.
void CombatBehaviorBase::TryFlinch(float damage)
{
	const double now = Time::GameTime;

	if (!CanFlinch() || now - lastFlinchTime < CombatTuning::Get().brain.flinchCooldown) return;
	if (owner->Health <= 0.0f || owner->isStunned() || owner->IsPlayingActionAnimation()) return;

	if (damage < CombatTuning::Get().brain.flinchDamage && RandomHelper::RandomFloat() > CombatTuning::Get().brain.smallHitFlinchChance) return;

	lastFlinchTime = now;
	flinchStopTime = now + CombatTuning::Get().brain.flinchAnimationTime;

	owner->PlayActionAnimation("stun", false, 0.08f, 0.15f);
	owner->DelayAttack(CombatTuning::Get().brain.flinchAnimationTime + 0.15f);
}

bool CombatBehaviorBase::SeesTargetNow() const
{
	return targetView.Valid() && owner->SeesTarget();
}

// The behaviour tree keeps the NPC in combat while the squad still has a
// belief about the target (visible, hiding or being checked). Without one the
// NPC holds through a brief loss of sight only.
bool CombatBehaviorBase::HoldsContact() const
{
	if (!targetView.Valid()) return false;

	const TargetBelief* belief = CombatCoordinator::FindBelief(targetView.entity->Id);
	if (belief && belief->contact != TargetContact::None) return true;

	return fire.UnseenTime() < CombatTuning::Get().brain.briefLossGraceTime;
}

// Cover is for fighting a shooter at range: not when the NPC is busy with
// something else, stunned, or the target is close enough that hiding is pointless.
bool CombatBehaviorBase::ShouldUseCover(const CombatContext& ctx) const
{
	if (ctx.meleeMode || !ctx.target.Valid() || ctx.Lost()) return false;

	// Flankers are on the move, and a flanker that has just got its angle fights
	// from it in the open (sidestepping, firing) instead of ducking away: being
	// spotted that close is the moment to be aggressive. Afterwards it may use
	// cover again, but none further from the target (CoverPlanner). While the
	// target hides only the holders and the suppressors fight from cover;
	// everybody else is pushing for a line of sight.
	if (ctx.isFlanker || IsPressingAttack()) return false;
	if (ctx.Concealed() && !ctx.isSuppressor) return false; // holders hold where they are
	if (!owner->WantsToAttackTarget() || owner->DoingTask || owner->isStunned()) return false;

	const float distance = glm::distance(owner->Position, ctx.threatPosition);
	if (distance < CombatTuning::Get().cover.minThreatDistance) return false;

	// Cover only protects from bullets: against a target that can't shoot the NPC
	// keeps its distance and shoots instead (TacticKite).
	if (!ctx.target.hasGun) return false;

	return true;
}

vec3 CombatBehaviorBase::LookPointFor(const vec3& point) const
{
	constexpr float NearBelieved = 2.5f; // peek points and the estimate itself count as "the target"

	if (!threatOccluded || TacticHelpers::DistanceXZ(point, believedPosition) > NearBelieved)
		return point;

	return watchPoint;
}

// Is there a wall between this NPC and where it believes the target is? Then it
// watches the corner the target would come round: the first bend of its own
// path there.
void CombatBehaviorBase::UpdateWatchPoint(bool sees)
{
	const auto& tune = CombatTuning::Get().brain;

	if (sees || !targetView.Valid())
	{
		threatOccluded = false;
		return;
	}

	const double now = Time::GameTime;
	if (now < nextWatchUpdate) return;
	nextWatchUpdate = now + tune.watchUpdateInterval;

	const vec3 eye = owner->Position + vec3(0, 0.6f, 0);
	const auto hit = Physics::LineTrace(eye, context.threatEye, CoverMap::BlockerMask());
	threatOccluded = hit.hasHit && glm::distance(hit.position, context.threatEye) > tune.watchOcclusionMargin;
	if (!threatOccluded) return;

	bool reached = false;
	const std::vector<vec3> path = NavigationSystem::FindSimplePath(owner->Position, context.threatPosition, 0.0f, &reached, true);

	// The farthest point of its path to the target that it can see: where the
	// target would come into view.
	bool found = false;
	int checked = 0;
	for (const vec3& point : path)
	{
		if (checked++ >= tune.watchMaxCorners) break;
		if (TacticHelpers::DistanceXZ(owner->Position, point) < tune.watchCornerDistance) continue;

		const vec3 corner = point + vec3(0, tune.watchEyeHeight, 0);
		if (Physics::LineTrace(eye, corner, CoverMap::BlockerMask()).hasHit) break;

		watchPoint = corner;
		found = true;
	}

	// A corner off behind it (the way round goes back first): face the
	// target's direction instead, the pitch kept level by the body.
	if (found)
	{
		const vec3 toCorner = MathHelper::Normalized(MathHelper::XZ(watchPoint - owner->Position));
		const vec3 toThreat = MathHelper::Normalized(MathHelper::XZ(context.threatPosition - owner->Position));
		found = glm::dot(toCorner, toThreat) >= std::cos(glm::radians(tune.watchMaxAngleDegrees));
	}

	threatOccluded = found;
}

void CombatBehaviorBase::RefreshContext()
{
	Entity* target = nullptr;

	if (owner->IsFollowingTarget() && !owner->GetTargetId().empty())
		target = Level::Current->FindEntityWithId(owner->GetTargetId());

	CombatTargetView::Fill(targetView, owner, target);

	context.owner = owner;
	context.target = targetView;
	context.brain = this;

	// What the squad believes. Aiming, cover and flanking use the squad's estimate
	// while the target is hidden, never its real position.
	context.belief = targetView.Valid() ? CombatCoordinator::FindBelief(targetView.entity->Id) : nullptr;

	const bool sees = targetView.Valid() && owner->SeesTarget();

	if (context.belief)
		context.contact = context.belief->contact;
	else
		context.contact = sees ? TargetContact::Visible : TargetContact::None;

	// The believed target position (§11.3): the live target only while this NPC
	// sees it. Nothing the NPC can't see is ever read off the target itself.
	context.threatPosition = targetView.position;
	context.threatEye = targetView.eyePosition;

	if (!sees)
	{
		vec3 believed = owner->GetTargetLastLOSPosition();

		if (context.belief && context.Hunting())
			believed = context.belief->estimatePosition;
		else if (context.belief && context.contact == TargetContact::Visible)
			believed = context.belief->lastVisiblePosition;
		else if (owner->GetTargetLastLOSTime() < 0.0f && targetView.Valid())
			believed = owner->GetTargetLastSeenPosition();

		context.threatPosition = believed;
		context.threatEye = believed + (targetView.eyePosition - targetView.position);
	}

	believedPosition = context.threatPosition;
	UpdateWatchPoint(sees);

	context.squadExposed = 0;
	if (context.belief)
	{
		const bool selfExposed = !IsMeleeMode() && fire.ExposedWithin(CombatTuning::Get().belief.exposedNowWindow);
		context.squadExposed = std::max(0, context.belief->exposedCount - (selfExposed ? 1 : 0));
	}

	context.squad = context.belief ? squadOrder : SquadOrder();

	const SquadRole role = context.squad.role;
	context.isSuppressor = role == SquadRole::Suppress;
	context.isFlanker = role == SquadRole::Flank;
	context.isPusher = role == SquadRole::Push;
	context.isChecker = role == SquadRole::Check;
	context.isWatcher = role == SquadRole::Watch;
	context.isHolder = role == SquadRole::Hold;
}

void CombatBehaviorBase::SelectTactic()
{
	CombatTacticBase* best = nullptr;
	float bestScore = 0.0f;
	float currentScore = 0.0f;

	for (TacticEntry& entry : tactics)
	{
		entry.lastScore = std::max(0.0f, entry.tactic->Score(context)) * entry.weight;

		if (entry.tactic.get() == currentTactic)
			currentScore = entry.lastScore;

		if (entry.lastScore > bestScore)
		{
			bestScore = entry.lastScore;
			best = entry.tactic.get();
		}
	}

	if (!best || best == currentTactic) return;

	// A running tactic is replaced only once it is done, no longer applicable,
	// or clearly beaten; while it is inside its commit window it is favoured.
	if (currentTactic && currentScore > 0.0f && !currentTactic->IsDone())
	{
		const bool committing = Time::GameTime - currentStartTime < currentTactic->MinCommitTime();
		const float effectiveScore = currentScore + (committing ? CommitBonus() : 0.0f);

		if (bestScore <= effectiveScore + SwitchMargin()) return;
	}

	SwitchTo(best);
}

void CombatBehaviorBase::SwitchTo(CombatTacticBase* next)
{
	if (currentTactic && currentStarted)
		currentTactic->Stop();

	currentTactic = next;
	currentStarted = false;
}

nlohmann::json CombatBehaviorBase::Serialize() const
{
	nlohmann::json data = nlohmann::json::object();

	data["fire"] = fire.Serialize();

	if (currentTactic)
	{
		data["tactic"] = currentTactic->Id();
		data["tacticData"] = currentTactic->Serialize();
	}

	return data;
}

void CombatBehaviorBase::Deserialize(const nlohmann::json& data)
{
	currentTactic = nullptr;
	currentStarted = false;

	if (data.contains("fire"))
		fire.Deserialize(data["fire"]);

	if (!data.contains("tactic")) return;

	const std::string id = data["tactic"].get<std::string>();

	for (const TacticEntry& entry : tactics)
	{
		if (id != entry.tactic->Id()) continue;

		currentTactic = entry.tactic.get();

		// Started again by the next tick; restores the tactic's own state first.
		if (data.contains("tacticData"))
			currentTactic->Deserialize(data["tacticData"]);

		return;
	}
}

void CombatBehaviorBase::DrawDebugUI()
{
	ImGui::Separator();
	static const char* contactNames[] = { "none", "visible", "concealed", "lost" };

	ImGui::Text("combat: %s", IsEngaged() ? "engaged" : "no target (not following / not attacking)");
	ImGui::Text("contact: %s  target has gun: %s  squad out shooting: %d",
		contactNames[static_cast<int>(context.contact)], targetView.hasGun ? "yes" : "no", context.squadExposed);
	ImGui::Text("follow: %s  attack: %s  sees: %s",
		owner->IsFollowingTarget() ? "yes" : "no", owner->WantsToAttackTarget() ? "yes" : "no", owner->SeesTarget() ? "yes" : "no");

	ImGui::Text("squad role: %s%s%s", SquadRoleName(context.squad.role),
		context.isFlanker && !context.squad.hasLane ? (context.squad.side < 0.0f ? " (left)" : " (right)") : "",
		flankFinished ? " (finished)" : "");
	if (context.squad.hasLane)
		ImGui::Text("lane: %s%s%s", LanePhaseName(lanePhase), context.squad.breach ? "  BREACH" : "", context.squad.waitTurn ? "  (waiting turn)" : "");
	if (context.belief)
		ImGui::Text("front lane: %s  assault: %s", context.belief->hasFrontLane ? "yes" : "no (open ground)",
			Time::GameTime < context.belief->assaultUntil ? "ON" : "off");
	if (context.belief)
		ImGui::Text("squad: %s  target: %s  flank under way: %s%s",
			context.belief->squadSettled ? "settled" : "settling",
			context.belief->targetHolding ? "holding" : (context.belief->targetRetreating ? "running away" : "moving"),
			context.belief->flankUnderWay ? "yes (patient)" : "no",
			IsPressingAttack() ? "  PRESSING" : "");

	static const char* postureNames[] = { "default", "aimed", "ready", "lowered" };
	const CombatMoveOrder& move = owner->GetCombatMoveOrder();
	ImGui::Text("body: %s  speed %.1f  %s%s", postureNames[static_cast<int>(move.posture)], move.speed,
		move.stance == NpcStance::Crouched ? "crouched" : "standing", move.careful ? "  careful" : "");

	if (targetView.Valid())
		CombatCoordinator::DrawApproachMap(targetView.entity->Id);
	ImGui::Text("tactic: %s", currentTactic ? currentTactic->Id() : "-");
	ImGui::Text("suppression: %.2f  cover: %s", suppression, cover.HasCover() ? (cover.Arrived() ? "in cover" : "going") : "none");
	ImGui::Text("ammo: %d%s  token: %s", fire.Magazine(), fire.IsReloading() ? " (reloading)" : "", fire.HasToken() ? "yes" : "no");

	for (const TacticEntry& entry : tactics)
		ImGui::Text("  %s  %.2f", entry.tactic->Id(), entry.lastScore);

	ImGui::Text("brains: %d (engaged %d)", CombatCoordinator::BrainCount(), CombatCoordinator::EngagedCount());
}

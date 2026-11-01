#include "CombatCoordinator.h"

#include "CombatBehaviorBase.h"
#include "CoverFinder.h"
#include "CoverMap.h"
#include "Tactics/TacticHelpers.h"
#include "../NpcBase.h"
#include "CombatTuning.h"

#include <AiPerception/Observer.h>
#include <EngineMain.h>
#include <Physics.h>
#include <RandomHelper.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <unordered_map>

void CombatCoordinator::Register(CombatBehaviorBase* brain)
{
	brains.push_back(brain);
}

void CombatCoordinator::Unregister(CombatBehaviorBase* brain)
{
	brains.erase(std::remove(brains.begin(), brains.end(), brain), brains.end());

	ReleaseCover(brain);
}

bool CombatCoordinator::ReserveCover(const CombatBehaviorBase* brain, const vec3& floor)
{
	std::lock_guard<std::mutex> lock(coverMutex);

	for (const CoverReservation& reservation : coverReservations)
	{
		if (reservation.brain == brain) continue;

		if (glm::distance(reservation.floor, floor) < CombatTuning::Get().tokens.coverSpacing) return false;
	}

	for (CoverReservation& reservation : coverReservations)
	{
		if (reservation.brain != brain) continue;

		reservation.floor = floor;
		return true;
	}

	coverReservations.push_back({ brain, floor });
	return true;
}

void CombatCoordinator::ReleaseCover(const CombatBehaviorBase* brain)
{
	std::lock_guard<std::mutex> lock(coverMutex);

	coverReservations.erase(std::remove_if(coverReservations.begin(), coverReservations.end(),
		[brain](const CoverReservation& r) { return r.brain == brain; }), coverReservations.end());
}

void CombatCoordinator::Step()
{
	const unsigned long frame = EngineMain::MainInstance->frame;
	if (frame == lastStepFrame) return;
	lastStepFrame = frame;

	// Drain the damage budget.
	float used = damageBudgetUsed.load();
	const float drained = std::max(0.0f, used - CombatTuning::Get().tokens.playerDamageBudget * Time::DeltaTimeF);
	damageBudgetUsed.compare_exchange_strong(used, drained);

	int engaged = 0;
	for (const CombatBehaviorBase* brain : brains)
		if (brain->IsEngaged()) engaged++;

	engagedCount = engaged;

	UpdateBeliefs();
	ResolveAttackTokens();
	ResolveMeleeSlots();
}

bool CombatCoordinator::TryConsumeDamageBudget(float damage)
{
	float used = damageBudgetUsed.load();

	while (true)
	{
		if (used + damage > CombatTuning::Get().tokens.playerDamageBudget) return false;

		if (damageBudgetUsed.compare_exchange_weak(used, used + damage)) return true;
	}
}

// Against the player only PlayerAttackTokens NPCs shoot to hit; the rest are
// held to suppression accuracy. Against NPC targets there is no limit.
// Holders keep a token for at least MinTokenHoldTime unless they stop being
// able to shoot; free tokens go to the highest bids.
void CombatCoordinator::ResolveAttackTokens()
{
	const double now = Time::GameTime;

	struct Bidder
	{
		CombatBehaviorBase* brain;
		float bid;
	};
	std::unordered_map<std::string, std::vector<Bidder>> bidsByTarget;

	for (CombatBehaviorBase* brain : brains)
	{
		RangedFireControl& fire = brain->Fire();
		const CombatTargetView& view = brain->GetTargetView();

		if (!view.Valid())
		{
			fire.SetToken(false, now);
			continue;
		}

		if (!view.isPlayer)
		{
			fire.SetToken(true, now);
			continue;
		}

		if (!fire.WantsToken())
		{
			fire.SetToken(false, now);
			continue;
		}

		float bid = fire.TokenBid(now);

		// A holder inside its minimum hold time can't be outbid.
		if (fire.HasToken() && now - fire.TokenHeldSince() < CombatTuning::Get().tokens.minTokenHoldTime)
			bid += 1000.0f;

		bidsByTarget[view.entity->Id].push_back({ brain, bid });
	}

	for (auto& entry : bidsByTarget)
	{
		std::vector<Bidder>& bidders = entry.second;

		std::sort(bidders.begin(), bidders.end(),
			[](const Bidder& a, const Bidder& b) { return a.bid > b.bid; });

		for (size_t i = 0; i < bidders.size(); ++i)
			bidders[i].brain->Fire().SetToken(i < CombatTuning::Get().tokens.playerAttackTokens, now);
	}
}

// ── Squad belief ─────────────────────────────────────────────────────────────

const TargetBelief* CombatCoordinator::FindBelief(const std::string& targetId)
{
	const auto it = beliefs.find(targetId);
	return it == beliefs.end() ? nullptr : &it->second;
}

namespace
{
	using Members = std::vector<CombatBehaviorBase*>;

	float DistanceToEstimate(const CombatBehaviorBase* brain, const TargetBelief& belief)
	{
		return glm::distance(brain->GetOwner()->Position, belief.estimatePosition);
	}

	// The member closest to `point` that passes `accept`.
	template <typename Accept>
	CombatBehaviorBase* Nearest(const Members& members, const vec3& point, Accept accept, const CombatBehaviorBase* exclude = nullptr)
	{
		CombatBehaviorBase* best = nullptr;
		float bestDistance = std::numeric_limits<float>::max();

		for (CombatBehaviorBase* brain : members)
		{
			if (brain == exclude || !accept(brain)) continue;

			const float distance = glm::distance(brain->GetOwner()->Position, point);
			if (distance < bestDistance)
			{
				bestDistance = distance;
				best = brain;
			}
		}

		return best;
	}

	// Where the target can reappear: over the cover, and round either side of it.
	void RebuildPeekPoints(TargetBelief& belief, const Members& members)
	{
		vec3 centroid{};
		for (const CombatBehaviorBase* brain : members)
			centroid += brain->GetOwner()->Position;
		centroid /= static_cast<float>(std::max<size_t>(1, members.size()));

		const vec3 towardSquad = MathHelper::Normalized(MathHelper::XZ(centroid - belief.estimatePosition));
		vec3 side = glm::cross(towardSquad, vec3(0, 1, 0));
		side = glm::length(side) < 0.001f ? vec3(1, 0, 0) : MathHelper::Normalized(side);

		belief.peekPoints[0] = belief.estimatePosition + vec3(0, 0.6f, 0);
		belief.peekPoints[1] = belief.estimatePosition + side * 1.2f + vec3(0, 0.3f, 0);
		belief.peekPoints[2] = belief.estimatePosition - side * 1.2f + vec3(0, 0.3f, 0);
	}

	double RandomBetween(double min, double max)
	{
		return min + RandomHelper::RandomFloat() * (max - min);
	}

	void ScheduleConcealment(TargetBelief& belief, double now)
	{
		belief.concealedSince = now;
		belief.concealedUntil = now + RandomBetween(CombatTuning::Get().belief.concealedMin, CombatTuning::Get().belief.concealedMax);
		belief.suppressUntil = now + RandomBetween(CombatTuning::Get().squad.suppressMinTime, CombatTuning::Get().squad.suppressMaxTime);
		belief.estimateRadius = 1.0f;
		belief.emptyTime = 0.0f;
		belief.nextPlanTime = 0.0;
	}

	void EnterConcealed(TargetBelief& belief, const Members& members, double now)
	{
		belief.contact = TargetContact::Concealed;
		belief.estimatePosition = belief.lastVisiblePosition
			+ MathHelper::XZ(belief.lastVisibleVelocity) * CombatTuning::Get().belief.estimateLead;

		ScheduleConcealment(belief, now);
		RebuildPeekPoints(belief, members);
	}

	void EnterLost(TargetBelief& belief, double now)
	{
		belief.contact = TargetContact::Lost;
		belief.lostSince = now;
		belief.nextPlanTime = 0.0;
	}

	// Does any nearby member have a clear look at the spot, standing and crouched?
	bool SpotLooksEmpty(const TargetBelief& belief, const Members& members)
	{
		const vec3 floor = belief.estimatePosition - vec3(0, 0.9f, 0);
		const vec3 crouchedBody = floor + vec3(0, 0.7f, 0);
		const vec3 standingBody = floor + vec3(0, 1.5f, 0);

		for (const CombatBehaviorBase* brain : members)
		{
			if (DistanceToEstimate(brain, belief) > CombatTuning::Get().belief.emptyCheckDistance) continue;

			const NpcBase* owner = brain->GetOwner();
			const vec3 eye = owner->Position + vec3(0, 0.6f, 0);

			// Only an NPC actually looking that way can tell the spot is empty.
			if (!owner->observer) continue;
			const vec3 toSpot = MathHelper::Normalized(belief.estimatePosition - eye);
			if (glm::dot(MathHelper::Normalized(owner->observer->forward), toSpot) < CombatTuning::Get().belief.emptyCheckViewCos) continue;

			if (!Physics::LineTrace(eye, crouchedBody, CoverMap::BlockerMask()).hasHit
				&& !Physics::LineTrace(eye, standingBody, CoverMap::BlockerMask()).hasHit)
				return true;
		}

		return false;
	}

	// Every engaged member gets what the squad knows of the target, seen or
	// heard, whether or not it can see the one who saw it.
	void ShareWithSquad(const Members& members, const std::string& targetId, const vec3& position, bool lineOfSight)
	{
		for (CombatBehaviorBase* brain : members)
			brain->GetOwner()->ReceiveSquadSighting(targetId, position, lineOfSight);
	}

	// Combat awareness (§5.7): a target that moves about close to the squad, or
	// noisily within earshot, is tracked: the estimate follows it (off by up to
	// trackError) and the hiding timers restart, so walking round a wall doesn't
	// shake a squad in a fight. A slow, quiet retreat out of earshot does.
	void TrackByEar(TargetBelief& belief, const Members& members, const CombatTargetView& view, double now)
	{
		const auto& tune = CombatTuning::Get().belief;
		if (!view.Valid() || now < belief.nextTrackTime) return;
		belief.nextTrackTime = now + tune.trackInterval;

		float nearest = std::numeric_limits<float>::max();
		for (const CombatBehaviorBase* brain : members)
			nearest = std::min(nearest, glm::distance(brain->GetOwner()->Position, view.position));

		const bool noisy = glm::length(MathHelper::XZ(view.velocity)) > tune.trackNoisySpeed;
		const bool heard = nearest < tune.trackCloseDistance || (noisy && nearest < tune.trackHearingDistance);
		if (!heard) return;

		const vec3 error = vec3(RandomHelper::RandomFloat() * 2.0f - 1.0f, 0.0f, RandomHelper::RandomFloat() * 2.0f - 1.0f) * tune.trackError;

		// Moved well away from where the squad had it: go and find it.
		if (glm::distance(MathHelper::XZ(view.position), MathHelper::XZ(belief.estimatePosition)) > tune.relocateDistance)
			belief.seekUntil = now + tune.seekTime;

		belief.estimatePosition = view.position + error;

		belief.concealedUntil = std::max(belief.concealedUntil,
			now + RandomBetween(tune.concealedMin, tune.concealedMax) * 0.5);
		belief.concealedSince = std::max(belief.concealedSince, now - tune.minConcealedTime * 0.5);
		belief.estimateRadius = 1.0f;
		belief.emptyTime = 0.0f;
		RebuildPeekPoints(belief, members);

		ShareWithSquad(members, belief.targetId, belief.estimatePosition, false);
	}

	// Did any member hear the target this frame (its shots above all), or have a
	// shot from it whizz past? The position, if so.
	bool HearTarget(const TargetBelief& belief, const Members& members, vec3& outPosition)
	{
		bool heard = false;
		int loudest = -1;

		for (CombatBehaviorBase* brain : members)
		{
			vec3 position;
			if (brain->TakeTargetHeard(position))
			{
				outPosition = position;
				heard = true;
			}
		}

		for (const CombatBehaviorBase* brain : members)
		{
			const auto& observer = brain->GetOwner()->observer;
			if (!observer) continue;

			for (const HeardSound& sound : observer->heardSounds)
			{
				if (sound.causerId != belief.targetId || sound.severity < loudest) continue;

				loudest = sound.severity;
				outPosition = sound.position;
				heard = true;
			}
		}

		return heard;
	}

	// Evidence (§5.4): a sound from the target (a shot above all) gives its
	// position away. Hiding: the estimate moves there and the hiding timers and
	// the suppression restart. Lost, or never seen yet: the squad is back on it,
	// hunting there. Every engaged member gets the position.
	void OnTargetHeard(TargetBelief& belief, const Members& members, const vec3& position, double now)
	{
		// Nobody sees it right now (or the squad had lost it): it is hiding there.
		if (belief.contact != TargetContact::Concealed)
		{
			if (belief.contact == TargetContact::None)
			{
				belief.fightSince = now;
				belief.anchorPosition = position;
				belief.anchorSince = now;
			}

			belief.contact = TargetContact::Concealed;
		}

		// Somewhere else than the squad had it: it moved, go and find it.
		if (glm::distance(MathHelper::XZ(position), MathHelper::XZ(belief.estimatePosition)) > CombatTuning::Get().belief.relocateDistance
			|| belief.contact != TargetContact::Concealed)
			belief.seekUntil = now + CombatTuning::Get().belief.seekTime;

		belief.estimatePosition = position;
		ScheduleConcealment(belief, now);
		RebuildPeekPoints(belief, members);

		// Every member's last sight moves there when it is off by more than
		// belief.heardUpdateDistance.
		for (CombatBehaviorBase* brain : members)
			brain->GetOwner()->ReceiveHeardTarget(belief.targetId, position, false);
	}

	void UpdateConcealed(TargetBelief& belief, const Members& members, const CombatTargetView& view, float deltaTime, double now)
	{
		TrackByEar(belief, members, view, now);

		const double elapsed = belief.ConcealedFor(now);

		belief.estimateRadius = std::min(CombatTuning::Get().belief.maxEstimateRadius + 1.0f,
			1.0f + CombatTuning::Get().belief.estimateGrowth * static_cast<float>(elapsed));

		if (elapsed >= CombatTuning::Get().belief.minConcealedTime && SpotLooksEmpty(belief, members))
			belief.emptyTime += deltaTime;
		else
			belief.emptyTime = std::max(0.0f, belief.emptyTime - deltaTime);

		if (belief.emptyTime >= CombatTuning::Get().belief.emptyConfirmTime
			|| now >= belief.concealedUntil
			|| belief.estimateRadius > CombatTuning::Get().belief.maxEstimateRadius)
		{
			EnterLost(belief, now);
		}
	}

	// ── Squad plan (§8.3) ────────────────────────────────────────────────────

	bool IsGunner(const CombatBehaviorBase* brain) { return !brain->IsMeleeMode(); }

	// Signed angle (radians) of the NPC round the target, measured from the squad
	// axis. Same rotation convention as CoverFinder::FindFlankPosition, so the
	// sign is the `side` a flank order on that side gets.
	float AngleFromAxis(const CombatBehaviorBase* brain, const vec3& target, const vec3& axis)
	{
		const vec3 to = MathHelper::Normalized(MathHelper::XZ(brain->GetOwner()->Position - target));
		if (glm::length(to) < 0.001f) return 0.0f;

		return std::atan2(axis.x * to.z - axis.z * to.x, glm::dot(axis, to));
	}

	// The line the squad attacks along: from the target toward its gunners.
	vec3 SquadAxis(const Members& gunners, const vec3& target, const vec3& previous)
	{
		vec3 sum{};
		for (const CombatBehaviorBase* brain : gunners)
			sum += MathHelper::Normalized(MathHelper::XZ(brain->GetOwner()->Position - target));

		// The flankers swinging round pull the average too, but they are a minority.
		const vec3 axis = MathHelper::Normalized(MathHelper::XZ(sum));
		return glm::length(axis) < 0.001f ? previous : axis;
	}

	// Can this NPC put rounds into the hiding spot from where it stands? Same
	// test ReadyToFireAtPoint shoots with.
	bool HasLineOfFire(const CombatBehaviorBase* brain, const vec3& point)
	{
		const vec3 muzzle = brain->GetOwner()->Position + vec3(0, 0.6f, 0);
		const auto hit = Physics::LineTrace(muzzle, point, BodyType::World);

		return !hit.hasHit || glm::distance(hit.position, point) <= CombatTuning::Get().shot.suppressMaxBlockDistance;
	}

	// "Almost always flank": one flanker from two gunners up.
	// At most half the gun NPCs go round: the main force keeps the pressure on.
	int FlankersWanted(int gunners)
	{
		const auto& tune = CombatTuning::Get().squad;
		if (gunners < 2) return 0;

		const int share = static_cast<int>(static_cast<float>(gunners) * tune.maxFlankFraction);
		return std::min(tune.maxFlankers, std::max(1, share));
	}


	bool CanTakeErrand(const CombatBehaviorBase* brain)
	{
		NpcBase* owner = brain->GetOwner();
		return !brain->IsReloading() && !owner->DoingTask && !owner->isStunned() && !brain->IsPressingAttack()
			&& Time::GameTime - brain->FlankEndedAt() > CombatTuning::Get().squad.flankRestTime;
	}

	SquadOrder MakeOrder(SquadRole role, const TargetBelief& belief, double now)
	{
		SquadOrder order;
		order.role = role;
		order.axis = belief.squadAxis;
		order.since = now;
		return order;
	}

	// Keeps the role (and when it started) if the NPC already has it.
	void Assign(CombatBehaviorBase* brain, SquadRole role, const TargetBelief& belief, double now)
	{
		SquadOrder order = brain->GetSquadOrder();

		if (order.role != role)
			order = MakeOrder(role, belief, now);

		order.axis = belief.squadAxis;
		brain->SetSquadOrder(order);
	}

	// This NPC's place in a lane's stack: the stack spot for the first, then
	// StackSpacing further back along the way in for each one behind it.
	vec3 StackSlot(const vec3& entry, const vec3& stack, int index)
	{
		const vec3 back = MathHelper::Normalized(MathHelper::XZ(stack - entry));
		return stack + back * (CombatTuning::Get().squad.stackSpacing * static_cast<float>(index));
	}

	void GiveLane(SquadOrder& order, const vec3& entry, const vec3& stack, int slot)
	{
		order.hasLane = true;
		order.entry = entry;
		order.stack = StackSlot(entry, stack, slot);
	}

	// The lane whose entry is within LaneKeepDistance of `entry` (the same way
	// in after a rebuild moved it a little), or -1.
	int SameLane(const ApproachMap& map, const vec3& entry)
	{
		int best = -1;
		float bestDistance = CombatTuning::Get().squad.laneKeepDistance;

		for (int l = 0; l < static_cast<int>(map.Lanes().size()); ++l)
		{
			const float distance = glm::distance(map.Lanes()[l].entry, entry);
			if (distance < bestDistance) { bestDistance = distance; best = l; }
		}

		return best;
	}

	// The lane most gun NPCs of the base of fire fight through. Sticky: the
	// NPCs moving about (pushers walking up it) must not flip it to another
	// doorway and send everybody round the room after the new one.
	int FrontLane(const ApproachMap& map, const Members& gunners, const TargetBelief& belief)
	{
		if (!map.Valid()) return -1;

		std::vector<int> votes(map.Lanes().size(), 0);
		for (const CombatBehaviorBase* brain : gunners)
		{
			const SquadRole role = brain->GetSquadOrder().role;
			if (role == SquadRole::Flank || role == SquadRole::Push) continue;

			const int lane = map.LaneOf(brain->GetOwner()->Position);
			if (lane >= 0) votes[lane]++;
		}

		const auto best = std::max_element(votes.begin(), votes.end());
		const int voted = best == votes.end() || *best == 0 ? -1 : static_cast<int>(best - votes.begin());

		const int previous = belief.hasFrontLane ? SameLane(map, belief.frontEntry) : -1;
		if (previous < 0) return voted;
		if (voted < 0 || votes[voted] - votes[previous] < CombatTuning::Get().squad.frontLaneSwitchVotes) return previous;

		return voted;
	}

	// Lane flank (§8.7): send flankers through the ways in that come at the
	// target from well round the front lane, each to the hidden stack spot short
	// of its entry, by the route least in the target's sight. One per lane
	// while there are lanes to spare. Returns false when there is no lane to use
	// (open ground): the caller falls back to the angle-based pincer.
	bool AddLaneFlankers(TargetBelief& belief, const Members& gunners, const ApproachMap& map, int frontLane,
		const vec3& target, const vec3& targetEye, int wanted, double now)
	{
		const std::vector<ApproachLane>& lanes = map.Lanes();

		std::vector<int> options;
		for (int l = 0; l < static_cast<int>(lanes.size()); ++l)
		{
			if (l != frontLane && map.AngleBetween(frontLane, l) >= CombatTuning::Get().squad.minFlankLaneAngle)
				options.push_back(l);
		}
		if (options.empty()) return false;

		std::sort(options.begin(), options.end(), [&](int a, int b)
		{
			const float scoreA = map.AngleBetween(frontLane, a) - CombatTuning::Get().squad.flankLaneCostWeight * lanes[a].stackCost;
			const float scoreB = map.AngleBetween(frontLane, b) - CombatTuning::Get().squad.flankLaneCostWeight * lanes[b].stackCost;
			return scoreA > scoreB;
		});

		// Who already goes where.
		std::vector<int> taken(lanes.size(), 0);
		int flankers = 0;
		for (const CombatBehaviorBase* brain : gunners)
		{
			const SquadOrder& order = brain->GetSquadOrder();
			if (order.role != SquadRole::Flank) continue;

			flankers++;
			if (order.hasLane)
			{
				const int lane = map.LaneOf(order.entry);
				if (lane >= 0) taken[lane]++;
			}
		}

		std::vector<int> exhausted;

		while (flankers < wanted)
		{
			// A free lane first; two may share the only one there is.
			int lane = -1;
			for (int option : options)
			{
				if (std::find(exhausted.begin(), exhausted.end(), option) != exhausted.end()) continue;
				if (taken[option] == 0 || (options.size() == 1 && taken[option] < 2)) { lane = option; break; }
			}
			if (lane < 0) break;

			const ApproachLane& approach = lanes[lane];
			const vec3 slot = StackSlot(approach.entry, approach.stack, taken[lane]);

			// The nearest few (attack token holders keep shooting from the front if anyone else can go).
			Members candidates;
			for (const bool allowTokenHolders : { false, true })
			{
				for (CombatBehaviorBase* brain : gunners)
				{
					if (brain->GetSquadOrder().role == SquadRole::Flank || !CanTakeErrand(brain)) continue;
					if (!allowTokenHolders && brain->Fire().HasToken()) continue;
					if (std::find(candidates.begin(), candidates.end(), brain) != candidates.end()) continue;
					candidates.push_back(brain);
				}
				if (static_cast<int>(candidates.size()) >= CombatTuning::Get().squad.flankCandidates) break;
			}

			std::sort(candidates.begin(), candidates.end(), [&](const CombatBehaviorBase* a, const CombatBehaviorBase* b)
			{
				return glm::distance(a->GetOwner()->Position, slot) < glm::distance(b->GetOwner()->Position, slot);
			});
			if (static_cast<int>(candidates.size()) > CombatTuning::Get().squad.flankCandidates)
				candidates.resize(CombatTuning::Get().squad.flankCandidates);

			CombatBehaviorBase* best = nullptr;
			float bestCost = std::numeric_limits<float>::max();

			for (CombatBehaviorBase* brain : candidates)
			{
				bool reached = false;
				const std::vector<vec3> path = NavigationSystem::FindSimplePath(brain->GetOwner()->Position, slot, 0.0f, &reached, false);
				if (path.empty() || !reached) continue;

				float cost = 0.0f;
				if (!CoverFinder::RouteCost(path, target, targetEye, CombatTuning::Get().squad.flankRouteClearance, 1.5f, cost)) continue;
				if (cost > CombatTuning::Get().squad.maxFlankRoute * 1.5f) continue;

				if (cost < bestCost)
				{
					bestCost = cost;
					best = brain;
				}
			}

			if (!best)
			{
				exhausted.push_back(lane);
				continue;
			}

			SquadOrder order = MakeOrder(SquadRole::Flank, belief, now);
			order.until = now + CombatTuning::Get().squad.laneFlankTimeout;
			GiveLane(order, approach.entry, approach.stack, taken[lane]);
			best->SetSquadOrder(order);

			if (belief.firstLaneOrderAt == 0.0) belief.firstLaneOrderAt = now;

			taken[lane]++;
			flankers++;
		}

		return true;
	}

	// Open ground (no lanes): flankers swing round either side of the squad axis
	// to see the target from the side. NPCs already on a side swing further
	// round it, and a second flanker takes the other side: a pincer.
	void AddOpenFlankers(TargetBelief& belief, const Members& gunners, const vec3& target, int wanted, double now)
	{
		int flankers = 0;
		float takenSide = 0.0f;

		for (const CombatBehaviorBase* brain : gunners)
		{
			if (brain->GetSquadOrder().role != SquadRole::Flank) continue;
			flankers++;
			takenSide = brain->GetSquadOrder().side;
		}

		while (flankers < wanted)
		{
			CombatBehaviorBase* best = nullptr;
			float bestScore = -std::numeric_limits<float>::max();
			float bestSide = 1.0f;

			for (const bool allowTokenHolders : { false, true })
			{
				for (CombatBehaviorBase* brain : gunners)
				{
					if (brain->GetSquadOrder().role == SquadRole::Flank || !CanTakeErrand(brain)) continue;
					if (!allowTokenHolders && brain->Fire().HasToken()) continue;

					const float distance = TacticHelpers::DistanceXZ(brain->GetOwner()->Position, target);
					if (distance > CombatTuning::Get().squad.roleMaxDistance || distance < CombatTuning::Get().squad.flankMinStartDistance) continue;

					const float angle = AngleFromAxis(brain, target, belief.squadAxis);

					float side = angle < 0.0f ? -1.0f : 1.0f;
					if (takenSide != 0.0f) side = -takenSide;
					else if (std::abs(angle) < 0.05f) side = TacticHelpers::StableRandom01(brain->GetOwner()->Id, 21) < 0.5f ? -1.0f : 1.0f;

					// Already round that side: less to walk. Closer: there sooner.
					const float score = angle * side - CombatTuning::Get().squad.flankDistanceWeight * distance;
					if (score > bestScore)
					{
						bestScore = score;
						best = brain;
						bestSide = side;
					}
				}

				if (best) break;
			}

			if (!best) return;

			SquadOrder order = MakeOrder(SquadRole::Flank, belief, now);
			order.side = bestSide;
			order.until = now + CombatTuning::Get().squad.flankTimeout;
			best->SetSquadOrder(order);

			takenSide = bestSide;
			flankers++;
		}
	}

	// Breach (§8.7): the NPCs stacked on lanes go in together, and for a moment
	// the whole base of fire comes out shooting, so the target is hit from the
	// front and the side at once. Called when everybody with a lane is stacked,
	// when the first one has waited StackWaitMax, when the target reloads or is
	// stunned with someone stacked, or BreachTimeout after the first lane order.
	void UpdateBreach(TargetBelief& belief, const Members& gunners, const CombatTargetView& view, double now)
	{
		Members assault;
		bool anyStacked = false;
		bool allStacked = true;
		bool spotted = false;
		int baseOfFire = 0;

		for (CombatBehaviorBase* brain : gunners)
		{
			const SquadOrder& order = brain->GetSquadOrder();
			if (order.role != SquadRole::Flank && order.role != SquadRole::Push) baseOfFire++;
			if (!order.hasLane || (order.role != SquadRole::Flank && order.role != SquadRole::Push)) continue;

			const LanePhase phase = brain->GetLanePhase();
			if (phase == LanePhase::Done) continue;

			assault.push_back(brain);
			if (phase == LanePhase::Stacked) anyStacked = true;
			if (phase == LanePhase::Breaching) spotted = true; // went in on its own: spotted, no waiting
			if (phase == LanePhase::Moving && !order.waitTurn) allStacked = false;
		}

		// Open-ground flankers count too: the base of fire waits for them.
		bool openFlankers = false;
		for (const CombatBehaviorBase* brain : gunners)
		{
			const SquadOrder& order = brain->GetSquadOrder();
			if (order.role == SquadRole::Flank && !order.hasLane && !brain->FlankFinished()) openFlankers = true;
		}

		if (assault.empty())
		{
			belief.flankUnderWay = openFlankers;
			belief.firstLaneOrderAt = 0.0;
			belief.firstStackedAt = 0.0;
			belief.breachCalled = false;
			return;
		}

		if (anyStacked && belief.firstStackedAt == 0.0)
			belief.firstStackedAt = now;

		if (!belief.breachCalled)
		{
			const bool opening = view.Valid() && (view.isReloading || view.isStunned);
			const double waitMax = belief.contact == TargetContact::Visible ? CombatTuning::Get().squad.stackWaitVisible : CombatTuning::Get().squad.stackWaitMax;
			const bool ready = anyStacked && (allStacked || now - belief.firstStackedAt >= waitMax || opening);
			const bool late = belief.firstLaneOrderAt > 0.0 && now - belief.firstLaneOrderAt >= CombatTuning::Get().squad.breachTimeout;

			// Nobody left at the front to keep the target busy: nothing to wait for.
			const bool frontGone = baseOfFire == 0;

			if (ready || late || frontGone || spotted)
			{
				belief.breachCalled = true;
				belief.assaultUntil = now + CombatTuning::Get().squad.assaultTime;
			}
		}

		belief.flankUnderWay = !belief.breachCalled;

		if (!belief.breachCalled) return;

		for (CombatBehaviorBase* brain : assault)
		{
			SquadOrder order = brain->GetSquadOrder();
			if (order.breach && !order.waitTurn) continue;

			order.breach = true;
			order.waitTurn = false;
			brain->SetSquadOrder(order);
		}
	}

	// Pushers go up the front lane in turns: one moves up while the others hold
	// where they are with the gun on the entry, then the next. A pusher already
	// close to the hiding target gets no lane: it works round the corner the
	// target is behind (TacticRegainSight, close quarters) instead of running
	// off to a doorway. Pushers keep the lane they were given while it exists.
	void OrderPushers(TargetBelief& belief, const Members& pushers, const vec3& target, const ApproachMap& map)
	{
		Members laned;
		for (CombatBehaviorBase* brain : pushers)
		{
			SquadOrder order = brain->GetSquadOrder();

			if (!belief.hasFrontLane
				|| TacticHelpers::DistanceXZ(brain->GetOwner()->Position, target) < CombatTuning::Get().squad.closeQuartersDistance)
			{
				order.hasLane = false;
				order.waitTurn = false;
				brain->SetSquadOrder(order);
				continue;
			}

			laned.push_back(brain);
		}

		Members ordered = laned;
		std::sort(ordered.begin(), ordered.end(), [&](const CombatBehaviorBase* a, const CombatBehaviorBase* b)
		{
			return glm::distance(a->GetOwner()->Position, belief.frontStack) < glm::distance(b->GetOwner()->Position, belief.frontStack);
		});

		bool someoneMoving = false;
		int slot = 0;

		for (CombatBehaviorBase* brain : ordered)
		{
			SquadOrder order = brain->GetSquadOrder();

			// Its own lane (re-found after a rebuild), or the front for a new pusher.
			const int kept = order.hasLane ? SameLane(map, order.entry) : -1;
			if (kept >= 0)
				GiveLane(order, map.Lanes()[kept].entry, map.Lanes()[kept].stack, slot++);
			else
				GiveLane(order, belief.frontEntry, belief.frontStack, slot++);

			const LanePhase phase = brain->GetLanePhase();
			const bool moving = phase == LanePhase::Moving;

			order.waitTurn = moving && someoneMoving && !order.breach;
			if (moving && !order.waitTurn) someoneMoving = true;

			brain->SetSquadOrder(order);
		}
	}

	// Settling (§8.8): has the base of fire taken its positions, and is the
	// target holding its ground or running? The target moving more than
	// squad.anchorRadius re-anchors it; a move that took it away from the squad
	// is a retreat, and the squad's NPCs in cover move up after it.
	void UpdateSettling(TargetBelief& belief, const Members& gunners, const vec3& target, double now)
	{
		const auto& tune = CombatTuning::Get().squad;

		if (TacticHelpers::DistanceXZ(target, belief.anchorPosition) > tune.anchorRadius)
		{
			vec3 centroid{};
			for (const CombatBehaviorBase* brain : gunners)
				centroid += brain->GetOwner()->Position;
			centroid /= static_cast<float>(std::max<size_t>(1, gunners.size()));

			belief.targetRetreating = TacticHelpers::DistanceXZ(target, centroid)
				> TacticHelpers::DistanceXZ(belief.anchorPosition, centroid) + tune.retreatMargin;

			belief.anchorPosition = target;
			belief.anchorSince = now;

			// Pursuit: only the NPCs it has left out of their fighting range follow.
			if (belief.targetRetreating)
			{
				for (CombatBehaviorBase* brain : gunners)
				{
					const NpcBase* owner = brain->GetOwner();
					const float range = owner->GetAttackRange() * CombatTuning::Get().cover.pursuitRangeFraction;

					if (brain->GetSquadOrder().role != SquadRole::Flank && TacticHelpers::DistanceXZ(owner->Position, target) > range)
						brain->Cover().RequestAdvance();
				}
			}
		}

		belief.targetHolding = now - belief.anchorSince >= tune.holdTime;

		int base = 0;
		int inCover = 0;
		for (const CombatBehaviorBase* brain : gunners)
		{
			if (brain->GetSquadOrder().role == SquadRole::Flank) continue;
			base++;
			if (brain->Cover().Arrived()) inCover++;
		}

		const double settling = now - belief.fightSince;
		const bool enoughInCover = base > 0 && static_cast<float>(inCover) >= tune.settledCoverFraction * static_cast<float>(base);
		belief.squadSettled = settling >= tune.settleMinTime && (enoughInCover || settling >= tune.settleMaxTime);
	}

	// Hands each gun NPC its part. Visible: everybody fights from the front
	// (the base of fire) while flankers go round, through the approach map's
	// lanes when there are any. Concealed: the flankers keep going, the NPCs with
	// a line on the hiding spot suppress it for a few seconds, the rest push up
	// the front lane in turns; when the suppression ends one NPC holds the
	// entry and the others push. Whoever is stacked breaches together. Lost: one
	// checks (through another lane than the front), one watches.
	void PlanSquad(TargetBelief& belief, const Members& members, const CombatTargetView& view, const ApproachMap& map, double now)
	{
		Members gunners;
		for (CombatBehaviorBase* brain : members)
		{
			if (IsGunner(brain)) gunners.push_back(brain);
			else if (belief.contact != TargetContact::Lost) brain->SetSquadOrder(SquadOrder()); // melee slots are their coordination
		}

		const bool visible = belief.contact == TargetContact::Visible;
		const vec3 target = visible ? belief.lastVisiblePosition : belief.estimatePosition;
		const vec3 targetEye = target + (view.Valid() ? view.eyePosition - view.position : vec3(0, 0.6f, 0));

		// The target starts a reload: the NPCs in cover move up to closer cover
		// while it can't shoot back.
		const bool reloading = visible && view.isReloading;
		if (reloading && !belief.targetWasReloading && CombatTuning::Get().cover.advanceOnReload)
		{
			for (CombatBehaviorBase* brain : gunners)
			{
				if (brain->GetSquadOrder().role != SquadRole::Flank)
					brain->Cover().RequestAdvance();
			}
		}
		belief.targetWasReloading = reloading;

		if (now < belief.nextPlanTime) return;
		belief.nextPlanTime = now + CombatTuning::Get().squad.planInterval;

		belief.squadAxis = SquadAxis(gunners, target, belief.squadAxis);

		const int frontLane = FrontLane(map, gunners, belief);
		belief.hasFrontLane = frontLane >= 0;
		if (belief.hasFrontLane)
		{
			const ApproachLane& front = map.Lanes()[frontLane];
			belief.frontEntry = front.entry;
			belief.frontStack = front.stack;
			belief.frontDirection = front.direction;
		}

		UpdateSettling(belief, gunners, target, now);

		// Flankers that got there (or gave up), whose lane has become the front
		// (the target moved), or that are still on the way while the target runs
		// off (the squad pursues instead), rejoin the base of fire.
		for (CombatBehaviorBase* brain : gunners)
		{
			const SquadOrder& order = brain->GetSquadOrder();
			if (order.role != SquadRole::Flank) continue;

			const bool onTheWay = brain->GetLanePhase() == LanePhase::Moving;

			bool laneIsFront = false;
			if (order.hasLane && belief.hasFrontLane && onTheWay)
			{
				const vec3 laneDirection = MathHelper::Normalized(MathHelper::XZ(order.entry - target));
				laneIsFront = glm::dot(laneDirection, belief.frontDirection) > std::cos(glm::radians(CombatTuning::Get().squad.minFlankLaneAngle));
			}

			const bool targetRanOff = belief.targetRetreating && !belief.targetHolding && onTheWay;

			if (brain->FlankFinished() || now > order.until || laneIsFront || targetRanOff
				|| belief.contact == TargetContact::Lost || belief.contact == TargetContact::None)
			{
				brain->SetSquadOrder(MakeOrder(SquadRole::None, belief, now));
				belief.nextFlankTime = std::max(belief.nextFlankTime, now + CombatTuning::Get().squad.flankCooldown);
			}
		}

		const auto addFlankers = [&](bool hasGun)
		{
			const int wanted = FlankersWanted(static_cast<int>(gunners.size()));
			if (wanted == 0) return;

			if (!(belief.hasFrontLane && AddLaneFlankers(belief, gunners, map, frontLane, target, targetEye, wanted, now)) && hasGun)
				AddOpenFlankers(belief, gunners, target, wanted, now);
		};

		switch (belief.contact)
		{
		case TargetContact::Visible:
		{
			for (CombatBehaviorBase* brain : gunners)
			{
				if (brain->GetSquadOrder().role != SquadRole::Flank)
					Assign(brain, SquadRole::None, belief, now);
			}

			// Flankers go once the squad has taken its positions and the target has
			// shown it is staying put; a target on the move is pursued instead.
			// Going round a target that can't shoot gains nothing in the open (the
			// NPCs kite it); through a building's doors it still corners it.
			if (now >= belief.nextFlankTime && belief.squadSettled && belief.targetHolding)
				addFlankers(view.hasGun);
			break;
		}

		case TargetContact::Concealed:
		{
			if (belief.ConcealedFor(now) >= CombatTuning::Get().squad.flankerDelay)
				addFlankers(true);

			// The main force keeps the pressure on from where it is: the NPCs with a
			// line on the spot suppress it for a while, everybody else holds the ways
			// out (moving a little for a line when it has none). Only the flankers
			// move in; with nobody flanking, pushersWithoutFlank NPCs push up.
			const auto& tune = CombatTuning::Get().squad;
			const bool suppressing = now < belief.suppressUntil;

			int flanking = 0;
			for (const CombatBehaviorBase* brain : gunners)
			{
				if (brain->GetSquadOrder().role == SquadRole::Flank && !brain->FlankFinished()) flanking++;
			}
			const int pushersAllowed = flanking == 0 ? tune.pushersWithoutFlank : 0;

			// Nearest first: the suppressors are the ones best placed to hit the spot,
			// and a pusher (if any) has the least ground to cover.
			Members ordered = gunners;
			std::sort(ordered.begin(), ordered.end(), [&](const CombatBehaviorBase* a, const CombatBehaviorBase* b)
			{
				return glm::distance(a->GetOwner()->Position, target) < glm::distance(b->GetOwner()->Position, target);
			});

			int suppressors = 0;
			Members pushers;
			for (CombatBehaviorBase* brain : ordered)
			{
				if (brain->GetSquadOrder().role == SquadRole::Flank) continue;

				const bool inRange = glm::distance(brain->GetOwner()->Position, target) < tune.roleMaxDistance;

				if (suppressing && suppressors < tune.maxSuppressors && inRange && HasLineOfFire(brain, target))
				{
					Assign(brain, SquadRole::Suppress, belief, now);
					suppressors++;
				}
				else if (static_cast<int>(pushers.size()) < pushersAllowed
					&& (brain->GetSquadOrder().role == SquadRole::Push || !HasLineOfFire(brain, target)))
				{
					Assign(brain, SquadRole::Push, belief, now);
					pushers.push_back(brain);
				}
				else
				{
					Assign(brain, SquadRole::Hold, belief, now);

					SquadOrder order = brain->GetSquadOrder();
					order.hasLane = belief.hasFrontLane;
					order.entry = belief.frontEntry;
					order.stack = belief.frontStack;
					brain->SetSquadOrder(order);
				}
			}

			OrderPushers(belief, pushers, target, map);
			break;
		}

		case TargetContact::Lost:
		{
			// Melee NPCs can check a corner too.
			CombatBehaviorBase* checker = nullptr;
			CombatBehaviorBase* watcher = nullptr;

			for (CombatBehaviorBase* brain : members)
			{
				if (brain->GetSquadOrder().role == SquadRole::Check) checker = brain;
				if (brain->GetSquadOrder().role == SquadRole::Watch) watcher = brain;
			}

			const auto anyone = [](const CombatBehaviorBase*) { return true; };
			if (!checker) checker = Nearest(members, target, anyone, watcher);
			if (!watcher) watcher = Nearest(members, target, anyone, checker);

			// The checker comes in another way than the obvious one; the watcher
			// covers the obvious one from its stack spot.
			int checkLane = -1;
			if (map.Valid())
			{
				float bestCost = std::numeric_limits<float>::max();
				for (int l = 0; l < static_cast<int>(map.Lanes().size()); ++l)
				{
					if (l == frontLane) continue;
					if (map.Lanes()[l].stackCost < bestCost) { bestCost = map.Lanes()[l].stackCost; checkLane = l; }
				}
			}

			for (CombatBehaviorBase* brain : members)
			{
				const SquadRole role = brain == checker ? SquadRole::Check : (brain == watcher ? SquadRole::Watch : SquadRole::None);
				const bool fresh = brain->GetSquadOrder().role != role;
				Assign(brain, role, belief, now);

				if (!fresh) continue;

				SquadOrder order = brain->GetSquadOrder();
				if (role == SquadRole::Check && checkLane >= 0)
					GiveLane(order, map.Lanes()[checkLane].entry, map.Lanes()[checkLane].stack, 0);
				else if (role == SquadRole::Watch && belief.hasFrontLane)
					GiveLane(order, belief.frontEntry, belief.frontStack, 0);
				brain->SetSquadOrder(order);
			}
			break;
		}

		case TargetContact::None:
			for (CombatBehaviorBase* brain : members)
				brain->SetSquadOrder(SquadOrder());
			break;
		}

		UpdateBreach(belief, gunners, view, now);
	}
}

const char* LanePhaseName(LanePhase phase)
{
	switch (phase)
	{
	case LanePhase::Stacked:   return "stacked";
	case LanePhase::Breaching: return "breaching";
	case LanePhase::Done:      return "done";
	default:                   return "moving";
	}
}

void CombatCoordinator::DrawApproachMap(const std::string& targetId)
{
	const auto it = approachMaps.find(targetId);
	if (it != approachMaps.end())
		it->second.DebugDraw();
}

const char* SquadRoleName(SquadRole role)
{
	switch (role)
	{
	case SquadRole::Flank:    return "flank";
	case SquadRole::Suppress: return "suppress";
	case SquadRole::Hold:     return "hold";
	case SquadRole::Push:     return "push";
	case SquadRole::Check:    return "check";
	case SquadRole::Watch:    return "watch";
	default:                  return "base of fire";
	}
}

void CombatCoordinator::UpdateBeliefs()
{
	const double now = Time::GameTime;
	const float deltaTime = Time::DeltaTimeF;

	std::unordered_map<std::string, Members> groups;
	for (CombatBehaviorBase* brain : brains)
	{
		if (brain->IsEngaged())
			groups[brain->GetTargetView().entity->Id].push_back(brain);
		else
			brain->SetSquadOrder(SquadOrder());
	}

	for (auto it = beliefs.begin(); it != beliefs.end();)
	{
		if (groups.count(it->first) == 0) it = beliefs.erase(it);
		else ++it;
	}

	for (auto it = approachMaps.begin(); it != approachMaps.end();)
	{
		if (groups.count(it->first) == 0) it = approachMaps.erase(it);
		else ++it;
	}

	for (auto& entry : groups)
	{
		const Members& members = entry.second;
		TargetBelief& belief = beliefs[entry.first];

		belief.targetId = entry.first;
		belief.memberCount = static_cast<int>(members.size());

		belief.exposedCount = 0;
		for (const CombatBehaviorBase* brain : members)
		{
			if (!brain->IsMeleeMode() && brain->Fire().ExposedWithin(CombatTuning::Get().belief.exposedNowWindow))
				belief.exposedCount++;
		}

		CombatBehaviorBase* seer = nullptr;
		for (CombatBehaviorBase* brain : members)
		{
			if (brain->SeesTargetNow()) { seer = brain; break; }
		}

		const CombatTargetView& view = (seer ? seer : members.front())->GetTargetView();
		belief.targetIsPlayer = view.isPlayer;

		if (seer)
		{
			// A new fight (not the target popping back out): the squad settles
			// before anyone goes round.
			if (belief.contact == TargetContact::None || belief.contact == TargetContact::Lost)
			{
				belief.fightSince = now;
				belief.nextFlankTime = now + CombatTuning::Get().squad.settleMinTime;
				belief.anchorPosition = view.position;
				belief.anchorSince = now;
				belief.targetRetreating = false;
			}

			if (belief.contact != TargetContact::Visible)
				belief.nextPlanTime = 0.0;

			belief.contact = TargetContact::Visible;
			belief.lastVisiblePosition = view.position;
			belief.lastVisibleVelocity = view.velocity;
			belief.lastVisibleTime = now;

			ShareWithSquad(members, belief.targetId, view.position, true);
			belief.emptyTime = 0.0f;
			belief.blindExposedTime = 0.0f;
		}
		else
		{
			vec3 heardAt;
			if (HearTarget(belief, members, heardAt))
				OnTargetHeard(belief, members, heardAt, now);

			switch (belief.contact)
			{
			case TargetContact::Visible:
			{
				bool anyExposed = false;
				for (const CombatBehaviorBase* brain : members)
				{
					if (brain->IsMeleeMode() || brain->Fire().ExposedWithin(0.25)) { anyExposed = true; break; }
				}

				if (anyExposed)
					belief.blindExposedTime += deltaTime;

				if (belief.blindExposedTime >= CombatTuning::Get().belief.exposedBlindTime || now - belief.lastVisibleTime > CombatTuning::Get().belief.maxBlindTime)
					EnterConcealed(belief, members, now);
				break;
			}

			case TargetContact::Concealed:
				UpdateConcealed(belief, members, view, deltaTime, now);
				break;

			case TargetContact::Lost:
				// Backstop: the brains end this phase themselves when their check is done.
				if (now - belief.lostSince > CombatTuning::Get().belief.lostMaxTime)
				{
					belief.contact = TargetContact::None;
					belief.nextPlanTime = 0.0;
				}
				break;

			case TargetContact::None:
				break;
			}
		}

		// How the ground round the target is reached (§8.7), rebuilt as it moves.
		ApproachMap& map = approachMaps[entry.first];
		if (belief.contact != TargetContact::None)
		{
			const vec3 mapTarget = belief.contact == TargetContact::Visible ? belief.lastVisiblePosition : belief.estimatePosition;
			if (map.NeedsRebuild(mapTarget, now))
				map.Build(mapTarget, mapTarget + (view.Valid() ? view.eyePosition - view.position : vec3(0, 0.6f, 0)), now);
		}

		PlanSquad(belief, members, view, map, now);
	}
}

// ── Melee slots ──────────────────────────────────────────────────────────────

// Against the player only MeleeSlotsPerTarget melee NPCs attack at once; the
// others wait on a ring around him. Against NPC targets there is no limit.
void CombatCoordinator::ResolveMeleeSlots()
{
	const double now = Time::GameTime;

	struct Bidder
	{
		CombatBehaviorBase* brain;
		float bid;
	};
	std::unordered_map<std::string, std::vector<Bidder>> bidsByTarget;

	for (CombatBehaviorBase* brain : brains)
	{
		const CombatTargetView& view = brain->GetTargetView();

		if (!view.Valid())
		{
			brain->SetMeleeSlot(false, now);
			continue;
		}

		if (!view.isPlayer)
		{
			brain->SetMeleeSlot(true, now);
			continue;
		}

		if (!brain->WantsMeleeSlot())
		{
			brain->SetMeleeSlot(false, now);
			continue;
		}

		float bid = 1.0f / (1.0f + glm::distance(brain->GetOwner()->Position, view.position));

		if (brain->HasMeleeSlot() && now - brain->MeleeSlotHeldSince() < CombatTuning::Get().tokens.minSlotHoldTime)
			bid += 1000.0f;

		bidsByTarget[view.entity->Id].push_back({ brain, bid });
	}

	for (auto& entry : bidsByTarget)
	{
		std::vector<Bidder>& bidders = entry.second;

		std::sort(bidders.begin(), bidders.end(),
			[](const Bidder& a, const Bidder& b) { return a.bid > b.bid; });

		const int slots = bidders.front().brain->GetTargetView().hasGun ? CombatTuning::Get().tokens.meleeSlotsVsGunTarget : CombatTuning::Get().tokens.meleeSlotsPerTarget;

		for (size_t i = 0; i < bidders.size(); ++i)
			bidders[i].brain->SetMeleeSlot(static_cast<int>(i) < slots, now);
	}
}

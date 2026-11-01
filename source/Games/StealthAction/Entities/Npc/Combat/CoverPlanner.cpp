#include "CoverPlanner.h"

#include "CombatBehaviorBase.h"
#include "CombatCoordinator.h"
#include "CoverFinder.h"
#include "../NpcBase.h"
#include "CombatTuning.h"

#include <RandomHelper.h>

#include <algorithm>

void CoverPlanner::Abandon()
{
	if (!spot.Valid()) return;

	previousFloor = spot.floor;
	hasPrevious = true;

	Release();
	nextSearch = 0.0;
}

void CoverPlanner::Release()
{
	if (spot.Valid())
		CombatCoordinator::ReleaseCover(brain);

	spot = CoverSpot();
	arrived = false;
}


// Standing in the spot, or (Full cover) anywhere on the short step out to the
// peek position and back.
bool CoverPlanner::InCoverArea(const vec3& position) const
{
	const vec3 p = MathHelper::XZ(position);
	const vec3 a = MathHelper::XZ(spot.position);
	const vec3 b = MathHelper::XZ(spot.peekPosition);

	const vec3 segment = b - a;
	const float lengthSquared = glm::dot(segment, segment);
	const float along = lengthSquared > 0.0001f ? glm::clamp(glm::dot(p - a, segment) / lengthSquared, 0.0f, 1.0f) : 0.0f;

	return glm::distance(p, a + segment * along) < CombatTuning::Get().cover.arriveDistance;
}

void CoverPlanner::Update(const CombatContext& ctx, bool wantCover)
{
	if (!wantCover)
	{
		Release();
		return;
	}

	const double now = Time::GameTime;
	const NpcBase* owner = ctx.owner;

	if (spot.Valid())
	{
		const bool wasArrived = arrived;
		arrived = InCoverArea(owner->Position);

		if (arrived && !wasArrived)
		{
			arrivedSince = now;
			stayTime = CombatTuning::Get().cover.minStayTime + RandomHelper::RandomFloat() * CombatTuning::Get().cover.stayJitter;
		}

		// Time for different cover, once the target has stopped showing from this one.
		if (arrived && now - arrivedSince > stayTime
			&& brain->Fire().UnseenTime() > CombatTuning::Get().cover.relocateBlindTime)
		{
			previousFloor = spot.floor;
			hasPrevious = true;

			Release();
			nextSearch = now;
		}
	}

	if (advanceRequested && spot.Valid())
	{
		// Only from a spot it has settled in (one on the way is close enough
		// already), and not again and again while the target keeps moving.
		if (!arrived || now - lastAdvanceTime < CombatTuning::Get().cover.advanceCooldown)
		{
			advanceRequested = false;
		}
		else if (now - arrivedSince > CombatTuning::Get().cover.advanceMinStay)
		{
			advanceRequested = false;

			CoverSpot closer;
			if (Search(ctx, true, closer))
			{
				previousFloor = spot.floor;
				hasPrevious = true;

				spot = closer; // the reservation moved with the search
				arrived = false;
				lastAdvanceTime = now;
				nextValidate = now + CombatTuning::Get().cover.validateInterval;
			}
		}
	}

	if (spot.Valid())
	{
		// The target moved: if the spot no longer hides us, find another one at once.
		if (now >= nextValidate)
		{
			nextValidate = now + CombatTuning::Get().cover.validateInterval;

			if (!CoverFinder::IsProtective(spot, ctx.threatEye))
			{
				Release();
				nextSearch = now;
			}
		}
	}

	if (spot.Valid() || now < nextSearch) return;

	CoverSpot found;
	if (Search(ctx, false, found))
	{
		spot = found;
		arrived = false;
		nextValidate = now + CombatTuning::Get().cover.validateInterval;
	}
	else
	{
		nextSearch = now + CombatTuning::Get().cover.retryInterval;
		return;
	}

	nextSearch = now + CombatTuning::Get().cover.searchInterval + RandomHelper::RandomFloat() * CombatTuning::Get().cover.searchJitter;
}

// One cover search, reserving what it finds. `advance` only accepts spots
// AdvanceDistance closer to the threat than the NPC stands now.
bool CoverPlanner::Search(const CombatContext& ctx, bool advance, CoverSpot& found) const
{
	const NpcBase* owner = ctx.owner;

	CoverQuery query;
	query.self = owner;
	query.npcPosition = owner->Position;
	query.npcCenterHeight = owner->GetCenterHeight();
	query.threatEye = ctx.threatEye;
	query.threatPosition = ctx.threatPosition;
	query.minThreatDistance = CombatTuning::Get().cover.minThreatDistance;
	query.maxThreatDistance = owner->GetAttackRange() * CombatTuning::Get().cover.rangedMaxFraction;
	query.preferredThreatDistance = owner->GetAttackDesiredRange();
	query.maxTravelDistance = CombatTuning::Get().cover.maxTravelDistance;
	query.maxRetreat = CombatTuning::Get().cover.maxRetreat;

	if (ctx.belief && ctx.belief->hasFrontLane && !ctx.Hunting())
	{
		query.hasLaneEntry = true;
		query.laneEntry = ctx.belief->frontEntry;
	}

	if (hasPrevious)
	{
		query.avoidPosition = previousFloor;
		query.avoidRadius = CombatTuning::Get().cover.avoidPreviousRadius;
	}

	// Back from a flank: cover here or closer, not back out the way it came.
	if (Time::GameTime - brain->FlankEndedAt() < CombatTuning::Get().squad.flankHoldTime)
		query.maxRetreat = std::min(query.maxRetreat, 0.0f);

	if (advance)
	{
		query.maxRetreat = -CombatTuning::Get().cover.advanceDistance;
		query.maxTravelDistance = CombatTuning::Get().cover.advanceTravelDistance;
		query.preferredThreatDistance = std::max(CombatTuning::Get().cover.minThreatDistance,
			glm::distance(owner->Position, ctx.threatPosition) - CombatTuning::Get().cover.advanceDistance - 1.0f);
		query.avoidPosition = spot.floor;
		query.avoidRadius = CombatTuning::Get().cover.avoidPreviousRadius;
	}

	return CoverFinder::Find(query,
		[this](const vec3& floor) { return CombatCoordinator::ReserveCover(brain, floor); },
		found);
}

#include "CoverFinder.h"

#include "CoverMap.h"
#include "CombatTuning.h"

#include <AiPerception/AiPerceptionSystem.h>
#include <AiPerception/Observer.h>
#include <MathHelper.hpp>
#include <Navigation/Navigation.hpp>
#include <Physics.h>
#include <RandomHelper.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <mutex>

namespace
{
	std::vector<CoverHint> hints;
	std::mutex hintsMutex;

	constexpr int RingCount = 4;
	constexpr int SamplesPerRing = 8;
	constexpr float FirstRingRadius = 3.0f;

	constexpr int VerifyCount = 4; // best candidates given real traces and a path check

	constexpr float MaxGroundStep = 2.0f;         // candidate ground vs the NPC's ground
	constexpr float ShootHeight = 1.5f;           // gun height when standing
	constexpr float PeekSideDistance = 1.0f;      // how far Full cover steps out sideways
	constexpr float PathLengthRatio = 1.6f;       // reject paths longer than this x straight distance + slack
	constexpr float PathLengthSlack = 1.5f;

	constexpr float ClusterSigma = 2.0f;
	constexpr float RandomScore = 0.2f;

	struct Candidate
	{
		vec3 floor{};
		CoverType type = CoverType::None;
		float score = 0.0f;
	};

	bool FindGround(const vec3& around, vec3& outFloor)
	{
		const auto hit = Physics::LineTrace(around + vec3(0, 1.5f, 0), around - vec3(0, 3.0f, 0), BodyType::World);
		if (!hit.hasHit) return false;

		outFloor = hit.position;
		return true;
	}

	float PathLength(const std::vector<vec3>& path)
	{
		float length = 0.0f;
		for (size_t i = 1; i < path.size(); ++i)
			length += glm::distance(path[i - 1], path[i]);
		return length;
	}

	// Fills the peek position for a verified candidate. Low cover shoots from
	// where it stands; Full cover steps out on whichever side sees the threat.
	bool PlanShot(const Candidate& candidate, const CoverQuery& query, CoverSpot& out)
	{
		const vec3 center = vec3(0, query.npcCenterHeight, 0);
		const vec3 toThreat = MathHelper::Normalized(MathHelper::XZ(query.threatPosition - candidate.floor));

		out.type = candidate.type;
		out.floor = candidate.floor;
		out.position = candidate.floor + center;
		out.score = candidate.score;

		if (candidate.type == CoverType::Low)
		{
			// Must be able to shoot over it.
			if (Physics::LineTrace(candidate.floor + vec3(0, ShootHeight, 0), query.threatEye, CoverMap::BlockerMask()).hasHit)
				return false;

			out.peekPosition = out.position;
			return true;
		}

		const vec3 side = MathHelper::Normalized(glm::cross(toThreat, vec3(0, 1, 0)));

		for (float sign : { 1.0f, -1.0f })
		{
			const vec3 peekFloor = candidate.floor + side * sign * PeekSideDistance;

			vec3 ground;
			if (!FindGround(peekFloor, ground) || std::abs(ground.y - candidate.floor.y) > 0.5f) continue;

			if (Physics::LineTrace(ground + vec3(0, ShootHeight, 0), query.threatEye, CoverMap::BlockerMask()).hasHit) continue;

			out.peekPosition = ground + center;
			return true;
		}

		return false;
	}
}

void CoverFinder::AddHint(const CoverHint& hint)
{
	std::lock_guard<std::mutex> lock(hintsMutex);
	hints.push_back(hint);
}

void CoverFinder::RemoveHint(const std::string& entityId)
{
	std::lock_guard<std::mutex> lock(hintsMutex);
	hints.erase(std::remove_if(hints.begin(), hints.end(),
		[&](const CoverHint& h) { return h.entityId == entityId; }), hints.end());
}

bool CoverFinder::IsProtective(const CoverSpot& spot, const vec3& threatEye)
{
	if (!spot.Valid()) return false;

	if (!Physics::LineTrace(threatEye, spot.floor + vec3(0, CoverMap::CrouchHeadHeight, 0), CoverMap::BlockerMask()).hasHit)
		return false;

	// Full cover is used standing up.
	if (spot.type == CoverType::Full)
		return Physics::LineTrace(threatEye, spot.floor + vec3(0, CoverMap::StandHeadHeight, 0), CoverMap::BlockerMask()).hasHit;

	return true;
}

bool CoverFinder::Find(const CoverQuery& query, const Claim& claim, CoverSpot& out)
{
	const float npcGroundY = query.npcPosition.y - query.npcCenterHeight;
	const float currentThreatDistance = glm::distance(MathHelper::XZ(query.npcPosition), MathHelper::XZ(query.threatPosition));

	// Other observers: spread out, don't pile into one corner.
	std::vector<vec3> allies;
	for (const auto& observer : AiPerceptionSystem::GetObserversInRadius(query.npcPosition, query.searchRadius + 2.0f))
	{
		if (observer->ownerPtr == query.self) continue;
		allies.push_back(observer->position - vec3(0, 0.7f, 0));
	}

	// ── Candidates: rings around the NPC, plus hints ─────────────────────────
	std::vector<vec3> samples;
	std::vector<float> sampleBonus;
	std::vector<CoverType> sampleType;

	const float ringStep = (query.searchRadius - FirstRingRadius) / std::max(1, RingCount - 1);
	for (int ring = 0; ring < RingCount; ++ring)
	{
		const float radius = FirstRingRadius + ringStep * ring;
		const float phase = RandomHelper::RandomFloat() * (2.0f * glm::pi<float>());

		for (int i = 0; i < SamplesPerRing; ++i)
		{
			const float angle = phase + (2.0f * glm::pi<float>()) * i / SamplesPerRing;
			samples.push_back(query.npcPosition + vec3(std::cos(angle), 0, std::sin(angle)) * radius);
			sampleBonus.push_back(0.0f);
			sampleType.push_back(CoverType::None);
		}
	}

	{
		std::lock_guard<std::mutex> lock(hintsMutex);

		for (const CoverHint& hint : hints)
		{
			if (glm::distance(MathHelper::XZ(hint.floor), MathHelper::XZ(query.npcPosition)) > query.searchRadius) continue;

			samples.push_back(hint.floor);
			sampleBonus.push_back(hint.priority);
			sampleType.push_back(hint.type);
		}
	}

	// ── Score from the cache ─────────────────────────────────────────────────
	int traceBudget = query.traceBudget;
	std::vector<Candidate> candidates;

	for (size_t i = 0; i < samples.size(); ++i)
	{
		const float threatDistance = glm::distance(MathHelper::XZ(samples[i]), MathHelper::XZ(query.threatPosition));
		if (threatDistance < query.minThreatDistance || threatDistance > query.maxThreatDistance) continue;
		if (threatDistance > currentThreatDistance + query.maxRetreat) continue;
		if (glm::distance(MathHelper::XZ(samples[i]), MathHelper::XZ(query.npcPosition)) > query.maxTravelDistance) continue;

		vec3 ground;
		if (!FindGround(samples[i], ground)) continue;
		if (std::abs(ground.y - npcGroundY) > MaxGroundStep) continue;

		const vec3 cell = CoverMap::SnapToCell(ground);

		if (query.avoidRadius > 0.0f && glm::distance(MathHelper::XZ(cell), MathHelper::XZ(query.avoidPosition)) < query.avoidRadius) continue;

		const vec3 toThreat = MathHelper::Normalized(MathHelper::XZ(query.threatPosition - cell));

		CoverMap::Exposure exposure;
		if (!CoverMap::Query(cell, toThreat, exposure, traceBudget)) continue;
		if (!exposure.lowBlocked) continue;

		Candidate candidate;
		candidate.floor = cell;
		candidate.type = exposure.highBlocked ? CoverType::Full : CoverType::Low;

		if (sampleType[i] != CoverType::None && sampleType[i] != candidate.type) continue;

		const float moveDistance = glm::distance(MathHelper::XZ(cell), MathHelper::XZ(query.npcPosition));

		float cluster = 0.0f;
		for (const vec3& ally : allies)
		{
			const float d = glm::distance(MathHelper::XZ(cell), MathHelper::XZ(ally));
			cluster += std::exp(-(d * d) / (2.0f * ClusterSigma * ClusterSigma));
		}

		float laneLine = 0.0f;
		if (query.hasLaneEntry)
		{
			// Beyond the entry, on the ray from the threat through it.
			const vec3 ray = MathHelper::Normalized(MathHelper::XZ(query.laneEntry - query.threatPosition));
			const vec3 fromEntry = MathHelper::XZ(cell - query.laneEntry);
			const float along = glm::dot(fromEntry, ray);
			if (along > 0.0f && glm::length(fromEntry - ray * along) < CombatTuning::Get().cover.laneLineWidth + along * 0.15f)
				laneLine = CombatTuning::Get().cover.laneLinePenalty;
		}

		candidate.score = 2.0f
			- moveDistance * CombatTuning::Get().cover.movePenalty
			- std::abs(threatDistance - query.preferredThreatDistance) * CombatTuning::Get().cover.rangePenalty
			+ (candidate.type == CoverType::Low ? CombatTuning::Get().cover.lowCoverBonus : 0.0f)
			- cluster * CombatTuning::Get().cover.clusterPenalty
			- laneLine
			+ sampleBonus[i]
			+ RandomHelper::RandomFloat() * RandomScore;

		candidates.push_back(candidate);
	}

	std::sort(candidates.begin(), candidates.end(),
		[](const Candidate& a, const Candidate& b) { return a.score > b.score; });

	// ── Verify the best few with real traces and a path ──────────────────────
	int verified = 0;
	for (const Candidate& candidate : candidates)
	{
		if (verified >= VerifyCount) break;
		verified++;

		CoverSpot spot;
		spot.type = candidate.type;
		spot.floor = candidate.floor;

		if (!IsProtective(spot, query.threatEye)) continue;
		if (!PlanShot(candidate, query, spot)) continue;

		bool reached = false;
		const std::vector<vec3> path = NavigationSystem::FindSimplePath(query.npcPosition, spot.position, 0.0f, &reached, false);
		if (path.empty() || !reached) continue;

		const float straight = glm::distance(query.npcPosition, spot.position);
		if (PathLength(path) > straight * PathLengthRatio + PathLengthSlack) continue;

		if (!claim(spot.floor)) continue;

		out = spot;
		return true;
	}

	return false;
}

namespace
{
	// How much worse than its length a route is: metres in the target's sight
	// cost extra, and a route that brushes past the target is out.
	bool ScoreRoute(const std::vector<vec3>& path, const FlankQuery& query, const vec3& threatEye, float& outCost)
	{
		constexpr float SampleSpacing = 2.0f;
		constexpr int MaxSamples = 12;
		constexpr float BodyHeight = 1.2f; // above the path points (navmesh floor)

		float cost = 0.0f;
		int samples = 0;

		for (size_t i = 1; i < path.size(); ++i)
		{
			const vec3 a = path[i - 1];
			const vec3 b = path[i];
			const float length = glm::distance(a, b);
			cost += length;

			const int steps = std::max(1, static_cast<int>(length / SampleSpacing));
			for (int s = 1; s <= steps; ++s)
			{
				const vec3 point = a + (b - a) * (static_cast<float>(s) / static_cast<float>(steps));

				if (glm::distance(MathHelper::XZ(point), MathHelper::XZ(query.estimatePosition)) < query.routeClearance)
					return false;

				if (samples >= MaxSamples) continue;
				samples++;

				if (!Physics::LineTrace(threatEye, point + vec3(0, BodyHeight, 0), CoverMap::BlockerMask()).hasHit)
					cost += query.exposedRouteCost * std::min(length / static_cast<float>(steps), SampleSpacing);
			}
		}

		outCost = cost;
		return true;
	}
}

bool CoverFinder::RouteCost(const std::vector<vec3>& path, const vec3& targetPosition, const vec3& targetEye,
	float clearance, float exposedRouteCost, float& outCost)
{
	FlankQuery query;
	query.estimatePosition = targetPosition;
	query.routeClearance = clearance;
	query.exposedRouteCost = exposedRouteCost;

	return ScoreRoute(path, query, targetEye, outCost);
}

bool CoverFinder::FindFlankPosition(const FlankQuery& query, vec3& outPosition)
{
	constexpr int Attempts = 20;
	constexpr int PathChecks = 6;
	constexpr float EyeHeight = 1.5f;

	const vec3 center = vec3(0, query.npcCenterHeight, 0);
	const float npcGroundY = query.npcPosition.y - query.npcCenterHeight;
	const vec3 threatEye = glm::length(query.threatEye) > 0.001f ? query.threatEye : query.estimatePosition + vec3(0, 0.6f, 0);

	// Direction the NPC currently attacks from (or the squad's axis); flank positions swing away from it.
	vec3 baseDirection = MathHelper::Normalized(MathHelper::XZ(query.baseDirection));
	if (glm::length(baseDirection) < 0.001f)
		baseDirection = MathHelper::Normalized(MathHelper::XZ(query.npcPosition - query.estimatePosition));
	if (glm::length(baseDirection) < 0.001f) baseDirection = vec3(1, 0, 0);

	struct Spot
	{
		vec3 floor;
		float walkDistance;
	};
	std::vector<Spot> spots;

	for (int i = 0; i < Attempts; ++i)
	{
		const float sign = query.side != 0.0f ? (query.side < 0.0f ? -1.0f : 1.0f)
			: (RandomHelper::RandomFloat() < 0.5f ? -1.0f : 1.0f);
		const float degrees = query.minAngleDegrees + RandomHelper::RandomFloat() * (query.maxAngleDegrees - query.minAngleDegrees);
		const float angle = glm::radians(degrees) * sign;
		const float distance = query.minDistance + RandomHelper::RandomFloat() * (query.maxDistance - query.minDistance);

		const vec3 direction(
			baseDirection.x * std::cos(angle) - baseDirection.z * std::sin(angle),
			0.0f,
			baseDirection.x * std::sin(angle) + baseDirection.z * std::cos(angle));

		vec3 floor;
		if (!FindGround(query.estimatePosition + direction * distance, floor)) continue;
		if (std::abs(floor.y - npcGroundY) > MaxGroundStep + 1.0f) continue;

		// It has to actually see the estimate from there.
		if (Physics::LineTrace(floor + vec3(0, EyeHeight, 0), query.estimatePosition, CoverMap::BlockerMask()).hasHit) continue;

		spots.push_back({ floor, glm::distance(MathHelper::XZ(floor), MathHelper::XZ(query.npcPosition)) });
	}

	std::sort(spots.begin(), spots.end(), [](const Spot& a, const Spot& b) { return a.walkDistance < b.walkDistance; });

	bool found = false;
	float bestCost = std::numeric_limits<float>::max();

	int checked = 0;
	for (const Spot& spot : spots)
	{
		if (checked++ >= PathChecks) break;

		bool reached = false;
		const std::vector<vec3> path = NavigationSystem::FindSimplePath(query.npcPosition, spot.floor + center, 0.0f, &reached, false);

		if (path.empty() || !reached || PathLength(path) > query.maxPathLength) continue;

		float cost = 0.0f;
		if (!ScoreRoute(path, query, threatEye, cost)) continue;

		if (cost < bestCost)
		{
			bestCost = cost;
			outPosition = spot.floor + center;
			found = true;
		}
	}

	return found;
}

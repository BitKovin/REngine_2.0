#include "ApproachMap.h"

#include "CoverMap.h"
#include "CombatTuning.h"

#include <DebugDraw.hpp>
#include <MathHelper.hpp>
#include <Physics.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
	vec3 FlatDirection(const vec3& from, const vec3& to)
	{
		const vec3 d = MathHelper::XZ(to - from);
		const float length = glm::length(d);
		return length > 0.001f ? d / length : vec3(0.0f);
	}

	float AngleDegrees(const vec3& a, const vec3& b)
	{
		return glm::degrees(std::acos(glm::clamp(glm::dot(a, b), -1.0f, 1.0f)));
	}
}

bool ApproachMap::NeedsRebuild(const vec3& target, double now) const
{
	return !valid
		|| now - builtAt > CombatTuning::Get().approach.rebuildInterval
		|| glm::distance(MathHelper::XZ(target), MathHelper::XZ(builtFor)) > CombatTuning::Get().approach.rebuildMoveDistance;
}

bool ApproachMap::Build(const vec3& target, const vec3& eye, double now)
{
	builtFor = target;
	builtAt = now;
	valid = false;
	lanes.clear();

	if (!NavigationSystem::BuildReachTree(target, CombatTuning::Get().approach.radius, MaxNodes, nodes) || nodes.empty())
		return false;

	const int count = static_cast<int>(nodes.size());

	// What the target can see.
	visible.assign(count, 0);
	for (int i = 0; i < count; ++i)
	{
		if (nodes[i].parent < 0) { visible[i] = 1; continue; }
		if (nodes[i].cost > CombatTuning::Get().approach.maxVisibleCost) continue;

		visible[i] = !Physics::LineTrace(eye, nodes[i].center + vec3(0, CombatTuning::Get().approach.bodyHeight, 0), CoverMap::BlockerMask()).hasHit;
	}

	// Boundary crossings: a hidden node whose next step toward the target is in
	// sight. Its portal is the doorway / corner the route comes into view at.
	struct Crossing
	{
		int node;
		vec3 point;
		vec3 direction;
		float cost;
	};
	std::vector<Crossing> crossings;

	for (int i = 0; i < count; ++i)
	{
		const int parent = nodes[i].parent;
		if (visible[i] || parent < 0 || !visible[parent]) continue;

		const vec3 direction = FlatDirection(target, nodes[i].portal);
		if (glm::length(direction) < 0.5f) continue;

		crossings.push_back({ i, nodes[i].portal, direction, nodes[parent].cost });
	}

	// Nearest crossings first, so a lane's entry is the closest way in.
	std::sort(crossings.begin(), crossings.end(), [](const Crossing& a, const Crossing& b) { return a.cost < b.cost; });

	std::vector<int> laneOfCrossingNode(count, -1);

	for (size_t c = 0; c < crossings.size(); ++c)
	{
		const Crossing& crossing = crossings[c];

		int lane = -1;
		for (int l = 0; l < static_cast<int>(lanes.size()); ++l)
		{
			if (AngleDegrees(lanes[l].direction, crossing.direction) <= CombatTuning::Get().approach.laneAngleDegrees
				|| glm::distance(lanes[l].entry, crossing.point) <= CombatTuning::Get().approach.laneMergeDistance)
			{
				lane = l;
				break;
			}
		}

		if (lane < 0)
		{
			ApproachLane created;
			created.entry = crossing.point;
			created.direction = crossing.direction;
			created.entryCost = crossing.cost;
			created.stackCost = std::numeric_limits<float>::max();
			lanes.push_back(created);
			lane = static_cast<int>(lanes.size()) - 1;
		}

		laneOfCrossingNode[crossing.node] = lane;
	}

	// Every hidden node belongs to the lane of the crossing its route meets.
	laneOfNode.assign(count, -2); // -2 = not resolved yet
	for (int i = 0; i < count; ++i)
	{
		std::vector<int> chain;
		int j = i;
		int lane = -1;

		while (j >= 0)
		{
			if (laneOfNode[j] != -2) { lane = laneOfNode[j]; break; }
			if (visible[j]) { lane = -1; break; }
			if (laneOfCrossingNode[j] >= 0) { lane = laneOfCrossingNode[j]; chain.push_back(j); break; }

			chain.push_back(j);
			j = nodes[j].parent;
		}

		for (int k : chain)
			laneOfNode[k] = lane;
	}

	for (int i = 0; i < count; ++i)
	{
		if (laneOfNode[i] == -2) laneOfNode[i] = -1;
		if (laneOfNode[i] >= 0) lanes[laneOfNode[i]].hiddenNodes++;
	}

	// The stack spot: hidden, on the lane's route, as close to the entry as it
	// gets while still out of sight: a little way from the crossing edge into
	// the hidden polygon (the segment edge midpoint -> centre stays inside it).
	for (const Crossing& crossing : crossings)
	{
		const int lane = laneOfNode[crossing.node];
		if (lane < 0 || crossing.cost >= lanes[lane].stackCost) continue;

		const vec3 inward = nodes[crossing.node].center - crossing.point;
		const float length = glm::length(MathHelper::XZ(inward));
		const vec3 spot = length > 0.001f ? crossing.point + inward * (std::min(CombatTuning::Get().approach.stackBehind, length) / length) : crossing.point;

		lanes[lane].stack = spot;
		lanes[lane].stackCost = crossing.cost + std::min(CombatTuning::Get().approach.stackBehind, length);
	}

	// A sliver of shadow behind a crate is not a way in. Nodes behind a dropped
	// lane fall back to the lane facing them (LaneOf).
	std::vector<int> remap(lanes.size(), -1);
	std::vector<ApproachLane> kept;
	for (size_t l = 0; l < lanes.size(); ++l)
	{
		if (lanes[l].hiddenNodes < CombatTuning::Get().approach.minHiddenNodes) continue;

		remap[l] = static_cast<int>(kept.size());
		kept.push_back(lanes[l]);
	}
	lanes = std::move(kept);

	for (int i = 0; i < count; ++i)
	{
		if (laneOfNode[i] >= 0)
			laneOfNode[i] = remap[laneOfNode[i]];
	}

	valid = !lanes.empty();
	return valid;
}

int ApproachMap::LaneFacing(const vec3& position) const
{
	const vec3 direction = FlatDirection(builtFor, position);

	int best = -1;
	float bestAngle = std::numeric_limits<float>::max();

	for (int l = 0; l < static_cast<int>(lanes.size()); ++l)
	{
		const float angle = AngleDegrees(lanes[l].direction, direction);
		if (angle < bestAngle) { bestAngle = angle; best = l; }
	}

	return best;
}

int ApproachMap::LaneOf(const vec3& position) const
{
	if (!valid) return -1;

	constexpr float MaxNodeDistance = 3.0f;

	int nearest = -1;
	float nearestDistance = MaxNodeDistance;

	for (int i = 0; i < static_cast<int>(nodes.size()); ++i)
	{
		const vec3 d = nodes[i].center - position;
		if (std::abs(d.y) > 2.5f) continue;

		const float distance = glm::length(MathHelper::XZ(d));
		if (distance < nearestDistance)
		{
			nearestDistance = distance;
			nearest = i;
		}
	}

	if (nearest >= 0 && laneOfNode[nearest] >= 0)
		return laneOfNode[nearest];

	return LaneFacing(position);
}

float ApproachMap::AngleBetween(int laneA, int laneB) const
{
	if (laneA < 0 || laneB < 0 || laneA >= static_cast<int>(lanes.size()) || laneB >= static_cast<int>(lanes.size()))
		return 0.0f;

	return AngleDegrees(lanes[laneA].direction, lanes[laneB].direction);
}

void ApproachMap::DebugDraw() const
{
	if (!valid) return;

	constexpr float Duration = 0.05f;

	for (const ApproachLane& lane : lanes)
	{
		::DebugDraw::Line(builtFor, lane.entry, Duration, 0.03f, DebugColor::Yellow);
		::DebugDraw::Point(lane.entry + vec3(0, 0.5f, 0), Duration, 0.3f, DebugColor::Yellow);
		::DebugDraw::Line(lane.entry, lane.stack, Duration, 0.03f, DebugColor::Cyan);
		::DebugDraw::Point(lane.stack + vec3(0, 0.5f, 0), Duration, 0.25f, DebugColor::Cyan);
	}
}

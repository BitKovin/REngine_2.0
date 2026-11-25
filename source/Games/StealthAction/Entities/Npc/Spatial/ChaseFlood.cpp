#include "ChaseFlood.h"

#include <MathHelper.hpp>
#include <Navigation/NavGrid.h>

#include <algorithm>

void ChaseFlood::Start(const glm::vec3& position, const glm::vec3& velocity, const NpcChaseTuning& newTuning)
{
	tuning = newTuning;
	origin = position;
	destination = position;
	steps = 0;
	done = true;

	heat.clear();
	barrier.clear();
	heatFront.clear();
	barrierFront.clear();

	const glm::vec3 flatVelocity = MathHelper::XZ(velocity);
	behind = glm::length(flatVelocity) >= tuning.minVelocity ? -glm::normalize(flatVelocity) : glm::vec3(0.0f);

	const int seed = NavGrid::FindCell(position, 3.0f);
	if (seed < 0) return;

	// A barrier on the neighbours of the first cell that lie behind the way it was going.
	const NavGrid::Cell& cell = NavGrid::GetCell(seed);
	for (int link = 0; link < NavGrid::LinkCount; ++link)
	{
		const int neighbour = cell.link[link];
		if (neighbour >= 0 && IsBehind(neighbour))
		{
			barrier.insert(neighbour);
			barrierFront.push_back(neighbour);
		}
	}

	heat[seed] = tuning.heat;
	heatFront.push_back(seed);
	done = false;
}

// Whether a cell lies in the cone behind the first one.
bool ChaseFlood::IsBehind(int cellIndex) const
{
	if (glm::length(behind) < 0.5f) return false;

	const glm::vec3 offset = MathHelper::XZ(NavGrid::GetCell(cellIndex).floor - origin);
	const float length = glm::length(offset);
	if (length < 0.01f) return false;

	// "Behind" is the opposite of the way it was going: the dot with the way it came is positive.
	return glm::dot(offset / length, behind) > -tuning.barrierDot;
}

bool ChaseFlood::Advance(int count)
{
	for (int i = 0; i < count && done == false; ++i)
	{
		++steps;

		// The barrier grows first, so the heat of this step cannot slip past it.
		std::vector<int> nextBarrier;
		for (const int index : barrierFront)
		{
			const NavGrid::Cell& cell = NavGrid::GetCell(index);
			for (int link = 0; link < NavGrid::LinkCount; ++link)
			{
				const int neighbour = cell.link[link];
				if (neighbour < 0 || barrier.count(neighbour) || heat.count(neighbour) || IsBehind(neighbour) == false) continue;

				barrier.insert(neighbour);
				nextBarrier.push_back(neighbour);
			}
		}
		barrierFront = std::move(nextBarrier);

		std::vector<int> nextHeat;
		for (const int index : heatFront)
		{
			const NavGrid::Cell& cell = NavGrid::GetCell(index);
			const float value = heat[index] * tuning.heatFalloff;

			for (int link = 0; link < NavGrid::LinkCount; ++link)
			{
				const int neighbour = cell.link[link];
				if (neighbour < 0 || barrier.count(neighbour) || heat.count(neighbour)) continue;

				heat[neighbour] = value;
				nextHeat.push_back(neighbour);
			}
		}

		if (nextHeat.empty())
		{
			Finish();
			break;
		}

		// An open room heats too many cells at once: the flood stops at its door, the front is not warm.
		if (static_cast<int>(nextHeat.size()) > tuning.maxHeatedPerStep)
		{
			for (const int index : nextHeat)
				heat.erase(index);

			Finish();
			break;
		}

		heatFront = std::move(nextHeat);

		if (steps >= tuning.maxSteps)
			Finish();
	}

	return done;
}

// The destination is the cell nearest to the heat-weighted centroid of the warm ones, so it is always a place that can
// be walked to.
void ChaseFlood::Finish()
{
	done = true;

	glm::vec3 sum(0.0f);
	float weight = 0.0f;
	for (const auto& warm : heat)
	{
		sum += NavGrid::GetCell(warm.first).floor * warm.second;
		weight += warm.second;
	}

	if (weight <= 0.0f) return;

	const glm::vec3 centroid = sum / weight;

	float best = 1.0e9f;
	for (const auto& warm : heat)
	{
		const glm::vec3& floor = NavGrid::GetCell(warm.first).floor;
		const float distance = glm::distance(floor, centroid);
		if (distance < best)
		{
			best = distance;
			destination = floor;
		}
	}
}

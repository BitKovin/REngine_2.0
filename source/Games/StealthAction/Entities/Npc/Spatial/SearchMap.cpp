#include "SearchMap.h"

#include <MathHelper.hpp>
#include <Navigation/NavGrid.h>

#include <algorithm>
#include <unordered_map>

bool SearchMap::Build(const glm::vec3& place, const NpcSearchTuning& newTuning)
{
	tuning = newTuning;
	origin = place;
	cells.clear();
	mass = 0.0f;
	startMass = 0.0f;

	const int seed = NavGrid::FindCell(place, 3.0f);
	if (seed < 0) return false;

	// Breadth first over the grid: the cells nearest the place come first, so a map cut short by maxCells keeps the middle.
	std::unordered_map<int, int> local;
	std::vector<int> order;
	order.push_back(seed);
	local[seed] = 0;

	for (size_t head = 0; head < order.size() && static_cast<int>(order.size()) < tuning.maxCells; ++head)
	{
		const NavGrid::Cell& cell = NavGrid::GetCell(order[head]);
		for (int direction = 0; direction < NavGrid::LinkCount; ++direction)
		{
			const int neighbour = cell.link[direction];
			if (neighbour < 0 || local.count(neighbour)) continue;
			if (static_cast<int>(order.size()) >= tuning.maxCells) break;

			local[neighbour] = static_cast<int>(order.size());
			order.push_back(neighbour);
		}
	}

	cells.resize(order.size());
	for (size_t i = 0; i < order.size(); ++i)
	{
		const NavGrid::Cell& source = NavGrid::GetCell(order[i]);
		Cell& cell = cells[i];
		cell.grid = order[i];
		cell.floor = source.floor;

		int open = 0;
		for (int direction = 0; direction < NavGrid::LinkCount; ++direction)
		{
			const auto found = source.link[direction] >= 0 ? local.find(source.link[direction]) : local.end();
			cell.link[direction] = found == local.end() ? -1 : found->second;
			if (source.link[direction] >= 0) ++open;
		}

		// Two links, opposite each other: a doorway or a one-cell corridor.
		cell.choke = false;
		if (open == 2)
		{
			for (int direction = 0; direction < 4; ++direction)
			{
				if (source.link[direction] >= 0 && source.link[direction + 4] >= 0)
					cell.choke = true;
			}
		}
	}

	return true;
}

bool SearchMap::Seed(const glm::vec3& place, float certainty, const NpcSearchTuning& newTuning)
{
	return SeedRadius(place, (1.0f - std::clamp(certainty, 0.0f, 1.0f)) * newTuning.uncertaintyRadius, newTuning);
}

bool SearchMap::SeedRadius(const glm::vec3& place, float radius, const NpcSearchTuning& newTuning)
{
	if (Build(place, newTuning) == false) return false;

	std::vector<int> spread;
	for (int i = 0; i < Count(); ++i)
	{
		if (glm::length(MathHelper::XZ(cells[i].floor - place)) <= std::max(radius, NavGrid::CellSize * 0.75f))
			spread.push_back(i);
	}

	if (spread.empty())
		spread.push_back(0);

	for (const int i : spread)
		cells[i].probability = 1.0f / static_cast<float>(spread.size());

	RecountMass();
	startMass = mass;
	lastSpread = 0.0;
	return true;
}

void SearchMap::RecountMass()
{
	mass = 0.0f;
	for (const Cell& cell : cells)
		mass += cell.probability;
}

bool SearchMap::IsSeen(int cell, double now) const
{
	return cells[cell].seenUntil > now;
}

void SearchMap::MarkSeen(int cell, double now)
{
	Cell& target = cells[cell];
	mass -= target.probability;
	target.probability = 0.0f;
	target.seenUntil = now + tuning.visibleHold;
}

void SearchMap::Spread(float dt, double now)
{
	lastSpread += dt;
	if (lastSpread < tuning.spreadInterval) return;

	// The share of a cell that moves on in one step: the walk speed over the cell size.
	const float share = std::min(0.5f, tuning.spreadSpeed * static_cast<float>(lastSpread) / NavGrid::CellSize);
	lastSpread = 0.0;

	std::vector<float> next(cells.size(), 0.0f);

	for (size_t i = 0; i < cells.size(); ++i)
	{
		const float here = cells[i].probability;
		if (here <= 0.0f) continue;

		int open[8];
		int count = 0;
		for (const int neighbour : cells[i].link)
		{
			if (neighbour >= 0 && IsSeen(neighbour, now) == false)
				open[count++] = neighbour;
		}

		if (count == 0)
		{
			next[i] += here;
			continue;
		}

		const float moving = here * share;
		next[i] += here - moving;
		for (int k = 0; k < count; ++k)
			next[open[k]] += moving / static_cast<float>(count);
	}

	for (size_t i = 0; i < cells.size(); ++i)
		cells[i].probability = next[i];

	RecountMass();
}

float SearchMap::Nearby(int cell) const
{
	float total = cells[cell].probability;
	for (const int neighbour : cells[cell].link)
	{
		if (neighbour >= 0)
			total += cells[neighbour].probability;
	}
	return total;
}

int SearchMap::Nearest(const glm::vec3& position) const
{
	int best = -1;
	float bestDistance = 1.0e9f;

	for (int i = 0; i < Count(); ++i)
	{
		const float distance = glm::length(MathHelper::XZ(cells[i].floor - position)) + std::abs(cells[i].floor.y - position.y);
		if (distance < bestDistance)
		{
			bestDistance = distance;
			best = i;
		}
	}
	return best;
}

bool SearchMap::FrontierGaze(const glm::vec3& from, float range, double now, glm::vec3& point) const
{
	int frontier = -1;
	float frontierScore = 0.0f;
	int interior = -1;
	float interiorScore = 0.0f;

	for (int i = 0; i < Count(); ++i)
	{
		const Cell& cell = cells[i];
		if (cell.probability <= 0.0f || IsSeen(i, now)) continue;

		const float distance = glm::length(MathHelper::XZ(cell.floor - from));
		if (distance > range) continue;

		const float score = Nearby(i) / (1.0f + distance / 10.0f);

		bool nextToSeen = false;
		for (const int neighbour : cell.link)
		{
			if (neighbour >= 0 && IsSeen(neighbour, now))
			{
				nextToSeen = true;
				break;
			}
		}

		if (nextToSeen && score > frontierScore)
		{
			frontierScore = score;
			frontier = i;
		}
		if (score > interiorScore)
		{
			interiorScore = score;
			interior = i;
		}
	}

	// Nothing seen yet touches the probability (the search has just begun): the heaviest unseen cell in reach.
	const int chosen = frontier >= 0 ? frontier : interior;
	if (chosen < 0) return false;

	point = cells[chosen].floor + glm::vec3(0.0f, tuning.clearHeight * 1.2f, 0.0f);
	return true;
}

json SearchMap::Serialize(float relativeThreshold) const
{
	json data;
	data["origin"] = origin;
	data["startMass"] = startMass;

	float peak = 0.0f;
	for (const Cell& cell : cells)
		peak = std::max(peak, cell.probability);

	json list = json::array();
	for (const Cell& cell : cells)
	{
		if (cell.probability > 0.0f && cell.probability >= peak * relativeThreshold)
			list.push_back(json::array({ cell.grid, cell.probability }));
	}
	data["cells"] = std::move(list);

	return data;
}

bool SearchMap::Deserialize(const json& data, const NpcSearchTuning& newTuning)
{
	if (data.contains("origin") == false || Build(data["origin"].get<glm::vec3>(), newTuning) == false) return false;

	std::unordered_map<int, int> byGrid;
	for (int i = 0; i < Count(); ++i)
		byGrid[cells[i].grid] = i;

	for (const json& item : data.value("cells", json::array()))
	{
		const auto found = byGrid.find(item[0].get<int>());
		if (found != byGrid.end())
			cells[found->second].probability = item[1].get<float>();
	}

	RecountMass();
	startMass = data.value("startMass", mass);
	lastSpread = 0.0;
	return true;
}

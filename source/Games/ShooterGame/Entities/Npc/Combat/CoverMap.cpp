#include "CoverMap.h"

#include <Physics.h>
#include <MathHelper.hpp>

#include <bitset>
#include <cmath>

vec3 CoverMap::SnapToCell(const vec3& floorPos)
{
	return vec3(
		(std::floor(floorPos.x / CellSize) + 0.5f) * CellSize,
		floorPos.y,
		(std::floor(floorPos.z / CellSize) + 0.5f) * CellSize);
}

int64_t CoverMap::KeyOf(const vec3& pos)
{
	const int64_t x = static_cast<int64_t>(std::floor(pos.x / CellSize));
	const int64_t y = static_cast<int64_t>(std::floor(pos.y));
	const int64_t z = static_cast<int64_t>(std::floor(pos.z / CellSize));

	// 21 bits per axis is plenty for a level.
	return ((x & 0x1FFFFF) << 42) | ((y & 0x1FFFFF) << 21) | (z & 0x1FFFFF);
}

int CoverMap::SectorOf(const vec3& direction)
{
	const float angle = std::atan2(direction.z, direction.x); // -pi..pi
	const float turns = angle / (2.0f * glm::pi<float>());   // -0.5..0.5

	int sector = static_cast<int>(std::floor((turns + 0.5f) * SectorCount + 0.5f)) % SectorCount;
	if (sector < 0) sector += SectorCount;

	return sector;
}

vec3 CoverMap::SectorDirection(int sector)
{
	const float angle = (static_cast<float>(sector) / SectorCount - 0.5f) * 2.0f * glm::pi<float>();
	return vec3(std::cos(angle), 0.0f, std::sin(angle));
}

bool CoverMap::Query(const vec3& cellCentre, const vec3& direction, Exposure& out, int& traceBudget)
{
	const int sector = SectorOf(direction);
	const uint16_t bit = static_cast<uint16_t>(1u << sector);
	const int64_t key = KeyOf(cellCentre);

	{
		std::lock_guard<std::mutex> lock(mutex);

		auto it = cells.find(key);
		if (it != cells.end() && (it->second.known & bit))
		{
			out.lowBlocked = (it->second.low & bit) != 0;
			out.highBlocked = (it->second.high & bit) != 0;
			return true;
		}
	}

	if (traceBudget < TracesPerProbe) return false;
	traceBudget -= TracesPerProbe;

	// Probe outside the lock; two threads probing the same sector agree anyway.
	const vec3 dir = SectorDirection(sector);

	const vec3 lowStart = cellCentre + vec3(0, CrouchHeadHeight, 0);
	const vec3 highStart = cellCentre + vec3(0, StandHeadHeight, 0);

	const bool lowBlocked = Physics::SphereTrace(lowStart, lowStart + dir * ProbeDistance, ProbeRadius, BlockerMask()).hasHit;
	const bool highBlocked = Physics::SphereTrace(highStart, highStart + dir * ProbeDistance, ProbeRadius, BlockerMask()).hasHit;

	{
		std::lock_guard<std::mutex> lock(mutex);

		Cell& cell = cells[key];
		cell.known |= bit;
		if (lowBlocked) cell.low |= bit;
		if (highBlocked) cell.high |= bit;
	}

	out.lowBlocked = lowBlocked;
	out.highBlocked = highBlocked;
	return true;
}

void CoverMap::Clear()
{
	std::lock_guard<std::mutex> lock(mutex);
	cells.clear();
}

size_t CoverMap::KnownSectors()
{
	std::lock_guard<std::mutex> lock(mutex);

	size_t count = 0;
	for (const auto& entry : cells)
		count += std::bitset<SectorCount>(entry.second.known).count();

	return count;
}

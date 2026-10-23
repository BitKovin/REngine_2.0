#include "SimWalk.h"

#include <glm/geometric.hpp>

#include <algorithm>

namespace sim::movement
{
	AdvanceResult Advance(const PathView& path, Progress& progress, double seconds, double speed, double portalDelay)
	{
		AdvanceResult result;
		const size_t count = path.Count();
		double budget = std::max(seconds, 0.0);
		if (speed <= 0.0)
			return result;
		const auto& corners = *path.corners;

		while (progress.leg + 1 < count)
		{
			if (progress.crossing)
			{
				if (budget < progress.crossingLeft)
				{
					progress.crossingLeft -= budget;
					result.timeUsed += budget;
					return result;
				}
				budget -= progress.crossingLeft;
				result.timeUsed += progress.crossingLeft;
				result.crossings.push_back(progress.leg);
				progress.crossing = false;
				progress.crossingLeft = 0.0;
				++progress.leg;
				progress.legDistance = 0.0;
				continue;
			}

			if (IsPortalLeg(path, progress.leg))
			{
				progress.crossing = true;
				progress.crossingLeft = portalDelay;
				progress.legDistance = 0.0;
				continue;
			}

			const double length = glm::distance(corners[progress.leg], corners[progress.leg + 1]);
			const double remaining = length - progress.legDistance;
			const double needed = remaining / speed;
			if (budget >= needed)
			{
				budget -= needed;
				result.timeUsed += needed;
				result.walked += remaining;
				++progress.leg;
				progress.legDistance = 0.0;
				continue;
			}
			const double distance = budget * speed;
			progress.legDistance += distance;
			result.walked += distance;
			result.timeUsed += budget;
			return result;
		}
		result.arrived = true;
		return result;
	}

	glm::vec3 PositionOf(const PathView& path, const Progress& progress)
	{
		const size_t count = path.Count();
		if (count == 0)
			return glm::vec3(0.0f);
		const auto& corners = *path.corners;
		if (progress.leg + 1 >= count)
			return corners.back();
		if (progress.crossing || IsPortalLeg(path, progress.leg))
			return corners[progress.leg];
		const glm::vec3& from = corners[progress.leg];
		const glm::vec3& to = corners[progress.leg + 1];
		const double length = glm::distance(from, to);
		if (length <= 0.0)
			return from;
		const float fraction = static_cast<float>(std::min(progress.legDistance / length, 1.0));
		return from + (to - from) * fraction;
	}

	int LevelOf(const PathView& path, const Progress& progress)
	{
		const size_t count = path.Count();
		if (count == 0)
			return -1;
		return (*path.levels)[std::min<size_t>(progress.leg, count - 1)];
	}

	double WalkLength(const PathView& path)
	{
		double length = 0.0;
		const size_t count = path.Count();
		for (uint32_t leg = 0; leg + 1 < count; ++leg)
			if (!IsPortalLeg(path, leg))
				length += glm::distance((*path.corners)[leg], (*path.corners)[leg + 1]);
		return length;
	}
}

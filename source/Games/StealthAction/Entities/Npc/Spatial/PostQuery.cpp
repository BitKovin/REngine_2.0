#include "PostQuery.h"

#include <Navigation/NavGrid.h>
#include <Physics.h>

#include <algorithm>

namespace PostQuery
{
	std::vector<int> CellsAround(const glm::vec3& center, float radius, float maxVertical)
	{
		std::vector<int> cells;
		if (NavGrid::IsReady())
			NavGrid::CellsInRadius(center, radius, cells, maxVertical);
		return cells;
	}

	bool Find(const std::vector<int>& cells, const PostSelector& selector, glm::vec3& post, float* score)
	{
		struct Scored
		{
			PostCandidate candidate;
			float value = 0.0f;
		};

		std::vector<Scored> scored;
		scored.reserve(cells.size());

		for (const int cell : cells)
		{
			Scored entry;
			entry.candidate.cell = cell;
			entry.candidate.floor = NavGrid::GetCell(cell).floor;
			entry.value = 1.0f;

			for (const PostCriterion& criterion : selector)
			{
				if (criterion.castsRays) continue;

				entry.value *= criterion.score(entry.candidate);
				if (entry.value <= 0.0f) break;
			}

			if (entry.value > 0.0f)
				scored.push_back(entry);
		}

		std::sort(scored.begin(), scored.end(), [](const Scored& a, const Scored& b) { return a.value > b.value; });

		float bestValue = 0.0f;
		bool found = false;

		const size_t count = std::min<size_t>(scored.size(), MaxCandidates);
		for (size_t i = 0; i < count; ++i)
		{
			Scored& entry = scored[i];

			for (const PostCriterion& criterion : selector)
			{
				if (criterion.castsRays == false) continue;

				entry.value *= criterion.score(entry.candidate);
				if (entry.value <= 0.0f) break;
			}

			if (entry.value > bestValue)
			{
				bestValue = entry.value;
				post = entry.candidate.floor;
				found = true;
			}
		}

		if (score) *score = bestValue;
		return found;
	}

	PostCriterion DistanceBand(const glm::vec3& point, float min, float max)
	{
		return { [=](const PostCandidate& candidate)
		{
			const float distance = glm::length(glm::vec2(candidate.floor.x - point.x, candidate.floor.z - point.z));
			return distance >= min && distance <= max ? 1.0f : 0.0f;
		}, false };
	}

	PostCriterion SightTo(const glm::vec3& point, float eyeHeight)
	{
		return { [=](const PostCandidate& candidate)
		{
			const glm::vec3 eye = candidate.floor + glm::vec3(0.0f, eyeHeight, 0.0f);
			return Physics::LineTrace(eye, point, BodyType::World).hasHit ? 0.0f : 1.0f;
		}, true };
	}

	PostCriterion NotClaimed(const std::vector<glm::vec3>& claimed, float spacing)
	{
		return { [claimed, spacing](const PostCandidate& candidate)
		{
			for (const glm::vec3& other : claimed)
			{
				if (glm::length(glm::vec2(candidate.floor.x - other.x, candidate.floor.z - other.z)) < spacing)
					return 0.0f;
			}
			return 1.0f;
		}, false };
	}

	PostCriterion Near(const glm::vec3& from, float scale)
	{
		return { [=](const PostCandidate& candidate)
		{
			return 1.0f / (1.0f + glm::distance(candidate.floor, from) / scale);
		}, false };
	}

	PostSelector OpenPost(const glm::vec3& point, const glm::vec3& from, float min, float max, const std::vector<glm::vec3>& claimed)
	{
		PostSelector selector;
		selector.push_back(DistanceBand(point, min, max));
		selector.push_back(NotClaimed(claimed, 1.5f));
		selector.push_back(Near(from, 10.0f));
		selector.push_back(SightTo(point));
		return selector;
	}
}

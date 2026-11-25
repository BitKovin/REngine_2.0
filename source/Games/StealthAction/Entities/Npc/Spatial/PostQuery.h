#pragma once

// Where to stand (NPC_AI_DESIGN.md §11.2): candidate cells of the engine NavGrid scored by a selector, a list of
// criteria that each give 0..1 and are multiplied. A criterion that gives 0 rules the cell out.
//
// Cheap criteria (distance, claims, ...) run on every cell first; only the best candidates (20 at most) reach the ones
// that cast rays, so a query costs at most that many rays.

#include <glm.h>

#include <functional>
#include <vector>

struct PostCandidate
{
	int cell = -1;
	glm::vec3 floor = glm::vec3(0.0f);
};

struct PostCriterion
{
	std::function<float(const PostCandidate&)> score;
	bool castsRays = false;   // evaluated only for the best candidates of the cheap ones
};

using PostSelector = std::vector<PostCriterion>;

namespace PostQuery
{
	constexpr int MaxCandidates = 20;

	// The cells within `radius` of `center` (horizontally, `maxVertical` of its height).
	std::vector<int> CellsAround(const glm::vec3& center, float radius, float maxVertical = 2.0f);

	// The best-scoring cell of `cells` (> 0), false when every cell is ruled out. `score` receives the product.
	bool Find(const std::vector<int>& cells, const PostSelector& selector, glm::vec3& post, float* score = nullptr);

	// ── Criteria ────────────────────────────────────────────────────────────────────────────────────────────────────

	// 1 when the cell is `min` to `max` metres from the point (horizontally), 0 otherwise.
	PostCriterion DistanceBand(const glm::vec3& point, float min, float max);

	// 1 when a ray from the cell (at standing eye height) reaches the point, 0 when the world is in the way.
	PostCriterion SightTo(const glm::vec3& point, float eyeHeight = 1.5f);

	// 0 for a cell within `spacing` of a post somebody else holds.
	PostCriterion NotClaimed(const std::vector<glm::vec3>& claimed, float spacing);

	// Shorter walks from `from` score higher: 1 / (1 + distance / scale).
	PostCriterion Near(const glm::vec3& from, float scale);

	// ── Selectors ───────────────────────────────────────────────────────────────────────────────────────────────────

	// A spot `min` to `max` metres from a point with a view of it, near `from` (the investigator looks from there).
	PostSelector OpenPost(const glm::vec3& point, const glm::vec3& from, float min, float max, const std::vector<glm::vec3>& claimed);
}

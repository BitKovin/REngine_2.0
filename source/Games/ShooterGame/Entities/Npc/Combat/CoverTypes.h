#pragma once

#include <glm.h>

#include <string>

// How a cover spot protects (COMBAT_AI_DESIGN.md §10).
enum class CoverType
{
	None,
	Low,  // blocks at crouch height, open at standing height: crouch behind it, stand to shoot
	Full  // blocks at both heights: step out sideways to shoot
};

struct CoverSpot
{
	CoverType type = CoverType::None;

	vec3 position{};     // where the NPC stands in cover (NPC centre height, like attack positions)
	vec3 floor{};        // the same spot on the ground
	vec3 peekPosition{}; // Full cover: where the NPC steps to shoot; equals position for Low cover
	float score = 0.0f;

	bool Valid() const { return type != CoverType::None; }
};

// A designer-placed hint (cover_point entity). Verified against the real
// threat like any other candidate, so a bad hint can't produce bad cover.
struct CoverHint
{
	std::string entityId;
	vec3 floor{};
	float priority = 0.0f; // 0..1 score bonus
	CoverType type = CoverType::None; // None = classify automatically
};

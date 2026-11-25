#pragma once

// Behaviour (NPC_AI_DESIGN.md §8). A skill says how much it wants to run (`Want`), and the one that wants most runs and
// writes the NPC's BodyIntent for the frame. A running skill keeps a small bonus inside its MinCommit, so near-equal
// skills do not flicker; bands (10 routine ... 90 survival) keep the order obvious.

#include <Helpers/JsonHelper.hpp>
#include <Helpers/StringHelper.h> // the global `json` alias

#include "NpcBody.h"
#include "NpcData.h"
#include "NpcMind.h"

#include <memory>
#include <string>

class NpcBase;

struct NpcContext
{
	NpcBase& npc;
	NpcMind& mind;
	NpcBody& body;
	const NpcProfile& profile;

	double now = 0.0;
	float dt = 0.0f;

	// The only thing a skill writes (§4.1 rule 5). Cleared before every update of the running skill.
	BodyIntent intent;
};

class NpcSkill
{
public:

	virtual ~NpcSkill() = default;

	virtual const char* Id() const = 0;

	// 0 = not now; otherwise the priority.
	virtual float Want(const NpcContext& context) const = 0;

	virtual void Start(NpcContext& context) {}
	virtual void Update(NpcContext& context) = 0;
	virtual void Stop(NpcContext& context) {}

	// Seconds the skill keeps its place against skills that want about as much.
	virtual float MinCommit() const { return 0.0f; }

	virtual json Serialize(double now) const { return json::object(); }
	virtual void Deserialize(const json& data, double now) {}

	// One line for the debug panel.
	virtual std::string Describe(double now) const { return ""; }
};

// A point to look at in a sweep of the head: `yawDegrees` to the side of a flat direction, from the eye.
inline vec3 SweepGazePoint(const NpcBody& body, const vec3& flatDirection, float yawDegrees)
{
	return body.EyePosition() + MathHelper::RotateVector(flatDirection, vec3(0.0f, yawDegrees, 0.0f)) * 6.0f;
}

// The skill with this id (profile `skills`), or null for an unknown id.
std::unique_ptr<NpcSkill> CreateNpcSkill(const std::string& id);

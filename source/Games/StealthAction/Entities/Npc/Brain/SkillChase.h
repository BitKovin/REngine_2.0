#pragma once

#include "NpcSkill.h"

// Runs to where the chase flood points (NPC_AI_DESIGN.md §11.3). Works for the role Chaser of a search situation: the
// order says where, the NPC runs there with the weapon ready, and on arrival without contact it looks round for a moment
// and reports to the situation, which then turns the chase into a search. An NPC that sees the hostile again is not
// chasing any more: the fight takes over.
class ChaseSkill : public NpcSkill
{
public:

	const char* Id() const override { return "chase"; }

	float Want(const NpcContext& context) const override;
	void Start(NpcContext& context) override { context.mind.Bark("target_lost"); }
	void Update(NpcContext& context) override;
	void Stop(NpcContext& context) override { Reset(); }
	float MinCommit() const override { return 1.0f; }

	json Serialize(double now) const override;
	void Deserialize(const json& data, double now) override;
	std::string Describe(double now) const override;

private:

	enum class Phase : int
	{
		Run,
		Look   // arrived: the head sweeps
	};

	Phase phase = Phase::Run;
	double lookStartedAt = 0.0;
	bool reported = false;
	vec3 lookBase = vec3(0.0f, 0.0f, 1.0f);

	void Reset();
};

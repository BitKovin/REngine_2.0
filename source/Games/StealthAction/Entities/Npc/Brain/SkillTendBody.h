#pragma once

#include "NpcSkill.h"

// Looks after a body (NPC_AI_DESIGN.md §13.1): the one that checks the place of a body search walks to the body, kneels by
// it for a couple of seconds to read what happened, and then the search spreads out round it. A dead or drained body leaves
// the area on guard for good; an unconscious one is shaken awake (it stands up, alarmed, and joins the search).
class TendBodySkill : public NpcSkill
{
public:

	const char* Id() const override { return "tend_body"; }

	float Want(const NpcContext& context) const override;
	void Update(NpcContext& context) override;
	void Stop(NpcContext& context) override { phase = Phase::Walk; }
	float MinCommit() const override { return 2.0f; }

	json Serialize(double now) const override;
	void Deserialize(const json& data, double now) override;
	std::string Describe(double now) const override;

private:

	enum class Phase : int
	{
		Walk,     // to the body
		Tend,     // on one knee beside it
		Done      // has told the search; waits for it to move the checker on
	};

	Phase phase = Phase::Walk;
	double tendStartedAt = 0.0;
};

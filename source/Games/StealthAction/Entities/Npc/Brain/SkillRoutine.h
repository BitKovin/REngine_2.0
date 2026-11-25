#pragma once

#include "NpcSkill.h"

// Task points (NPC_AI_DESIGN.md §8): the NPC walks to the point its task sends it to and the task point does the rest.
// The task point and its animation are released when another skill takes over and picked up again on return (the
// existing NpcInterrupted / NpcReturned of TaskPoint, driven through NpcBase::DoingTask).
class RoutineSkill : public NpcSkill
{
public:

	const char* Id() const override { return "routine"; }

	float Want(const NpcContext& context) const override { return 10.0f; }

	void Start(NpcContext& context) override;
	void Update(NpcContext& context) override;
	void Stop(NpcContext& context) override;

	std::string Describe(double now) const override { return "task points"; }
};

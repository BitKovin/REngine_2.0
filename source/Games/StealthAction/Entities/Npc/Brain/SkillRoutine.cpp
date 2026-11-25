#include "SkillRoutine.h"

#include "../NpcBase.h"

void RoutineSkill::Start(NpcContext& context)
{
	context.npc.DoingTask = true;
}

void RoutineSkill::Stop(NpcContext& context)
{
	context.npc.DoingTask = false;
}

void RoutineSkill::Update(NpcContext& context)
{
	NpcBase& npc = context.npc;
	BodyIntent& intent = context.intent;

	npc.DoingTask = true;

	if (context.mind.alert >= NpcAlert::Wary)
		intent.weapon = NpcWeaponState::Held;

	const TaskState& task = npc.taskState;

	if (task.HasToMoveToTarget)
	{
		intent.move = true;
		intent.destination = task.TargetLocation;
		intent.acceptRadius = task.AcceptanceRadius;
		intent.gait = NpcGait::Walk;

		// The path follower must have been given this very target (a stale arrival of the last one does not count).
		if (context.body.Arrived() && glm::distance(npc.pathFollow.desiredTarget, task.TargetLocation) < 0.05f)
			npc.Task_TargetReached();

		return;
	}

	if (task.HasToLookAtTarget)
	{
		intent.facing = BodyFacing::Point;
		intent.facingPoint = npc.Position + MathHelper::GetForwardVector(task.TargetOrientation) * 3.0f;
	}
}

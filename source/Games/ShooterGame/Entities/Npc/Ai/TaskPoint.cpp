#include "TaskPoint.h"

#include "../NpcBase.h"

void TaskPoint::FromData(EntityData data)
{

	Entity::FromData(data);

	Rotation.y = data.GetPropertyFloat("angle") + 90;

}

TaskState TaskPoint::GetDefaultTaskState()
{

	TaskState state;

	state.TaskName = Name;

	return state;
}

void TaskPoint::NpcEntered(NpcBase* npc)
{

	npc->taskState = GetDefaultTaskState();

}

void TaskPoint::NpcExited(NpcBase* npc)
{
}

void TaskPoint::NpcUpdate(NpcBase* npc)
{
}

void TaskPoint::NpcInterrupred(NpcBase* npc)
{


	Logger::Log("npc interrupted task");

	npc->animator.StopTaskAnimation();

}

void TaskPoint::NpcReturned(NpcBase* npc)
{

	Logger::Log("npc returned to task");

}

void TaskPoint::OnNpcTargetReached(NpcBase* npc)
{

	auto taskState = npc->GetTaskStateRef();
	
	taskState.HasToMoveToTarget = false;

}

void TaskPoint::MoveNpcTo(NpcBase* npc, vec3 target, float acceptanceRadius)
{
	TaskState& taskState = npc->GetTaskStateRef();

	taskState.TargetLocation = target;
	taskState.HasToMoveToTarget = true;
	taskState.AcceptanceRadius = acceptanceRadius;

}

void TaskPoint::PlayTaskAnimation(NpcBase* npc, std::string animationName, bool loop)
{

	npc->animator.PlayTaskAnimation(animationName, loop);

}

void TaskPoint::StopTaskAnimation(NpcBase* npc)
{

	npc->animator.StopTaskAnimation();

}

// TaskSimpleAction.cpp
#include "TaskSimpleAction.h"
#include "../NpcBase.h"

void TaskSimpleAction::FromData(EntityData data)
{
    TaskPoint::FromData(data); // Loads "target" as nextTask
    acceptanceRadius = data.GetPropertyFloat("acceptanceRadius", acceptanceRadius);
    duration = data.GetPropertyFloat("duration", duration);
    if (duration < 0) duration = 0;
}

void TaskSimpleAction::OnNpcTargetReached(NpcBase* npc)
{
    TaskPoint::OnNpcTargetReached(npc);
    auto& s = npc->GetTaskStateRef();
    s.AllowWeapon = false;
    s.CanBeCanceled = true;
    s.DoingJob = true;
    s.TaskStage = "acting";
    s.Timer1 = 0.0f;

    if (!animName.empty())
        PlayTaskAnimation(npc, animName, animLoop);
}

void TaskSimpleAction::NpcEntered(NpcBase* npc)
{
    TaskPoint::NpcEntered(npc);
    if (currentNpc != npc) return;
    MoveNpcTo(npc, Position, acceptanceRadius);
    auto& s = npc->GetTaskStateRef();
    s.TaskStage = "moving";
    s.AllowWeapon = true;
    s.TargetOrientation = Rotation;
    s.HasToLookAtTarget = true;
}

void TaskSimpleAction::NpcUpdate(NpcBase* npc)
{
    TaskPoint::NpcUpdate(npc);
    auto& s = npc->GetTaskStateRef();
    if (s.TaskStage == "acting")
    {
        if (duration == 0.0f) return;
        s.Timer1 += Time::DeltaTimeF;
        if (s.Timer1 >= duration)
        {
            StopTaskAnimation(npc);
            FinishTask(npc);
        }
    }
}

void TaskSimpleAction::NpcTryInterrupt(NpcBase* npc) {}
void TaskSimpleAction::NpcInterrupted(NpcBase* npc)
{
    TaskPoint::NpcInterrupted(npc);
    npc->GetTaskStateRef().AllowWeapon = true;
}

void TaskSimpleAction::NpcReturned(NpcBase* npc)
{
    TaskPoint::NpcReturned(npc);
    MoveNpcTo(npc, Position, acceptanceRadius);
    auto& s = npc->GetTaskStateRef();
    s.TaskStage = "moving";
    s.AllowWeapon = true;
    s.TargetOrientation = Rotation;
    s.HasToLookAtTarget = true;
}
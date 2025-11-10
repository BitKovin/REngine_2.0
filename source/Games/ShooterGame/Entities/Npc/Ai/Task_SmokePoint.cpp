#include "TaskPoint.h"
#include "../NpcBase.h"

class Task_SmokePoint : public TaskPoint
{
public:

	float WaitTimeAfterReach = 25;
	std::string NextPoint;

	float acceptanceRadius = 0.3f;

	void FromData(EntityData data) override
	{

		TaskPoint::FromData(data);

		NextPoint = data.GetPropertyString("target");
		acceptanceRadius = data.GetPropertyFloat("acceptanceRadius", acceptanceRadius);
	}

	void NpcGoToNextTarget(NpcBase* npc)
	{
		npc->StartTask(NextPoint);
	}

	void NpcInterrupred(NpcBase* npc)
	{
		npc->GetTaskStateRef().TaskStage = "moving";
		npc->GetTaskStateRef().Timer1 = 0;
		StopTaskAnimation(npc);
	}

	void NpcReturned(NpcBase* npc)
	{
		MoveNpcTo(npc, Position, acceptanceRadius);
		npc->GetTaskStateRef().TaskStage = "moving";
		npc->GetTaskStateRef().Timer1 = 0;

		npc->GetTaskStateRef().HasToLookAtTarget = true;
		npc->GetTaskStateRef().TargetOrientation = Rotation;
	}

	void OnNpcTargetReached(NpcBase* npc) override
	{

		TaskPoint::OnNpcTargetReached(npc);

		if (WaitTimeAfterReach < 0.01f)
		{
			NpcGoToNextTarget(npc);
		}
		else
		{
			npc->GetTaskStateRef().TaskStage = "reached";

			PlayTaskAnimation(npc, "smoking", false);

		}
	}

	void NpcEntered(NpcBase* npc) override
	{

		TaskPoint::NpcEntered(npc);

		MoveNpcTo(npc, Position, acceptanceRadius);
		npc->GetTaskStateRef().TaskStage = "moving";
		npc->GetTaskStateRef().Timer1 = 0;

		npc->GetTaskStateRef().HasToLookAtTarget = true;
		npc->GetTaskStateRef().TargetOrientation = Rotation;
	}

	void NpcUpdate(NpcBase* npc) override
	{
		TaskPoint::NpcUpdate(npc);

		if (npc->GetTaskStateRef().TaskStage == "reached")
		{

			if (npc->GetTaskStateRef().Timer1 >= WaitTimeAfterReach)
			{
				StopTaskAnimation(npc);
				NpcGoToNextTarget(npc);
				return;
			}

			npc->GetTaskStateRef().Timer1 += Time::DeltaTimeF;
		}


	}

private:

};


REGISTER_ENTITY(Task_SmokePoint, "task_smoke")
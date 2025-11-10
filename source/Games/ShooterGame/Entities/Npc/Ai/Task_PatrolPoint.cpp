#include "TaskPoint.h"
#include "../NpcBase.h"

class Task_PatrolPoint : public TaskPoint
{
public:
	
	float WaitTimeAfterReach = 0;
	std::string NextPoint;

	float acceptanceRadius = 0.3f;

	void FromData(EntityData data) override
	{

		TaskPoint::FromData(data);

		NextPoint = data.GetPropertyString("target");
		WaitTimeAfterReach = data.GetPropertyFloat("waitTime");
		acceptanceRadius = data.GetPropertyFloat("acceptanceRadius", acceptanceRadius);
	}

	void NpcInterrupred(NpcBase* npc)
	{
		npc->GetTaskStateRef().TaskStage = "moving";
		npc->GetTaskStateRef().Timer1 = 0;
	}

	void NpcGoToNextTarget(NpcBase* npc)
	{
		npc->StartTask(NextPoint);
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
		}
	}

	void NpcEntered(NpcBase* npc) override
	{

		TaskPoint::NpcEntered(npc);

		MoveNpcTo(npc, Position, acceptanceRadius);
		npc->GetTaskStateRef().TaskStage = "moving";
	}

	void NpcUpdate(NpcBase* npc) override
	{
		TaskPoint::NpcUpdate(npc);

		if (npc->GetTaskStateRef().TaskStage == "reached")
		{
			npc->GetTaskStateRef().HasToLookAtTarget = true;
			npc->GetTaskStateRef().TargetOrientation = Rotation;

			if (npc->GetTaskStateRef().Timer1 >= WaitTimeAfterReach)
			{
				NpcGoToNextTarget(npc);
				return;
			}

			npc->GetTaskStateRef().Timer1 += Time::DeltaTimeF;
		}


	}

private:

};


REGISTER_ENTITY(Task_PatrolPoint, "nav_point")
#include <BehaviourTree/CustomTask.h>
#include <BehaviourTree/NodeFactory.h>

#include "../../Entities/Npc/NpcBase.h"

class MoveToPosition : public CustomTask
{

private:

	NpcBase* npcRef = nullptr;

	bool firstTick = true;

	void CheckNpcRef(BehaviorTreeContext& context)
	{
		if (npcRef == nullptr)
		{
			npcRef = dynamic_cast<NpcBase*>((Entity*)(context.owner));

			if (npcRef == nullptr)
			{
				Logger::Log("task node used with wrong Entity type");
			}

		}
	}

public:
	MoveToPosition() : CustomTask("Move To Position", "MoveToPosition")
	{
		Variables["target"] = vec3(0);
		Variables["acceptance radius"] = 1.0f;
		Variables["pretty path"] = false;
	}

	void OnStop(BehaviorTreeContext& context) override
	{

		CustomTask::OnStop(context);
		return;
		CheckNpcRef(context);

		npcRef->pathFollow.reachedTarget = true;
		npcRef->StopMovement();

	}

	void OnStart(BehaviorTreeContext& context) override
	{

		CustomTask::OnStart(context);

		CheckNpcRef(context);

		if (npcRef)
		{
			vec3 target = GetVariable<vec3>("target");
			npcRef->MoveTo(target, GetVariable<float>("acceptance radius"));
			npcRef->PrepareToStartMovement();
			npcRef->pathFollow.reachedTarget = false;

			firstTick = true;

		}

	}

	void Tick(BehaviorTreeContext& context)
	{

		CheckNpcRef(context);

		vec3 target = GetVariable<vec3>("target");

		if (firstTick)
		{
			npcRef->MoveTo(target, GetVariable<float>("acceptance radius"));

			npcRef->pathFollow.reachedTarget = false;
			npcRef->PrepareToStartMovement();

			firstTick = false;
			return;
		}

		if (npcRef->pathFollow.reachedTarget && npcRef->pathFollow.CalculatedPath)
		{
			FinishExecution(true);
			return;
		}


		npcRef->MoveTo(target, GetVariable<float>("acceptance radius"));

	}

};

REGISTER_BT_NODE(MoveToPosition);
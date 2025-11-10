#include <BehaviourTree/CustomTask.h>
#include <BehaviourTree/NodeFactory.h>

#include "../../Entities/Npc/NpcBase.h"

class LockTask : public CustomTask
{

private:


public:
	LockTask() : CustomTask("Lock", "LockTask")
	{
		Variables["lock"] = false;
		Variables["inverse"] = false;
	}


	void OnStart(BehaviorTreeContext& context) override
	{

		bool condition = GetVariable<bool>("lock", false);
		if (GetVariable<bool>("inverse", false))
			condition = !condition;

		if (condition == false)
			FinishExecution(true);

	}

	void Tick(BehaviorTreeContext& context)
	{

		bool condition = GetVariable<bool>("lock", false);
		if (GetVariable<bool>("inverse", false))
			condition = !condition;

		if (condition == false)
			FinishExecution(true);

	}

};

REGISTER_BT_NODE(LockTask);
#include "HoldTask.h"
#include "NodeFactory.h"

HoldTask::HoldTask() : CustomTask("Hold", "HoldTask")
{

	Variables["hold"] = false;

}

HoldTask::~HoldTask()
{
}

void HoldTask::Start(BehaviorTreeContext& context)
{

}

void HoldTask::Tick(BehaviorTreeContext& context)
{
	if (GetVariable<bool>("hold") == false)
	{
		FinishExecution();
	}
}

//REGISTER_BT_NODE(HoldTask)
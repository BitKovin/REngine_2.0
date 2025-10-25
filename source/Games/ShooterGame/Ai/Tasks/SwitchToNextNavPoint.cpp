#include <BehaviourTree/CustomTask.h>
#include <BehaviourTree/NodeFactory.h>

#include "../../Entities/Npc/NpcBase.h"
#include "../../Entities/Npc/Ai/nav_point.h"

class SwitchToNextNavPoint : public CustomTask
{

private:

	NpcBase* npcRef = nullptr;

	void CheckNpcRef(BehaviorTreeContext& context)
	{
		if (npcRef == nullptr)
		{
			npcRef = dynamic_cast<NpcBase*>((Entity*)(context.owner));

			if (npcRef == nullptr)
			{
				throw exception("task node used with wrong Entity type");
			}

		}
	}

public:
	SwitchToNextNavPoint() : CustomTask("Switch To Next Nav Point", "SwitchToNextNavPoint")
	{

	}

	void OnStart(BehaviorTreeContext& context) override
	{

		CustomTask::OnStart(context);

		CheckNpcRef(context);

		if (npcRef)
		{

			auto navPointRef = dynamic_cast<nav_point*>(Level::Current->FindEntityWithName(npcRef->CurrentTargetNavPoint));

			npcRef->CurrentTargetNavPoint = navPointRef->NextPoint;

		}

		FinishExecution(true);

	}

	

};

REGISTER_BT_NODE(SwitchToNextNavPoint);
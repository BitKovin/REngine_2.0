#include <BehaviourTree/CustomTask.h>
#include <BehaviourTree/NodeFactory.h>

#include "../../Entities/Npc/NpcBase.h"

class MoveToEntity : public CustomTask
{
public:
	MoveToEntity() : CustomTask ("Move To Entity", "MoveToEntity")
	{
		Variables["target"] = "";
		Variables["acceptance radius"] = 1.0f;
		Variables["pretty path"] = false;
	}

	void Tick(BehaviorTreeContext& context)
	{

		if (npcRef == nullptr)
		{
			npcRef = dynamic_cast<NpcBase*>((Entity*)(context.owner));

			if (npcRef == nullptr)
			{
				throw exception("task node used with wrong Entity type");
			}

		}

		std::string target = GetVariable<std::string>("target");
		auto targetEntity = Level::Current->FindEntityWithId(target);

		if (targetEntity == nullptr)
		{
			FinishExecution(false);
			return;
		}

		if (npcRef->pathFollow.reachedTarget)
		{
			FinishExecution(true);
		}

		npcRef->MoveTo(targetEntity->Position, GetVariable<float>("acceptance radius"));

	}

private:

	NpcBase* npcRef = nullptr;

};

REGISTER_BT_NODE(MoveToEntity);
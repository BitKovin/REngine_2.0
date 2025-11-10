#pragma once

#include <Entity.h>
#include "../../../Ai/NpcTasks/TaskState.h"

class NpcBase;

class TaskPoint : public Entity
{
public:
	
	void FromData(EntityData data) override;

	virtual TaskState GetDefaultTaskState();

	virtual void NpcEntered(NpcBase* npc);
	virtual void NpcExited(NpcBase* npc);
	virtual void NpcUpdate(NpcBase* npc);

	virtual void OnNpcTargetReached(NpcBase* npc);

	void MoveNpcTo(NpcBase* npc,vec3 target, float acceptanceRadius);

private:



};


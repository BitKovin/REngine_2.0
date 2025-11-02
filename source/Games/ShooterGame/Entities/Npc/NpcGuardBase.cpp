#include "NpcBase.h"

class NpcGuardBase : public NpcBase
{
public:
	NpcGuardBase();
	~NpcGuardBase();

private:

};

NpcGuardBase::NpcGuardBase()
{

	isGuard = true;

	modelPath = "GameData/models/npc/guard.glb";

}

NpcGuardBase::~NpcGuardBase()
{
}

REGISTER_ENTITY(NpcGuardBase, "npc_guard")
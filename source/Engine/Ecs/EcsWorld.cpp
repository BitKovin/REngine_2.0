#include "EcsWorld.h"
#include <Entity.h>
void EcsWorld::Destroy(entt::entity e)
{
    if (Entity* owner = GetEntity(e))
        owner->Destroy();
    else
    {
        EcsGuard::ExclusiveGuard g("EcsWorld::Destroy(rowOnly)");
        if (Storage().valid(e))
            Storage().destroy(e);
    }
}
#pragma once

#include "GoreComponent.h"

class Entity;

namespace Gore
{
    class GoreSkeletalMesh;

    // The gore mesh of an entity (first drawable that is a GoreSkeletalMesh), nullptr if none.
    GoreSkeletalMesh* FindGoreMesh(Entity* entity);

    // Adds a hit that does not fit into the pending buffer to the last pending hit (keeps the count bounded).
    void MergeIntoLastPending(GoreComponent& gore, const Hit& hit);

    // Turns the pending hits of one body into wounds and splats (bumps gore.version).
    void ProcessPendingHits(Entity* self, GoreComponent& gore);

    // Copies the component to the mesh (SetGoreState) when gore.version changed.
    void PushRenderStateIfChanged(Entity* self, GoreComponent& gore);
}

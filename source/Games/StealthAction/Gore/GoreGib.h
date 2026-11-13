#pragma once

#include "GoreProfile.h"
#include "GoreTypes.h"

#include <Entity.h>
#include <EntityHandle.h>

namespace Gore
{
    class GoreSkeletalMesh;

    // A severed body part: its own entity with the body's mesh, a ragdoll made of the part's hitboxes, and a gore state
    // that keeps only the removed side of the cut (cap included). It can be shot (wounds) but never severed again.
    class GoreGib : public Entity
    {
    public:
        GoreGib();
        ~GoreGib() override;

        // Copies pose, hitboxes (the part's subtree only) and gore from the body. Call before adding to the level.
        bool Init(Entity* body, GoreSkeletalMesh* bodyMesh, const Profile& profile, uint8_t cutIndex, const glm::vec3& impulse);

        void Update() override;
        void Destroy() override;
        void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone = "", Entity* DamageCauser = nullptr, Entity* Weapon = nullptr) override;

        GoreSkeletalMesh* mesh = nullptr;
        double spawnTime = 0.0;
        bool wasRendered = false;
        uint32_t hitCounter = 0;
    };

    struct GibRequest
    {
        EntityHandle body;
        uint8_t cutIndex = 0;
        glm::vec3 impulse = glm::vec3(0);
    };

    namespace GibPool
    {
        void Request(const GibRequest& request);   // queued; spawned by GoreGibSpawnSystem (bounded per frame)
        void Tick();                                // spawn the queue, retire the oldest beyond the budget
    }
}

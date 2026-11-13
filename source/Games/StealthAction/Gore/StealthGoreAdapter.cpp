#include "StealthGoreAdapter.h"

#include "GoreEvents.h"
#include "GoreProfile.h"
#include "GoreSettings.h"
#include "GoreSource.h"

#include <Entities/Explosion.h>
#include <Entity.h>
#include <Ecs/Ecs.h>

#include <cstring>

namespace
{
    // Counts the hits a victim has taken (seeds and ordering stay stable for a given hit sequence).
    struct GoreHitCounter
    {
        uint32_t value = 0;
    };

    struct PendingSweep
    {
        std::string victimId;
        glm::vec3 normal = glm::vec3(0);
    };
    PendingSweep g_sweep;

    bool g_loaded = false;
}
REGISTER_COMPONENT(GoreHitCounter)

namespace StealthGore
{
    void LoadAssets()
    {
        if (g_loaded) return;
        g_loaded = true;
        Gore::GoreSettings::Load();
        Gore::LoadSources();
    }

    void OnNpcPointDamage(Entity* npc, float damage, const glm::vec3& point, const glm::vec3& direction,
                          const std::string& bone, Entity* causer, Entity* weapon, bool wasAliveBefore)
    {
        if (npc == nullptr) return;
        (void)causer;

        // Source: the projectile's own tag, else the weapon / attacker class.
        uint16_t source = 0;
        if (weapon != nullptr)
        {
            if (weapon->HasComponent<Gore::GoreSourceComponent>())
                source = weapon->GetComponent<Gore::GoreSourceComponent>()->sourceIndex;
            else
                source = Gore::SourceIndexForClass(weapon->ClassName);
        }

        Gore::Hit hit;
        hit.point = point;
        hit.direction = glm::length(direction) > 1.0e-5f ? glm::normalize(direction) : glm::vec3(0, 0, 1);
        hit.damage = damage;
        hit.sourceIndex = source;
        hit.lethal = wasAliveBefore && npc->Health <= 0.0f;

        // An explosion entity knows where it went off and how far it reaches.
        if (const Explosion* explosion = dynamic_cast<const Explosion*>(weapon))
        {
            hit.blastCenter = explosion->Position;
            hit.blastRadius = explosion->Radius;
        }

        Gore::SetHitBone(hit, bone);

        if (g_sweep.victimId == npc->Id)
        {
            hit.sweepNormal = g_sweep.normal;
            g_sweep.victimId.clear();
        }

        if (!npc->HasComponent<GoreHitCounter>())
            npc->AddComponent<GoreHitCounter>();
        hit.counter = ++npc->GetComponent<GoreHitCounter>()->value;
        hit.seed = Gore::HashSeed(npc->Id, hit.counter);

        GoreHitEvent event;
        event.victim = EntityHandle::FromEntity(npc);
        event.hit = hit;
        EcsScheduler::Emit(event);
    }

    void TagProjectile(Entity* projectile, Entity* weapon)
    {
        if (projectile == nullptr || weapon == nullptr) return;

        const uint16_t source = Gore::SourceIndexForClass(weapon->ClassName);
        if (projectile->HasComponent<Gore::GoreSourceComponent>())
            projectile->GetComponent<Gore::GoreSourceComponent>()->sourceIndex = source;
        else
            projectile->AddComponent<Gore::GoreSourceComponent>().sourceIndex = source;
    }

    void SetNextMeleeSweep(Entity* victim, const glm::vec3& sweepNormal)
    {
        if (victim == nullptr) return;
        g_sweep.victimId = victim->Id;
        g_sweep.normal = sweepNormal;
    }
}

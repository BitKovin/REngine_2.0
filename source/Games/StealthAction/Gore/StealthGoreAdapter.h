#pragma once

#include <glm.h>
#include <string>

class Entity;

// The only door between StealthAction gameplay and the gore module: gameplay calls these, nothing else.
namespace StealthGore
{
    // Loads the sources table and settings. Call from NpcBase::LoadAssets (cheap after the first call).
    void LoadAssets();

    // Call right after the damage was applied. wasAliveBefore: the victim was alive before this hit.
    void OnNpcPointDamage(Entity* npc, float damage, const glm::vec3& point, const glm::vec3& direction,
                          const std::string& bone, Entity* causer, Entity* weapon, bool wasAliveBefore);

    // A projectile fired by a weapon: it carries the weapon's gore source (the weapon is not the one hitting).
    void TagProjectile(Entity* projectile, Entity* weapon);

    // The next hit on this victim is a blade swing with the given sweep plane normal (world).
    void SetNextMeleeSweep(Entity* victim, const glm::vec3& sweepNormal);
}

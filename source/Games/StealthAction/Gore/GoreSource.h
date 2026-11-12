#pragma once

#include <Ecs/ComponentRegistry.h>

#include <cstdint>
#include <string>

namespace Gore
{
    enum class SourceType : uint8_t { Bullet, HeavyBullet, Pellet, Slash, Blunt, Blast };

    // What one kind of weapon / projectile / explosion does to a body. Loaded from GameData/gore/sources.json.
    struct Source
    {
        std::string id;
        SourceType type = SourceType::Bullet;
        float woundRadius = 0.012f;
        float refDamage = 20.0f;
        float clusterRadius = 0.07f;
        float gashLength = 0.15f;
        float severPower = 0.0f;
        bool exit = false;
        uint32_t tagBits = 0;   // bit per tag, see TagBit()
    };

    uint32_t TagBit(const std::string& tag);   // stable: index in a static table, max 32 tags, logs when full

    void LoadSources();   // GameData/gore/sources.json
    uint16_t SourceIndexForClass(const std::string& className);   // index of "default" if unknown
    const Source& GetSource(uint16_t index);

    // Attached to projectiles so a bullet carries its weapon's source.
    struct GoreSourceComponent
    {
        uint16_t sourceIndex = 0;
    };
    REGISTER_COMPONENT(GoreSourceComponent)
}

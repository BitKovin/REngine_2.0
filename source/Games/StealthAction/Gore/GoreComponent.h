#pragma once

#include "GoreTypes.h"

#include <Ecs/ComponentRegistry.h>

namespace Gore
{
    // Gore state of one body. Added on the first gore hit (undamaged enemies do not carry it).
    // Fixed-size arrays only - no heap in the hot data.
    struct GoreComponent
    {
        std::array<Wound, kMaxWounds> wounds; uint8_t woundCount = 0;
        std::array<Cut,   kMaxCuts>   cuts;   uint8_t cutCount = 0;
        std::array<Splat, kMaxSplats> splats; uint8_t splatCount = 0;

        std::array<Hit, kMaxPendingHits> pending; uint8_t pendingCount = 0;

        uint32_t version = 1;       // ++ on every change of wounds/cuts/splats
        uint32_t hitCounter = 0;    // per victim, seeds
        uint32_t orderCounter = 0;  // creation order for eviction

        // Regen clock (phase 5). clock(t) = t < pausedUntil ? frozenClock : frozenClock + (t - pausedUntil)
        double frozenClock = 0.0;
        double pausedUntil = 0.0;

        // Accumulated sever damage per severable bone, indexed like Profile::severable (max 16, checked by the loader).
        std::array<float, 16>  severDamage{};
        std::array<double, 16> severDamageTime{};
    };
    REGISTER_COMPONENT(GoreComponent)
}

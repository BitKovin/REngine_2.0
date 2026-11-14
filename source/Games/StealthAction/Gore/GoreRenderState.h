#pragma once

#include "GoreTypes.h"

#include <glm.h>
#include <vector>

namespace Gore
{
    struct Profile;

    // Snapshot of a GoreComponent for rendering. Copied into the mesh on the game thread when the version changes.
    struct RenderState
    {
        uint32_t version = 0;
        bool isGib = false;                     // gib: the single cut keeps its removed side (inverted)
        std::array<Wound, kMaxWounds> wounds; uint8_t woundCount = 0;
        std::array<Cut, kMaxCuts> cuts; uint8_t cutCount = 0;
        std::array<Splat, kMaxSplats> splats; uint8_t splatCount = 0;
        double frozenClock = 0.0, pausedUntil = 0.0;   // regen clock (phase 5)
        std::vector<glm::mat4> gibSeverPalette; // gib only: body palette at the moment of the sever (phase 3)
        const Profile* profile = nullptr;       // set by the system; profiles live until the level changes
    };

    // clock(t) = t < pausedUntil ? frozenClock : frozenClock + (t - pausedUntil)
    inline double RegenClock(double frozenClock, double pausedUntil, double t)
    {
        return t < pausedUntil ? frozenClock : frozenClock + (t - pausedUntil);
    }
}

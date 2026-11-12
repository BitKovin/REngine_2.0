#pragma once

#include "GoreTypes.h"

#include <EntityHandle.h>
#include <Ecs/EventRegistry.h>

// A hit that may leave gore. Emitted by StealthGoreAdapter; handled by GoreSystem.
struct GoreHitEvent
{
    EntityHandle victim;
    Gore::Hit hit;
};
REGISTER_EVENT(GoreHitEvent)

// A body part was lost / regained (phase 4).
struct GorePartEvent
{
    EntityHandle victim;
    uint16_t boneSkel = 0;
    bool lost = true;
};
REGISTER_EVENT(GorePartEvent)

#pragma once
#include <UI/Preview/UiPreviewRegistry.h>

namespace UiHudPreviews
{
    // Sample runtime previews for the hosted UI of phase 5: a HUD crosshair (host hud-layer) and a world-surface panel (host world-surface).
    // Both are synthetic: no player, no level and no gameplay entity is touched.
    void RegisterProviders();   // from UiSystem::Init
}

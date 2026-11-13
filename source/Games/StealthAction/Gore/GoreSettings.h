#pragma once

namespace Gore
{
    // Budgets and the player-facing gore level. Loaded from GameData/gore/settings.json.
    struct GoreSettings
    {
        enum class Level : int { Full = 0, Reduced = 1, Minimal = 2 };

        int   maxGibs = 48;
        int   maxGibSpawnsPerFrame = 6;
        int   maxParticlesPerFrame = 64;
        Level level = Level::Full;
        float freshBand = 0.05f;
        // false: bullets leave decals, big wounds come from the profile's authored slots, parts are removed at their joints.
        // true: the older free-form wounds (ellipsoids dug into the skin wherever a hit lands, merged and grown by stacking).
        bool freeformWounds = false;

        static GoreSettings& Get();
        static void Load();   // GameData/gore/settings.json (missing file = defaults)
    };
}

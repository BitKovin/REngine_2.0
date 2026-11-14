#pragma once

namespace Gore
{
    // Budgets and the player-facing gore level. Loaded from GameData/gore/settings.json.
    struct GoreSettings
    {
        enum class Level : int { Full = 0, Reduced = 1, Minimal = 2 };

        int   maxGibs = 48;
        int   maxGibSpawnsPerFrame = 6;
        int   maxChunks = 48;                // flying meat chunks alive at once
        int   maxChunkSpawnsPerFrame = 16;
        int   maxParticlesPerFrame = 64;
        Level level = Level::Full;
        float freshBand = 0.05f;
        bool  severAlive = false;   // false: limbs and heads only come off with the killing blow or on a corpse

        static GoreSettings& Get();
        static void Load();   // GameData/gore/settings.json (missing file = defaults)
    };
}

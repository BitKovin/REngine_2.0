#pragma once

namespace Gore
{
    // How one body behaves. Every enemy sets it on its GoreSkeletalMesh in its initializer (mesh->goreSettings = ...), so
    // different enemies can differ.
    struct GoreSettings
    {
        enum class Level : int { Full = 0, Reduced = 1, Minimal = 2 };

        Level level = Level::Full;   // Full: everything; Reduced: dull guts and no chunks; Minimal: no blood tint either
        bool  severAlive = false;    // false: limbs and heads only come off with the killing blow or on a corpse
        bool  holesWhileAlive = true; // false: a living enemy only gets decals (bullet hole, slash, shotgun spread, bruise) from hits
                                      // that do not kill it; the wounds, holes and meat come with the killing blow
    };

    // Shared pools and per-frame limits: they belong to the whole scene, not to a body.
    namespace GoreBudget
    {
        constexpr int kMaxGibs = 48;
        constexpr int kMaxGibSpawnsPerFrame = 6;
        constexpr int kMaxChunks = 48;               // flying meat chunks alive at once
        constexpr int kMaxChunkSpawnsPerFrame = 16;
        constexpr int kMaxParticlesPerFrame = 96;
    }
}

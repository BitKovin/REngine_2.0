#pragma once

#include <glm.h>
#include <cstdint>
#include <array>
#include <string>

// Procedural gore: shared plain data types. See GORE_IMPLEMENTATION.md.
namespace Gore
{
    constexpr int kMaxWounds = 8;
    constexpr int kMaxCuts = 4;
    constexpr int kMaxSplats = 16;
    constexpr int kMaxPendingHits = 32;
    constexpr int kMaxSkeletonBones = 512;   // models with more skeleton nodes get no gore (logged)

    // Deterministic RNG (same results on every platform/compiler - never use std:: distributions here).
    struct Rng
    {
        uint32_t state;
        explicit Rng(uint32_t seed) : state(seed ? seed : 0x9E3779B9u) {}
        uint32_t NextU32() { state ^= state << 13; state ^= state >> 17; state ^= state << 5; return state; }
        float Next01() { return (NextU32() >> 8) * (1.0f / 16777216.0f); }
        float Range(float a, float b) { return a + (b - a) * Next01(); }
    };

    uint32_t HashSeed(const std::string& victimId, uint32_t hitCounter);   // FNV-1a over id bytes, then mix counter

    enum class WoundKind : uint8_t { Entry, Exit, Crater, Gash, Crush };
    enum class CutPhase  : uint8_t { Severed, Regrowing };

    struct Wound
    {
        glm::vec3 center = glm::vec3(0);        // bind space
        glm::quat rotation = glm::quat(1, 0, 0, 0); // bind space, local z = shot direction / blade normal
        glm::vec3 radii = glm::vec3(0.02f);     // bind-space metres
        int16_t   anchorSkin = -1;              // skin index of the anchor bone (rigid frame for the interior pass)
        uint16_t  anchorSkel = 0;
        WoundKind kind = WoundKind::Entry;
        float roughness = 0.25f;                // edge noise amplitude in unit-sphere units
        float rimWidth = 0.35f;                 // blood rim width in unit-sphere units
        float seed = 0.0f;                      // 0..1, offsets the noise
        float reach = 0.3f;                     // max body thickness behind the wound (bind metres)
        double healStart = 0.0;                 // regen clock; healDuration 0 = permanent
        float  healDuration = 0.0f;
        uint32_t order = 0;                     // creation order (eviction picks the oldest)
    };

    struct Cut
    {
        glm::vec4 plane = glm::vec4(0, 1, 0, 0); // bind: removed side is dot(n,p) - d > 0
        glm::vec3 center = glm::vec3(0);        // bind: bone centre on the plane
        float influenceRadius = 0.5f;           // bind: cut does nothing farther than this from center
        float boneRadius = 0.015f;              // bind metres
        float roughness = 0.012f;               // edge noise amplitude in bind metres
        float seed = 0.0f;
        uint16_t boneSkel = 0;                  // severed bone S (skel index)
        int16_t  boneSkin = -1;
        CutPhase phase = CutPhase::Severed;
        double severedAt = 0.0;                 // Time::GameTime (FX only)
        double regrowStart = 0.0;               // regen clock (phase 5)
        float  regrowDuration = 0.0f;           // 0 = never regrows
        float  regrowMaxRadius = 0.0f;          // bind metres, covers the whole removed subtree
        bool   hitboxesBack = false;
        bool   charred = false;                 // fire: never regrows, charred cap look
        std::string gibId;                      // entity id of the severed piece (empty if none / not saved)
    };

    struct Splat
    {
        glm::vec3 center = glm::vec3(0);        // bind
        float radius = 0.05f;                   // bind metres
        float strength = 1.0f;                  // 0..1
        uint32_t order = 0;
    };

    // Per-hit data from the adapter. Plain data, ~100 bytes.
    struct Hit
    {
        glm::vec3 point = glm::vec3(0);         // world
        glm::vec3 direction = glm::vec3(0, 0, 1); // world, normalised, direction the damage travels
        glm::vec3 sweepNormal = glm::vec3(0);   // world, blade sweep plane normal (slash only), 0 = none
        float damage = 0.0f;
        uint16_t sourceIndex = 0;               // index into the sources table
        char hitBone[32] = {};                  // hitbox bone name (fixed buffer - no heap in the event)
        bool lethal = false;                    // this hit killed
        bool finisher = false;                  // apply the body rules' finisher gore
        uint32_t seed = 0;
        uint32_t counter = 0;                   // per-victim hit counter (ordering + seeds)
    };
}

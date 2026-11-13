#pragma once

#include <glm.h>
#include <VertexData.h>
#include <cstdint>
#include <array>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

// Procedural gore: shared plain data types. See GORE_IMPLEMENTATION.md.
namespace Gore
{
    constexpr int kMaxWounds = 8;
    constexpr int kMaxCuts = 4;
    constexpr int kMaxSplats = 16;
    constexpr int kMaxDecals = 32;
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

    // The inside of a wound as real skinned geometry: a bowl built against the model faces under the hole. Every vertex
    // carries bone indices/weights blended from the triangles it sits under, so the cavity bends with the skin and
    // follows the ragdoll. Positions are bind space, like the model's own vertices.
    struct CavityGeometry
    {
        std::vector<VertexData> vertices;
        std::vector<uint32_t> indices;
    };

    // A cavity being built on the worker thread. The game thread polls `state` (0 building, 1 done) once a tick.
    struct CavityTicket
    {
        std::atomic<int> state{ 0 };
        std::shared_ptr<const CavityGeometry> result;   // written before state becomes 1; null = no skin under the wound
    };

    struct Wound
    {
        std::shared_ptr<CavityTicket> cavityTicket;     // set while the cavity is being built
        bool cavityFailed = false;                      // nothing to build against: the wound is not drawn
        std::shared_ptr<const CavityGeometry> cavity;   // built by BuildCavity after the wound is created / merged
        glm::vec3 center = glm::vec3(0);        // bind space
        glm::quat rotation = glm::quat(1, 0, 0, 0); // bind space, local z = shot direction / blade normal
        glm::vec3 radii = glm::vec3(0.02f);     // bind-space metres
        int16_t   anchorSkin = -1;              // skin index of the anchor bone (rigid frame for the interior pass)
        uint16_t  anchorSkel = 0;
        int8_t    slot = -1;                    // profile wound slot this wound was opened from (-1 = free-form wound)
        WoundKind kind = WoundKind::Entry;
        bool      through = false;              // goes through the whole body: the cavity is a tunnel with an open middle
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
        float outlineRadius = 0.04f;            // bind metres: radius of the cross-section at the plane (cap layers)
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
        double fxNext = 0.0;                    // stump effects: next spawn time (FX only)
        std::string gibId;                      // entity id of the severed piece (empty if none / not saved)
    };

    struct Splat
    {
        glm::vec3 center = glm::vec3(0);        // bind
        float radius = 0.05f;                   // bind metres
        float strength = 1.0f;                  // 0..1
        uint32_t order = 0;
    };

    // A bullet hole: a small dark mark with a blood ring painted by the body shader (bind space, no geometry).
    struct Decal
    {
        glm::vec3 center = glm::vec3(0);        // bind
        float radius = 0.02f;                   // bind metres, radius of the blood ring
        uint32_t order = 0;
    };

    // Per-hit data from the adapter. Plain data, ~100 bytes.
    struct Hit
    {
        glm::vec3 point = glm::vec3(0);         // world
        glm::vec3 direction = glm::vec3(0, 0, 1); // world, normalised, direction the damage travels
        glm::vec3 sweepNormal = glm::vec3(0);   // world, blade sweep plane normal (slash only), 0 = none
        glm::vec3 blastCenter = glm::vec3(0);   // world, centre of the explosion (only when blastRadius > 0)
        float blastRadius = 0.0f;               // world, 0 = not an explosion entity (the source decides)
        float damage = 0.0f;
        uint16_t sourceIndex = 0;               // index into the sources table
        char hitBone[32] = {};                  // hitbox bone name (fixed buffer - no heap in the event)
        bool lethal = false;                    // this hit killed
        bool alive = false;                     // the victim was alive before this hit (limbs and the head stay on for now)
        bool finisher = false;                  // apply the body rules' finisher gore
        uint32_t seed = 0;
        uint32_t counter = 0;                   // per-victim hit counter (ordering + seeds)
    };

    // Copies a bone name into the fixed buffer (truncates; no strncpy - MSVC rejects it as unsafe).
    inline void SetHitBone(Hit& hit, const std::string& bone)
    {
        const size_t n = bone.size() < sizeof(hit.hitBone) - 1 ? bone.size() : sizeof(hit.hitBone) - 1;
        for (size_t i = 0; i < n; ++i) hit.hitBone[i] = bone[i];
        hit.hitBone[n] = 0;
    }
}

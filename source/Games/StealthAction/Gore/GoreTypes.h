#pragma once

#include <glm.h>
#include <cstdint>
#include <array>
#include <memory>
#include <string>
#include <vector>

// Procedural gore: shared plain data types. See GORE_IMPLEMENTATION.md.
namespace Gore
{
    constexpr int kMaxWounds = 8;
    constexpr int kMaxCuts = 4;
    constexpr int kMaxSplats = 16;
    constexpr int kMaxPendingHits = 32;
    constexpr int kMaxSkeletonBones = 512;   // models with more skeleton nodes get no gore (logged)

    // Flesh layers (GORE_V2_PLAN.md section 0.3). Layer 0 is the skin, 1..kShellLayers are shells, then the anatomy.
    constexpr int   kShellLayers = 3;
    constexpr float kShellFraction[kShellLayers] = { 0.30f, 0.60f, 0.88f };   // share of a vertex's max inset
    constexpr int   kAnatomyLayer = kShellLayers + 1;                          // = 4
    constexpr float kMinTaper = 0.10f;

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
        float   taper = 0.5f;       // how fast the hole shrinks per layer: scale(layer) = heal * max(0, 1 - layer * taper)
        int16_t limitSkel = -1;     // >= 0: the wound only touches this skel bone and its subtree (sever wounds)
    };

    struct CapGeometry;   // GoreCap.h

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
        bool clean = false;                         // a blade cut (nothing is thrown); false: torn by a bullet or a blast
        std::shared_ptr<const CapGeometry> cap;     // the face of the cut, shared by the body and the gib; null only for a cut whose plane found no skin (sever wound)
        // Bit (b - boneSkel) set: part bone b carries cloth that hangs far below the plane (the hem of a robe). The body keeps that
        // bone's range, clipped by the plane, instead of dropping it with the rest of the part. Trunk cuts only.
        uint64_t stragglers[2] = { 0, 0 };
    };

    // A blood decal on the skin. With a direction it is a picture from the decal atlas (GameData/textures/gore/blood_decals.png),
    // laid on the plane square to that direction like a projected decal: a bullet hole (bulletHole) or a blood splash; without a
    // direction it is a plain soft soak (a stump, see AddSplat).
    struct Splat
    {
        glm::vec3 center = glm::vec3(0);        // bind
        float radius = 0.05f;                   // bind metres: the whole extent of the decal, drops included
        float strength = 1.0f;                  // 0..1
        uint32_t order = 0;
        glm::vec3 direction = glm::vec3(0);     // bind, the way the blow travelled (unit), zero = none
        bool bulletHole = false;                // the picture is a bullet hole with blood around it, not a splash
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

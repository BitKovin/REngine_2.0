#pragma once
#include "../glm.h"
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>
#include "../utility/hashed_string.hpp"

// What the target's owner is doing, as far as being noticed goes.
enum class TargetMotion : uint8_t
{
    Still,
    Walk,
    Run,
    CrouchWalk,
    CrouchStill
};

class ObservationTarget
{
public:
    glm::vec3 position;

    std::string ownerId;

    std::unordered_set<hashed_string> tags;

    bool active = true;
    bool npc = false;
    bool isTriggeredNpc = false;

    float noticeMaxDistanceMultiplier = 1.0f;

    // ---- Used by observers that have a VisionProfile (see Observer.h) ------------------------------------------

    // Points on the body that sight rays go to, in world space, set by the owner every frame. Without them the ray
    // goes to `position`.
    glm::vec3 chestPoint{};
    glm::vec3 headPoint{};
    bool hasSamplePoints = false;

    // 0..1 scale the owner applies to how visible it is (stance, mist form, powers). 0 = invisible.
    float visibility = 1.0f;

    // True while the owner moves or makes noise. The near zone only notices a target that does.
    bool moving = true;

    // Set by the owner every frame; the game's detection rules read it (how fast an observer's awareness fills).
    TargetMotion motion = TargetMotion::Walk;

    // Brightness at the chest, 0..1, written by the engine (AiPerceptionSystem::Update, 4 times a second) from the
    // BSP light volumes when `sampleLight` is set; stays 1 (fully lit) otherwise.
    bool sampleLight = false;
    float lightLevel = 1.0f;
    double nextLightSampleTime = 0.0;

    ObservationTarget(const glm::vec3& pos,
        const std::vector<std::string>& tagList)
        : position(pos) {
        tags.reserve(tagList.size());
        for (const auto& s : tagList)
            tags.emplace(s);
    }

    bool HasTag(const hashed_string& tag) const { return tags.count(tag) > 0; }

    // The point sight rays aim at.
    glm::vec3 SightPoint() const { return hasSamplePoints ? chestPoint : position; }
};

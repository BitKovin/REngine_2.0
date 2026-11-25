#pragma once
#include "../glm.h"
#include <vector>
#include <memory>
#include <string>
#include "../utility/hashed_string.hpp"

class Entity;

class ObservationTarget;

struct HeardSound
{
    glm::vec3   position;   // World position where the sound was emitted
    float       radius;     // Radius the sound was emitted with
    int         severity;   // AI logic value: higher = more alarming (for EmitSound: the sound type)
    std::string causerId;

    // Filled by AiPerceptionSystem::EmitSound (EmitSoundAt leaves the defaults).
    glm::vec3   perceivedPosition{};   // Where the listener thinks it came from (blurred when the sound is faint)
    float       loudnessAtListener = 1.0f; // 0..1 after distance and walls
    std::string apparentId;            // Who the sound seems to come from (differs from causerId under a disguise)
};

// What a result of an observer's senses is about.
enum class SenseKind : uint8_t
{
    Sight,
    Smell,
    Aura,
    Touch
};

// Which part of the vision profile saw the target.
enum class VisionZone : uint8_t
{
    None,
    Focus,
    Peripheral,
    Near
};

// Shape and reach of an observer's eyes (distances in metres, angles in degrees, half-angles from the gaze).
// An observer has one when `useVisionProfile` is set; otherwise it keeps the old single cone (`fovDeg`, `maxDistance`).
struct VisionProfile
{
    // Focus: a keyhole. Wide inside focusWideRange, narrowing to focusHalfAngleDeg at focusRange.
    float focusRange = 25.0f;
    float focusHalfAngleDeg = 30.0f;
    float focusWideRange = 5.0f;
    float focusWideHalfAngleDeg = 60.0f;
    float focusFactor = 1.0f;

    // Peripheral: wide and short.
    float peripheralRange = 8.0f;
    float peripheralHalfAngleDeg = 100.0f;
    float peripheralFactor = 0.35f;

    // Near: a sphere that only notices a target that moves or makes noise (ObservationTarget::moving).
    float nearRange = 1.5f;
    float nearFactor = 0.6f;

    // Scales every range (alert level, combat sight). The owner sets it from its state.
    float rangeScale = 1.0f;

    // A target seen in the last update keeps its zone this much beyond the edge (0.1 = 10%).
    float hysteresis = 0.1f;

    // Strength falls linearly from 1 at closeDistance to farFactor at the zone's range.
    float closeDistance = 6.0f;
    float farFactor = 0.2f;

    // Light: strength scales from darkFactor in the dark to 1 at litLevel (ObservationTarget::lightLevel) and above.
    float darkFactor = 0.6f;
    float litLevel = 0.5f;
};

// A sense that works inside a radius instead of a cone (smell, aura). Results carry the sense kind.
struct ExtraSense
{
    SenseKind kind = SenseKind::Smell;
    float range = 6.0f;
    bool needsClearPath = true; // a wall between the two blocks it
    float strength = 1.0f;
};

// One thing an observer sensed this update.
struct SightResult
{
    std::shared_ptr<ObservationTarget> target;
    SenseKind sense = SenseKind::Sight;
    VisionZone zone = VisionZone::None;
    float strength = 0.0f;    // 0..1: zone x distance x visibility x light (sight); the extra sense's strength otherwise
    float distance = 0.0f;
    glm::vec3 point{};        // The point that was seen
};

class Observer
{
public:
    glm::vec3 position;
    glm::vec3 forward; // The gaze: what the owner visibly looks at. Set every frame by the owner.
    float fovDeg;
    float maxDistance = 60;

    hashed_string owner;

    Entity* ownerPtr = nullptr;

    bool searchForTriggeredNpc = true;

    int id = 0;

    // ---- Vision profile (optional; the old cone is used while useVisionProfile is false) -------------------------
    bool useVisionProfile = false;
    VisionProfile vision;
    std::vector<ExtraSense> extraSenses;

    // The observer's own target, which it never senses (profile path).
    const ObservationTarget* ownTarget = nullptr;

    // Also trace to the target's head when the chest is covered (combat sight).
    bool traceHead = false;

    // Results of the last update when useVisionProfile is set (sight first, then the extra senses).
    // `visibleTargets` holds the targets seen by sight, as before.
    std::vector<SightResult> sightResults;

    // Touches delivered by AiPerceptionSystem::Touch; like heardSounds they arrive one frame after.
    std::vector<SightResult> touches;
    std::vector<SightResult> pendingTouches;

    std::vector<std::shared_ptr<ObservationTarget>> visibleTargets;

    // Sounds readable by AI this frame. Populated at the start of Update() from
    // pendingSounds, so every entry was emitted during the *previous* frame.
    std::vector<HeardSound> heardSounds;

    // Staging buffer filled by AiPerceptionSystem::EmitSoundAt(). Promoted to
    // heardSounds at the start of the next Update() call — do not read directly.
    std::vector<HeardSound> pendingSounds;

    Observer(const glm::vec3& pos, const glm::vec3& fwd, float fov)
        : position(pos), forward(glm::normalize(fwd)), fovDeg(fov) {}

    void UpdateVisibility(const std::vector<std::shared_ptr<ObservationTarget>>& allTargets);

    // The zone of the vision profile that contains a point right now (None: outside every zone, or no profile).
    // Looks at the shape only: there is no line-of-sight test.
    VisionZone ZoneAt(const glm::vec3& point) const;

private:
    void UpdateVisibilityLegacy(const std::vector<std::shared_ptr<ObservationTarget>>& allTargets);
    void UpdateVisibilityProfiled(const std::vector<std::shared_ptr<ObservationTarget>>& allTargets);

    bool SeesPoint(const glm::vec3& point) const;
};

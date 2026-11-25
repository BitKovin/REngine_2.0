#pragma once
#include "../glm.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>

class Observer;
class ObservationTarget;

// A sound for AiPerceptionSystem::EmitSound.
struct SoundEvent
{
    glm::vec3 position{};
    float loudness = 10.0f;        // Radius in metres at which it is just heard
    int type = 0;                  // Game-defined sound type; delivered as HeardSound::severity
    std::string causerId;          // Who really made it
    std::string apparentId;        // Who it seems to come from (a disguise changes it); causerId when empty
    bool minorPlayerSound = false; // Footsteps, landing, a door, a thrown object: reduced for listeners off every player's screen
};

// Tuning of EmitSound (data the game may set once at start).
struct SoundSettings
{
    float wallPenalty = 6.0f;        // Metres of distance each solid wall on the straight line adds
    int maxWalls = 3;                // Walls counted per listener
    float wallSkip = 0.6f;           // Metres skipped past a wall before looking for the next one
    float fairnessDistance = 6.0f;   // Listeners nearer than this always hear a minor player sound at full radius
    float fairnessRadiusScale = 0.5f;
    float blurFraction = 0.25f;      // Faint sounds are placed up to this fraction of the distance off
};

class AiPerceptionSystem
{
public:
    static std::shared_ptr<Observer> CreateObserver(const glm::vec3& position, const glm::vec3& forward, float fovDeg);
    static std::shared_ptr<ObservationTarget> CreateTarget(const glm::vec3& position, const std::string& ownerId, const std::vector<std::string>& tags);

    static void RemoveObserver(const std::shared_ptr<Observer>& observer);
    static void RemoveTarget(const std::shared_ptr<ObservationTarget>& target);

    static void RemoveAll();

    static void Update(); // Main update loop - updates visibility for all observers

    // Old sound: every observer inside the radius hears it exactly, walls do not matter.
    static void EmitSoundAt(const glm::vec3& position, float radius, int severity, std::string causerId);

    // Sound with walls: the straight distance plus a penalty per solid wall must fit in the loudness. The result
    // carries the loudness at the listener (1 at the source, 0 at the edge) and a perceived position that is
    // blurred for faint sounds.
    static void EmitSound(const SoundEvent& sound);

    // The distance a sound or a call travelling from `from` to `to` has to cover: the straight distance plus the
    // wall penalty for each solid wall on the line (the rule EmitSound hears by; the game's messages use it too).
    static float MuffledDistance(const glm::vec3& from, const glm::vec3& to);

    // The observer touched the target (a bump, a grab): reported next frame in observer->touches, strength 1.
    static void Touch(const std::shared_ptr<Observer>& observer, const std::shared_ptr<ObservationTarget>& target);

    static inline SoundSettings soundSettings;

    // The game tells whether any local player has the position on screen (for the minor-sound fairness rule).
    // Without it the rule is off.
    static inline std::function<bool(const glm::vec3&)> isOnPlayerScreen;

    // Draws the gaze and vision zones of every observer with a vision profile, what they see, and recent sounds.
    static inline bool DebugDrawEnabled = false;
    static void DrawDebug();

    static const std::vector<std::shared_ptr<Observer>>& GetObservers() { return observers; }
    static const std::vector<std::shared_ptr<ObservationTarget>>& GetTargets() { return targets; }

    static std::vector<std::shared_ptr<Observer>> GetObserversInRadius(const glm::vec3& position, float radius);

    static std::vector<std::shared_ptr<ObservationTarget>>
        GetTargetsInRadiusWithTagOrdered(const glm::vec3& position, float radius, const std::string& tag);

	static std::shared_ptr<ObservationTarget>GetObserverFromEntityId(std::string entityId);

private:
    struct RecentSound
    {
        glm::vec3 position;
        float loudness;
        double time;
    };

    static std::vector<std::shared_ptr<Observer>> observers;
    static std::vector<std::shared_ptr<ObservationTarget>> targets;
    static std::vector<RecentSound> recentSounds; // only kept while DebugDrawEnabled

    static inline int nextId = 0;

    static void SampleLight();
};

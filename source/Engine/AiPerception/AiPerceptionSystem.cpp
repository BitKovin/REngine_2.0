#include "AiPerceptionSystem.h"
#include "Observer.h"
#include "ObservationTarget.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <utility>
#include "../EngineMain.h"
#include "../Physics.h"
#include "../DebugDraw.hpp"
#include "../Time.hpp"
#include <tracy/tracy/Tracy.hpp>
#include <Level.hpp>

std::vector<std::shared_ptr<Observer>> AiPerceptionSystem::observers;
std::vector<std::shared_ptr<ObservationTarget>> AiPerceptionSystem::targets;
std::vector<AiPerceptionSystem::RecentSound> AiPerceptionSystem::recentSounds;

std::shared_ptr<Observer> AiPerceptionSystem::CreateObserver(const glm::vec3& position, const glm::vec3& forward, float fovDeg)
{
    auto observer = std::make_shared<Observer>(position, forward, fovDeg);
    observer->id = nextId;
    nextId++;
    observers.push_back(observer);
    return observer;
}

std::shared_ptr<ObservationTarget> AiPerceptionSystem::CreateTarget(const glm::vec3& position, const std::string& ownerId, const std::vector<std::string>& tags)
{
    auto target = std::make_shared<ObservationTarget>(position, tags);
    target->ownerId = ownerId;
    targets.push_back(target);
    return target;
}

void AiPerceptionSystem::RemoveObserver(const std::shared_ptr<Observer>& observer)
{
    if (observer == nullptr) return;
    observer->ownerPtr = nullptr;

    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void AiPerceptionSystem::RemoveTarget(const std::shared_ptr<ObservationTarget>& target)
{
    if (target == nullptr) return;
    targets.erase(std::remove(targets.begin(), targets.end(), target), targets.end());
}

void AiPerceptionSystem::RemoveAll()
{
    observers.clear();
    targets.clear();
    recentSounds.clear();
}

// Brightness at the chest of every target that asks for it, 4 times a second each. Main thread: it reads the
// BSP light volumes and the physics world, which the observers' worker threads do not touch.
void AiPerceptionSystem::SampleLight()
{
    if (Level::Current == nullptr) return;

    const double now = Time::GameTime;
    const float interval = 0.25f;

    for (auto& target : targets)
    {
        if (target->sampleLight == false || target->active == false) continue;
        if (now < target->nextLightSampleTime) continue;

        target->nextLightSampleTime = now + interval;

        const LightVolPointData light = Level::Current->BspData.GetLightvolColorPoint(target->SightPoint());
        const glm::vec3 color = light.ambientColor + light.directColor;
        const float luma = glm::dot(color, glm::vec3(0.299f, 0.587f, 0.114f));
        target->lightLevel = std::min(1.0f, std::max(0.0f, luma));
    }
}

void AiPerceptionSystem::Update()
{
    const int frameDistrib = 4;

    observers.erase(
        std::remove(observers.begin(), observers.end(), nullptr),
        observers.end()
    );

    SampleLight();

    std::vector<Observer*> toUpdate;
    toUpdate.reserve(observers.size() / frameDistrib + 1);

    int currentFrame = EngineMain::MainInstance->frame;

    for (auto& observer : observers)
    {
        observer->heardSounds = std::move(observer->pendingSounds);
        observer->pendingSounds.clear();

        observer->touches = std::move(observer->pendingTouches);
        observer->pendingTouches.clear();

        if (observer->id % frameDistrib == currentFrame % frameDistrib)
            toUpdate.push_back(observer.get());
    }

    if (!toUpdate.empty())
    {
        Level::Current->asyncUpdateThreadPool->ParallelFor(toUpdate.size(), [&toUpdate](size_t i)
            {
                ZoneScoped;
                std::string zoneName = "Observer " + toUpdate[i]->owner.str();
                ZoneName(zoneName.c_str(), zoneName.size());
                toUpdate[i]->UpdateVisibility(targets);
            });
    }
}

void AiPerceptionSystem::EmitSoundAt(const glm::vec3& position, float radius, int severity, std::string causerId)
{
    if (observers.empty() || radius <= 0.0f)
        return;

    const float radiusSq = radius * radius;

    for (auto& observer : observers)
    {
        const glm::vec3 delta = observer->position - position;
        const float distSq = glm::dot(delta, delta);

        if (distSq <= radiusSq)
        {
            HeardSound heard{ position, radius, severity, causerId };
            heard.perceivedPosition = position;
            heard.apparentId = causerId;
            observer->pendingSounds.push_back(std::move(heard));
        }
    }
}

namespace
{
    // How many solid world walls the straight line crosses (at most `maxWalls`). After a wall the search restarts
    // a little past it, so a thick wall is not counted twice.
    int CountWalls(const glm::vec3& from, const glm::vec3& to, const SoundSettings& settings)
    {
        int walls = 0;
        glm::vec3 start = from;

        while (walls < settings.maxWalls)
        {
            const glm::vec3 remaining = to - start;
            const float length = glm::length(remaining);
            if (length < 0.001f) break;

            const auto hit = Physics::LineTrace(start, to, BodyType::WorldOpaque);
            if (hit.hasHit == false) break;

            walls++;

            const glm::vec3 dir = remaining / length;
            start = hit.position + dir * settings.wallSkip;
            if (glm::dot(to - start, dir) <= 0.0f) break;
        }

        return walls;
    }

    // A random point on a horizontal disc.
    glm::vec3 RandomOffset(float radius)
    {
        static thread_local std::mt19937 rng{ std::random_device{}() };
        std::uniform_real_distribution<float> unit(0.0f, 1.0f);

        const float angle = unit(rng) * 6.2831853f;
        const float r = std::sqrt(unit(rng)) * radius;
        return glm::vec3(std::cos(angle) * r, 0.0f, std::sin(angle) * r);
    }
}

float AiPerceptionSystem::MuffledDistance(const glm::vec3& from, const glm::vec3& to)
{
    const SoundSettings& settings = soundSettings;
    return glm::length(to - from) + CountWalls(from, to, settings) * settings.wallPenalty;
}

void AiPerceptionSystem::EmitSound(const SoundEvent& sound)
{
    if (observers.empty() || sound.loudness <= 0.0f)
        return;

    const SoundSettings& settings = soundSettings;

    if (DebugDrawEnabled)
        recentSounds.push_back({ sound.position, sound.loudness, Time::GameTimeNoPause });

    for (auto& observer : observers)
    {
        const float distance = glm::length(observer->position - sound.position);

        // Walls only ever make a sound shorter, so anything beyond the loudness is out without a ray.
        float radius = sound.loudness;
        if (distance > radius)
            continue;

        if (sound.minorPlayerSound && isOnPlayerScreen && distance > settings.fairnessDistance
            && isOnPlayerScreen(observer->position) == false)
            radius *= settings.fairnessRadiusScale;

        if (distance > radius)
            continue;

        const float effective = MuffledDistance(sound.position, observer->position);
        if (effective > radius)
            continue;

        const float loudness = 1.0f - effective / radius;

        HeardSound heard;
        heard.position = sound.position;
        heard.radius = sound.loudness;
        heard.severity = sound.type;
        heard.causerId = sound.causerId;
        heard.loudnessAtListener = loudness;
        heard.apparentId = sound.apparentId.empty() ? sound.causerId : sound.apparentId;
        heard.perceivedPosition = sound.position + RandomOffset(settings.blurFraction * distance * (1.0f - loudness));

        observer->pendingSounds.push_back(std::move(heard));
    }
}

void AiPerceptionSystem::Touch(const std::shared_ptr<Observer>& observer, const std::shared_ptr<ObservationTarget>& target)
{
    if (observer == nullptr || target == nullptr) return;

    SightResult result;
    result.target = target;
    result.sense = SenseKind::Touch;
    result.strength = 1.0f;
    result.point = target->SightPoint();
    result.distance = glm::length(result.point - observer->position);

    observer->pendingTouches.push_back(std::move(result));
}

namespace
{
    glm::vec3 RotateY(const glm::vec3& v, float radians)
    {
        const float c = std::cos(radians);
        const float s = std::sin(radians);
        return glm::vec3(v.x * c + v.z * s, v.y, -v.x * s + v.z * c);
    }

    void DrawCircle(const glm::vec3& center, float radius, uint32_t color, float duration)
    {
        std::vector<glm::vec3> points;
        const int steps = 32;
        points.reserve(steps + 1);
        for (int i = 0; i <= steps; ++i)
        {
            const float a = 6.2831853f * i / steps;
            points.push_back(center + glm::vec3(std::cos(a) * radius, 0.0f, std::sin(a) * radius));
        }
        DebugDraw::Path(points, duration, 0.02f, color);
    }

    // The outline of a zone drawn on the eye's horizontal plane: along the left edge out to the range, around the
    // far edge, and back along the right edge. `halfAngleAt` gives the half-angle (degrees) at a distance.
    template <typename HalfAngleFn>
    void DrawZone(const glm::vec3& eye, const glm::vec3& flatGaze, float range, HalfAngleFn halfAngleAt,
        uint32_t color, float duration)
    {
        const int steps = 10;
        std::vector<glm::vec3> outline;

        outline.push_back(eye);
        for (int i = 1; i <= steps; ++i)
        {
            const float d = range * i / steps;
            outline.push_back(eye + RotateY(flatGaze, glm::radians(halfAngleAt(d))) * d);
        }

        // Far edge from the left angle to the right angle.
        const float farHalf = halfAngleAt(range);
        for (int i = 1; i < steps; ++i)
        {
            const float a = farHalf - 2.0f * farHalf * i / steps;
            outline.push_back(eye + RotateY(flatGaze, glm::radians(a)) * range);
        }

        for (int i = steps; i >= 1; --i)
        {
            const float d = range * i / steps;
            outline.push_back(eye + RotateY(flatGaze, glm::radians(-halfAngleAt(d))) * d);
        }
        outline.push_back(eye);

        DebugDraw::Path(outline, duration, 0.02f, color);
    }
}

void AiPerceptionSystem::DrawDebug()
{
    if (DebugDrawEnabled == false) return;

    const float duration = 0.05f;

    for (auto& observer : observers)
    {
        if (observer->useVisionProfile == false) continue;

        const VisionProfile& p = observer->vision;
        const glm::vec3 eye = observer->position;
        const glm::vec3 gaze = glm::normalize(observer->forward);

        glm::vec3 flat(gaze.x, 0.0f, gaze.z);
        if (glm::length(flat) < 0.001f) flat = glm::vec3(0.0f, 0.0f, 1.0f);
        flat = glm::normalize(flat);

        DebugDraw::Line(eye, eye + gaze * 2.0f, duration, 0.02f, DebugColor::White);

        const float focusRange = p.focusRange * p.rangeScale;
        DrawZone(eye, flat, focusRange,
            [&](float d)
            {
                if (d <= p.focusWideRange || focusRange <= p.focusWideRange) return p.focusWideHalfAngleDeg;
                const float t = std::min(1.0f, (d - p.focusWideRange) / (focusRange - p.focusWideRange));
                return glm::mix(p.focusWideHalfAngleDeg, p.focusHalfAngleDeg, t);
            },
            DebugColor::Yellow, duration);

        DrawZone(eye, flat, p.peripheralRange * p.rangeScale,
            [&](float) { return p.peripheralHalfAngleDeg; }, DebugColor::Orange, duration);

        DrawCircle(eye, p.nearRange, DebugColor::Cyan, duration);

        for (const SightResult& r : observer->sightResults)
        {
            const uint32_t color = r.sense == SenseKind::Sight ? DebugColor::Green
                : r.sense == SenseKind::Smell ? DebugColor::Magenta
                : DebugColor::Blue;
            DebugDraw::Line(eye, r.point, duration, 0.02f, color);
        }

        for (const SightResult& r : observer->touches)
            DebugDraw::Line(eye, r.point, duration, 0.04f, DebugColor::Red);
    }

    // Sounds: a ring at their loudness for one second.
    const double now = Time::GameTimeNoPause;
    recentSounds.erase(std::remove_if(recentSounds.begin(), recentSounds.end(),
        [now](const RecentSound& s) { return now - s.time > 1.0; }), recentSounds.end());

    for (const RecentSound& s : recentSounds)
        DrawCircle(s.position, s.loudness, DebugColor::Magenta, duration);
}

std::vector<std::shared_ptr<Observer>> AiPerceptionSystem::GetObserversInRadius(const glm::vec3& position, float radius)
{
    std::vector<std::shared_ptr<Observer>> result;
    if (observers.empty() || radius <= 0.0f)
        return result;

    float radiusSq = radius * radius;

    for (auto& obs : observers)
    {
        float distSq = glm::length2(obs->position - position);
        if (distSq <= radiusSq)
        {
            result.push_back(obs);
        }
    }

    return result;
}

std::vector<std::shared_ptr<ObservationTarget>>
AiPerceptionSystem::GetTargetsInRadiusWithTagOrdered(const glm::vec3& position, float radius, const std::string& tag)
{
    std::vector<std::shared_ptr<ObservationTarget>> result;

    if (targets.empty() || radius <= 0.0f)
        return result;

    const float radiusSq = radius * radius;

    // Collect candidate pairs (distSq, index) to avoid bumping shared_ptr refcounts
    std::vector<std::pair<float, size_t>> candidates;
    candidates.reserve(targets.size());

    for (size_t i = 0; i < targets.size(); ++i)
    {
        const auto& t = targets[i];
        if (!t) continue;                         // defensive: skip null shared_ptr
        if (!t->HasTag(tag)) continue;           // tag test (O(1) average, unordered_set)

        const glm::vec3 delta = t->position - position;
        const float distSq = glm::dot(delta, delta); // same as glm::length2(delta)

        if (distSq <= radiusSq)
            candidates.emplace_back(distSq, i);
    }

    if (candidates.empty())
        return result;

    // Sort candidates by distance (ascending)
    std::sort(candidates.begin(), candidates.end(),
        [](const std::pair<float, size_t>& a, const std::pair<float, size_t>& b) {
            return a.first < b.first;
        });

    // Build result copying shared_ptrs only for selected (and ordered) targets
    result.reserve(candidates.size());
    for (const auto& p : candidates)
        result.push_back(targets[p.second]);

    return result;
}

std::shared_ptr<ObservationTarget> AiPerceptionSystem::GetObserverFromEntityId(std::string entityId)
{
    for (const auto& target : targets)
    {
        if (target && target->ownerId == entityId)
        {
            return target;
        }
    }
    return nullptr;
}

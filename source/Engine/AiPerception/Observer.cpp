#include "Observer.h"
#include "ObservationTarget.h"
#include "../glm.h"
#include <cmath>
#include <algorithm>
#include "../Physics.h"
#include "../DebugDraw.hpp"

#include <Level.hpp>

void Observer::UpdateVisibility(const std::vector<std::shared_ptr<ObservationTarget>>& allTargets)
{
    if (useVisionProfile)
        UpdateVisibilityProfiled(allTargets);
    else
        UpdateVisibilityLegacy(allTargets);
}

void Observer::UpdateVisibilityLegacy(const std::vector<std::shared_ptr<ObservationTarget>>& allTargets)
{
    visibleTargets.clear();

    for (auto& target : allTargets)
    {

        if (target->active == false) continue;

        if (searchForTriggeredNpc == false)
        {
            if (target->isTriggeredNpc && target->npc)
            {
                continue;
            }
        }

        glm::vec3 toTarget = target->position - position;

        float dist = glm::length(toTarget) / target->noticeMaxDistanceMultiplier;

        if (dist < 0.001f || dist > maxDistance)
            continue; // Skip if same position or too far

        toTarget = glm::normalize(toTarget);
        float angle = glm::degrees(glm::angle(forward, toTarget));

        if (angle <= fovDeg * 0.5f)
        {

			int observerCluster = Level::Current->BspData.FindClusterAtPosition(position);
			int targetCluster = Level::Current->BspData.FindClusterAtPosition(target->position);

            if (Level::Current->BspData.IsClusterVisible(observerCluster, targetCluster) == false) continue;

            auto hit = Physics::LineTrace(position, target->position, BodyType::WorldOpaque | BodyType::MainBody);

            if (hit.hasHit == false)
            {
                visibleTargets.push_back(target);
            }


        }
    }
}

// True when the BSP says the two places can see each other and nothing opaque or solid is on the segment.
bool Observer::SeesPoint(const glm::vec3& point) const
{
    auto& bsp = Level::Current->BspData;

    const int observerCluster = bsp.FindClusterAtPosition(position);
    const int targetCluster = bsp.FindClusterAtPosition(point);
    if (bsp.IsClusterVisible(observerCluster, targetCluster) == false)
        return false;

    return Physics::LineTrace(position, point, BodyType::WorldOpaque | BodyType::MainBody).hasHit == false;
}

namespace
{
    float Saturate(float v) { return std::min(1.0f, std::max(0.0f, v)); }

    // Half-angle of the focus keyhole at a distance: wide up close, narrowing to the far angle at the range.
    float FocusHalfAngle(const VisionProfile& p, float dist, float range)
    {
        if (dist <= p.focusWideRange || range <= p.focusWideRange)
            return p.focusWideHalfAngleDeg;

        const float t = Saturate((dist - p.focusWideRange) / (range - p.focusWideRange));
        return glm::mix(p.focusWideHalfAngleDeg, p.focusHalfAngleDeg, t);
    }

    // The best zone of the profile that contains a point `dist` away (already scaled by how hard the target is to
    // notice) at `angle` degrees from the gaze. `margin` widens every zone (the hysteresis of a target already seen).
    VisionZone ClassifyZone(const VisionProfile& p, float dist, float angle, bool moving, float margin, float& zoneFactor, float& zoneRange)
    {
        VisionZone zone = VisionZone::None;
        zoneFactor = 0.0f;
        zoneRange = 0.0f;

        const float focusRange = p.focusRange * p.rangeScale * margin;
        if (dist <= focusRange && angle <= FocusHalfAngle(p, dist, focusRange) * margin)
        {
            zone = VisionZone::Focus;
            zoneFactor = p.focusFactor;
            zoneRange = focusRange;
        }

        const float peripheralRange = p.peripheralRange * p.rangeScale * margin;
        if (zoneFactor < p.peripheralFactor && dist <= peripheralRange && angle <= p.peripheralHalfAngleDeg * margin)
        {
            zone = VisionZone::Peripheral;
            zoneFactor = p.peripheralFactor;
            zoneRange = peripheralRange;
        }

        const float nearRange = p.nearRange * margin;
        if (zoneFactor < p.nearFactor && moving && dist <= nearRange)
        {
            zone = VisionZone::Near;
            zoneFactor = p.nearFactor;
            zoneRange = nearRange;
        }

        return zone;
    }
}

VisionZone Observer::ZoneAt(const glm::vec3& point) const
{
    if (useVisionProfile == false) return VisionZone::None;

    const glm::vec3 toPoint = point - position;
    const float distance = glm::length(toPoint);
    if (distance < 0.001f) return VisionZone::Near;

    const float angle = glm::degrees(glm::angle(glm::normalize(forward), toPoint / distance));

    float zoneFactor, zoneRange;
    return ClassifyZone(vision, distance, angle, true, 1.0f, zoneFactor, zoneRange);
}

void Observer::UpdateVisibilityProfiled(const std::vector<std::shared_ptr<ObservationTarget>>& allTargets)
{
    // Targets seen in the last update get the hysteresis margin.
    std::vector<const ObservationTarget*> wasSeen;
    wasSeen.reserve(sightResults.size());
    for (const SightResult& r : sightResults)
    {
        if (r.sense == SenseKind::Sight)
            wasSeen.push_back(r.target.get());
    }

    sightResults.clear();
    visibleTargets.clear();

    const VisionProfile& p = vision;
    const glm::vec3 gaze = glm::normalize(forward);

    for (const auto& target : allTargets)
    {
        if (target->active == false || target.get() == ownTarget) continue;

        if (searchForTriggeredNpc == false && target->isTriggeredNpc && target->npc)
            continue;

        const glm::vec3 sightPoint = target->SightPoint();
        const glm::vec3 toTarget = sightPoint - position;
        const float rawDistance = glm::length(toTarget);
        if (rawDistance < 0.001f)
            continue;

        // A target that is hard to notice (noticeMaxDistanceMultiplier < 1) counts as farther away.
        const float dist = rawDistance / target->noticeMaxDistanceMultiplier;
        const glm::vec3 dir = toTarget / rawDistance;
        const float angle = glm::degrees(glm::angle(gaze, dir));

        // ---- Sight: the best zone that contains the target ----
        bool seen = false;
        float zoneFactor = 0.0f;
        float zoneRange = 0.0f;
        VisionZone zone = VisionZone::None;

        if (target->visibility > 0.0f)
        {
            const bool again = std::find(wasSeen.begin(), wasSeen.end(), target.get()) != wasSeen.end();
            const float margin = again ? 1.0f + p.hysteresis : 1.0f;

            zone = ClassifyZone(p, dist, angle, target->moving, margin, zoneFactor, zoneRange);
            seen = zone != VisionZone::None;
        }

        if (seen)
        {
            glm::vec3 seenPoint = sightPoint;
            bool visible = SeesPoint(sightPoint);

            if (visible == false && traceHead && target->hasSamplePoints)
            {
                visible = SeesPoint(target->headPoint);
                seenPoint = target->headPoint;
            }

            if (visible)
            {
                float distanceFactor = 1.0f;
                if (dist > p.closeDistance && zoneRange > p.closeDistance)
                    distanceFactor = glm::mix(1.0f, p.farFactor, Saturate((dist - p.closeDistance) / (zoneRange - p.closeDistance)));

                const float lightFactor = glm::mix(p.darkFactor, 1.0f, Saturate(target->lightLevel / std::max(p.litLevel, 0.001f)));

                SightResult result;
                result.target = target;
                result.sense = SenseKind::Sight;
                result.zone = zone;
                result.strength = zoneFactor * distanceFactor * Saturate(target->visibility) * lightFactor;
                result.distance = rawDistance;
                result.point = seenPoint;

                if (result.strength > 0.0f)
                {
                    visibleTargets.push_back(target);
                    sightResults.push_back(std::move(result));
                }
            }
        }
    }

    // ---- Extra senses: a radius, optionally blocked by walls ----
    for (const ExtraSense& sense : extraSenses)
    {
        for (const auto& target : allTargets)
        {
            if (target->active == false || target.get() == ownTarget) continue;

            if (searchForTriggeredNpc == false && target->isTriggeredNpc && target->npc)
                continue;

            const glm::vec3 point = target->SightPoint();
            const float dist = glm::length(point - position);
            if (dist < 0.001f || dist > sense.range)
                continue;

            if (sense.needsClearPath
                && Physics::LineTrace(position, point, BodyType::WorldOpaque).hasHit)
                continue;

            SightResult result;
            result.target = target;
            result.sense = sense.kind;
            result.strength = sense.strength;
            result.distance = dist;
            result.point = point;
            sightResults.push_back(std::move(result));
        }
    }
}

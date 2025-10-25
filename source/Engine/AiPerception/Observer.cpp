#include "Observer.h"
#include "ObservationTarget.h"
#include <glm/gtx/vector_angle.hpp>
#include <cmath>
#include "../Physics.h"

void Observer::UpdateVisibility(const std::vector<std::shared_ptr<ObservationTarget>>& allTargets)
{
    visibleTargets.clear();

    for (auto& target : allTargets)
    {
        glm::vec3 toTarget = target->position - position;

        float dist = glm::length(toTarget);

        if (dist < 0.001f || dist > maxDistance)
            continue; // Skip if same position or too far

        toTarget = glm::normalize(toTarget);
        float angle = glm::degrees(glm::angle(forward, toTarget));

        if (angle <= fovDeg * 0.5f)
        {

            auto hit = Physics::LineTrace(position, target->position, BodyType::WorldOpaque | BodyType::MainBody);

            if (hit.hasHit == false)
            {
                visibleTargets.push_back(target);
            }


        }
    }
}

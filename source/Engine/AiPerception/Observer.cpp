#include "Observer.h"
#include "ObservationTarget.h"
#include <glm/gtx/vector_angle.hpp>
#include <cmath>

void Observer::UpdateVisibility(const std::vector<std::shared_ptr<ObservationTarget>>& allTargets)
{
    visibleTargets.clear();

    for (auto& target : allTargets)
    {
        glm::vec3 toTarget = target->position - position;
        if (glm::length(toTarget) < 0.001f)
            continue; // Skip if same position

        toTarget = glm::normalize(toTarget);
        float angle = glm::degrees(glm::angle(forward, toTarget));

        if (angle <= fovDeg * 0.5f)
            visibleTargets.push_back(target);
    }
}

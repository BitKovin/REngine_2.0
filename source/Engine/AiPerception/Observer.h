#pragma once
#include "../glm.h"
#include <vector>
#include <memory>
#include <string>
#include "../utility/hashed_string.hpp"

class Entity;

class ObservationTarget;

class Observer
{
public:
    glm::vec3 position;
    glm::vec3 forward;
    float fovDeg;
    float maxDistance = 60;

    hashed_string owner;

    Entity* ownerPtr = nullptr;

    bool searchForTriggeredNpc = true;

    int id = 0;

    std::vector<std::shared_ptr<ObservationTarget>> visibleTargets;

    Observer(const glm::vec3& pos, const glm::vec3& fwd, float fov)
        : position(pos), forward(glm::normalize(fwd)), fovDeg(fov) {
    }

    void UpdateVisibility(const std::vector<std::shared_ptr<ObservationTarget>>& allTargets);
};

#pragma once
#include "../glm.h"
#include <string>
#include <unordered_set>
#include <vector>

class ObservationTarget
{
public:
    glm::vec3 position;

    std::string ownerId;

    std::unordered_set<std::string> tags;

    bool active = true;

    float noticeMaxDistanceMultiplier = 1.0f;

    ObservationTarget(const glm::vec3& pos, const std::vector<std::string>& tagList)
        : position(pos), tags(tagList.begin(), tagList.end()) {
    }

    bool HasTag(const std::string& tag) const { return tags.count(tag) > 0; }
};

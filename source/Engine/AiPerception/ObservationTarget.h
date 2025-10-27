#pragma once
#include "../glm.h"
#include <string>
#include <unordered_set>
#include <vector>
#include "../utility/hashed_string.hpp"

class ObservationTarget
{
public:
    glm::vec3 position;

    std::string ownerId;

    std::unordered_set<hashed_string> tags;

    bool active = true;

    float noticeMaxDistanceMultiplier = 1.0f;

    ObservationTarget(const glm::vec3& pos,
        const std::vector<std::string>& tagList)
        : position(pos) {
        tags.reserve(tagList.size());
        for (const auto& s : tagList)
            tags.emplace(s);
    }

    bool HasTag(const hashed_string& tag) const { return tags.count(tag) > 0; }
};

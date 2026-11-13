#pragma once

#include "GoreProfile.h"
#include "GoreTypes.h"

#include <glm.h>
#include <vector>

namespace Gore
{
    struct CutRequest
    {
        uint16_t severableIndex = 0;                 // index into Profile::severable
        glm::vec3 worldPoint = glm::vec3(0);         // where the blow landed
        glm::vec3 worldNormal = glm::vec3(0, 1, 0);  // desired plane normal (any sign)
    };

    struct CutResult
    {
        bool ok = false;
        Cut cut;
    };

    // Finds a valid cut plane across a severable bone (bind space). Everything beyond the plane, in the bone's whole
    // subtree, has to be on the removed side - otherwise the plane is pulled towards the bone axis, and if that still
    // fails the cut is refused (the caller makes a wound instead).
    CutResult SolveCut(const Profile& profile, const roj::SkinnedModel& model, const glm::mat4& world,
                       const std::vector<glm::mat4>& palette, const CutRequest& req, Rng& rng);

    // Bone world-matrix helper shared by the sever code: world * palette[skin] (world when the bone has no skin slot).
    glm::mat4 BoneBindToWorld(const glm::mat4& world, const std::vector<glm::mat4>& palette, int skin);

    // Corners of a bone bound.
    void BoundCorners(const roj::BoneBound& b, glm::vec3 (&out)[8]);
}

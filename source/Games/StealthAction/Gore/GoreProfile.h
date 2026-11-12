#pragma once

#include <skinned_model.hpp>
#include <glm.h>

#include <cstdint>
#include <string>
#include <vector>

class Texture;

namespace Gore
{
    enum class SubmeshRole : uint8_t { Volume, Shell, Attachment };   // attachment = no gore at all

    struct SeverableBone
    {
        enum class GibKind : uint8_t { Rigid, Ragdoll };

        std::string bone;
        uint16_t skel = 0;
        int16_t skin = -1;
        glm::vec2 range = glm::vec2(0.2f, 0.8f);
        float maxTiltDeg = 30.0f;
        float boneRadius = 0.015f;
        GibKind gib = GibKind::Rigid;
        bool canDestroy = false;
        bool lethal = false;
        float spurtSeconds = 4.0f;
        float pulseHz = 1.6f;
        std::string boneStub;
    };

    // What one model can do: textures, submesh roles, severable bones, and the bind-space data derived from the mesh.
    struct Profile
    {
        bool valid = false;

        Texture* flesh = nullptr;
        Texture* fleshNormal = nullptr;
        Texture* blood = nullptr;
        Texture* edgeNoise = nullptr;
        Texture* boneT = nullptr;

        float fleshScale = 12.0f;   // texture repeats per metre
        glm::vec3 bloodColor = glm::vec3(0.35f, 0.02f, 0.02f);

        std::vector<SubmeshRole> submeshRoles;   // per model->meshes index
        std::vector<SeverableBone> severable;
        std::vector<int16_t> severableBySkel;    // size skeleton.boneCount, index into severable or -1

        // Derived at load (skel-indexed, merged over all submeshes):
        std::vector<roj::BoneBound> anyBounds;   // union of mesh.boneAnyBounds
        std::vector<uint16_t> parent, subtreeEnd;
    };

    // Loads (once, cached by model path) and validates. Returns a profile with valid=false when the file is missing,
    // the model isn't partitioned (mesh.boneIndexFirst empty), or validation fails - logged once per model.
    // Game thread only. A model without geometry yet is not cached: try again next time.
    const Profile& GetProfile(const std::string& modelPath, const roj::SkinnedModel& model);

    // Dev hot reload and level change.
    void ClearProfiles();

    // Largest half-extent of the any-influence bounds of a skel bone (bind metres), 0 if none.
    float MaxExtent(const Profile& profile, uint16_t skel);
}

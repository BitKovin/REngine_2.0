#pragma once

#include <memory>

#include "GoreTypes.h"

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
        float detachDamage = 60.0f;   // weighted damage (damage * source severPower) that removes the part at its joint
        float spurtSeconds = 4.0f;
        float pulseHz = 1.6f;
        std::string boneStub;

        // Vertex samples (bind space) that decide where a plane may cut: vertices carried mostly by the bone's
        // descendants (they leave with the part - all must lie beyond the plane) and mostly by the bone itself.
        // Bounding boxes are far too coarse for this (a diagonal arm's box reaches past the elbow).
        std::vector<glm::vec3> partPoints, ownPoints;
    };

    // An authored torso wound: a fixed half-sphere dent that opens when enough damage arrives from the front of it.
    // Everything is in model bind space, so the result looks the same on every body of the model and cannot break.
    struct WoundSlot
    {
        std::string name;
        std::string bone;                       // anchor bone: the rigid frame the wound follows
        uint16_t skel = 0;
        int16_t skin = -1;
        glm::vec3 center = glm::vec3(0);        // on the skin surface
        glm::vec3 facing = glm::vec3(0, 0, 1);  // outward normal; the wound digs along -facing
        glm::vec3 radii = glm::vec3(0.1f);      // x, y across the surface, z depth
        float threshold = 100.0f;               // weighted damage (damage * source holePower) that opens it
        float cosCone = 0.5f;                   // shots must come from within this cone around facing
        float catchRadius = 0.2f;               // ... and land this close to the centre
        float roughness = 0.4f;
        float rimWidth = 0.5f;
        // true: complete destruction. The hole goes through the whole body (front to back), opens from either side, and
        // `center` is the middle of the body. false: a deep dent from one side that leaves the far side intact.
        bool through = false;

        // The cavity geometry is the same for every body: built once for the first body that opens the slot.
        mutable std::shared_ptr<const CavityGeometry> cavity;
    };

    // What one model can do: textures, submesh roles, severable bones, and the bind-space data derived from the mesh.
    struct TriangleGrid;   // GoreWoundMesh.h

    struct Profile
    {
        std::shared_ptr<const TriangleGrid> triGrid;   // spatial index of the skin triangles (wound cavities)
        bool valid = false;
        bool loading = false;   // not valid yet, but the model is still being prepared: ask again later (hits are kept)

        Texture* flesh = nullptr;
        Texture* fleshNormal = nullptr;
        Texture* blood = nullptr;
        Texture* edgeNoise = nullptr;
        Texture* boneT = nullptr;

        float fleshScale = 12.0f;   // texture repeats per metre
        glm::vec3 bloodColor = glm::vec3(0.35f, 0.02f, 0.02f);

        std::vector<SubmeshRole> submeshRoles;   // per model->meshes index
        std::vector<SeverableBone> severable;
        std::vector<WoundSlot> slots;            // at most kMaxWounds
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

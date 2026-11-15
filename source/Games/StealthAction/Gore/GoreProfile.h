#pragma once

#include <memory>

#include "GoreTypes.h"

#include <skinned_model.hpp>
#include <glm.h>

#include <cstdint>
#include <string>
#include <vector>

class Texture;

class ThreadPool;

namespace Gore
{
    // attachment = no gore at all. prop = a held or worn object (an axe, a gun): it follows its bone (hidden with it, drawn with the part
    // that carries it) but is no part of the body: never wounded or cut, no flesh, and it never closes a cavity (its bind pose lies
    // inside the body, so a wound would otherwise stretch surface between the prop and the body when the prop moves).
    enum class SubmeshRole : uint8_t { Volume, Shell, Attachment, Prop };

    struct SeverableBone
    {
        enum class GibKind : uint8_t { Rigid, Ragdoll };

        std::string bone;
        uint16_t skel = 0;
        int16_t skin = -1;
        glm::vec2 range = glm::vec2(0.2f, 0.8f);
        float maxTiltDeg = 30.0f;
        float boneRadius = 0.015f;
        bool clipAncestors = false;   // trunk cuts: the plane also clips the triangles stored under the bone's ancestors (a jacket spanning the waist),
                                      // and cloth that hangs far below the plane stays on the body (Cut::stragglers) instead of blocking the cut
        float coreRadius = 0.0f;      // > 0: only part vertices this close to the bone axis must lie beyond the plane (arms hang beside a torso)
        GibKind gib = GibKind::Rigid;
        bool canDestroy = false;
        bool lethal = false;
        bool holdsProtected = false;  // the part contains a bone of Profile::protectedWhileAlive: a living enemy cannot lose it
        float detachDamage = 60.0f;   // weighted damage (damage * source severPower) that removes the part at its joint
        float spurtSeconds = 4.0f;
        float pulseHz = 1.6f;
        std::string boneStub;

        // Vertex samples (bind space) that decide where a plane may cut: vertices carried mostly by the bone's
        // descendants (they leave with the part - all must lie beyond the plane) and mostly by the bone itself.
        // Bounding boxes are far too coarse for this (a diagonal arm's box reaches past the elbow).
        std::vector<glm::vec3> partPoints, ownPoints;
        std::vector<uint16_t> partPointBones;   // parallel to partPoints: the skel bone that carries each most
    };

    // What one model can do: textures, submesh roles, severable bones, and the bind-space data derived from the mesh.
    struct ModelFlesh;   // GoreFlesh.h

    // Where the generated skeleton and organs go. Bone names are skeleton names after "boneMap" was applied.
    struct AnatomyConfig
    {
        glm::vec3 forward = glm::vec3(0, 0, 1);   // bind space: the direction the character faces
        glm::vec3 up = glm::vec3(0, 1, 0);        // bind space
        std::vector<uint16_t> torso;              // skel: bones of the trunk (they get a spine at the back, no limb bone)
        std::vector<uint16_t> ribs;               // skel: trunk bones that get ribs and chest organs
        std::vector<uint16_t> guts;               // skel: trunk bones that get a gut mass
        int head = -1;                            // skel: gets a skull, -1 = none
        float minBoneLength = 0.06f;              // metres: shorter bone segments get no bone inside
    };

    struct Profile
    {
        bool valid = false;
        bool loading = false;   // not valid yet, but the model is still being prepared: ask again later (hits are kept)

        // The textures belong to the asset registry, which frees every texture that was not requested during a level load
        // (AssetRegistry::ClearUnusedMemory at the end of a load). These are first requested at the first hit, after the load, so
        // a profile that is kept across a level change would point at freed textures. GetProfile therefore asks the registry for
        // them again on every call (the same pointer while they live, a fresh texture after they were freed): never keep these
        // pointers anywhere but in a draw that was set up in the same frame.
        Texture* fleshTexture = nullptr;   // the flesh texture (the generated geometry is `flesh`, below)
        Texture* blood = nullptr;
        Texture* decal = nullptr;          // the blood decal atlas (Tools/gore/make_blood_decals.py)
        Texture* edgeNoise = nullptr;
        Texture* boneT = nullptr;
        Texture* guts = nullptr;
        std::string fleshPath, bloodPath, decalPath, edgeNoisePath, bonePath, gutsPath;   // registry file names of the six above ("" = none)
        AnatomyConfig anatomy;
        std::shared_ptr<ModelFlesh> flesh;        // shell vertices + anatomy mesh, built once per model (GoreFlesh.cpp)
        struct PendingFlesh;                      // the worker that builds `flesh`; the profile is `loading` until it is done
        std::shared_ptr<PendingFlesh> pendingFlesh;

        float fleshScale = 12.0f;   // texture repeats per metre
        glm::vec3 bloodColor = glm::vec3(0.35f, 0.02f, 0.02f);

        std::vector<SubmeshRole> submeshRoles;   // per model->meshes index
        std::vector<SeverableBone> severable;
        std::vector<int16_t> severableBySkel;    // size skeleton.boneCount, index into severable or -1
    // Bones that stay whole while the enemy is alive: size skeleton.boneCount, 1 for every bone listed in "protectedWhileAlive"
    // and for everything below it. A hit on such a bone leaves no hole, cut or meat unless it is the hit that kills, and no cut
    // may remove a part that contains one (SeverableBone::holdsProtected). Blood still shows the hit.
    std::vector<uint8_t> protectedWhileAlive;

        // Derived at load (skel-indexed, merged over all submeshes):
        std::vector<roj::BoneBound> anyBounds;   // union of mesh.boneAnyBounds
        std::vector<uint16_t> parent, subtreeEnd;
    };

    // Loads (once, cached by model path) and validates. Returns a profile with valid=false when the file is missing,
    // the model isn't partitioned (mesh.boneIndexFirst empty), or validation fails - logged once per model.
    // Game thread only. A model without geometry yet is not cached: try again next time.
    const Profile& GetProfile(const std::string& modelPath, const roj::SkinnedModel& model);

    // Dev hot reload. Waits for the worker first. Not needed on a level change: a profile holds nothing of a level but the texture
    // pointers, and GetProfile renews those.
    void ClearProfiles();

    // The worker thread for everything heavy and one-off (model flesh, cut caps). One thread, so jobs finish in the order
    // they were queued. Where threads are not supported the job runs inline.
    ThreadPool& BuildPool();

    // Largest half-extent of the any-influence bounds of a skel bone (bind metres), 0 if none.
    float MaxExtent(const Profile& profile, uint16_t skel);
}

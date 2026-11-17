#pragma once

#include "GoreTypes.h"

#include <VertexData.h>
#include <skinned_model.hpp>
#include <bgfx/bgfx.h>
#include <glm.h>

#include <memory>
#include <vector>

namespace Gore
{
    struct Profile;
    class TriGrid;   // GoreTriGrid.h

    // One generated part of the anatomy mesh (a bone, a rib ring, the skull, a gut mass): a range of its index buffer.
    struct FleshPiece
    {
        uint32_t first = 0, count = 0;            // indices
        glm::vec3 bbMin = glm::vec3(0), bbMax = glm::vec3(0);   // bind space
        uint16_t skel = 0;                        // the bone it is rigidly attached to
    };

    // Generated once per model and shared by every body of that model.
    struct ModelFlesh
    {
        // Shells: per submesh (same order and size as model->meshes[i].vertices; empty = this submesh has no flesh).
        // Vertex fields: Position = model position, Normal = INWARD direction (unit), Tangent = the model's real normal,
        // ShadowMapCoords.x = max inset in metres. Bone indices / weights / uv are copied. Drawn with the model's index buffer.
        std::vector<std::vector<VertexData>> shellVertices;

        // Anatomy: one mesh for the whole model. Each vertex follows exactly one bone (weight 1).
        // Color.r = material (0 bone, 1 guts), Color.g = brightness, ShadowMapCoords.x = 1.
        std::vector<VertexData> anatomyVertices;
        std::vector<uint32_t> anatomyIndices;
        std::vector<FleshPiece> pieces;

        // The Volume submeshes in a grid (bind space), kept for the wound cavities. Null when the model has no Volume submesh.
        std::shared_ptr<const TriGrid> body;

        // The profile's core body fitted to this model's skeleton (GoreCore.h), in this model's bind space with this model's skin
        // indices. When present the wound cavities are generated against it (and clipped by `body`). Null: against `body`.
        std::shared_ptr<const TriGrid> fill;

        // GPU side. Main thread only (EnsureGpu).
        std::vector<bgfx::VertexBufferHandle> shellVbh;      // parallel to shellVertices, invalid where empty
        bgfx::VertexBufferHandle anatomyVbh = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle  anatomyIbh = BGFX_INVALID_HANDLE;
        bool gpuReady = false;

        void EnsureGpu();   // creates the buffers on first call; cheap afterwards
    };

    // Runs on the worker thread (BuildPool): reads the model and the profile, touches no GPU object. Needs
    // profile.submeshRoles, anyBounds, parent, subtreeEnd and anatomy to be filled.
    // `core`: the loaded core model of profile.corePath, or null.
    std::shared_ptr<ModelFlesh> BuildModelFlesh(const roj::SkinnedModel& model, const Profile& profile, const roj::SkinnedModel* core);

    // Four small meshes around the origin, about 1 unit in radius (scaled per chunk): variants[0..2] meat lumps,
    // variants[3] a bone splinter. Every vertex follows skin bone 0 (the chunk system gives it an identity matrix).
    void BuildChunkMeshes(std::vector<VertexData>& vertices, std::vector<uint32_t>& indices, std::vector<FleshPiece>& variants);
}

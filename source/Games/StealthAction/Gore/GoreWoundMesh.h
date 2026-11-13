#pragma once

#include "GoreProfile.h"
#include "GoreTypes.h"

#include <memory>
#include <vector>

namespace Gore
{
    // Spatial index of the skin triangles of a model (bind space), built once per profile. Attachment submeshes are left out.
    struct TriangleGrid
    {
        struct Tri
        {
            uint16_t mesh;
            uint32_t i0, i1, i2;
        };

        std::vector<Tri> tris;
        glm::vec3 origin = glm::vec3(0);
        float cell = 1.0f;
        glm::ivec3 dims = glm::ivec3(1);
        std::vector<uint32_t> cellStart;   // CSR: triangles of cell c are cellTris[cellStart[c] .. cellStart[c + 1])
        std::vector<uint32_t> cellTris;
    };

    std::shared_ptr<const TriangleGrid> BuildTriangleGrid(const roj::SkinnedModel& model, const Profile& profile);

    // Starts building the cavity of a wound (see CavityGeometry). Cheap on the calling (game) thread: a grid lookup and a copy
    // of the few hundred triangles around the wound. The geometry is built on a worker thread; the result arrives in
    // wound.cavityTicket (poll it with PollCavity).
    void RequestCavity(Wound& wound, const roj::SkinnedModel& model, const Profile& profile);

    // Game thread, once per tick: moves a finished result into wound.cavity. Returns true when the wound changed.
    bool PollCavity(Wound& wound);
}

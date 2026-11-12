#pragma once

// shadow_volume.hpp — see shadow_volume.cpp for full design notes.

#include <glm.h>
#include <bgfx/bgfx.h>
#include <vector>

namespace roj { struct SkinnedMesh; struct BonePartitionContext; }

namespace roj
{

struct ShadowVolumePrecomp
{
    // Cap geometry — indexed triangle list, VertexData layout.
    bgfx::VertexBufferHandle capVbh        = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle  capIbh        = BGFX_INVALID_HANDLE;
    uint32_t                 capIndexCount = 0;

    // Edge geometry — indexed triangle list, VertexData layout.
    bgfx::VertexBufferHandle edgeVbh        = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle  edgeIbh        = BGFX_INVALID_HANDLE;
    uint32_t                 edgeIndexCount = 0;

    // Only filled when built with a BonePartitionContext. Index offsets (6 indices per triangle / edge), size
    // boneCount + 1, sorted like SkinnedMesh::boneIndexFirst: bone b owns [first[b], first[b+1]).
    std::vector<uint32_t>    capBoneFirst;
    std::vector<uint32_t>    edgeBoneFirst;

    ShadowVolumePrecomp() = default;
    ShadowVolumePrecomp(const ShadowVolumePrecomp&)            = delete;
    ShadowVolumePrecomp& operator=(const ShadowVolumePrecomp&) = delete;

    ShadowVolumePrecomp(ShadowVolumePrecomp&& o) noexcept
        : capVbh(o.capVbh),   capIbh(o.capIbh),   capIndexCount(o.capIndexCount),
          edgeVbh(o.edgeVbh), edgeIbh(o.edgeIbh), edgeIndexCount(o.edgeIndexCount),
          capBoneFirst(std::move(o.capBoneFirst)), edgeBoneFirst(std::move(o.edgeBoneFirst))
    {
        o.capVbh = o.edgeVbh = BGFX_INVALID_HANDLE;
        o.capIbh = o.edgeIbh = BGFX_INVALID_HANDLE;
        o.capIndexCount = o.edgeIndexCount = 0;
    }

    ShadowVolumePrecomp& operator=(ShadowVolumePrecomp&& o) noexcept
    {
        if (this != &o) { this->~ShadowVolumePrecomp(); new(this) ShadowVolumePrecomp(std::move(o)); }
        return *this;
    }

    ~ShadowVolumePrecomp()
    {
        if (bgfx::isValid(capVbh))  bgfx::destroy(capVbh);
        if (bgfx::isValid(capIbh))  bgfx::destroy(capIbh);
        if (bgfx::isValid(edgeVbh)) bgfx::destroy(edgeVbh);
        if (bgfx::isValid(edgeIbh)) bgfx::destroy(edgeIbh);
    }
};

// Build once at load time while mesh.vertices/indices are still populated.
// With a valid ctx the cap triangles and edges are sorted by the skel-index LCA of their stitch bones
// (capBoneFirst / edgeBoneFirst); without it the behaviour is unchanged.
ShadowVolumePrecomp BuildShadowVolumePrecomp(const SkinnedMesh& mesh, const BonePartitionContext* ctx = nullptr);

} // namespace roj

#include "bone_partition.hpp"
#include "skinned_model.hpp"
#include "Logger.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

namespace roj
{

BonePartitionContext BuildBonePartitionContext(const FlatSkeleton& skeleton)
{
    BonePartitionContext ctx;

    const int n = skeleton.boneCount;
    if (n == 0 || skeleton.bones == nullptr)
        return ctx;

    ctx.parent.resize(n);
    ctx.subtreeEnd.resize(n);
    for (int i = 0; i < n; ++i)
    {
        ctx.parent[i] = skeleton.bones[i].parentIdx;
        ctx.subtreeEnd[i] = static_cast<uint16_t>(i + 1);

        if (ctx.parent[i] != INVALID_BONE_IDX && ctx.parent[i] >= i)
        {
            Logger::Error("BuildBonePartitionContext: bone %d has parent %d (not parent-before-child), no bone partition",
                          i, (int)ctx.parent[i]);
            ctx.parent.clear();
            ctx.subtreeEnd.clear();
            return ctx;
        }
    }

    for (int i = n - 1; i >= 1; --i)
    {
        const uint16_t p = ctx.parent[i];
        if (p != INVALID_BONE_IDX)
            ctx.subtreeEnd[p] = std::max(ctx.subtreeEnd[p], ctx.subtreeEnd[i]);
    }

    ctx.skinToSkel.assign(MAX_SKINNED_BONES, INVALID_BONE_IDX);
    for (int i = 0; i < n; ++i)
    {
        const int skin = skeleton.bones[i].skinIdx;
        if (skin >= 0 && skin < MAX_SKINNED_BONES)
            ctx.skinToSkel[skin] = static_cast<uint16_t>(i);
    }

    ctx.valid = true;
    return ctx;
}

uint16_t BoneLCA(const BonePartitionContext& ctx, uint16_t a, uint16_t b)
{
    const uint16_t n = static_cast<uint16_t>(ctx.parent.size());
    while (a != b)
    {
        if (a >= n || b >= n) return 0;
        if (a > b) a = ctx.parent[a]; else b = ctx.parent[b];
    }
    return a >= n ? 0 : a;
}

void PartitionMeshByBone(SkinnedMesh& mesh, const BonePartitionContext& ctx)
{
    if (!ctx.valid) return;

    const size_t n = ctx.parent.size();
    const size_t triCount = mesh.indices.size() / 3;

    // 1. Key per triangle = LCA of every bone that influences one of its vertices.
    std::vector<uint16_t> key(triCount, INVALID_BONE_IDX);
    for (size_t t = 0; t < triCount; ++t)
    {
        uint16_t k = INVALID_BONE_IDX;
        for (int c = 0; c < 3; ++c)
        {
            const VertexData& v = mesh.vertices[mesh.indices[t * 3 + c]];
            for (int j = 0; j < MAX_BONE_INFLUENCE; ++j)
            {
                if (!(v.BlendWeights[j] > 0.0f)) continue;
                const int skin = static_cast<int>(v.BlendIndices[j]);
                if (skin < 0 || skin >= MAX_SKINNED_BONES) continue;
                const uint16_t skel = ctx.skinToSkel[skin];
                if (skel == INVALID_BONE_IDX) continue;
                k = (k == INVALID_BONE_IDX) ? skel : BoneLCA(ctx, k, skel);
            }
        }
        key[t] = (k == INVALID_BONE_IDX) ? 0 : k;
    }

    // 2. Stable sort of the triangle order by key.
    std::vector<uint32_t> order(triCount);
    std::iota(order.begin(), order.end(), 0u);
    std::stable_sort(order.begin(), order.end(), [&](uint32_t x, uint32_t y) { return key[x] < key[y]; });

    // 3. New index order.
    std::vector<uint32_t> sorted(mesh.indices.size());
    for (size_t i = 0; i < triCount; ++i)
    {
        const uint32_t t = order[i];
        sorted[i * 3 + 0] = mesh.indices[t * 3 + 0];
        sorted[i * 3 + 1] = mesh.indices[t * 3 + 1];
        sorted[i * 3 + 2] = mesh.indices[t * 3 + 2];
    }
    // Leftover indices (a partial last triangle, should not happen) stay at the end, owned by no bone.
    for (size_t i = triCount * 3; i < mesh.indices.size(); ++i)
        sorted[i] = mesh.indices[i];
    mesh.indices = std::move(sorted);

    // 4. boneIndexFirst (index offsets).
    mesh.boneIndexFirst.assign(n + 1, 0);
    for (size_t t = 0; t < triCount; ++t)
        mesh.boneIndexFirst[key[t] + 1] += 3;
    for (size_t b = 0; b < n; ++b)
        mesh.boneIndexFirst[b + 1] += mesh.boneIndexFirst[b];
    mesh.boneIndexFirst[n] = static_cast<uint32_t>(mesh.indices.size());
    assert(mesh.boneIndexFirst[n] >= triCount * 3);

    // 5. Own bounds: bind-space AABB of the vertices of the triangles each bone owns.
    mesh.boneOwnBounds.assign(n, BoneBound{});
    for (size_t i = 0; i < triCount; ++i)
    {
        BoneBound& bb = mesh.boneOwnBounds[key[order[i]]];
        for (int c = 0; c < 3; ++c)
        {
            const glm::vec3& p = mesh.vertices[mesh.indices[i * 3 + c]].Position;
            bb.bbMin = glm::min(bb.bbMin, p);
            bb.bbMax = glm::max(bb.bbMax, p);
            bb.hasData = true;
        }
    }

    // 6. Any bounds: every vertex with any weight on that bone.
    mesh.boneAnyBounds.assign(n, BoneBound{});
    for (const VertexData& v : mesh.vertices)
    {
        for (int j = 0; j < MAX_BONE_INFLUENCE; ++j)
        {
            if (!(v.BlendWeights[j] > 0.0f)) continue;
            const int skin = static_cast<int>(v.BlendIndices[j]);
            if (skin < 0 || skin >= MAX_SKINNED_BONES) continue;
            const uint16_t skel = ctx.skinToSkel[skin];
            if (skel == INVALID_BONE_IDX) continue;
            BoneBound& bb = mesh.boneAnyBounds[skel];
            bb.bbMin = glm::min(bb.bbMin, v.Position);
            bb.bbMax = glm::max(bb.bbMax, v.Position);
            bb.hasData = true;
        }
    }
}

} // namespace roj

#pragma once

// bone_partition.hpp - sorts a skinned mesh's triangles by skeleton bone so that "the triangles of bone b" and "the
// triangles of the whole subtree of b" are each one contiguous index range.
//
// Two bone numberings exist (see skinned_model.hpp):
//   skel index - position in FlatSkeleton (DFS pre-order, parent < child, subtree of b = [b, subtreeEnd[b]))
//   skin index - slot in the bone palette / BlendIndices
// Everything in this file that says "bone" without a prefix is a skel index.

#include <glm.h>
#include <vector>
#include <cstdint>

namespace roj
{
    struct FlatSkeleton;
    struct SkinnedMesh;

    // Skeleton topology in the form the partition needs. Built from a FlatSkeleton.
    struct BonePartitionContext
    {
        std::vector<uint16_t> parent;       // parent[skel], INVALID_BONE_IDX for a root
        std::vector<uint16_t> subtreeEnd;   // subtree of skel b = [b, subtreeEnd[b])
        std::vector<uint16_t> skinToSkel;   // skinToSkel[skin], INVALID_BONE_IDX if unused; size MAX_SKINNED_BONES
        bool valid = false;
    };

    // valid == false when the skeleton is empty or is not in parent-before-child order.
    BonePartitionContext BuildBonePartitionContext(const FlatSkeleton& skeleton);

    // Lowest common ancestor of two skel indices. Works because parent index < child index (DFS pre-order).
    // Bones of different roots (or an invalid index) give 0.
    uint16_t BoneLCA(const BonePartitionContext& ctx, uint16_t a, uint16_t b);

    // Reorders mesh.indices (stable, by the LCA of the bones that influence each triangle) and fills
    // mesh.boneIndexFirst, mesh.boneOwnBounds and mesh.boneAnyBounds. Call before the GPU upload.
    void PartitionMeshByBone(SkinnedMesh& mesh, const BonePartitionContext& ctx);
}

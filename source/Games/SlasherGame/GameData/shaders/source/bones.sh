#ifndef BONES_SH_HEADER_GUARD
#define BONES_SH_HEADER_GUARD

// Skinning palette set by the engine (SkeletalMeshDrawCommand), MAX_SKINNED_BONES = 256 slots. It is split into
// two arrays because bgfx shader binaries store a uniform array's size in 8 bits:
// slots 0..127 in finalBonesMatrices0, slots 128..255 in finalBonesMatrices1.
#define BONES_PER_UNIFORM 128
uniform mat4 finalBonesMatrices0[BONES_PER_UNIFORM];
uniform mat4 finalBonesMatrices1[BONES_PER_UNIFORM];

mat4 GetBoneMatrix(int index)
{
    // Both reads stay in range; the select picks the array the slot lives in.
    mat4 low  = finalBonesMatrices0[min(index, BONES_PER_UNIFORM - 1)];
    mat4 high = finalBonesMatrices1[max(index - BONES_PER_UNIFORM, 0)];
    return index < BONES_PER_UNIFORM ? low : high;
}

#endif // BONES_SH_HEADER_GUARD

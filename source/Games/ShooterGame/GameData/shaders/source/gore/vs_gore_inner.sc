$input a_position, a_normal, a_texcoord0, a_texcoord2, a_indices, a_weight, a_color0, a_tangent
$output v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_goreBind

#include <../bgfx_shader.sh>

uniform mat4 projection;
uniform mat4 view;
uniform mat4 world;
uniform vec4 viewmodelScaleFactor;
uniform vec4 isViewmodel;
uniform vec4 u_goreLayer;   // x layer, y inset fraction, z 1 = anatomy

#define MAX_BONES 128
uniform mat4 finalBonesMatrices[MAX_BONES];

mat4 GetBoneTransforms(vec4 indices, vec4 weights)
{
    float sum = weights.x + weights.y + weights.z + weights.w;
    if (sum < 0.01)
        return mat4(1.0, 0.0, 0.0, 0.0,  0.0, 1.0, 0.0, 0.0,  0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 0.0, 1.0);
    return
        finalBonesMatrices[int(indices.x)] * (weights.x / sum) +
        finalBonesMatrices[int(indices.y)] * (weights.y / sum) +
        finalBonesMatrices[int(indices.z)] * (weights.z / sum) +
        finalBonesMatrices[int(indices.w)] * (weights.w / sum);
}

void main()
{
    bool anatomy = u_goreLayer.z > 0.5;

    // Shell: a_normal is the INWARD direction, a_tangent the real normal, a_texcoord2.x the max inset (metres).
    // Anatomy: a_normal is the real normal and nothing moves.
    vec3 bindPos = anatomy ? a_position : a_position + a_normal * (a_texcoord2.x * u_goreLayer.y);
    vec3 bindNormal = anatomy ? a_normal : a_tangent;

    mat4 boneTrans      = GetBoneTransforms(a_indices, a_weight);
    mat4 vertWorldTrans = mul(world, boneTrans);

    vec4 worldPos = mul(vertWorldTrans, vec4(bindPos, 1.0));
    v_world = worldPos;

    vec4 clipPos = mul(projection, mul(view, worldPos));
    if (isViewmodel.x > 0.5)
        clipPos.z *= 0.2 * viewmodelScaleFactor.x;
    gl_Position = clipPos;

    v_normal    = normalize(mul(vertWorldTrans, vec4(bindNormal, 0.0)).xyz);
    v_color0    = a_color0;                    // raw: anatomy keeps its material id in .r
    v_texcoord0 = a_texcoord0;
    v_texcoord1 = a_texcoord2;
    v_goreBind  = vec4(bindPos, anatomy ? 1.0 : a_texcoord2.x);   // w: max inset (shells) - 0 means "no flesh here"
}

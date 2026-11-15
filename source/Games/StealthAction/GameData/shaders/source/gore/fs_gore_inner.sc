$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_goreBind

#include <../bgfx_shader.sh>

SAMPLER2D(u_texture,         0);
SAMPLER2D(u_textureEmissive, 1);

#include "../lighting_common.sh"
// Sampler slots are parsed from this file's own text by the engine (includes are not scanned), so they are declared here.
SAMPLER2D(u_goreFlesh,       2);
SAMPLER2D(u_goreEdgeNoise,   4);
SAMPLER2D(u_goreBlood,       5);
SAMPLER2D(u_goreDecal,       8);
SAMPLER2D(u_goreBone,        6);
SAMPLER2D(u_goreGuts,        7);
#include "gore_common.sh"
#include "gore_flesh.sh"

void main()
{
    bool anatomy = u_goreLayer.z > 0.5;
    bool cavity = u_goreLayer.w > 1.5;                       // the closing surface of a wound
    bool cap = u_goreLayer.w > 0.5 && !cavity;
    if (!anatomy && v_goreBind.w < 0.004) discard;          // a thin sheet has no flesh behind it

    float rim = 0.0; float soak = 0.0; float clot = 0.0; float fresh = 0.0;
    bool anyBits = (u_goreRangeMask.x + u_goreRangeMask.y) > 0.0;
    if (cavity) { if (goreCavityRemoved(v_goreBind.xyz, v_color0.r)) discard; }
    else if (anyBits && goreSurface(v_goreBind.xyz, rim, soak, clot, fresh)) discard;

    vec3 P = v_world.xyz;
    vec3 viewDir = normalize(u_goreCameraPos.xyz - P);
    vec3 N = normalize(v_normal);
    float inside = 0.0;                                      // 1: we look at the inner side of this surface
    if (dot(N, viewDir) < 0.0) { N = -N; inside = 1.0; }

    vec3 bind = v_goreBind.xyz;
    vec3 albedo;
    float scatter = 1.0;

    if (cavity)
    {
        // The inside of a hole: wet meat that is pale fat under the skin and gets redder and darker with depth (v_color0.g).
        float deep = v_color0.g;
        vec3 meat = goreTriplanar(bind, N, u_goreParams.z);
        vec3 tint = mix(vec3(1.15, 0.80, 0.70), vec3(1.0, 0.62, 0.58), smoothstep(0.0, 0.35, deep));
        tint = mix(tint, vec3(0.62, 0.36, 0.34), smoothstep(0.4, 1.0, deep));
        float film = texture2DLod(u_goreBlood, bind.xy * 7.0 + bind.z * 2.0, 0.0).r;
        albedo = mix(meat * tint, u_goreBloodColor.rgb * 1.1, clamp(0.40 + film * 0.25, 0.0, 0.7));
        scatter = 0.5;
    }
    else if (cap)
    {
        // The flat face of a blade cut. v_color0.rgb = the tint of this ring, v_texcoord1.y = material: 0 flesh, 1 bone and
        // marrow, 2 organ (gut wall).
        float isOrgan = step(1.5, v_texcoord1.y);
        float isBone = step(0.5, v_texcoord1.y) * (1.0 - isOrgan);
        vec3 fleshC = texture2DLod(u_goreFlesh, v_texcoord0, 0.0).rgb;
        vec3 boneC  = texture2DLod(u_goreBone, v_texcoord0 * 3.0, 0.0).rgb;
        vec3 gutC   = texture2DLod(u_goreGuts, v_texcoord0 * 2.0, 0.0).rgb;
        albedo = mix(mix(fleshC, boneC, isBone), gutC * 1.15, isOrgan) * v_color0.rgb;
        float film = texture2DLod(u_goreBlood, v_texcoord0 * 1.7, 0.0).r;      // a thin film of blood over the slice
        albedo = mix(albedo, u_goreBloodColor.rgb * 1.3, clamp(0.12 + film * 0.4, 0.0, 0.6) * (1.0 - 0.6 * isBone - 0.4 * isOrgan));
        // A flat face: the rings read through their colours and textures (no bump, no highlight).
        scatter = mix(mix(0.45, 0.25, isBone), 0.30, isOrgan);
    }
    else if (anatomy)
    {
        vec3 boneC = texture2DLod(u_goreBone, v_texcoord0, 0.0).rgb;
        vec3 gutC  = texture2DLod(u_goreGuts, v_texcoord0, 0.0).rgb;
        float isGuts = step(0.5, v_color0.r);
        // Bone: off-white with blood in the pores and a lot of blood near a break.
        float stain = texture2DLod(u_goreBlood, v_texcoord0 * 1.7 + bind.xy * 3.0, 0.0).r;
        vec3 bone = mix(boneC * vec3(1.0, 0.93, 0.82), u_goreBloodColor.rgb * 0.9, clamp(stain * 0.55 + rim * 0.8, 0.0, 0.95));
        albedo = mix(bone, gutC * 1.15, isGuts) * v_color0.g;
        scatter = mix(0.55, 1.0, isGuts);
    }
    else
    {
        float layer = u_goreLayer.x;
        albedo = goreTriplanar(bind + vec3(layer * 0.13, layer * 0.07, 0.0), N, u_goreParams.z);
        // Layer 1: pale fat and torn skin. Layer 2: red muscle. Layer 3: dark.
        vec3 tint = layer < 1.5 ? vec3(1.15, 0.80, 0.70) : (layer < 2.5 ? vec3(1.0, 0.62, 0.58) : vec3(0.62, 0.36, 0.34));
        albedo *= tint;
        albedo = mix(albedo, u_goreBloodColor.rgb * 1.1, rim * 0.7);          // blood wells up at every torn edge
    }

    albedo *= mix(1.0, 0.30, inside);                        // the inside of a wall is in shadow
    albedo *= 0.75;                                          // raw flesh is darker than lit skin

    if (u_goreParams.w > 0.5) { float l = dot(albedo, vec3(0.3, 0.59, 0.11)); albedo = vec3(l, l, l) * 0.4; }   // reduced / minimal gore

    vec3 finalColor = goreShade(albedo, N, P, viewDir, scatter);
    gl_FragColor = ApplyFog(vec4(finalColor, 1.0), P);
}

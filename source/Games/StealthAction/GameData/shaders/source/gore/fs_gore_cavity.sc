$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent, v_goreBind
#include <../bgfx_shader.sh>
SAMPLER2D(u_texture, 0);
SAMPLER2D(u_textureEmissive, 1);
#include "../lighting_common.sh"
// Sampler slots are parsed from this file's own text by the engine (includes are not scanned), so they are declared here.
SAMPLER2D(u_goreFlesh,       2);
SAMPLER2D(u_goreFleshNormal, 3);
SAMPLER2D(u_goreEdgeNoise,   4);
SAMPLER2D(u_goreBlood,       5);
#include "gore_common.sh"
#include "gore_flesh.sh"

// The inside of a wound: a skinned bowl built against the model faces (GoreWoundMesh.cpp). It is plain geometry, so it
// bends with the skin and follows the ragdoll. v_texcoord1.x = lateral fraction (1 at the rim, 0 in the middle),
// v_texcoord1.y = depth fraction (0 at the rim, 1 at the bottom). v_texcoord0 = planar uv of the bowl.

void main()
{
    vec3 P = v_world.xyz;
    vec3 viewDir = normalize(u_goreCameraPos.xyz - P);

    vec3 N = normalize(v_normal);
    if (dot(N, viewDir) < 0.0) N = -N;                       // the bowl is seen from its open side
    vec3 T = normalize(v_tangent - N * dot(N, v_tangent));
    vec3 B = cross(N, T);

    float f = clamp(v_texcoord1.x, 0.0, 1.0);
    float depth = clamp(v_texcoord1.y, 0.0, 1.0);
    vec2 uv = v_texcoord0 * 2.4;

    float jitter = texture2DLod(u_goreEdgeNoise, v_texcoord0 * 1.7, 0.0).r - 0.5;

    vec3 albedo = texture2DLod(u_goreFlesh, uv + jitter * 0.15, 0.0).rgb;
    vec3 nm = texture2DLod(u_goreFleshNormal, uv * 1.3, 0.0).xyz * 2.0 - 1.0;
    N = normalize(N + (T * nm.x + B * nm.y) * 0.8);

    // Torn skin lip at the edge, muscle further in, darker and wetter towards the bottom, blood pooled low.
    float lip = smoothstep(0.80, 0.96, f);
    albedo = mix(albedo, vec3(0.62, 0.40, 0.34), lip * 0.5);
    float ao = mix(0.28, 1.0, f * f * (3.0 - 2.0 * f));
    albedo *= ao;
    float pool = (1.0 - f) * (0.35 + 0.35 * texture2DLod(u_goreBlood, uv * 0.8, 0.0).r);
    albedo = mix(albedo, u_goreBloodColor.rgb * 1.2, pool);
    albedo *= 1.0 - 0.25 * depth;

    if (u_goreParams.w > 0.5) { float l = dot(albedo, vec3(0.3, 0.59, 0.11)); albedo = vec3(l, l, l) * 0.4; }   // reduced/minimal

    vec3 finalColor = goreWetShade(albedo, N, P, viewDir, 1.0, mix(1.0, 0.6, lip));
    gl_FragColor = ApplyFog(vec4(finalColor, 1.0), P);
}

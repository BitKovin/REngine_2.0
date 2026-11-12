$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent, v_goreBind
#include <../bgfx_shader.sh>
SAMPLER2D(u_texture, 0);
SAMPLER2D(u_textureEmissive, 1);
#include "../lighting_common.sh"
#include "gore_common.sh"

uniform vec4 u_goreShell;   // x > 0.5: shell submesh (cloth back side only)

vec3 goreTriplanar(vec3 p, vec3 n, float scale)
{
    vec3 w = abs(n); w = w / (w.x + w.y + w.z + 0.0001);
    return texture2D(u_goreFlesh, p.yz * scale).rgb * w.x
         + texture2D(u_goreFlesh, p.xz * scale).rgb * w.y
         + texture2D(u_goreFlesh, p.xy * scale).rgb * w.z;
}

void main()
{
    vec3 P; float kind; float prim; float thickness;
    if (!goreInterior(v_world.xyz, v_goreBind.xyz, P, kind, prim, thickness)) discard;

    vec3 viewDir = normalize(u_goreCameraPos.xyz - P);
    vec3 N; vec3 albedo; float wet = 1.0;
    int i = int(prim + 0.5);

    if (u_goreShell.x > 0.5)
    {
        N = viewDir; albedo = texture2D(u_texture, v_texcoord0).rgb * 0.25;   // dark cloth back side
        wet = 0.0;
    }
    else if (kind < 0.5)                       // cavity wall of wound i
    {
        vec4 r0 = u_goreWoundWorld[i*3]; vec4 r1 = u_goreWoundWorld[i*3+1]; vec4 r2 = u_goreWoundWorld[i*3+2];
        vec3 q = goreWoundLocal(r0, r1, r2, P) / max(u_goreWoundParams[i].z, 0.001);
        N = -normalize(r0.xyz * q.x + r1.xyz * q.y + r2.xyz * q.z);   // inner wall faces the cavity centre
        albedo = goreTriplanar(q * u_goreWoundExtra[i].y, q, u_goreParams.z);
        albedo *= mix(0.35, 1.0, clamp(length(q), 0.0, 1.0));           // darker deeper inside
    }
    else if (kind < 1.5)                       // cap of cut i
    {
        N = u_goreParams.y > 0.5 ? -u_goreCutWorld[i].xyz : u_goreCutWorld[i].xyz;
        vec3 c = u_goreCutCenter[i].xyz;
        vec3 T = u_goreCutFrame[i*2].xyz; vec3 B = u_goreCutFrame[i*2+1].xyz;
        vec2 uv = vec2(dot(P - c, T), dot(P - c, B));
        albedo = texture2D(u_goreFlesh, uv * u_goreParams.z).rgb;
        float fat = 1.0 - smoothstep(0.004, 0.015, thickness);          // ring near the outline
        albedo = mix(albedo, vec3(0.85, 0.72, 0.45), fat * 0.8);
        float boneR = u_goreCutFrame[i*2].w;
        float dc = length(uv);
        if (dc < boneR)        { albedo = dc < boneR * 0.55 ? vec3(0.35, 0.05, 0.04) : vec3(0.92, 0.88, 0.78); wet = 0.4; }
        if (mod(u_goreCutParams[i].w, 2.0) >= 1.0) albedo *= vec3(0.25, 0.18, 0.15);   // charred
    }
    else                                       // regrow front of cut i
    {
        N = normalize(P - u_goreCutRegrowWorld[i].xyz);
        albedo = goreTriplanar(P - u_goreCutCenter[i].xyz, N, u_goreParams.z) * vec3(1.2, 0.8, 0.8);
    }

    if (u_goreParams.w > 0.5) { float l = dot(albedo, vec3(0.3, 0.59, 0.11)); albedo = vec3(l, l, l) * 0.4; }  // reduced/minimal

    vec3 lightDir = normalize(direct_light_dir.xyz);
    vec3 diffuse = CalculateDirectionalDiffuse(N, lightDir);
    vec3 spec = CalculateDirectionalSpecular(N, lightDir, viewDir) * (0.5 + 2.0 * wet);
    vec3 pd = vec3(0.0, 0.0, 0.0); vec3 ps = vec3(0.0, 0.0, 0.0);
    CalculatePointLights(N, P, viewDir, pd, ps);
    vec3 finalColor = albedo * (diffuse + pd) + (spec + ps) * wet;

    gl_FragColor = ApplyFog(vec4(finalColor, 1.0), P);
    gl_FragDepth = goreDepth(P) - 0.000001;    // tiny bias toward the camera (matches the depth pre-pass value)
}

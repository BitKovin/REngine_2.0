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

uniform vec4 u_goreShell;   // x > 0.5: shell submesh (cloth back side only)

void main()
{
    vec3 P; float kind; float prim; float thickness;
    if (!goreInterior(v_world.xyz, v_goreBind.xyz, P, kind, prim, thickness)) discard;

    vec3 viewDir = normalize(u_goreCameraPos.xyz - P);
    vec3 N; vec3 albedo; float wet = 1.0; float gloss = 1.0;
    int i = int(prim + 0.5);

    if (u_goreShell.x > 0.5)
    {
        N = viewDir; albedo = texture2DLod(u_texture, v_texcoord0, 0.0).rgb * 0.25;   // dark cloth back side
        wet = 0.0;
    }
    else if (kind < 0.5)                       // cavity wall of wound i
    {
        vec4 r0 = u_goreWoundWorld[i*3]; vec4 r1 = u_goreWoundWorld[i*3+1]; vec4 r2 = u_goreWoundWorld[i*3+2];
        vec3 q = goreWoundLocal(r0, r1, r2, P) / max(u_goreWoundParams[i].z, 0.001);
        N = -normalize(r0.xyz * q.x + r1.xyz * q.y + r2.xyz * q.z);   // inner wall faces the cavity centre
        vec3 tp = q * 0.6 + u_goreWoundParams[i].w;                    // the texture spans the cavity
        albedo = goreTriplanar(tp, q, 1.0);
        N = goreBumpTriplanar(N, tp, 1.0, 0.9);

        float depthK = clamp(length(q), 0.0, 1.0);
        albedo *= mix(0.40, 1.0, depthK * depthK);                    // dark and saturated deep inside
        albedo = mix(albedo, u_goreBloodColor.rgb * 1.1, (1.0 - depthK) * 0.35);   // blood pooled at the bottom
        gloss = 1.0;
    }
    else if (kind < 1.5)                       // cap of cut i: skin edge, fat, muscle, bone and marrow
    {
        vec3 nPlane = u_goreParams.y > 0.5 ? -u_goreCutWorld[i].xyz : u_goreCutWorld[i].xyz;
        vec3 c = u_goreCutCenter[i].xyz;
        vec3 T = u_goreCutFrame[i*2].xyz; vec3 B = u_goreCutFrame[i*2+1].xyz;
        float R = max(u_goreCutCenter[i].w, 0.012);                    // outline radius of the cross-section
        vec2 uv = vec2(dot(P - c, T), dot(P - c, B));
        float dc = length(uv);

        float jitter = texture2DLod(u_goreEdgeNoise, uv / R * 0.9, 0.0).r - 0.5;
        float jitter2 = texture2DLod(u_goreEdgeNoise, uv / R * 3.1 + 0.37, 0.0).r - 0.5;
        float t = dc / R + jitter * 0.10 + jitter2 * 0.03;             // 0 centre .. 1 outline

        // Muscle, planar projection, bundles swirl a little around the bone.
        vec2 muv = uv / R * 0.85 + jitter * 0.12;
        albedo = texture2DLod(u_goreFlesh, muv, 0.0).rgb;
        vec3 nm = texture2DLod(u_goreFleshNormal, muv, 0.0).xyz * 2.0 - 1.0;
        N = normalize(nPlane + (T * nm.x + B * nm.y) * 0.45 + normalize(T * uv.x + B * uv.y + nPlane * 0.0001) * (t * t) * 0.22);

        // Tissue layers from the outside in: dermis edge, fat, muscle.
        float skin = smoothstep(0.90, 0.955, t);
        float fat  = smoothstep(0.76, 0.84, t) * (1.0 - smoothstep(0.92, 0.97, t));
        vec3 fatCol = vec3(0.86, 0.66, 0.38) * (0.85 + 0.3 * texture2DLod(u_goreFlesh, muv * 2.0, 0.0).g);
        albedo = mix(albedo, fatCol, fat * 0.85);
        albedo = mix(albedo, vec3(0.72, 0.46, 0.40), skin * 0.9);       // torn dermis, pale pink
        float ao = 1.0 - skin * 0.45;                                    // the rim is shadowed by the skin

        float boneR = u_goreCutFrame[i*2].w;
        float tb = dc / max(boneR, 0.0001) + jitter * 0.18;
        float cortical = smoothstep(1.02, 0.94, tb) * smoothstep(0.50, 0.62, tb);
        float marrow = smoothstep(0.58, 0.50, tb);
        float haloBand = smoothstep(1.30, 1.0, tb) * smoothstep(0.98, 1.05, tb);    // periosteum / bruised ring
        vec3 boneCol = vec3(0.93, 0.89, 0.78) * (0.8 + 0.3 * texture2DLod(u_goreEdgeNoise, uv * 90.0 / R, 0.0).r);
        vec3 marrowCol = vec3(0.50, 0.10, 0.08) * (0.7 + 0.6 * texture2DLod(u_goreBlood, uv * 60.0 / R, 0.0).r);
        albedo = mix(albedo, albedo * vec3(0.6, 0.7, 0.7), haloBand * 0.6);
        albedo = mix(albedo, boneCol, cortical);
        albedo = mix(albedo, marrowCol, marrow);
        N = normalize(mix(N, nPlane, cortical * 0.8));
        wet = mix(1.0, 0.45, cortical);
        gloss = mix(1.0, 0.5, cortical);

        // A blood film over the whole cut, thicker in the lower parts.
        float film = texture2DLod(u_goreBlood, uv / R * 1.7, 0.0).r;
        albedo = mix(albedo, u_goreBloodColor.rgb * 1.5, (1.0 - cortical) * clamp(0.18 + film * 0.5, 0.0, 0.7));
        albedo *= ao;

        if (mod(u_goreCutParams[i].w, 2.0) >= 1.0) albedo *= vec3(0.25, 0.18, 0.15);   // charred
    }
    else                                       // regrow front of cut i
    {
        N = normalize(P - u_goreCutRegrowWorld[i].xyz);
        albedo = goreTriplanar(P - u_goreCutCenter[i].xyz, N, u_goreParams.z) * vec3(1.2, 0.8, 0.8);
        N = goreBumpTriplanar(N, P - u_goreCutCenter[i].xyz, u_goreParams.z, 0.6);
    }

    if (u_goreParams.w > 0.5) { float l = dot(albedo, vec3(0.3, 0.59, 0.11)); albedo = vec3(l, l, l) * 0.4; }  // reduced/minimal

    vec3 finalColor = goreWetShade(albedo, N, P, viewDir, wet, gloss);

    gl_FragColor = ApplyFog(vec4(finalColor, 1.0), P);

    gl_FragDepth = goreDepth(P) - 0.000001;    // tiny bias toward the camera (matches the depth pre-pass value)
}

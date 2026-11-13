$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent, v_goreBind

#include <../bgfx_shader.sh>

SAMPLER2D(u_texture,         0);
SAMPLER2D(u_textureEmissive, 1);

#include "../lighting_common.sh"
// Sampler slots are parsed from this file's own text by the engine (includes are not scanned), so they are declared here.
SAMPLER2D(u_goreFlesh,       2);
SAMPLER2D(u_goreFleshNormal, 3);
SAMPLER2D(u_goreEdgeNoise,   4);
SAMPLER2D(u_goreBlood,       5);
#include "gore_common.sh"

// ==========================================
// 3. MAIN
// ==========================================

void main()
{
    float goreRim = 0.0; float soak = 0.0; float fresh = 0.0;
    bool anyBits = (u_goreRangeMask.x + u_goreRangeMask.y + u_goreRangeMask.z) > 0.0;
    if (anyBits && goreSurface(v_goreBind.xyz, goreRim, soak, fresh)) discard;

    vec4  texColor = texture2D(u_texture, v_texcoord0) * v_color0 * modelColor;
    vec3  color    = texColor.rgb;
    float alpha    = texColor.a;

    if (masked.x > 0.0)
    {
        if (alpha < 0.5)
            discard;
        alpha = 1.0;
    }

    float blood = u_goreParams.w > 1.5 ? 0.0 : max(goreRim, soak);
    color = mix(color, u_goreBloodColor.rgb * (0.6 + 0.4 * soak), blood);
    color = mix(color, u_goreBloodColor.rgb * 1.4, fresh * 0.8);     // raw new skin while regrowing

    vec3 N = normalize(v_normal);
    vec3 T = normalize(v_tangent);
    vec3 B = normalize(v_bitangent);

    if (flipedNormals.x > 0.5)
    {
        N = -N; T = -T; B = -B;
    }

    T = normalize(T - N * dot(N, T));
    float handedness = sign(dot(cross(N, T), B));
    B = normalize(cross(N, T) * handedness);

    mat3 TBN = mtxFromCols(T, B, N);

    vec3 mapN = vec3(0.5, 0.5, 1.0); // Assume flat normal map for default
    mapN = mapN * 2.0 - 1.0;
    mapN = normalize(mapN);

    vec3 normal = normalize(mul(TBN, mapN));

    if (gl_FrontFacing)
        normal = -normal;

    if(length(T) < 0.1)
        normal = N;

    vec3 lightDir = normalize(direct_light_dir.xyz);
    vec3 viewDir  = normalize(cameraPosition.xyz - v_world.xyz);

    // Accumulate Lighting
    vec3 dirDiffuse  = CalculateDirectionalDiffuse(normal, lightDir);


    float luma = pow(dot(color, vec3(0.299, 0.587, 0.114)), 1.5);

    float specularFactor = mix(luma * 1.0f, 0.2, 0.2) * 2.0 * (1.0 + 2.5 * max(blood, fresh));

    vec3 dirSpecular = CalculateDirectionalSpecular(normal, lightDir, viewDir) * specularFactor;
    
    vec3 pointDiffuse  = vec3(0.0,0.0,0.0);
    vec3 pointSpecular = vec3(0.0,0.0,0.0);
    CalculatePointLights(normal, v_world.xyz, viewDir, pointDiffuse, pointSpecular);

    pointSpecular *= specularFactor;

    vec3 emissive = texture2D(u_textureEmissive, v_texcoord0).rgb;

    // View-dependent rim light
    vec3 rim = vec3(0,0,0);
    rim = CalculateContourRim(normal, viewDir);

    // Composition (Light Weight Mixing)
    vec3 finalColor =
        color * (dirDiffuse + pointDiffuse) // Albedo modulated by stepped diffuse
        + dirSpecular + pointSpecular       // Additive quantized specular
        + rim                               // Additive camera-attached rim
        + emissive;                         // Self-illumination


    gl_FragColor = ApplyFog(vec4(finalColor, alpha), v_world.xyz);
}
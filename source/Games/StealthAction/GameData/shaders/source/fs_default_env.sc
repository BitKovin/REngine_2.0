$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent

#include <bgfx_shader.sh>

SAMPLER2D(u_texture,         0);
SAMPLER2D(u_textureEmissive, 1);

uniform vec4 cameraPosition;
uniform vec4 masked;
uniform vec4 flipedNormals;

uniform vec4 light_color;
uniform vec4 direct_light_color;
uniform vec4 direct_light_dir;

uniform vec4 fog_start;
uniform vec4 fog_end;
uniform vec4 fog_opacity;
uniform vec4 fog_color;


uniform vec4 modelColor;
uniform vec4 PointLightsNumber;

uniform vec4 isViewmodel;

#ifndef MAX_POINT_LIGHTS
    #define MAX_POINT_LIGHTS 5
#endif

uniform mat4 PointLights[MAX_POINT_LIGHTS];

// ==========================================
// 2. LIGHT ACCUMULATION
// ==========================================

void AccumulatePointLight(int i, vec3 pixelPosition, vec3 normal, inout vec3 outDiffuse)
{
    vec4 col0 = mtxGetColumn(PointLights[i], 0);
    vec4 col1 = mtxGetColumn(PointLights[i], 1);
    vec4 col2 = mtxGetColumn(PointLights[i], 2);

    vec3 lightPos   = col0.xyz;
    float innerCone = col0.w;
    vec3 lightCol   = col1.rgb;
    float radius    = col1.w;
    vec3 lightFwd   = col2.xyz;
    float outerCone = col2.w;

    if (radius <= 0.0) return;

    vec3 lightVector = lightPos - pixelPosition;
    float distanceToLight = length(lightVector);
    if (distanceToLight > radius) return;

    vec3 lightDir = lightVector / distanceToLight;
    float lightDot = dot(-lightDir, normalize(lightFwd));
    float dirFactor = smoothstep(outerCone, innerCone, lightDot);
    if (dirFactor <= 0.001) return;

    float distAtten = max((radius - distanceToLight) / radius, 0.0);

    outDiffuse += lightCol * max(dot(normal, lightDir), 0.0) * distAtten * dirFactor;
}

void CalculatePointLights(vec3 normal, vec3 worldPosition, inout vec3 totalDiffuse)
{
    int numLights = min(MAX_POINT_LIGHTS, int(PointLightsNumber.x));
    for (int i = 0; i < numLights; i++)
    {
        AccumulatePointLight(i, worldPosition, normal, totalDiffuse);
    }
}

// Environment props: the light grid as the editor previews it, ambient + directed * max(0, N.L), with no stylisation
// (no half-lambert wrap, no 0.6 / 0.9 factors, no specular, no rim light).
vec3 CalculateDirectionalDiffuse(vec3 normal, vec3 lightDir)
{
    return light_color.rgb + direct_light_color.rgb * max(dot(normal, lightDir), 0.0);
}

vec4 ApplyFog(vec4 fragColor, vec3 worldPosition)
{
    float fragDistance = distance(worldPosition, cameraPosition.xyz);
    float fogFactor    = (fragDistance - fog_start.x) / (fog_end.x - fog_start.x);
    fogFactor          = clamp(fogFactor, 0.0, 1.0) * fog_opacity.x;
    return vec4(mix(fragColor.rgb, fog_color.rgb, fogFactor), fragColor.a);
}

// ==========================================
// 3. MAIN
// ==========================================

void main()
{
    vec4  texColor = texture2D(u_texture, v_texcoord0) * v_color0 * modelColor;
    vec3  color    = texColor.rgb;
    float alpha    = texColor.a;

    if (masked.x > 0.0)
    {
        if (alpha < 0.5)
            discard;
        alpha = 1.0;
    }

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


    vec3 pointDiffuse = vec3(0.0, 0.0, 0.0);
    CalculatePointLights(normal, v_world.xyz, pointDiffuse);

    vec3 emissive = texture2D(u_textureEmissive, v_texcoord0).rgb;

    vec3 finalColor =
        color * (dirDiffuse + pointDiffuse)
        + emissive;


    gl_FragColor = ApplyFog(vec4(finalColor, alpha), v_world.xyz);
}
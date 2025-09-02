#version 300 es
precision highp float;

in vec2 g_TexCoord;
in vec2 g_LmapCoord;
in vec3 g_normal;
in vec4 g_color;

in vec4 g_world;

uniform vec3 cameraPosition;

//Texture samplers
uniform sampler2D s_bspTexture;
uniform sampler2D s_bspLightmap;
uniform vec3 light_color; 
uniform vec3 direct_light_color; 
uniform vec3 direct_light_dir; 

uniform float fog_start;
uniform float fog_end;
uniform float fog_opacity;
uniform vec3  fog_color;

uniform bool useVertexLight;

//final color
out vec4 FragColor;

vec4 ApplyFog(vec4 fragColor);

void main()
{
    // base color
    vec4 baseTex = texture(s_bspTexture, g_TexCoord);

    // lightmap (RGB)
    vec3 lmSample = texture(s_bspLightmap, g_LmapCoord).rgb;
    vec3 lightmap = lmSample * light_color;

    float vertexLightScale = 4.04;

    // vertex/lightgrid color (RGB) scaled by tunable uniform
    vec3 vertexLight = vec3(1.0);
    
    if(useVertexLight)
    {
        vertexLight = g_color.rgb * vertexLightScale;
    }


    // directional light (per-pixel diffuse-ish)
    vec3 N = normalize(g_normal);
    vec3 L = normalize(direct_light_dir);
    float NdotL = max(dot(N, L), 0.0);

    // keep the shape of your previous bias but computed cleanly
    float dirFactor = clamp(NdotL * 0.7 + 0.3, 0.0, 1.0);
    vec3 directLight = dirFactor * direct_light_color;

    // combine: vertexLight * lightmap are multiplicative on base texture;
    // directLight is added (emissive) so it brightens the result.
    vec3 totalLighting = vertexLight * lightmap + directLight;

    // prevent negative or NaN (safety) and apply to base texture
    totalLighting = max(totalLighting, vec3(0.0));
    vec3 finalRGB = baseTex.rgb * totalLighting;

    FragColor = ApplyFog(vec4(finalRGB, baseTex.a));
}

vec4 ApplyFog(vec4 fragColor)
{
    float fragDistance = distance(g_world.xyz, cameraPosition);

    // Adjust for negative fog_start
    float start = max(fog_start, 0.0); // optional, to avoid weird behavior if you want
    float end = max(fog_end, 0.0001);  // avoid division by zero

    // Compute fog factor
    float fogFactor = (fragDistance - fog_start) / (fog_end - fog_start);

    // Clamp between 0 and 1
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Apply global fog opacity
    fogFactor *= fog_opacity;

    // Blend scene color with fog color
    return vec4(mix(fragColor.rgb, fog_color, fogFactor), fragColor.a);
}

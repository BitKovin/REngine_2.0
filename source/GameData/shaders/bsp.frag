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

//final color
out vec4 FragColor;

vec4 ApplyFog(vec4 fragColor);

void main()
{
    vec4 o_texture  = texture(s_bspTexture,  g_TexCoord);


    float vertexLightComp = 4.04;


    vec4 o_lightmap = texture(s_bspLightmap, g_LmapCoord) * vec4(light_color, 1) * vertexLightComp * g_color;
    o_lightmap.a = 1.0;

    vec3 normal = normalize(g_normal);

    o_lightmap += clamp(dot(normal, normalize(direct_light_dir))*0.7 + 0.3,0.0,1.0) * vec4(direct_light_color,0) * 2.0;

    FragColor = ApplyFog(o_texture * o_lightmap * 1.0);
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

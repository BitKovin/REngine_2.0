#version 300 es
precision highp float;

in vec2 g_TexCoord;
in vec2 g_LmapCoord;
in vec3 g_normal;
in vec4 g_color;

//Texture samplers
uniform sampler2D s_bspTexture;
uniform sampler2D s_bspLightmap;
uniform vec3 light_color; 
uniform vec3 direct_light_color; 
uniform vec3 direct_light_dir; 

//final color
out vec4 FragColor;

void main()
{
    vec4 o_texture  = texture(s_bspTexture,  g_TexCoord);


    float vertexLightComp = 4.04;


    vec4 o_lightmap = texture(s_bspLightmap, g_LmapCoord) * vec4(light_color, 1) * vertexLightComp * g_color;
    o_lightmap.a = 1.0;

    vec3 normal = normalize(g_normal);

    o_lightmap += clamp(dot(normal, normalize(direct_light_dir))*0.7 + 0.3,0.0,1.0) * vec4(direct_light_color,0) ;

    FragColor = o_texture * o_lightmap * 1.0;
}

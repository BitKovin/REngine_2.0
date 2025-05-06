#version 330 core

in vec2 g_TexCoord;
in vec2 g_LmapCoord;

//Texture samplers
uniform sampler2D s_bspTexture;
uniform sampler2D s_bspLightmap;

//final color
out vec4 FragColor;

void main()
{
    vec4 o_texture  = texture(s_bspTexture,  g_TexCoord);
    vec4 o_lightmap = texture(s_bspLightmap, g_LmapCoord);

    FragColor = o_texture * o_lightmap * 3;
}

#version 330 core

layout (location = 0) in vec3 position; //camera
layout (location = 1) in vec2 in_texCoord;
layout (location = 2) in vec2 in_lmapCoord;

out vec2 g_TexCoord;
out vec2 g_LmapCoord;



uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);

    g_TexCoord = vec2(in_texCoord.x, in_texCoord.y);
    g_LmapCoord = vec2(in_lmapCoord.x, in_lmapCoord.y);
}


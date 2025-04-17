#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
out vec4 FragColor;
uniform sampler2D u_texture;  // Changed from "texture" to avoid keyword conflict

void main() {
    FragColor = texture(u_texture, v_texcoord)*v_color;
    //FragColor = vec4(1,1,1,1);
}
#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;


out vec4 FragColor;
uniform sampler2D u_texture;  // Changed from "texture" to avoid keyword conflict


vec3 CalculateDirectionalLight();

void main() {

    vec4 texColor = texture(u_texture, v_texcoord) * v_color;

    if(texColor.a<0.99)
    {
        discard;
        return;
    }
        

    //FragColor = vec4(0,0,0, 1);
}
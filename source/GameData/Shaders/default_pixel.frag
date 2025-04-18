#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
in vec3 v_normal;
in vec3 v_worldPosition;
in vec4 v_clipPosition;

out vec4 FragColor;
uniform sampler2D u_texture;  // Changed from "texture" to avoid keyword conflict

uniform bool is_particle;

uniform vec3 lightDirection;

vec3 CalculateDirectionalLight();

void main() {
    vec4 texColor = texture(u_texture, v_texcoord)*v_color;

    vec3 color = texColor.rgb;
    float alpha = texColor.a;

    if(alpha <0.01)
        discard;

    vec3 ligthColor = CalculateDirectionalLight();

    color*=ligthColor;

    FragColor = vec4(color, alpha);
}

vec3 CalculateDirectionalLight()
{
    float directionPower = 1.0;
    
    if(is_particle == false)
    {
        directionPower = clamp(dot(-v_normal, lightDirection),0.0,1.0);
    }    
    
    return mix(vec3(0.2),vec3(1),directionPower);
}
#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
in vec3 v_normal;

out vec4 FragColor;
uniform sampler2D u_texture; 
uniform sampler2D u_textureEmissive; 

uniform vec3 cameraPosition;

uniform bool is_particle;
uniform bool is_decal;

uniform vec3 light_color; 
uniform vec3 direct_light_color; 
uniform vec3 direct_light_dir; 



vec3 CalculateLight();

void main() {
    vec4 texColor = texture(u_texture, v_texcoord)*v_color;

    vec3 color = texColor.rgb;
    float alpha = texColor.a;

    if(alpha <0.01)
        discard;

    vec3 ligthColor = CalculateLight() + texture(u_textureEmissive, v_texcoord).rgb;

    color *= ligthColor;

    FragColor = vec4(color, alpha);
}

vec3 CalculateLight()
{

    vec3 normal = normalize(v_normal);

    vec3 light = light_color + clamp(dot(normal, normalize(direct_light_dir))*0.8 + 0.2,0.0,1.0) * direct_light_color;

    return light.rgb*2.0;
}
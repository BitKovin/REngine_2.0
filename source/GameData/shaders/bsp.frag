// bgfx shaderc: GLSL fragment shader
$input v_texcoord, v_color, v_normal, v_worldPosition, v_light
$output o_color

#include <bgfx_shader.sh>

// Uniforms
SAMPLER2D(u_texture, 0);
SAMPLER2D(u_textureEmissive, 1);

uniform vec4 u_cameraPosition;   // xyz = camera position
uniform vec4 u_light_color;      // rgb = ambient/point light color
uniform vec4 u_direct_light_color; // rgb = directional light color
uniform vec4 u_direct_light_dir; // xyz = directional light direction
uniform vec4 u_fog_params;       // x=fog_start, y=fog_end, z=fog_opacity
uniform vec4 u_fog_color;        // rgb=fog color

vec3 CalculateLight()
{
    vec3 normal = normalize(v_normal);
    vec3 light = u_light_color.rgb +
                 clamp(dot(normal, normalize(u_direct_light_dir.xyz))*0.8 + 0.2, 0.0, 1.0)
                 * u_direct_light_color.rgb;

    light *= 2.0;
    light += v_light;
    return light;
}

vec4 ApplyFog(vec4 fragColor)
{
    float fragDistance = distance(v_worldPosition, u_cameraPosition.xyz);

    float fogStart = max(u_fog_params.x, 0.0);
    float fogEnd   = max(u_fog_params.y, 0.0001);

    float fogFactor = (fragDistance - fogStart) / (fogEnd - fogStart);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    fogFactor *= u_fog_params.z; // fog opacity
    return vec4(mix(fragColor.rgb, u_fog_color.rgb, fogFactor), fragColor.a);
}

void main()
{
    vec4 texColor = texture2D(u_texture, v_texcoord) * v_color;
    if (texColor.a < 0.01) discard;

    vec3 lightColor = CalculateLight() +
                      texture2D(u_textureEmissive, v_texcoord).rgb;

    vec3 color = texColor.rgb * lightColor;
    o_color = ApplyFog(vec4(color, texColor.a));
}

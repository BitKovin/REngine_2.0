#version 300 es
precision highp float;

in vec2 v_texcoord;
in vec4 v_color;
in vec4 v_clipPosition;  // particle’s clip position

out vec4 FragColor;

uniform sampler2D u_texture;
uniform sampler2D depthTexture;
float LinearizeDepth(float depth) 
{

    float nearPlane = 0.05; // near plane distance
    float farPlane = 3000.0; // far plane distance


    // depth is [0,1] sampled from depth buffer
    float z = depth * 2.0 - 1.0; // back to NDC [-1,1]
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

void main() {
    vec4 texColor = texture(u_texture, v_texcoord) * v_color;
    if (texColor.a < 0.001) discard;

    // Screen UV
    vec2 screenUV = (v_clipPosition.xy / v_clipPosition.w) * 0.5 + 0.5;

    // Fragment depth (from clip pos)
    float fragDepthNDC = (v_clipPosition.z / v_clipPosition.w) * 0.5 + 0.5;
    float fragDepthLinear = LinearizeDepth(fragDepthNDC);

    // Scene depth (from depth buffer)
    float sceneDepth = texture(depthTexture, screenUV).r;
    float sceneDepthLinear = LinearizeDepth(sceneDepth);

    // Difference in world units (eye-space z)
    float diff = sceneDepthLinear - fragDepthLinear;

    float fadeDistance = 0.1; // fade distance

    // Fade smoothly if close to geometry
    float fade = clamp(diff / fadeDistance, 0.0, 1.0);

    FragColor = vec4(texColor.rgb, texColor.a * fade);
}

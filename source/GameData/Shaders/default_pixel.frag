#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
in vec3 v_normal;
in vec3 v_worldPosition;
in vec4 v_clipPosition;

in vec4 v_shadowCoords;

out vec4 FragColor;
uniform sampler2D u_texture;  // Changed from "texture" to avoid keyword conflict

uniform vec3 cameraPosition;

uniform highp sampler2DShadow shadowMap;
uniform int shadowMapSize;  // width, height of shadow map
uniform vec3 lightDirection;   
uniform float shadowDistance;

uniform bool is_particle;



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

float GetDirectionalShadow()
{
    // 1) Project into [0,1] clip space
    vec4 sc  = v_shadowCoords / v_shadowCoords.w;
    vec3 pc  = sc.xyz * 0.5 + 0.5;

    pc *= vec3(0.5,0.5,1);

    float worldBias = -0.05 / 20.0 * shadowDistance * 512.0 / float(shadowMapSize);
    const int pcfRadius = 2;

    float shadowCameraFar = shadowDistance;
    const float shadowCameraNear = -100.0;

// for a perspective shadow map:
float linearDepth   = -v_shadowCoords.z / v_shadowCoords.w;  
float depthScale    = (shadowCameraFar - shadowCameraNear)
                    / (shadowCameraFar * shadowCameraNear);
float bias          = worldBias * depthScale / v_shadowCoords.w;

    // optional: if outside the map, consider fully lit
    if (pc.x < 0.0 || pc.x > 1.0 ||
        pc.y < 0.0 || pc.y > 1.0 ||
        pc.z > 1.0)
        return 1.0;

    // 2) cheap distance‐fade for far objects
    if (distance(cameraPosition, v_worldPosition) > shadowDistance)
        return 1.0;

    // 4) compute one texel in normalized coords, then scale by your desired radius
    vec2 texelSize = 0.5 / vec2(shadowMapSize,shadowMapSize);
    vec2 radiusUV = texelSize;

    // 5) PCF: average 3×3 samples
    float sum = 0.0;
    float n = 0.0;
    for (int xo = -1 * pcfRadius; xo <= 1 * pcfRadius; ++xo) 
    {
        for (int yo = -1 * pcfRadius; yo <= 1 * pcfRadius; ++yo) 
        {
            n++;
            vec2 offs = vec2(xo, yo) * radiusUV;
            sum += texture(shadowMap,
                           vec3(pc.xy + offs,
                                pc.z - bias));
        }
    }
    return sum / n;
}

vec3 CalculateDirectionalLight()
{
    float directionPower = 1.0;
    
    if(is_particle == false)
    {
        directionPower = clamp(dot(-v_normal, lightDirection),0.0,1.0);
    }    
    
    directionPower*= GetDirectionalShadow();

    return mix(vec3(0.2),vec3(1),directionPower);
}
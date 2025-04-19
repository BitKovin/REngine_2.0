#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
in vec3 v_normal;
in vec3 v_worldPosition;
in vec4 v_clipPosition;

in vec4 v_shadowCoords1;
in vec4 v_shadowCoords2;
in vec4 v_shadowCoords3;
in vec4 v_shadowCoords4;

out vec4 FragColor;
uniform sampler2D u_texture;  // Changed from "texture" to avoid keyword conflict

uniform vec3 cameraPosition;

uniform highp sampler2DShadow shadowMap;
uniform int shadowMapSize;  // width, height of shadow map
uniform vec3 lightDirection;   
uniform float shadowDistance1;
uniform float shadowDistance2;
uniform float shadowDistance3;
uniform float shadowDistance4;

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

float GetDirectionalShadow(vec2 mapOffset, float shadowDistance, vec4 shadowCoords, int pcfRadius)
{
    // 1) Project into [0,1] clip space
    vec4 sc  = shadowCoords / shadowCoords.w;
    vec3 pc  = sc.xyz * 0.5 + 0.5;

    pc *= vec3(0.5,0.5,1);

    pc+= vec3(mapOffset*vec2(0.5),0);

float worldBias = -0.7* (shadowDistance * 2.0 / float(shadowMapSize));
    float shadowCameraFar = shadowDistance;
    const float shadowCameraNear = -100.0;

// for a perspective shadow map:
float linearDepth   = -shadowCoords.z / shadowCoords.w;  
float depthScale    = (shadowCameraFar - shadowCameraNear)
                    / (shadowCameraFar * shadowCameraNear);
float bias          = worldBias * depthScale / shadowCoords.w;

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
    // 1) Basic N·L
    float directionPower = is_particle
        ? 1.0
        : clamp(dot(-v_normal, lightDirection), 0.0, 1.0);

    // 2) Distance from the camera
    float dist = distance(cameraPosition, v_worldPosition);

    // 3) Cascade parameters
    float cascadeDist[4]   = float[]( shadowDistance1*1.0,
                                      shadowDistance1+shadowDistance2*1.0,
                                      shadowDistance1+shadowDistance2+shadowDistance3*1.0,
                                      shadowDistance1+shadowDistance2+shadowDistance3+shadowDistance4*1.0 );
    float blendR[4]        = float[]( 2.0,
                                      5.0,
                                      10.0,
                                      30.0 );
    vec2  cascadeOff[4]    = vec2[]( vec2(0,0),
                                      vec2(1,0),
                                      vec2(0,1),
                                      vec2(1,1) );
    vec4  cascadeCoords[4] = vec4[]( v_shadowCoords1,
                                      v_shadowCoords2,
                                      v_shadowCoords3,
                                      v_shadowCoords4 );
    int   cascadePCF[4]   = int[]( 2, 1, 1, 1 );

    float shadow = 1.0;

    // 4) Check blend zones between cascades 0–2 and blend if inside
    bool blended = false;
    for (int i = 0; i < 3; ++i) {
        float fadeStart = cascadeDist[i] - blendR[i];
        float fadeEnd   = cascadeDist[i];

        if (dist >= fadeStart && dist <= fadeEnd) {
            float blendF = smoothstep(fadeStart, fadeEnd, dist);

            float sh0 = GetDirectionalShadow(
                cascadeOff[i],
                cascadeDist[i],
                cascadeCoords[i],
                cascadePCF[i]
            );
            float sh1 = GetDirectionalShadow(
                cascadeOff[i + 1],
                cascadeDist[i + 1],
                cascadeCoords[i + 1],
                cascadePCF[i + 1]
            );

            shadow = mix(sh0, sh1, blendF);
            blended = true;
            break;
        }
    }

    // 5) If not blended, just find the appropriate cascade directly
    if (!blended) {
        int idx = 0;
        for (int i = 0; i < 4; ++i) {
            if (dist <= cascadeDist[i]) {
                idx = i;
                break;
            }
        }

        shadow = GetDirectionalShadow(
            cascadeOff[idx],
            cascadeDist[idx],
            cascadeCoords[idx],
            cascadePCF[idx]
        );
    }

    // 6) Final light
    directionPower *= shadow;
    return mix(vec3(0.2), vec3(1.0), directionPower);
}


$input v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_normal, v_color0, v_world

#include <bgfx_shader.sh>

SAMPLER2D(s_bspTexture,  0);
SAMPLER2D(s_bspLightmap, 1);
SAMPLER2D(s_bspLightmap1,2);
SAMPLER2D(s_bspLightmap2,3);
SAMPLER2D(s_bspLightmap3,4);
// rBSP v2: slots 4..7 (they share the 4th lightmap UV set, v_texcoord4)
SAMPLER2D(s_bspLightmap4,5);
SAMPLER2D(s_bspLightmap5,6);
SAMPLER2D(s_bspLightmap6,7);
SAMPLER2D(s_bspLightmap7,8);

uniform vec4 cameraPosition;
uniform vec4 light_color;
uniform vec4 direct_light_color;
uniform vec4 direct_light_dir;
uniform vec4 fog_start;
uniform vec4 fog_end;
uniform vec4 fog_opacity;
uniform vec4 fog_color;
uniform vec4 useVertexLight;
uniform vec4 PointLightsNumber;
uniform vec4 brightness;
uniform vec4 numLightmapSlots;
uniform vec4 lmStyleColor0;
uniform vec4 lmStyleColor1;
uniform vec4 lmStyleColor2;
uniform vec4 lmStyleColor3;
uniform vec4 lmStyleColor4;
uniform vec4 lmStyleColor5;
uniform vec4 lmStyleColor6;
uniform vec4 lmStyleColor7;
uniform vec4 isRBSP;

#ifndef MAX_POINT_LIGHTS
    #define MAX_POINT_LIGHTS 5
#endif
uniform mat4 PointLights[MAX_POINT_LIGHTS];

vec3 CalculateSimplePointLight(int i, vec3 pixelPosition, vec3 normal)
{
    vec4 col0 = mtxGetColumn(PointLights[i], 0);
    vec4 col1 = mtxGetColumn(PointLights[i], 1);
    vec4 col2 = mtxGetColumn(PointLights[i], 2);
    vec3 lightPos = col0.xyz; float innerCone = col0.w;
    vec3 lightCol = col1.rgb; float radius    = col1.w;
    vec3 lightFwd = col2.xyz; float outerCone = col2.w;
    if (radius <= 0.0) return vec3_splat(0.0);
    vec3  lightVector     = lightPos - pixelPosition;
    float distanceToLight = length(lightVector);
    if (distanceToLight > radius) return vec3_splat(0.0);
    vec3  lightDir  = lightVector / distanceToLight;
    float dirFactor = smoothstep(outerCone, innerCone, dot(-lightDir, normalize(lightFwd)));
    if (dirFactor <= 0.001) return vec3_splat(0.0);
    vec3 n = normalize(normal);
    if (dot(n, lightDir) < 0.0) return vec3_splat(0.0);
    float dist    = max((radius - distanceToLight) / radius, 0.0);
    float intense = dist * max(dot(n, lightDir), 0.0);
    return lightCol * intense * dirFactor;
}

vec3 LinearFromSrgb(vec3 c) { return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3_splat(2.4)), step(vec3_splat(0.04045), c)); }
vec3 SrgbFromLinear(vec3 c) { c = clamp(c, 0.0, 1.0); return mix(c * 12.92, 1.055 * pow(c, vec3_splat(1.0 / 2.4)) - 0.055, step(vec3_splat(0.0031308), c)); }

// rBSP v2 (level editor): bicubic B-spline lookup from 4 bilinear fetches, as the editor's views do. Bilinear's slope jumps at every texel
// border, which shows soft shadows as creases on the texel grid. Reads up to 2 texels around a chart's edge (the editor pads charts by 2).
vec3 LightmapBicubic(sampler2D s, vec2 uv)
{
    vec2 size = vec2(textureSize(s, 0));
    vec2 p  = uv * size - 0.5;
    vec2 f  = fract(p);
    vec2 i  = p - f;
    vec2 f2 = f * f;
    vec2 f3 = f2 * f;
    vec2 w0 = (1.0 - 3.0 * f + 3.0 * f2 - f3) / 6.0;
    vec2 w1 = (4.0 - 6.0 * f2 + 3.0 * f3) / 6.0;
    vec2 w2 = (1.0 + 3.0 * f + 3.0 * f2 - 3.0 * f3) / 6.0;
    vec2 w3 = f3 / 6.0;
    vec2 g0 = w0 + w1;
    vec2 g1 = w2 + w3;
    vec2 a  = (i - 0.5 + w1 / g0) / size;
    vec2 b  = (i + 1.5 + w3 / g1) / size;
    return g0.y * (g0.x * texture2DLod(s, vec2(a.x, a.y), 0.0).rgb + g1.x * texture2DLod(s, vec2(b.x, a.y), 0.0).rgb)
         + g1.y * (g0.x * texture2DLod(s, vec2(a.x, b.y), 0.0).rgb + g1.x * texture2DLod(s, vec2(b.x, b.y), 0.0).rgb);
}

vec3 SampleLightmap(sampler2D s, vec2 uv)
{
    if (isRBSP.x > 1.5) return LightmapBicubic(s, uv);
    return texture2DLod(s, uv, 0.0).rgb;
}

vec4 ApplyFog(vec4 fragColor, vec3 pixelPosition)
{
    float fragDistance = distance(pixelPosition, cameraPosition.xyz);
    float fogFactor    = clamp((fragDistance - fog_start.x) / (fog_end.x - fog_start.x), 0.0, 1.0) * fog_opacity.x;
    return vec4(mix(fragColor.rgb, fog_color.rgb, fogFactor), fragColor.a);
}

void main()
{

    //gl_FragColor = vec4(v_texcoord4.x,v_texcoord4.y,0,1);
    //return;

    vec4 o_texture = texture2D(s_bspTexture, v_texcoord0);

    vec3 o_lightmap = vec3_splat(0.0);

    if(isRBSP.x > 0.0)
    {

        // Vertex colour = light scale (core/light_scale.h): rgb is the mantissa (linear, 0..1), A the shared exponent, multiplier 2^(5 * (1 - A)) = 1 .. 32.
        // A chart's texels are stored divided by this colour, so the lightmap times it is the baked light (HDR, and the darks use the page's whole 8 bits).
        // A vertex lit surface has no lightmap (the engine binds a white one) and all its light is in this colour. Opaque white (every older file) = no scale.
        // The surfaces of one smooth group share the exponent, so interpolating rgb and A separately and decoding after equals interpolating the light.
        // rBSP v2: the scale is part of the stored light (the pages are normalised by it), so it is ALWAYS applied, whatever useVertexLight says.
        // Older rBSP keeps the old switch (its vertex colour is plain white or a vertex light).
        vec3 vertexLightFactor = vec3_splat(1.0);
        if (isRBSP.x > 1.5)
            vertexLightFactor = v_color0.rgb * exp2(5.0 * (1.0 - v_color0.a));
        else if (useVertexLight.x > 0.0)
            vertexLightFactor = v_color0.rgb;

        // All slots share the same UV — they live in separate atlas textures
        int  slots     = int(numLightmapSlots.x);

        if (slots > 0) o_lightmap += SampleLightmap(s_bspLightmap,  v_texcoord1) * lmStyleColor0.rgb;
        if (slots > 1) o_lightmap += SampleLightmap(s_bspLightmap1, v_texcoord2) * lmStyleColor1.rgb;
        if (slots > 2) o_lightmap += SampleLightmap(s_bspLightmap2, v_texcoord3) * lmStyleColor2.rgb;
        if (slots > 3) o_lightmap += SampleLightmap(s_bspLightmap3, v_texcoord4) * lmStyleColor3.rgb;
        if (slots > 4) o_lightmap += SampleLightmap(s_bspLightmap4, v_texcoord4) * lmStyleColor4.rgb;
        if (slots > 5) o_lightmap += SampleLightmap(s_bspLightmap5, v_texcoord4) * lmStyleColor5.rgb;
        if (slots > 6) o_lightmap += SampleLightmap(s_bspLightmap6, v_texcoord4) * lmStyleColor6.rgb;
        if (slots > 7) o_lightmap += SampleLightmap(s_bspLightmap7, v_texcoord4) * lmStyleColor7.rgb;

        o_lightmap *= vertexLightFactor;

    }else
    {
        float vertexLightComp   = 4.04;
        vec3  vertexLightFactor = vec3(1.5, 1.5, 1.5);

    if (useVertexLight.x > 0.0)
        vertexLightFactor = vec3(vertexLightComp,vertexLightComp,vertexLightComp) * v_color0.rgb;

        vertexLightFactor *= 1.5;

        o_lightmap = texture2D(s_bspLightmap, v_texcoord1).rgb
                      * vertexLightFactor;

    }

    

    vec3  normal = normalize(v_normal);
    float dirDot = clamp(dot(normal, normalize(direct_light_dir.xyz)), 0.0, 1.0);
    // isRBSP 2 = rBSP v2 (level editor): light volume values are used as stored, no x1.1
    float lightvolScale = isRBSP.x > 1.5 ? 1.0 : 1.1;
    o_lightmap  += mix(light_color.rgb, direct_light_color.rgb + light_color.rgb, dirDot) * lightvolScale;

    vec3 dynamicLight = vec3_splat(0.0);
    int numLights = min(MAX_POINT_LIGHTS, int(PointLightsNumber.x));
    for (int i = 0; i < numLights; i++)
        dynamicLight += CalculateSimplePointLight(i, v_world.xyz, normal);

    vec3 lit;
    if (isRBSP.x > 1.5)
    {
        // rBSP v2 (level editor): the baked light is linear HDR and the albedo is sRGB, so they are multiplied in linear light and
        // encoded back to sRGB, as the editor's Lit view does. The game's dynamic lights keep their gamma-space look.
        lit = SrgbFromLinear(LinearFromSrgb(o_texture.rgb) * o_lightmap) + o_texture.rgb * dynamicLight;
    }
    else
    {
        lit = o_texture.rgb * (o_lightmap + dynamicLight);
    }

    gl_FragColor = ApplyFog(vec4(lit * brightness.x, o_texture.a), v_world.xyz);
}
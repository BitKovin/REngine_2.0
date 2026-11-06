$input v_texcoord0, v_local, v_misc

#include <../bgfx_shader.sh>
// Declared here, not in ui2_common.sh: the engine reads the sampler slots from this file's text.
SAMPLER2D(s_uiData, 0);
SAMPLER2D(s_uiTex0, 1);
SAMPLER2D(s_uiRamp, 2);
uniform vec4 u_uiParams;
#include <ui2_common.sh>

// Box shadows live in their own program so ordinary boxes never pay for the integration loop.
// Primitive layout (kind 3): T1 colour, T2 (shadowW, shadowH, aa, sigma), T3 shadow radii (the largest is used),
// T4 element box (x, y, w, h) and T5 element radii, both relative to the shadow box origin, T13.z = 1 for inset.
// v_local is relative to the shadow box origin.

vec2 uiErf2(vec2 x)
{
    vec2 s = sign(x);
    vec2 a = abs(x);
    x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
    x *= x;
    return s - s / (x * x);
}

float uiGauss(float x, float sigma)
{
    return exp(-(x * x) / (2.0 * sigma * sigma)) / (2.5066283 * sigma);
}

float uiShadowX(float x, float y, float sigma, float corner, vec2 halfSize)
{
    float delta = min(halfSize.y - corner - abs(y), 0.0);
    float curved = halfSize.x - corner + sqrt(max(0.0, corner * corner - delta * delta));
    vec2 integral = 0.5 + 0.5 * uiErf2((x + vec2(-curved, curved)) * (0.70710678 / sigma));
    return integral.y - integral.x;
}

float uiRoundedBoxShadow(vec2 halfSize, vec2 p, float sigma, float corner)
{
    float low = p.y - halfSize.y;
    float high = p.y + halfSize.y;
    float start = clamp(-3.0 * sigma, low, high);
    float end = clamp(3.0 * sigma, low, high);
    float step = (end - start) / 4.0;
    float y = start + step * 0.5;
    float value = 0.0;
    for (int i = 0; i < 4; i++)
    {
        value += uiShadowX(p.x, p.y - y, sigma, corner, halfSize) * uiGauss(y, sigma) * step;
        y += step;
    }
    return value;
}

void main()
{
    int prim = int(v_misc.x + 0.5);
    vec2 screenPos = v_misc.yz;

    vec4 header = uiFetch(prim, 0);
    vec4 colour = uiFetch(prim, 1);
    vec4 sz = uiFetch(prim, 2);
    vec4 radii = uiFetch(prim, 3);
    vec4 box = uiFetch(prim, 4);
    vec4 boxRadii = uiFetch(prim, 5);
    vec4 params = uiFetch(prim, 13);

    vec2 halfSize = sz.xy * 0.5;
    float px = max(sz.z, 0.0001);
    float sigma = max(sz.w, 0.5);
    float corner = min(max(max(radii.x, radii.y), max(radii.z, radii.w)), min(halfSize.x, halfSize.y));
    float shadow = uiRoundedBoxShadow(halfSize, v_local - halfSize, sigma, corner);

    vec2 boxHalf = box.zw * 0.5;
    float inBox = uiCoverage(uiRoundBoxSdf(v_local - (box.xy + boxHalf), boxHalf, boxRadii), px);

    // Outer shadows are not painted under the element box, inset shadows only inside it.
    float alpha = (params.z > 0.5) ? (1.0 - shadow) * inBox : shadow * (1.0 - inBox);
    gl_FragColor = uiFinish(uiPremul(colour) * alpha, prim, header, screenPos);
}

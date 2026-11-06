$input v_texcoord0, v_local, v_misc

#include <../bgfx_shader.sh>

// Primitive data texture: 16 RGBA32F texels per primitive (layout in UiRenderTypes.h).
SAMPLER2D(s_uiData, 0);
// Texture of the current batch (white 1x1 when the batch has none).
SAMPLER2D(s_uiTex0, 1);
// [P3] gradient ramp sampler goes here as slot 3.

// x = data texture width in texels, y = texels per primitive, z = time (seconds), w = unused
uniform vec4 u_uiParams;

// Reads one texel of the primitive data texture. This uses texture2DLod at the texel centre (point sampling, so exact)
// and not texelFetch: shaderc drops a sampler that is only used through texelFetch, and the engine then cannot bind it.
// The height of the data texture is 256 (UiRender::kDataHeight); the width comes from u_uiParams.x.
vec4 uiFetch(int prim, int t)
{
    float width = u_uiParams.x;
    float texel = float(prim * int(u_uiParams.y) + t);
    float row = floor(texel / width);
    float col = texel - row * width;
    return texture2DLod(s_uiData, vec2((col + 0.5) / width, (row + 0.5) / 256.0), 0.0);
}

bool uiFlag(float flags, float bit)
{
    return mod(floor(flags / bit), 2.0) > 0.5;
}

float uiCoverage(float d, float px)
{
    return clamp(0.5 - d / px, 0.0, 1.0);
}

// p relative to the box centre, y down; r = (top-left, top-right, bottom-right, bottom-left)
float uiRoundBoxSdf(vec2 p, vec2 halfSize, vec4 r)
{
    float rad = (p.x < 0.0) ? ((p.y < 0.0) ? r.x : r.w) : ((p.y < 0.0) ? r.y : r.z);
    rad = min(rad, min(halfSize.x, halfSize.y));
    vec2 q = abs(p) - halfSize + vec2(rad, rad);
    return min(max(q.x, q.y), 0.0) + length(max(q, vec2(0.0, 0.0))) - rad;
}

vec4 uiPremul(vec4 c)
{
    return vec4(c.rgb * c.a, c.a);
}

vec4 uiShadeImage(int prim, vec4 tex)
{
    // Identical to the legacy fs_ui_textured: texture * colour, then premultiply.
    return uiPremul(tex * uiFetch(prim, 1));
}

vec4 uiShadeBox(int prim, vec2 local, vec4 tex, float flags, float px)
{
    vec4 fill   = uiFetch(prim, 1);
    vec2 size   = uiFetch(prim, 2).xy;
    vec4 radii  = uiFetch(prim, 3);
    vec4 bw     = uiFetch(prim, 4);   // top, right, bottom, left

    vec2 halfSize = size * 0.5;
    float outer = uiCoverage(uiRoundBoxSdf(local - halfSize, halfSize, radii), px);

    float inner = outer;
    if (bw.x + bw.y + bw.z + bw.w > 0.0)
    {
        vec2 innerMin = vec2(bw.w, bw.x);
        vec2 innerMax = size - vec2(bw.y, bw.z);
        vec2 innerHalf = max((innerMax - innerMin) * 0.5, vec2(0.0, 0.0));
        vec2 innerCentre = (innerMin + innerMax) * 0.5;
        vec4 innerRadii = max(radii - vec4(max(bw.w, bw.x), max(bw.x, bw.y), max(bw.y, bw.z), max(bw.z, bw.w)),
                              vec4(0.0, 0.0, 0.0, 0.0));
        inner = min(uiCoverage(uiRoundBoxSdf(local - innerCentre, innerHalf, innerRadii), px), outer);
    }

    vec4 fillColor = fill;
    if (uiFlag(flags, 2.0))
        fillColor *= tex;
    vec4 result = uiPremul(fillColor) * inner;

    if (outer - inner > 0.0)
    {
        // Pick the side by distance normalised by that side's width: this reproduces CSS's diagonal corner split.
        float dt = (bw.x > 0.0) ? local.y / bw.x : 1.0e9;
        float dr = (bw.y > 0.0) ? (size.x - local.x) / bw.y : 1.0e9;
        float db = (bw.z > 0.0) ? (size.y - local.y) / bw.z : 1.0e9;
        float dl = (bw.w > 0.0) ? local.x / bw.w : 1.0e9;
        vec4 bc = uiFetch(prim, 5);
        // Exact ties (the corner diagonals) must resolve the same way on every backend, whatever the interpolation
        // rounding does, so a side only wins when it is clearly closer. Order of preference: top, right, bottom, left.
        float m = dt;
        if (dr < m - 0.001) { m = dr; bc = uiFetch(prim, 6); }
        if (db < m - 0.001) { m = db; bc = uiFetch(prim, 7); }
        if (dl < m - 0.001) { bc = uiFetch(prim, 8); }
        result += uiPremul(bc) * (outer - inner);
    }
    return result;
}

vec4 uiShadeSdfGlyph(int prim, vec4 tex, float px)
{
    vec4 color = uiFetch(prim, 1);
    vec4 g = uiFetch(prim, 13);        // x = UI units per atlas pixel (glyph scale), y = dilation in UI units
    // Atlas encodes distance as 128 + 16 * (distance in atlas pixels), inside > 128 (stbtt_GetCodepointSDF settings).
    float d = (tex.r * 255.0 - 128.0) / 16.0 * g.x + g.y;
    float cov = clamp(0.5 + d / px, 0.0, 1.0);
    return uiPremul(color) * cov;
}

float uiClip(int prim, vec2 screenPos, float flags, float pxScreen)
{
    if (!uiFlag(flags, 1.0))
        return 1.0;
    vec4 rect  = uiFetch(prim, 9);
    vec4 radii = uiFetch(prim, 10);
    vec2 halfSize = (rect.zw - rect.xy) * 0.5;
    return uiCoverage(uiRoundBoxSdf(screenPos - (rect.xy + halfSize), halfSize, radii), pxScreen);
}

void main()
{
    int prim = int(v_misc.x + 0.5);
    vec2 screenPos = v_misc.yz;

    // Implicit-LOD sampling only here, in uniform control flow.
    vec4 tex = texture2D(s_uiTex0, v_texcoord0);

    vec4 header = uiFetch(prim, 0);
    float kind = header.x;
    float flags = header.y;
    // Anti-aliasing width comes from the CPU (local units per target pixel), not from dFdx/dFdy:
    // derivatives differ between backends, and every backend must produce the same pixels.
    float px = max(uiFetch(prim, 2).z, 0.0001);
    float pxScreen = 1.0;   // screen positions are in target pixels already

    vec4 color;
    if (kind < 0.5)
        color = uiShadeImage(prim, tex);
    else if (kind < 1.5)
        color = uiShadeBox(prim, v_local, tex, flags, px);
    else if (kind < 2.5)
        color = uiShadeSdfGlyph(prim, tex, px);
    // [P3] kinds 3..5 go here.
    else
        color = vec4(1.0, 0.0, 1.0, 1.0);   // unknown kind: loud magenta

    color *= uiClip(prim, screenPos, flags, pxScreen);
    gl_FragColor = color;
}

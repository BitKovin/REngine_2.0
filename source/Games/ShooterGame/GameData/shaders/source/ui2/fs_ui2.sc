$input v_texcoord0, v_local, v_misc

#include <../bgfx_shader.sh>
// Declared here, not in ui2_common.sh: the engine reads the sampler slots from this file's text.
SAMPLER2D(s_uiData, 0);
SAMPLER2D(s_uiTex0, 1);
SAMPLER2D(s_uiRamp, 2);
uniform vec4 u_uiParams;
#include <ui2_common.sh>

// Anti-aliasing width comes from the CPU (texel 2 .z, local units per target pixel), never from dFdx/dFdy:
// derivatives differ between backends, and every backend must produce the same pixels.

// Gradient colour for a Box or Arc. Header T11 = (type, rampRow, period, start); geometry T12.
// type 1 linear (p0, p1), 2 radial (centre, radii), 3 conic (centre, start angle). Straight colour.
// A repeating gradient has period > 0 and repeats the part of the line from `start` on; otherwise the position is clamped to 0..1.
vec4 uiGradientColor(int prim, vec4 gh, vec2 local)
{
    vec4 g = uiFetch(prim, 12);
    float t;
    if (gh.x < 1.5)
    {
        vec2 d = g.zw - g.xy;
        t = dot(local - g.xy, d) / max(dot(d, d), 0.000001);
    }
    else if (gh.x < 2.5)
        t = length((local - g.xy) / max(g.zw, vec2(0.0001, 0.0001)));
    else
    {
        // clockwise from 12 o'clock, as in CSS
        vec2 d = local - g.xy;
        t = fract((atan2(d.x, -d.y) - g.z) / 6.2831853 + 1.0);
    }
    t = (gh.z > 0.0) ? fract((t - gh.w) / gh.z) : clamp(t, 0.0, 1.0);
    return uiRamp(gh.y, t);
}

// Tiled image fill: v_texcoord0 carries the tile coordinate, T12 the sub-rectangle in the texture. T11.z bit 0 = repeat x, bit 1 = repeat y.
// Sampled in a branch, so with an explicit level (this loses mips for tiled images). Returns the colour and zero outside a non-repeating tile.
vec4 uiTiledSample(int prim, vec4 gh, vec2 tile, float flags)
{
    vec4 r = uiFetch(prim, 12);
    bool repX = mod(gh.z, 2.0) > 0.5;
    bool repY = gh.z > 1.5;
    vec2 f = vec2(repX ? fract(tile.x) : clamp(tile.x, 0.0, 1.0), repY ? fract(tile.y) : clamp(tile.y, 0.0, 1.0));
    float inside = 1.0;
    if (!repX && (tile.x < 0.0 || tile.x > 1.0))
        inside = 0.0;
    if (!repY && (tile.y < 0.0 || tile.y > 1.0))
        inside = 0.0;
    vec2 uv = mix(r.xy, r.zw, f);
    if (uiFlag(flags, 16.0))
    {
        vec2 size = max(uiFetch(prim, 13).xy, vec2(1.0, 1.0));
        uv = (floor(uv * size) + 0.5) / size;
    }
    return texture2DLod(s_uiTex0, uv, 0.0) * inside;
}

vec4 uiShadeImage(int prim, vec4 tex, float flags)
{
    // A layer texture is premultiplied already: texture * tint (the tint carries the opacity).
    if (uiFlag(flags, 32.0))
        return tex * uiFetch(prim, 1);
    // Identical to the legacy fs_ui_textured: texture * colour, then premultiply.
    return uiPremul(tex * uiFetch(prim, 1));
}

vec4 uiShadeBox(int prim, vec2 local, vec2 tile, vec4 tex, float flags, float px)
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

    vec4 gh = uiFetch(prim, 11);
    vec4 fillTex = tex;
    if (gh.x > 0.5)
        fill = uiGradientColor(prim, gh, local) * fill;   // replaces the fill colour; the tint (white, with the opacity as alpha) scales it
    else if (gh.w > 0.5)
        fillTex = uiTiledSample(prim, gh, tile, flags);

    vec4 fillColor = fill;
    if (uiFlag(flags, 2.0))
        fillColor *= fillTex;
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
    // x = UI units per atlas pixel (glyph scale), y = dilation in UI units, z = softness (text-shadow blur / 2), w = stroke width
    vec4 g = uiFetch(prim, 13);
    // Atlas encodes distance as 128 + 16 * (distance in atlas pixels), inside > 128 (stbtt_GetCodepointSDF settings).
    float d = (tex.r * 255.0 - 128.0) / 16.0 * g.x + g.y;
    float cov = (g.z > 0.0) ? smoothstep(-g.z, g.z, d) : clamp(0.5 + d / px, 0.0, 1.0);
    vec4 result = uiPremul(color) * cov;
    if (g.w > 0.0)
    {
        // The stroke is composited under the fill and grows outwards.
        float strokeCov = clamp(0.5 + (d + g.w) / px, 0.0, 1.0);
        result += uiPremul(uiFetch(prim, 14)) * strokeCov * (1.0 - result.a);
    }
    return result;
}

// Arc: T1 colour, T2 (w, h, aa, thickness), T13 (start rad, sweep rad, 0, round caps). Angles clockwise from 12 o'clock.
vec4 uiShadeArc(int prim, vec2 local, float px)
{
    vec4 fill = uiFetch(prim, 1);
    vec4 sz = uiFetch(prim, 2);
    vec4 a = uiFetch(prim, 13);
    vec4 gh = uiFetch(prim, 11);
    vec2 p = local - sz.xy * 0.5;
    float radius = min(sz.x, sz.y) * 0.5;
    float thickness = sz.w;
    float mid = radius - thickness * 0.5;
    float d = abs(length(p) - mid) - thickness * 0.5;
    if (a.y < 6.2831)
    {
        float centre = a.x + a.y * 0.5;
        float halfAngle = a.y * 0.5;
        vec2 axis = vec2(sin(centre), -cos(centre));
        vec2 side = vec2(-axis.y, axis.x);
        vec2 q = vec2(abs(dot(p, side)), dot(p, axis));
        vec2 e = vec2(sin(halfAngle), cos(halfAngle));
        float m = length(q - e * max(dot(q, e), 0.0));
        float wedge = m * sign(e.y * q.x - e.x * q.y);
        d = max(d, wedge);
        if (a.w > 0.5)
        {
            float c0 = length(p - mid * vec2(sin(a.x), -cos(a.x))) - thickness * 0.5;
            float c1 = length(p - mid * vec2(sin(a.x + a.y), -cos(a.x + a.y))) - thickness * 0.5;
            d = min(d, min(c0, c1));
        }
    }
    if (gh.x > 0.5)
        fill = uiGradientColor(prim, gh, local) * fill;
    return uiPremul(fill) * uiCoverage(d, px);
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
    float px = max(uiFetch(prim, 2).z, 0.0001);

    // image-rendering: pixelated. The texture size is in T13.xy; tiled fills snap inside uiTiledSample.
    if (uiFlag(flags, 16.0) && kind < 1.5)
    {
        vec2 size = max(uiFetch(prim, 13).xy, vec2(1.0, 1.0));
        tex = texture2DLod(s_uiTex0, (floor(v_texcoord0 * size) + 0.5) / size, 0.0);
    }

    vec4 color;
    if (kind < 0.5)
        color = uiShadeImage(prim, tex, flags);
    else if (kind < 1.5)
        color = uiShadeBox(prim, v_local, v_texcoord0, tex, flags, px);
    else if (kind < 2.5)
        color = uiShadeSdfGlyph(prim, tex, px);
    else if (kind > 3.5 && kind < 4.5)
        color = uiShadeArc(prim, v_local, px);
    else if (kind > 4.5 && kind < 5.5)
        color = uiPremul(uiFetch(prim, 1)) * v_local.x;   // tessellated path: the fringe vertices carry the coverage in v_local.x
    else
        color = vec4(1.0, 0.0, 1.0, 1.0);   // unknown kind (or a shadow in the wrong program): loud magenta

    gl_FragColor = uiFinish(color, prim, header, screenPos);
}

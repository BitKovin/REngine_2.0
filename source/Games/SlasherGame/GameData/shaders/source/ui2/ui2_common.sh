// Shared by the ui2 fragment shaders (fs_ui2.sc, fs_ui2_shadow.sc). Not compiled on its own: it has no fs_/vs_ prefix.
// Include after <../bgfx_shader.sh>.

// The including shader declares (the engine reads the sampler slots from the .sc source text, not from here):
//   SAMPLER2D(s_uiData, 0);   primitive data texture, 16 RGBA32F texels per primitive (layout in UiRenderTypes.h); rows 224..255 hold auxiliary texels
//   SAMPLER2D(s_uiTex0, 1);   texture of the current batch (white 1x1 when the batch has none)
//   SAMPLER2D(s_uiRamp, 2);   gradient ramps: 256 x 256 RGBA8, one row per gradient, straight colour; sample only with texture2DLod
//   uniform vec4 u_uiParams;  x = data texture width in texels, y = texels per primitive, z = time (seconds), w = unused

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

// Auxiliary texels live in rows 224..255 of the same texture (UiRender::kAuxFirstRow).
vec4 uiFetchAux(int index)
{
    float width = u_uiParams.x;
    float row = floor(float(index) / width);
    float col = float(index) - row * width;
    return texture2DLod(s_uiData, vec2((col + 0.5) / width, (224.0 + row + 0.5) / 256.0), 0.0);
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

vec4 uiRamp(float rampRow, float t)
{
    return texture2DLod(s_uiRamp, vec2((t * 255.0 + 0.5) / 256.0, (rampRow + 0.5) / 256.0), 0.0);
}

float uiClip(int prim, vec2 screenPos, float flags)
{
    if (!uiFlag(flags, 1.0))
        return 1.0;
    vec4 rect  = uiFetch(prim, 9);
    vec4 radii = uiFetch(prim, 10);
    vec2 halfSize = (rect.zw - rect.xy) * 0.5;
    // Screen positions are in target pixels already, so the anti-aliasing width is one unit.
    return uiCoverage(uiRoundBoxSdf(screenPos - (rect.xy + halfSize), halfSize, radii), 1.0);
}

// clip-path circle() / ellipse(): T15.w is the aux index of (shape, cx, cy, r) and, for an ellipse, (rx, ry, 0, 0). UI units.
float uiClipShape(int prim, vec2 screenPos, float flags)
{
    if (!uiFlag(flags, 8.0))
        return 1.0;
    int aux = int(uiFetch(prim, 15).w + 0.5);
    vec4 s = uiFetchAux(aux);
    vec2 p = screenPos - s.yz;
    float d;
    if (s.x < 1.5)
        d = length(p) - s.w;
    else
    {
        vec4 e = uiFetchAux(aux + 1);
        vec2 r = max(e.xy, vec2(0.0001, 0.0001));
        float k0 = length(p / r);
        float k1 = length(p / (r * r));
        d = (k1 > 0.00001) ? k0 * (k0 - 1.0) / k1 : -min(r.x, r.y);
    }
    return uiCoverage(d, 1.0);
}

// mask-image: T15 = (type, rampId row, aux index, 0). Aux texel = (x0, y0, x1, y1) for linear, (cx, cy, rx, ry) for radial; UI units.
float uiMask(int prim, vec2 screenPos, float flags)
{
    if (!uiFlag(flags, 4.0))
        return 1.0;
    vec4 m = uiFetch(prim, 15);
    vec4 g = uiFetchAux(int(m.z + 0.5));
    float t;
    if (m.x < 1.5)
    {
        vec2 d = g.zw - g.xy;
        t = dot(screenPos - g.xy, d) / max(dot(d, d), 0.000001);
    }
    else
        t = length((screenPos - g.xy) / max(g.zw, vec2(0.0001, 0.0001)));
    return uiRamp(m.y, clamp(t, 0.0, 1.0)).a;
}

// 4 x 5 colour matrix in 5 aux texels: texels 0..3 are the columns for r, g, b, a (each holds the four output rows), texel 4 the offsets.
// Applied to the straight colour, then premultiplied again.
vec4 uiColorMatrix(vec4 premul, int aux)
{
    if (premul.a <= 0.0)
        return premul;
    vec4 c = vec4(premul.rgb / premul.a, premul.a);
    vec4 r = uiFetchAux(aux) * c.r + uiFetchAux(aux + 1) * c.g + uiFetchAux(aux + 2) * c.b + uiFetchAux(aux + 3) * c.a + uiFetchAux(aux + 4);
    r = clamp(r, 0.0, 1.0);
    return vec4(r.rgb * r.a, r.a);
}

// Colour matrix, clips and mask, for every kind. `header` is texel 0 of the primitive.
vec4 uiFinish(vec4 color, int prim, vec4 header, vec2 screenPos)
{
    float flags = header.y;
    if (header.w > -0.5)
        color = uiColorMatrix(color, int(header.w + 0.5));
    return color * (uiClip(prim, screenPos, flags) * uiClipShape(prim, screenPos, flags) * uiMask(prim, screenPos, flags));
}

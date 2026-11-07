#pragma once
#include <UI/Render/UiDrawList.h>
#include <glm.h>
#include <vector>

struct UiRect { float X = 0.f, Y = 0.f, W = 0.f, H = 0.f; };
struct UiCorners { float TopLeft = 0.f, TopRight = 0.f, BottomRight = 0.f, BottomLeft = 0.f; };
struct UiEdges { float Top = 0.f, Right = 0.f, Bottom = 0.f, Left = 0.f; };

enum class UiBlendMode : int { Normal = 0, PlusLighter = 1, Multiply = 2, Screen = 3 };

// A gradient that replaces (multiplies) the fill colour of a Box or Arc. Geometry is in the primitive's local units:
// linear (x0, y0, x1, y1), radial (cx, cy, rx, ry), conic (cx, cy, start angle in radians, 0).
struct UiGradientFill
{
    int Type = 0;            // 0 none, 1 linear, 2 radial, 3 conic
    int RampId = -1;         // UiGradientRamps::Id
    float Period = 0.f;      // repeating gradient: length of one repeat in line units (> 0), otherwise 0
    float Start = 0.f;       // repeating gradient: where the first repeat starts
    glm::vec4 Geometry = glm::vec4(0.f);
};

struct UiBoxPaint
{
    glm::vec4 Fill = glm::vec4(0.f);                   // straight RGBA
    bgfx::TextureHandle FillTexture = BGFX_INVALID_HANDLE;
    glm::vec4 FillUv = glm::vec4(0.f, 0.f, 1.f, 1.f);  // u0, v0, u1, v1 (tile coordinates at the rect corners when Tiled)
    UiCorners Radii;
    UiEdges BorderWidth;
    glm::vec4 BorderColor[4] = { glm::vec4(0.f), glm::vec4(0.f), glm::vec4(0.f), glm::vec4(0.f) }; // top, right, bottom, left
    UiGradientFill Gradient;
    // Tiled texture fill: FillUv gives the tile coordinate at the rect corners, TileRect the part of the texture that is one tile.
    bool Tiled = false;
    glm::vec4 TileRect = glm::vec4(0.f, 0.f, 1.f, 1.f);
    int TileAxes = 3;                                   // bit 0 repeat horizontally, bit 1 vertically; no repeat = drawn once
};

struct UiGlyphQuad { float X, Y, W, H; float U0, V0, U1, V1; uint16_t Page; bool Italic; };

// A box-shadow of one element. Offsets and sizes in UI units.
struct UiShadowPaint
{
    float X = 0.f, Y = 0.f, Blur = 0.f, Spread = 0.f;
    glm::vec4 Color = glm::vec4(0.f);
    bool Inset = false;
};

// Effects applied to every primitive recorded while they are set: colour matrix, mask, clip shape, blend mode, sampling.
struct UiPainterFx
{
    int ColorMatrixAux = -1;                 // aux index of the 5 matrix texels, -1 = none
    int MaskType = 0;                        // 0 none, 1 linear, 2 radial
    int MaskRampId = -1;
    int MaskAux = -1;
    int ClipShapeAux = -1;
    UiBlendMode Blend = UiBlendMode::Normal;
    bool Pixelated = false;
    glm::vec2 PixelatedSize = glm::vec2(1.f);   // texture size in texels
    bool HasMatrix = false;                  // the current colour matrix (4 x 5, row major), kept to combine with a child's
    float Matrix[20] = {};
};

class UiPainter
{
public:
    explicit UiPainter(UiDrawList& list) : m_list(list) {}

    // Element-local (origin top-left, y down) to UI units. Default identity.
    void SetTransform(const glm::mat3& m) { m_transform = m; }
    const glm::mat3& Transform() const { return m_transform; }
    float PixelScale() const;   // device pixels per UI unit under the current transform (sizes an SVG raster)

    void Box(const UiRect& rect, const UiBoxPaint& paint);
    void Image(const UiRect& rect, bgfx::TextureHandle texture, const glm::vec4& uv, const glm::vec4& tint);

    // Draws SDF glyph quads (positions element-local). One primitive block per call per atlas page.
    // Italic quads are sheared by 0.2 * height at the top edge. Glyphs are not snapped here; callers snap the run origin.
    // softness > 0 blurs the edge (text-shadow: blur / 2); strokeWidth > 0 adds an outline of strokeColor under the fill.
    void GlyphRun(const UiGlyphQuad* quads, size_t count, const glm::vec4& color, float glyphScale, float dilation,
                  float softness = 0.f, float strokeWidth = 0.f, const glm::vec4& strokeColor = glm::vec4(0.f));

    // Ring segment inside the rect (a circle of the smaller side). Angles in radians, clockwise from 12 o'clock.
    void Arc(const UiRect& rect, float thickness, float startRad, float sweepRad, bool roundCaps, const glm::vec4& color,
             const UiGradientFill& gradient = UiGradientFill());

    // Box shadow of the element box `elem` (element-local) with corner radii. Drawn with the shadow program.
    void BoxShadow(const UiRect& elem, const UiCorners& elemRadii, const UiShadowPaint& shadow, float opacity);

    // Anti-aliased triangles of a tessellated path (kind Solid). Vertex alpha is the coverage: fringe vertices carry 0.
    struct PathVertex { glm::vec2 P; float Coverage; };
    void Solid(const std::vector<PathVertex>& vertices, const std::vector<uint16_t>& indices, const glm::vec4& color);
    // Convex polygon fill and polyline stroke, tessellated with a one-pixel fringe.
    void FillPolygon(const std::vector<glm::vec2>& points, const glm::vec4& color);
    void StrokePolyline(const std::vector<glm::vec2>& points, float width, const glm::vec4& color, bool closed);

    // Clip in element-local space. Axis-aligned transforms use the shader clip; others use the legacy stencil mask.
    void PushClip(const UiRect& rect, const UiCorners& radii);
    void PopClip();

    // ── layers: everything recorded between PushLayer and PopLayer is rendered offscreen (with the effects of the spec) and drawn
    // back as one quad. Inside, the painter starts with no effects and no clip; PopLayer restores them for the composite quad,
    // which takes `opacity` and the blend mode `blend` (the element's mix-blend-mode, applied to the group as a whole).
    void PushLayer(const UiRender::LayerSpec& spec);
    void PopLayer(float opacity, UiBlendMode blend);
    // The blurred scene behind `rect` (backdrop-filter), clipped to the rounded shape. The texture is chosen and patched at submit.
    void Backdrop(const UiRect& rect, const UiCorners& radii, float sigma, float opacity);

    // ── effects
    const UiPainterFx& Fx() const { return m_fx; }
    void SetFx(const UiPainterFx& fx) { m_fx = fx; }
    // Aux index of a colour matrix (4 x 5, row major: R, G, B, A rows of 5 coefficients) in this painter's list; -1 when the aux area is full.
    int AddColorMatrix(const float m[20]);
    // Aux index of a mask geometry texel / clip-shape texels in this painter's list.
    int AddAux(const glm::vec4* texels, int count) { return m_list.AddAux(texels, count); }

private:
    struct Clip { glm::vec4 Rect; glm::vec4 Radii; bool Stencil; };
    struct LayerFrame { int Begin; struct UiPainterFx Fx; std::vector<Clip> Clips; };
    UiDrawList& m_list;
    glm::mat3 m_transform = glm::mat3(1.f);
    std::vector<Clip> m_clips;
    std::vector<LayerFrame> m_layerStack;
    UiPainterFx m_fx;
    bool IsTranslationOnly() const;
    void ApplyClip(UiRender::PrimBlock& block) const;
    void ApplyFx(UiRender::PrimBlock& block, bool sampled) const;
    uint64_t StateFor(const UiPainterFx& fx) const;
};

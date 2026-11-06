#pragma once
#include <UI/Render/UiDrawList.h>
#include <glm.h>
#include <vector>

struct UiRect { float X = 0.f, Y = 0.f, W = 0.f, H = 0.f; };
struct UiCorners { float TopLeft = 0.f, TopRight = 0.f, BottomRight = 0.f, BottomLeft = 0.f; };
struct UiEdges { float Top = 0.f, Right = 0.f, Bottom = 0.f, Left = 0.f; };

struct UiBoxPaint
{
    glm::vec4 Fill = glm::vec4(0.f);                   // straight RGBA
    bgfx::TextureHandle FillTexture = BGFX_INVALID_HANDLE;
    glm::vec4 FillUv = glm::vec4(0.f, 0.f, 1.f, 1.f);  // u0, v0, u1, v1
    UiCorners Radii;
    UiEdges BorderWidth;
    glm::vec4 BorderColor[4] = { glm::vec4(0.f), glm::vec4(0.f), glm::vec4(0.f), glm::vec4(0.f) }; // top, right, bottom, left
};

struct UiGlyphQuad { float X, Y, W, H; float U0, V0, U1, V1; uint16_t Page; bool Italic; };

class UiPainter
{
public:
    explicit UiPainter(UiDrawList& list) : m_list(list) {}

    // Element-local (origin top-left, y down) to UI units. Default identity.
    void SetTransform(const glm::mat3& m) { m_transform = m; }
    const glm::mat3& Transform() const { return m_transform; }

    void Box(const UiRect& rect, const UiBoxPaint& paint);
    void Image(const UiRect& rect, bgfx::TextureHandle texture, const glm::vec4& uv, const glm::vec4& tint);

    // Draws SDF glyph quads (positions element-local). One primitive block per call per atlas page.
    // Italic quads are sheared by 0.2 * height at the top edge. Glyphs are not snapped here; callers snap the run origin.
    void GlyphRun(const UiGlyphQuad* quads, size_t count, const glm::vec4& color, float glyphScale, float dilation);

    // Clip in element-local space. Axis-aligned transforms use the shader clip; others use the legacy stencil mask.
    void PushClip(const UiRect& rect, const UiCorners& radii);
    void PopClip();

private:
    struct Clip { glm::vec4 Rect; glm::vec4 Radii; bool Stencil; };
    UiDrawList& m_list;
    glm::mat3 m_transform = glm::mat3(1.f);
    std::vector<Clip> m_clips;
    bool IsTranslationOnly() const;
    void ApplyClip(UiRender::PrimBlock& block) const;
};

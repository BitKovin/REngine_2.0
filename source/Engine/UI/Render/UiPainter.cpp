#include "UiPainter.h"

#include <UI/Render/UiGpu.h>
#include <UI/Render/UiFontLibrary.h>
#include <UI/UiRenderer.h>

#include <algorithm>
#include <cmath>

using namespace UiRender;

namespace
{
    // Decision D10: elements thinner than one UI unit are drawn one unit thick with proportionally lower alpha.
    void ApplyThinRule(UiRect& rect, UiBoxPaint& paint)
    {
        if (rect.W > 0.f && rect.W < 1.f)
        {
            const float original = rect.W;
            rect.W = 1.f;
            paint.Fill.a *= original;
            for (glm::vec4& c : paint.BorderColor)
                c.a *= original;
        }
        if (rect.H > 0.f && rect.H < 1.f)
        {
            const float original = rect.H;
            rect.H = 1.f;
            paint.Fill.a *= original;
            for (glm::vec4& c : paint.BorderColor)
                c.a *= original;
        }
        float* widths[4] = { &paint.BorderWidth.Top, &paint.BorderWidth.Right, &paint.BorderWidth.Bottom, &paint.BorderWidth.Left };
        for (int i = 0; i < 4; ++i)
        {
            if (*widths[i] > 0.f && *widths[i] < 1.f)
            {
                paint.BorderColor[i].a *= *widths[i];
                *widths[i] = 1.f;
            }
        }
    }

    // Rounds a rect already in UI units to whole pixels; a rect that had size never collapses to nothing.
    void SnapRect(float x0, float y0, float x1, float y1, float& outX, float& outY, float& outW, float& outH)
    {
        const float rx0 = std::round(x0), ry0 = std::round(y0);
        float rx1 = std::round(x1), ry1 = std::round(y1);
        if (rx1 == rx0 && x1 > x0) rx1 = rx0 + 1.f;
        if (ry1 == ry0 && y1 > y0) ry1 = ry0 + 1.f;
        outX = rx0; outY = ry0; outW = rx1 - rx0; outH = ry1 - ry0;
    }

    float TransformScale(const glm::mat3& m)
    {
        const float det = m[0][0] * m[1][1] - m[1][0] * m[0][1];
        const float scale = std::sqrt(std::fabs(det));
        return scale > 0.0001f ? scale : 1.f;
    }

    glm::vec2 Apply(const glm::mat3& m, float x, float y)
    {
        return glm::vec2(m[0][0] * x + m[1][0] * y + m[2][0], m[0][1] * x + m[1][1] * y + m[2][1]);
    }
}

bool UiPainter::IsTranslationOnly() const
{
    const glm::mat3& m = m_transform;
    return m[0][0] == 1.f && m[1][1] == 1.f && m[0][1] == 0.f && m[1][0] == 0.f;
}

void UiPainter::ApplyClip(PrimBlock& block) const
{
    for (auto it = m_clips.rbegin(); it != m_clips.rend(); ++it)
    {
        if (it->Stencil)
            continue;
        block.AddFlag(kFlagClip);
        block.T[9] = it->Rect;
        block.T[10] = it->Radii;
        return;
    }
}

void UiPainter::Box(const UiRect& rectIn, const UiBoxPaint& paintIn)
{
    UiRect rect = rectIn;
    UiBoxPaint paint = paintIn;
    ApplyThinRule(rect, paint);
    if (rect.W <= 0.f || rect.H <= 0.f)
        return;

    glm::mat3 m = m_transform;
    if (IsTranslationOnly())
    {
        SnapRect(rect.X + m[2][0], rect.Y + m[2][1], rect.X + rect.W + m[2][0], rect.Y + rect.H + m[2][1], rect.X, rect.Y, rect.W, rect.H);
        m = glm::mat3(1.f);
    }

    const float scale = TransformScale(m);
    const float aa = 1.f / scale;
    const float e = aa;   // expand by one target pixel so anti-aliasing is not cut off

    PrimBlock block;
    block.SetKind(PrimKind::Box);
    block.T[1] = paint.Fill;
    block.T[2] = glm::vec4(rect.W, rect.H, aa, 0.f);
    block.T[3] = glm::vec4(paint.Radii.TopLeft, paint.Radii.TopRight, paint.Radii.BottomRight, paint.Radii.BottomLeft);
    block.T[4] = glm::vec4(paint.BorderWidth.Top, paint.BorderWidth.Right, paint.BorderWidth.Bottom, paint.BorderWidth.Left);
    for (int i = 0; i < 4; ++i)
        block.T[5 + i] = paint.BorderColor[i];
    const bool hasTexture = bgfx::isValid(paint.FillTexture);
    if (hasTexture)
        block.AddFlag(kFlagHasTexture);
    ApplyClip(block);

    const int prim = m_list.AddPrim(block);
    if (prim < 0)
        return;

    const float cx[4] = { -e, rect.W + e, rect.W + e, -e };   // TL, TR, BR, BL relative to the unexpanded origin
    const float cy[4] = { -e, -e, rect.H + e, rect.H + e };
    Vertex v[4];
    for (int i = 0; i < 4; ++i)
    {
        const glm::vec2 p = Apply(m, rect.X + cx[i], rect.Y + cy[i]);
        const float tu = paint.FillUv.x + (paint.FillUv.z - paint.FillUv.x) * (cx[i] / rect.W);
        const float tv = paint.FillUv.y + (paint.FillUv.w - paint.FillUv.y) * (cy[i] / rect.H);
        v[i] = { p.x, p.y, tu, tv, cx[i], cy[i], static_cast<float>(prim), 0.f };
    }

    BatchKey key;
    key.State = UiGpu::StateLegacyPremultiplied();
    key.TextureIdx = hasTexture ? paint.FillTexture.idx : bgfx::kInvalidHandle;
    key.StencilRef = UiRenderer::CurrentMaskDepth();
    m_list.AddQuad(key, v);
}

void UiPainter::Image(const UiRect& rectIn, bgfx::TextureHandle texture, const glm::vec4& uv, const glm::vec4& tint)
{
    UiRect rect = rectIn;
    if (rect.W <= 0.f || rect.H <= 0.f)
        return;

    glm::mat3 m = m_transform;
    if (IsTranslationOnly())
    {
        SnapRect(rect.X + m[2][0], rect.Y + m[2][1], rect.X + rect.W + m[2][0], rect.Y + rect.H + m[2][1], rect.X, rect.Y, rect.W, rect.H);
        m = glm::mat3(1.f);
    }

    PrimBlock block;
    block.SetKind(PrimKind::Image);
    block.T[1] = tint;
    block.T[2] = glm::vec4(rect.W, rect.H, 1.f / TransformScale(m), 0.f);
    ApplyClip(block);
    const int prim = m_list.AddPrim(block);
    if (prim < 0)
        return;

    const float cx[4] = { 0.f, rect.W, rect.W, 0.f };
    const float cy[4] = { 0.f, 0.f, rect.H, rect.H };
    const float us[4] = { uv.x, uv.z, uv.z, uv.x };
    const float vs[4] = { uv.y, uv.y, uv.w, uv.w };
    Vertex v[4];
    for (int i = 0; i < 4; ++i)
    {
        const glm::vec2 p = Apply(m, rect.X + cx[i], rect.Y + cy[i]);
        v[i] = { p.x, p.y, us[i], vs[i], cx[i], cy[i], static_cast<float>(prim), 0.f };
    }

    BatchKey key;
    key.State = UiGpu::StateLegacyPremultiplied();
    key.TextureIdx = bgfx::isValid(texture) ? texture.idx : bgfx::kInvalidHandle;
    key.StencilRef = UiRenderer::CurrentMaskDepth();
    m_list.AddQuad(key, v);
}

void UiPainter::GlyphRun(const UiGlyphQuad* quads, size_t count, const glm::vec4& color, float glyphScale, float dilation)
{
    if (count == 0 || color.a <= 0.f)
        return;
    const float aa = 1.f / TransformScale(m_transform);

    std::vector<uint16_t> pages;
    for (size_t i = 0; i < count; ++i)
        if (std::find(pages.begin(), pages.end(), quads[i].Page) == pages.end())
            pages.push_back(quads[i].Page);
    std::sort(pages.begin(), pages.end());

    for (uint16_t page : pages)
    {
        PrimBlock block;
        block.SetKind(PrimKind::SdfGlyph);
        block.T[1] = color;
        block.T[2] = glm::vec4(0.f, 0.f, aa, 0.f);
        block.T[13] = glm::vec4(glyphScale, dilation, 0.f, 0.f);
        ApplyClip(block);
        const int prim = m_list.AddPrim(block);
        if (prim < 0)
            return;

        BatchKey key;
        key.State = UiGpu::StateLegacyPremultiplied();
        const bgfx::TextureHandle texture = UiFontLibrary::PageTexture(page);
        key.TextureIdx = bgfx::isValid(texture) ? texture.idx : bgfx::kInvalidHandle;
        key.StencilRef = UiRenderer::CurrentMaskDepth();

        for (size_t i = 0; i < count; ++i)
        {
            const UiGlyphQuad& q = quads[i];
            if (q.Page != page)
                continue;
            const float shear = q.Italic ? 0.2f * q.H : 0.f;
            const float cx[4] = { q.X + shear, q.X + q.W + shear, q.X + q.W, q.X };
            const float cy[4] = { q.Y, q.Y, q.Y + q.H, q.Y + q.H };
            const float us[4] = { q.U0, q.U1, q.U1, q.U0 };
            const float vs[4] = { q.V0, q.V0, q.V1, q.V1 };
            Vertex v[4];
            for (int k = 0; k < 4; ++k)
            {
                const glm::vec2 p = Apply(m_transform, cx[k], cy[k]);
                v[k] = { p.x, p.y, us[k], vs[k], cx[k], cy[k], static_cast<float>(prim), 0.f };
            }
            m_list.AddQuad(key, v);
        }
    }
}

void UiPainter::PushClip(const UiRect& rect, const UiCorners& radii)
{
    if (IsTranslationOnly())
    {
        glm::vec4 r(rect.X + m_transform[2][0], rect.Y + m_transform[2][1],
                    rect.X + rect.W + m_transform[2][0], rect.Y + rect.H + m_transform[2][1]);
        glm::vec4 rad(radii.TopLeft, radii.TopRight, radii.BottomRight, radii.BottomLeft);
        for (auto it = m_clips.rbegin(); it != m_clips.rend(); ++it)
        {
            if (it->Stencil)
                continue;
            const glm::vec4 p = it->Rect;
            const bool inside = r.x >= p.x && r.y >= p.y && r.z <= p.z && r.w <= p.w;
            r = glm::vec4(std::max(r.x, p.x), std::max(r.y, p.y), std::min(r.z, p.z), std::min(r.w, p.w));
            if (!inside)
                rad = glm::vec4(0.f);   // documented limitation: a clipped clip loses its rounded corners
            break;
        }
        m_clips.push_back({ r, rad, false });
        return;
    }

    glm::mat3 translate(1.f);
    translate[2][0] = rect.X;
    translate[2][1] = rect.Y;
    UiRenderer::PushMask(m_transform * translate, glm::vec2(rect.W, rect.H));
    m_clips.push_back({ glm::vec4(0.f), glm::vec4(0.f), true });
}

void UiPainter::PopClip()
{
    if (m_clips.empty())
        return;
    if (m_clips.back().Stencil)
        UiRenderer::PopMask();
    m_clips.pop_back();
}

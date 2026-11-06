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

    void SetGradient(PrimBlock& block, const UiGradientFill& g)
    {
        block.T[11] = glm::vec4(static_cast<float>(g.Type), static_cast<float>(g.RampId), g.Period, g.Start);
        block.T[12] = g.Geometry;
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

void UiPainter::ApplyFx(PrimBlock& block, bool sampled) const
{
    if (m_fx.ColorMatrixAux >= 0)
        block.T[0].w = static_cast<float>(m_fx.ColorMatrixAux);
    if (m_fx.MaskType > 0 && m_fx.MaskAux >= 0 && m_fx.MaskRampId >= 0)
    {
        block.AddFlag(kFlagMask);
        block.T[15].x = static_cast<float>(m_fx.MaskType);
        block.T[15].y = static_cast<float>(m_fx.MaskRampId);
        block.T[15].z = static_cast<float>(m_fx.MaskAux);
    }
    if (m_fx.ClipShapeAux >= 0)
    {
        block.AddFlag(kFlagClipShape);
        block.T[15].w = static_cast<float>(m_fx.ClipShapeAux);
    }
    if (sampled && m_fx.Pixelated)
    {
        block.AddFlag(kFlagPixelated);
        block.T[13].x = m_fx.PixelatedSize.x;
        block.T[13].y = m_fx.PixelatedSize.y;
    }
}

uint64_t UiPainter::StateFor(const UiPainterFx& fx) const
{
    return UiGpu::StateForBlend(static_cast<int>(fx.Blend));
}

int UiPainter::AddColorMatrix(const float m[20])
{
    // texels 0..3: the columns for r, g, b, a (each holds the four output rows); texel 4: the offsets
    glm::vec4 texels[5];
    for (int col = 0; col < 4; ++col)
        texels[col] = glm::vec4(m[0 * 5 + col], m[1 * 5 + col], m[2 * 5 + col], m[3 * 5 + col]);
    texels[4] = glm::vec4(m[0 * 5 + 4], m[1 * 5 + 4], m[2 * 5 + 4], m[3 * 5 + 4]);
    return m_list.AddAux(texels, 5);
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
    if (paint.Gradient.Type > 0 && paint.Gradient.RampId >= 0)
        SetGradient(block, paint.Gradient);
    else if (paint.Tiled && hasTexture)
    {
        block.T[11] = glm::vec4(0.f, 0.f, static_cast<float>(paint.TileAxes), 1.f);
        block.T[12] = paint.TileRect;
    }
    ApplyClip(block);
    ApplyFx(block, hasTexture);

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
    key.State = StateFor(m_fx);
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
    ApplyFx(block, true);
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
    key.State = StateFor(m_fx);
    key.TextureIdx = bgfx::isValid(texture) ? texture.idx : bgfx::kInvalidHandle;
    key.StencilRef = UiRenderer::CurrentMaskDepth();
    m_list.AddQuad(key, v);
}

void UiPainter::GlyphRun(const UiGlyphQuad* quads, size_t count, const glm::vec4& color, float glyphScale, float dilation,
                         float softness, float strokeWidth, const glm::vec4& strokeColor)
{
    if (count == 0 || (color.a <= 0.f && strokeWidth <= 0.f))
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
        block.T[13] = glm::vec4(glyphScale, dilation, softness, strokeWidth);
        block.T[14] = strokeColor;
        ApplyClip(block);
        ApplyFx(block, false);
        const int prim = m_list.AddPrim(block);
        if (prim < 0)
            return;

        BatchKey key;
        key.State = StateFor(m_fx);
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

void UiPainter::Arc(const UiRect& rectIn, float thickness, float startRad, float sweepRad, bool roundCaps, const glm::vec4& color,
                    const UiGradientFill& gradient)
{
    UiRect rect = rectIn;
    if (rect.W <= 0.f || rect.H <= 0.f || thickness <= 0.f || sweepRad == 0.f)
        return;
    glm::mat3 m = m_transform;
    if (IsTranslationOnly())
    {
        SnapRect(rect.X + m[2][0], rect.Y + m[2][1], rect.X + rect.W + m[2][0], rect.Y + rect.H + m[2][1], rect.X, rect.Y, rect.W, rect.H);
        m = glm::mat3(1.f);
    }
    const float aa = 1.f / TransformScale(m);

    // A negative sweep runs counter-clockwise: swap the ends so the shader always sees a positive sweep.
    if (sweepRad < 0.f)
    {
        startRad += sweepRad;
        sweepRad = -sweepRad;
    }
    sweepRad = std::min(sweepRad, 6.2831853f);

    PrimBlock block;
    block.SetKind(PrimKind::Arc);
    block.T[1] = color;
    block.T[2] = glm::vec4(rect.W, rect.H, aa, thickness);
    block.T[13] = glm::vec4(startRad, sweepRad, 0.f, roundCaps ? 1.f : 0.f);
    if (gradient.Type > 0 && gradient.RampId >= 0)
        SetGradient(block, gradient);
    ApplyClip(block);
    ApplyFx(block, false);
    const int prim = m_list.AddPrim(block);
    if (prim < 0)
        return;

    const float e = aa;
    const float cx[4] = { -e, rect.W + e, rect.W + e, -e };
    const float cy[4] = { -e, -e, rect.H + e, rect.H + e };
    Vertex v[4];
    for (int i = 0; i < 4; ++i)
    {
        const glm::vec2 p = Apply(m, rect.X + cx[i], rect.Y + cy[i]);
        v[i] = { p.x, p.y, 0.f, 0.f, cx[i], cy[i], static_cast<float>(prim), 0.f };
    }
    BatchKey key;
    key.State = StateFor(m_fx);
    key.StencilRef = UiRenderer::CurrentMaskDepth();
    m_list.AddQuad(key, v);
}

void UiPainter::BoxShadow(const UiRect& elemIn, const UiCorners& radii, const UiShadowPaint& sh, float opacity)
{
    const uint8_t program = UiGpu::ProgramShadow();
    if (program == 0 || elemIn.W <= 0.f || elemIn.H <= 0.f)
        return;
    glm::vec4 color = sh.Color;
    color.a *= opacity;
    if (color.a <= 0.f)
        return;

    UiRect elem = elemIn;
    glm::mat3 m = m_transform;
    if (IsTranslationOnly())
    {
        SnapRect(elem.X + m[2][0], elem.Y + m[2][1], elem.X + elem.W + m[2][0], elem.Y + elem.H + m[2][1], elem.X, elem.Y, elem.W, elem.H);
        m = glm::mat3(1.f);
    }
    const float aa = 1.f / TransformScale(m);
    const float sigma = std::max(sh.Blur * 0.5f, 0.f);

    // The shadow box: the element box moved by the offset and grown (outer) or shrunk (inset) by the spread.
    const float grow = sh.Inset ? -sh.Spread : sh.Spread;
    UiRect box{ elem.X + sh.X - grow, elem.Y + sh.Y - grow, elem.W + 2.f * grow, elem.H + 2.f * grow };
    if (box.W <= 0.f || box.H <= 0.f)
    {
        if (!sh.Inset)
            return;
        box.W = std::max(box.W, 0.f);
        box.H = std::max(box.H, 0.f);
    }
    const float radiusGrow = sh.Inset ? -sh.Spread : sh.Spread;
    const float r[4] = { radii.TopLeft, radii.TopRight, radii.BottomRight, radii.BottomLeft };

    PrimBlock block;
    block.SetKind(PrimKind::BoxShadow);
    block.T[1] = color;
    block.T[2] = glm::vec4(box.W, box.H, aa, sigma);
    block.T[3] = glm::vec4(std::max(0.f, r[0] + radiusGrow), std::max(0.f, r[1] + radiusGrow), std::max(0.f, r[2] + radiusGrow),
                           std::max(0.f, r[3] + radiusGrow));
    block.T[4] = glm::vec4(elem.X - box.X, elem.Y - box.Y, elem.W, elem.H);
    block.T[5] = glm::vec4(r[0], r[1], r[2], r[3]);
    block.T[13] = glm::vec4(0.f, 0.f, sh.Inset ? 1.f : 0.f, 0.f);
    ApplyClip(block);
    ApplyFx(block, false);
    const int prim = m_list.AddPrim(block);
    if (prim < 0)
        return;

    // outer: the shadow box grown by 3 sigma; inset: the element box (the shader keeps only what lies inside it)
    UiRect quad;
    if (sh.Inset)
        quad = UiRect{ elem.X - aa, elem.Y - aa, elem.W + 2.f * aa, elem.H + 2.f * aa };
    else
    {
        const float reach = 3.f * std::max(sigma, 0.5f) + aa;
        quad = UiRect{ box.X - reach, box.Y - reach, box.W + 2.f * reach, box.H + 2.f * reach };
    }
    const float xs[4] = { quad.X, quad.X + quad.W, quad.X + quad.W, quad.X };
    const float ys[4] = { quad.Y, quad.Y, quad.Y + quad.H, quad.Y + quad.H };
    Vertex v[4];
    for (int i = 0; i < 4; ++i)
    {
        const glm::vec2 p = Apply(m, xs[i], ys[i]);
        v[i] = { p.x, p.y, 0.f, 0.f, xs[i] - box.X, ys[i] - box.Y, static_cast<float>(prim), 0.f };
    }
    BatchKey key;
    key.Program = program;
    key.State = StateFor(m_fx);
    key.StencilRef = UiRenderer::CurrentMaskDepth();
    m_list.AddQuad(key, v);
}

void UiPainter::Solid(const std::vector<PathVertex>& vertices, const std::vector<uint16_t>& indices, const glm::vec4& color)
{
    if (vertices.empty() || indices.size() < 3 || color.a <= 0.f)
        return;
    PrimBlock block;
    block.SetKind(PrimKind::Solid);
    block.T[1] = color;
    block.T[2] = glm::vec4(0.f, 0.f, 1.f / TransformScale(m_transform), 0.f);
    ApplyClip(block);
    ApplyFx(block, false);
    const int prim = m_list.AddPrim(block);
    if (prim < 0)
        return;
    std::vector<Vertex> v(vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        const glm::vec2 p = Apply(m_transform, vertices[i].P.x, vertices[i].P.y);
        v[i] = { p.x, p.y, 0.f, 0.f, vertices[i].Coverage, 0.f, static_cast<float>(prim), 0.f };
    }
    BatchKey key;
    key.State = StateFor(m_fx);
    key.StencilRef = UiRenderer::CurrentMaskDepth();
    m_list.AddTriangles(key, v.data(), static_cast<uint32_t>(v.size()), indices.data(), static_cast<uint32_t>(indices.size()));
}

void UiPainter::FillPolygon(const std::vector<glm::vec2>& points, const glm::vec4& color)
{
    const size_t n = points.size();
    if (n < 3)
        return;
    // orientation, so the fringe normals point outwards
    float area = 0.f;
    for (size_t i = 0; i < n; ++i)
    {
        const glm::vec2& a = points[i];
        const glm::vec2& b = points[(i + 1) % n];
        area += a.x * b.y - b.x * a.y;
    }
    const float side = area >= 0.f ? 1.f : -1.f;
    const float half = 0.5f / TransformScale(m_transform);

    std::vector<PathVertex> verts;
    std::vector<uint16_t> idx;
    for (size_t i = 0; i < n; ++i)
    {
        const glm::vec2 prev = points[(i + n - 1) % n], cur = points[i], next = points[(i + 1) % n];
        glm::vec2 e0 = cur - prev, e1 = next - cur;
        const float l0 = glm::length(e0), l1 = glm::length(e1);
        e0 = l0 > 0.f ? e0 / l0 : glm::vec2(1.f, 0.f);
        e1 = l1 > 0.f ? e1 / l1 : glm::vec2(1.f, 0.f);
        const glm::vec2 n0 = glm::vec2(e0.y, -e0.x) * side, n1 = glm::vec2(e1.y, -e1.x) * side;
        glm::vec2 normal = n0 + n1;
        const float len = glm::length(normal);
        normal = len > 0.0001f ? normal / len : n1;
        const float miter = std::min(1.f / std::max(glm::dot(normal, n1), 0.25f), 4.f);
        verts.push_back({ cur - normal * (half * miter), 1.f });
        verts.push_back({ cur + normal * (half * miter), 0.f });
    }
    for (size_t i = 1; i + 1 < n; ++i)
    {
        idx.push_back(0);
        idx.push_back(static_cast<uint16_t>(i * 2));
        idx.push_back(static_cast<uint16_t>((i + 1) * 2));
    }
    for (size_t i = 0; i < n; ++i)
    {
        const uint16_t a = static_cast<uint16_t>(i * 2), b = static_cast<uint16_t>(((i + 1) % n) * 2);
        idx.insert(idx.end(), { a, static_cast<uint16_t>(a + 1), static_cast<uint16_t>(b + 1), a, static_cast<uint16_t>(b + 1), b });
    }
    Solid(verts, idx, color);
}

void UiPainter::StrokePolyline(const std::vector<glm::vec2>& points, float width, const glm::vec4& color, bool closed)
{
    const size_t n = points.size();
    if (n < 2 || width <= 0.f)
        return;
    const float fringe = 0.5f / TransformScale(m_transform);
    const float hw = std::max(width * 0.5f, fringe);
    const float inner = std::min(1.f, width / (2.f * fringe));   // a stroke thinner than one pixel is drawn one pixel wide, fainter

    std::vector<PathVertex> verts;
    for (size_t i = 0; i < n; ++i)
    {
        const bool hasPrev = closed || i > 0, hasNext = closed || i + 1 < n;
        glm::vec2 dirIn(0.f), dirOut(0.f);
        if (hasPrev)
        {
            dirIn = points[i] - points[(i + n - 1) % n];
            const float l = glm::length(dirIn);
            dirIn = l > 0.f ? dirIn / l : glm::vec2(1.f, 0.f);
        }
        if (hasNext)
        {
            dirOut = points[(i + 1) % n] - points[i];
            const float l = glm::length(dirOut);
            dirOut = l > 0.f ? dirOut / l : glm::vec2(1.f, 0.f);
        }
        if (!hasPrev) dirIn = dirOut;
        if (!hasNext) dirOut = dirIn;
        const glm::vec2 n0(dirIn.y, -dirIn.x), n1(dirOut.y, -dirOut.x);
        glm::vec2 normal = n0 + n1;
        const float len = glm::length(normal);
        normal = len > 0.0001f ? normal / len : n1;
        const float miter = std::min(1.f / std::max(glm::dot(normal, n1), 0.25f), 4.f);
        const glm::vec2 c = points[i];
        verts.push_back({ c + normal * ((hw + fringe) * miter), 0.f });
        verts.push_back({ c + normal * ((hw - fringe) * miter), inner });
        verts.push_back({ c - normal * ((hw - fringe) * miter), inner });
        verts.push_back({ c - normal * ((hw + fringe) * miter), 0.f });
    }
    std::vector<uint16_t> idx;
    const size_t segments = closed ? n : n - 1;
    for (size_t i = 0; i < segments; ++i)
    {
        const uint16_t a = static_cast<uint16_t>(i * 4), b = static_cast<uint16_t>(((i + 1) % n) * 4);
        for (uint16_t k = 0; k < 3; ++k)
            idx.insert(idx.end(), { static_cast<uint16_t>(a + k), static_cast<uint16_t>(a + k + 1), static_cast<uint16_t>(b + k + 1),
                                    static_cast<uint16_t>(a + k), static_cast<uint16_t>(b + k + 1), static_cast<uint16_t>(b + k) });
    }
    Solid(verts, idx, color);
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

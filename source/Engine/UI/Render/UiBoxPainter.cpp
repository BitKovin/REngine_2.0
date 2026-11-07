#include "UiBoxPainter.h"

#include <UI/Elements/UiElem.h>
#include <UI/Render/UiGradientRamps.h>
#include <UI/Render/UiImageAtlas.h>

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float kPi = 3.14159265358979f;

    float TransformScale(const glm::mat3& m)
    {
        const float det = m[0][0] * m[1][1] - m[1][0] * m[0][1];
        const float scale = std::sqrt(std::fabs(det));
        return scale > 0.0001f ? scale : 1.f;
    }

    glm::vec2 Apply(const glm::mat3& m, glm::vec2 p)
    {
        return glm::vec2(m[0][0] * p.x + m[1][0] * p.y + m[2][0], m[0][1] * p.x + m[1][1] * p.y + m[2][1]);
    }

    // ── colour matrices ───────────────────────────────────────────────────────

    void Identity(float m[20])
    {
        for (int i = 0; i < 20; ++i) m[i] = 0.f;
        m[0] = m[6] = m[12] = m[18] = 1.f;
    }

    // out = a * b as 5 x 5 matrices with an implicit last row (0 0 0 0 1): b is applied first, a last.
    void Multiply(const float a[20], const float b[20], float out[20])
    {
        float r[20];
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 5; ++col)
            {
                float v = col == 4 ? a[row * 5 + 4] : 0.f;
                for (int k = 0; k < 4; ++k)
                    v += a[row * 5 + k] * b[k * 5 + col];
                r[row * 5 + col] = v;
            }
        for (int i = 0; i < 20; ++i) out[i] = r[i];
    }

    void Set3x3(float m[20], const float v[9])
    {
        Identity(m);
        for (int row = 0; row < 3; ++row)
            for (int col = 0; col < 3; ++col)
                m[row * 5 + col] = v[row * 3 + col];
    }

    // The W3C Filter Effects matrices.
    void FunctionMatrix(const UiFilterFn& f, float m[20])
    {
        Identity(m);
        const float a = f.Amount;
        switch (f.Type)
        {
        case UiFilterType::Brightness:
            m[0] = m[6] = m[12] = a;
            break;
        case UiFilterType::Contrast:
            m[0] = m[6] = m[12] = a;
            m[4] = m[9] = m[14] = 0.5f - 0.5f * a;
            break;
        case UiFilterType::Invert:
            m[0] = m[6] = m[12] = 1.f - 2.f * a;
            m[4] = m[9] = m[14] = a;
            break;
        case UiFilterType::Saturate:
        {
            const float v[9] = { 0.213f + 0.787f * a, 0.715f - 0.715f * a, 0.072f - 0.072f * a,
                                 0.213f - 0.213f * a, 0.715f + 0.285f * a, 0.072f - 0.072f * a,
                                 0.213f - 0.213f * a, 0.715f - 0.715f * a, 0.072f + 0.928f * a };
            Set3x3(m, v);
            break;
        }
        case UiFilterType::Grayscale:
        {
            const float k = 1.f - std::clamp(a, 0.f, 1.f);
            const float v[9] = { 0.2126f + 0.7874f * k, 0.7152f - 0.7152f * k, 0.0722f - 0.0722f * k,
                                 0.2126f - 0.2126f * k, 0.7152f + 0.2848f * k, 0.0722f - 0.0722f * k,
                                 0.2126f - 0.2126f * k, 0.7152f - 0.7152f * k, 0.0722f + 0.9278f * k };
            Set3x3(m, v);
            break;
        }
        case UiFilterType::Sepia:
        {
            const float k = 1.f - std::clamp(a, 0.f, 1.f);
            const float v[9] = { 0.393f + 0.607f * k, 0.769f - 0.769f * k, 0.189f - 0.189f * k,
                                 0.349f - 0.349f * k, 0.686f + 0.314f * k, 0.168f - 0.168f * k,
                                 0.272f - 0.272f * k, 0.534f - 0.534f * k, 0.131f + 0.869f * k };
            Set3x3(m, v);
            break;
        }
        case UiFilterType::HueRotate:
        {
            const float c = std::cos(a * kPi / 180.f), s = std::sin(a * kPi / 180.f);
            const float v[9] = { 0.213f + c * 0.787f - s * 0.213f, 0.715f - c * 0.715f - s * 0.715f, 0.072f - c * 0.072f + s * 0.928f,
                                 0.213f - c * 0.213f + s * 0.143f, 0.715f + c * 0.285f + s * 0.140f, 0.072f - c * 0.072f - s * 0.283f,
                                 0.213f - c * 0.213f - s * 0.787f, 0.715f - c * 0.715f + s * 0.715f, 0.072f + c * 0.928f + s * 0.072f };
            Set3x3(m, v);
            break;
        }
        }
    }

    // ── gradients ─────────────────────────────────────────────────────────────

    // Resolves a gradient for the box (gx, gy, gw, gh) (element-local units) into ramp and geometry.
    bool BuildGradient(const UiGradient& g, float gx, float gy, float gw, float gh, bool allowRepeat, UiGradientFill& out)
    {
        if (gw <= 0.f || gh <= 0.f || g.Stops.size() < 2)
            return false;
        out = UiGradientFill();
        float lineLength = 1.f;   // what px stop positions are relative to
        switch (g.Kind)
        {
        case UiGradKind::Linear:
        {
            glm::vec2 d;
            if (g.ToCorner)
                d = glm::normalize(glm::vec2(static_cast<float>(g.CornerX) * gh, static_cast<float>(g.CornerY) * gw));
            else
            {
                const float rad = g.AngleDeg * kPi / 180.f;
                d = glm::vec2(std::sin(rad), -std::cos(rad));
            }
            lineLength = std::fabs(gw * d.x) + std::fabs(gh * d.y);
            const glm::vec2 c(gx + gw * 0.5f, gy + gh * 0.5f);
            const glm::vec2 p0 = c - d * (lineLength * 0.5f), p1 = c + d * (lineLength * 0.5f);
            out.Type = 1;
            out.Geometry = glm::vec4(p0.x, p0.y, p1.x, p1.y);
            break;
        }
        case UiGradKind::Radial:
        {
            const float cx = gx + g.PosX.Resolve(gw), cy = gy + g.PosY.Resolve(gh);
            const float dxn = std::min(cx - gx, gx + gw - cx), dyn = std::min(cy - gy, gy + gh - cy);
            const float dxf = std::max(cx - gx, gx + gw - cx), dyf = std::max(cy - gy, gy + gh - cy);
            float rx, ry;
            switch (g.SizeKeyword)
            {
            case 1: rx = dxn; ry = dyn; if (g.Circle) rx = ry = std::min(dxn, dyn); break;
            case 2: rx = dxf; ry = dyf; if (g.Circle) rx = ry = std::max(dxf, dyf); break;
            case 3: rx = dxn * 1.41421356f; ry = dyn * 1.41421356f; if (g.Circle) rx = ry = std::sqrt(dxn * dxn + dyn * dyn); break;
            case 4:
                rx = g.RadiusX.Resolve(gw);
                ry = g.Circle ? rx : g.RadiusY.Resolve(gh);
                break;
            default: rx = dxf * 1.41421356f; ry = dyf * 1.41421356f; if (g.Circle) rx = ry = std::sqrt(dxf * dxf + dyf * dyf); break;
            }
            rx = std::max(rx, 0.01f);
            ry = std::max(ry, 0.01f);
            lineLength = rx;
            out.Type = 2;
            out.Geometry = glm::vec4(cx, cy, rx, ry);
            break;
        }
        case UiGradKind::Conic:
            out.Type = 3;
            out.Geometry = glm::vec4(gx + g.PosX.Resolve(gw), gy + g.PosY.Resolve(gh), g.FromDeg * kPi / 180.f, 0.f);
            lineLength = 1.f;
            break;
        case UiGradKind::None:
            return false;
        }

        // stop positions along the line (0..1); unspecified ones are spread evenly between their neighbours
        const size_t n = g.Stops.size();
        std::vector<float> pos(n, 0.f);
        std::vector<bool> has(n, false);
        for (size_t i = 0; i < n; ++i)
            if (g.Stops[i].HasPos)
            {
                const UiLength& l = g.Stops[i].Pos;
                pos[i] = l.Kind == UiLenKind::Percent ? l.Percent * 0.01f : l.Px / std::max(lineLength, 0.0001f);
                has[i] = true;
            }
        if (!has[0]) { pos[0] = 0.f; has[0] = true; }
        if (!has[n - 1]) { pos[n - 1] = 1.f; has[n - 1] = true; }
        float previous = pos[0];
        for (size_t i = 1; i < n; ++i)
            if (has[i])
            {
                pos[i] = std::max(pos[i], previous);
                previous = pos[i];
            }
        for (size_t i = 1; i + 1 < n; ++i)
        {
            if (has[i])
                continue;
            size_t j = i;
            while (!has[j]) ++j;
            const size_t k = i - 1;   // the previous stop already has a position by now
            const float step = (pos[j] - pos[k]) / static_cast<float>(j - k);
            pos[i] = pos[k] + step;
            has[i] = true;
        }

        std::vector<UiRampStop> stops(n);
        for (size_t i = 0; i < n; ++i)
            stops[i] = UiRampStop{ g.Stops[i].Color, pos[i] };
        if (g.Repeating && allowRepeat)
        {
            const float first = pos[0], period = pos[n - 1] - pos[0];
            if (period > 0.0001f)
            {
                for (UiRampStop& s : stops)
                    s.Pos = (s.Pos - first) / period;
                out.Period = period;
                out.Start = first;
            }
        }
        out.RampId = UiGradientRamps::Id(stops);
        return out.RampId >= 0;
    }

    // ── helpers ───────────────────────────────────────────────────────────────

    UiCorners InnerRadii(const UiCorners& r, const UiEdges& bw)
    {
        return UiCorners{ std::max(0.f, r.TopLeft - std::max(bw.Left, bw.Top)), std::max(0.f, r.TopRight - std::max(bw.Top, bw.Right)),
                          std::max(0.f, r.BottomRight - std::max(bw.Right, bw.Bottom)), std::max(0.f, r.BottomLeft - std::max(bw.Bottom, bw.Left)) };
    }

    void SetPixelatedSize(UiPainter& p, const UiAtlasImage& image)
    {
        UiPainterFx fx = p.Fx();
        fx.PixelatedSize = glm::vec2(static_cast<float>(std::max(image.TextureWidth, 1)), static_cast<float>(std::max(image.TextureHeight, 1)));
        p.SetFx(fx);
    }

    UiShadowPaint ToPaint(const UiShadow& s)
    {
        UiShadowPaint paint;
        paint.X = s.X; paint.Y = s.Y; paint.Blur = s.Blur; paint.Spread = s.Spread; paint.Color = s.Color; paint.Inset = s.Inset;
        return paint;
    }

    struct Area { float X, Y, W, H; };

    // ── background layers ─────────────────────────────────────────────────────

    void PaintGradientLayer(UiPainter& p, const UiBoxPaint& base, const UiGradient& gradient, const Area& area, const UiBgSize& size,
                            const UiBgPos& position, const UiBgRepeat& repeat)
    {
        float tw = area.W, th = area.H;
        if (size.M == UiBgSize::Mode::Explicit)
        {
            tw = size.WAuto ? area.W : size.W.Resolve(area.W);
            th = size.HAuto ? area.H : size.H.Resolve(area.H);
        }
        if (tw <= 0.f || th <= 0.f)
            return;
        const float ox = area.X + position.X.Px + position.X.Percent * 0.01f * (area.W - tw);
        const float oy = area.Y + position.Y.Px + position.Y.Percent * 0.01f * (area.H - th);
        const bool whole = tw >= area.W && th >= area.H && ox <= area.X + 0.01f && oy <= area.Y + 0.01f;

        // tiles that touch the area; the gradient of each is a separate primitive, clipped to its tile
        const int maxTiles = 64;
        int x0 = 0, x1 = 0, y0 = 0, y1 = 0;
        if (repeat.X) { x0 = static_cast<int>(std::floor((area.X - ox) / tw)); x1 = static_cast<int>(std::ceil((area.X + area.W - ox) / tw)) - 1; }
        if (repeat.Y) { y0 = static_cast<int>(std::floor((area.Y - oy) / th)); y1 = static_cast<int>(std::ceil((area.Y + area.H - oy) / th)) - 1; }
        if ((x1 - x0 + 1) * (y1 - y0 + 1) > maxTiles)
            return;
        for (int ty = y0; ty <= y1; ++ty)
            for (int tx = x0; tx <= x1; ++tx)
            {
                const float px = ox + static_cast<float>(tx) * tw, py = oy + static_cast<float>(ty) * th;
                UiGradientFill fill;
                if (!BuildGradient(gradient, px, py, tw, th, true, fill))
                    return;
                UiBoxPaint paint = base;
                paint.Gradient = fill;
                if (whole)
                {
                    p.Box(UiRect{ 0.f, 0.f, base.BorderWidth.Left + area.W + base.BorderWidth.Right, base.BorderWidth.Top + area.H + base.BorderWidth.Bottom }, paint);
                    continue;
                }
                const float cx0 = std::max(px, area.X), cy0 = std::max(py, area.Y);
                const float cx1 = std::min(px + tw, area.X + area.W), cy1 = std::min(py + th, area.Y + area.H);
                if (cx1 <= cx0 || cy1 <= cy0)
                    continue;
                p.PushClip(UiRect{ cx0, cy0, cx1 - cx0, cy1 - cy0 }, UiCorners());
                p.Box(UiRect{ 0.f, 0.f, base.BorderWidth.Left + area.W + base.BorderWidth.Right, base.BorderWidth.Top + area.H + base.BorderWidth.Bottom }, paint);
                p.PopClip();
            }
    }

    void PaintImageLayer(UiElem& e, UiPainter& p, const UiBoxPaint& base, const std::string& url, const Area& area, const UiBgSize& size,
                         const UiBgPos& position, const UiBgRepeat& repeat, bool pixelated)
    {
        const UiAtlasImage image = UiImageAtlas::Get(url);
        if (image.Pending)
        {
            e.MarkPaintDirty();
            return;
        }
        if (!image.Valid || image.Width <= 0 || image.Height <= 0)
            return;
        const float iw = static_cast<float>(image.Width), ih = static_cast<float>(image.Height);
        float tw = iw, th = ih;
        switch (size.M)
        {
        case UiBgSize::Mode::Cover: { const float s = std::max(area.W / iw, area.H / ih); tw = iw * s; th = ih * s; break; }
        case UiBgSize::Mode::Contain: { const float s = std::min(area.W / iw, area.H / ih); tw = iw * s; th = ih * s; break; }
        case UiBgSize::Mode::Explicit:
            if (!size.WAuto && !size.HAuto) { tw = size.W.Resolve(area.W); th = size.H.Resolve(area.H); }
            else if (!size.WAuto) { tw = size.W.Resolve(area.W); th = tw * ih / iw; }
            else if (!size.HAuto) { th = size.H.Resolve(area.H); tw = th * iw / ih; }
            break;
        case UiBgSize::Mode::Auto: break;
        }
        if (tw <= 0.f || th <= 0.f)
            return;
        const float ox = area.X + position.X.Px + position.X.Percent * 0.01f * (area.W - tw);
        const float oy = area.Y + position.Y.Px + position.Y.Percent * 0.01f * (area.H - th);

        UiBoxPaint paint = base;
        paint.FillTexture = image.Texture;
        paint.Tiled = true;
        paint.TileRect = image.Uv;
        paint.TileAxes = (repeat.X ? 1 : 0) | (repeat.Y ? 2 : 0);
        // tile coordinate at the corners of the primitive's rectangle (the border box)
        paint.FillUv = glm::vec4((0.f - ox) / tw, (0.f - oy) / th, (area.W + area.X + base.BorderWidth.Right - ox) / tw,
                                 (area.H + area.Y + base.BorderWidth.Bottom - oy) / th);
        const UiPainterFx saved = p.Fx();
        if (pixelated)
            SetPixelatedSize(p, image);
        p.Box(UiRect{ 0.f, 0.f, base.BorderWidth.Left + area.W + base.BorderWidth.Right, base.BorderWidth.Top + area.H + base.BorderWidth.Bottom }, paint);
        p.SetFx(saved);
    }

    // ── border image ──────────────────────────────────────────────────────────

    void PaintBorderImage(UiElem& e, UiPainter& p, float opacity, const UiEdges& bw, float W, float H)
    {
        const UiBorderImage& bi = e.Computed().Fx.BorderImage;
        const UiAtlasImage image = UiImageAtlas::Get(bi.Src);
        if (image.Pending)
        {
            e.MarkPaintDirty();
            return;
        }
        if (!image.Valid || image.Width <= 0 || image.Height <= 0)
            return;
        const float iw = static_cast<float>(image.Width), ih = static_cast<float>(image.Height);
        // slices in texels: top, right, bottom, left
        float slice[4];
        for (int i = 0; i < 4; ++i)
            slice[i] = bi.SlicePercent[i] ? bi.Slice[i] * 0.01f * ((i % 2 == 0) ? ih : iw) : bi.Slice[i];
        if (slice[0] + slice[2] > ih) { const float k = ih / (slice[0] + slice[2]); slice[0] *= k; slice[2] *= k; }
        if (slice[1] + slice[3] > iw) { const float k = iw / (slice[1] + slice[3]); slice[1] *= k; slice[3] *= k; }
        const float dest[4] = { bw.Top * bi.Width[0], bw.Right * bi.Width[1], bw.Bottom * bi.Width[2], bw.Left * bi.Width[3] };

        const float srcX[4] = { 0.f, slice[3], iw - slice[1], iw };
        const float srcY[4] = { 0.f, slice[0], ih - slice[2], ih };
        const float dstX[4] = { 0.f, dest[3], W - dest[1], W };
        const float dstY[4] = { 0.f, dest[0], H - dest[2], H };
        auto u = [&](float x) { return image.Uv.x + x / iw * (image.Uv.z - image.Uv.x); };
        auto v = [&](float y) { return image.Uv.y + y / ih * (image.Uv.w - image.Uv.y); };
        const glm::vec4 tint(1.f, 1.f, 1.f, opacity);

        const UiPainterFx saved = p.Fx();
        if (saved.Pixelated)
            SetPixelatedSize(p, image);
        for (int row = 0; row < 3; ++row)
            for (int col = 0; col < 3; ++col)
            {
                if (row == 1 && col == 1 && !bi.Fill)
                    continue;
                const float sw = srcX[col + 1] - srcX[col], sh = srcY[row + 1] - srcY[row];
                const float dw = dstX[col + 1] - dstX[col], dh = dstY[row + 1] - dstY[row];
                if (sw <= 0.f || sh <= 0.f || dw <= 0.f || dh <= 0.f)
                    continue;
                const UiRect rect{ dstX[col], dstY[row], dw, dh };
                const glm::vec4 uv(u(srcX[col]), v(srcY[row]), u(srcX[col + 1]), v(srcY[row + 1]));
                const bool horizontalEdge = row != 1 && col == 1;   // top and bottom: runs along x
                const bool verticalEdge = row == 1 && col != 1;
                if (bi.Round && (horizontalEdge || verticalEdge))
                {
                    // tile scaled to the border band, count rounded so a whole number fits
                    const float scale = horizontalEdge ? dh / sh : dw / sw;
                    const float tile = (horizontalEdge ? sw : sh) * scale;
                    const float length = horizontalEdge ? dw : dh;
                    const float count = std::max(1.f, std::round(length / std::max(tile, 0.01f)));
                    UiBoxPaint paint;
                    paint.Fill = tint;
                    paint.FillTexture = image.Texture;
                    paint.Tiled = true;
                    paint.TileRect = uv;
                    paint.TileAxes = horizontalEdge ? 1 : 2;
                    paint.FillUv = horizontalEdge ? glm::vec4(0.f, 0.f, count, 1.f) : glm::vec4(0.f, 0.f, 1.f, count);
                    p.Box(rect, paint);
                }
                else
                    p.Image(rect, image.Texture, uv, tint);
            }
        p.SetFx(saved);
    }
}

bool UiBoxPainter::FilterMatrix(const UiFxStyle& fx, float out[20])
{
    if (fx.Filter.empty() && fx.WashColor.a <= 0.f)
        return false;
    Identity(out);
    for (const UiFilterFn& f : fx.Filter)
    {
        float m[20];
        FunctionMatrix(f, m);
        Multiply(m, out, out);   // later functions act on the result of earlier ones
    }
    if (fx.WashColor.a > 0.f)
    {
        // colour matrix lerp towards the wash colour by its alpha
        float m[20];
        Identity(m);
        const float a = fx.WashColor.a;
        m[0] = m[6] = m[12] = 1.f - a;
        m[4] = a * fx.WashColor.r;
        m[9] = a * fx.WashColor.g;
        m[14] = a * fx.WashColor.b;
        Multiply(m, out, out);
    }
    return true;
}

UiBoxPainter::Scope UiBoxPainter::Begin(UiElem& e, UiPainter& p)
{
    Scope scope;
    scope.Saved = p.Fx();
    const UiFxStyle& fx = e.Computed().Fx;
    const float W = e.BorderRect.W, H = e.BorderRect.H;
    UiPainterFx f = scope.Saved;
    bool changed = false;

    float own[20];
    if (FilterMatrix(fx, own))
    {
        float total[20];
        if (f.HasMatrix)
            Multiply(f.Matrix, own, total);   // the parent's filter acts on the result of this element's
        else
            for (int i = 0; i < 20; ++i) total[i] = own[i];
        f.HasMatrix = true;
        for (int i = 0; i < 20; ++i) f.Matrix[i] = total[i];
        f.ColorMatrixAux = p.AddColorMatrix(total);
        changed = true;
    }
    if (fx.MixBlend != UiBlend::Normal)
    {
        f.Blend = static_cast<UiBlendMode>(static_cast<int>(fx.MixBlend));
        changed = true;
    }
    const bool pixelated = fx.ImageRendering == UiImageRendering::Pixelated;
    if (pixelated != f.Pixelated)
    {
        f.Pixelated = pixelated;
        changed = true;
    }

    const glm::mat3& m = p.Transform();
    if (fx.MaskImage.Kind == UiGradKind::Linear || fx.MaskImage.Kind == UiGradKind::Radial)
    {
        UiGradientFill gradient;
        if (BuildGradient(fx.MaskImage, 0.f, 0.f, W, H, false, gradient))
        {
            glm::vec4 texel;
            if (gradient.Type == 1)
            {
                const glm::vec2 a = Apply(m, glm::vec2(gradient.Geometry.x, gradient.Geometry.y));
                const glm::vec2 b = Apply(m, glm::vec2(gradient.Geometry.z, gradient.Geometry.w));
                texel = glm::vec4(a.x, a.y, b.x, b.y);
            }
            else
            {
                const glm::vec2 c = Apply(m, glm::vec2(gradient.Geometry.x, gradient.Geometry.y));
                const float sx = glm::length(glm::vec2(m[0][0], m[0][1])), sy = glm::length(glm::vec2(m[1][0], m[1][1]));
                texel = glm::vec4(c.x, c.y, gradient.Geometry.z * sx, gradient.Geometry.w * sy);
            }
            f.MaskType = gradient.Type;
            f.MaskRampId = gradient.RampId;
            f.MaskAux = p.AddAux(&texel, 1);
            changed = true;
        }
    }

    const UiClipPath& cp = fx.ClipPath;
    if (cp.Kind == UiClipPath::Shape::Inset)
    {
        const float top = cp.A[0].Resolve(H), right = cp.A[1].Resolve(W), bottom = cp.A[2].Resolve(H), left = cp.A[3].Resolve(W);
        const float side = std::min(W, H);
        const UiCorners radii{ cp.Round[0].Resolve(side), cp.Round[1].Resolve(side), cp.Round[2].Resolve(side), cp.Round[3].Resolve(side) };
        const UiRect rect{ left, top, std::max(0.f, W - left - right), std::max(0.f, H - top - bottom) };
        p.PushClip(rect, radii);
        scope.ClipPushed = true;
    }
    else if (cp.Kind == UiClipPath::Shape::Circle || cp.Kind == UiClipPath::Shape::Ellipse)
    {
        const float cx = cp.CX.Resolve(W), cy = cp.CY.Resolve(H);
        const glm::vec2 centre = Apply(m, glm::vec2(cx, cy));
        const float sx = glm::length(glm::vec2(m[0][0], m[0][1])), sy = glm::length(glm::vec2(m[1][0], m[1][1]));
        glm::vec4 texels[2];
        if (cp.Kind == UiClipPath::Shape::Circle)
        {
            const float basis = std::sqrt((W * W + H * H) * 0.5f);
            const float closest = std::min(std::min(cx, W - cx), std::min(cy, H - cy));
            const float r = cp.HasRadius[0] ? cp.A[0].Resolve(basis) : closest;
            texels[0] = glm::vec4(1.f, centre.x, centre.y, r * TransformScale(m));
            texels[1] = glm::vec4(0.f);
        }
        else
        {
            const float rx = cp.HasRadius[0] ? cp.A[0].Resolve(W) : std::min(cx, W - cx);
            const float ry = cp.HasRadius[1] ? cp.A[1].Resolve(H) : std::min(cy, H - cy);
            texels[0] = glm::vec4(2.f, centre.x, centre.y, rx * sx);
            texels[1] = glm::vec4(rx * sx, ry * sy, 0.f, 0.f);
        }
        f.ClipShapeAux = p.AddAux(texels, 2);
        changed = true;
    }

    if (changed)
        p.SetFx(f);
    return scope;
}

void UiBoxPainter::End(UiPainter& p, const Scope& scope)
{
    if (scope.ClipPushed)
        p.PopClip();
    p.SetFx(scope.Saved);
}

void UiBoxPainter::PaintBox(UiElem& e, UiPainter& p, float op, const UiCorners& radii)
{
    const UiComputedStyle& s = e.Computed();
    const UiFxStyle& fx = s.Fx;
    const float W = e.BorderRect.W, H = e.BorderRect.H;
    const UiEdges bw{ s.EffectiveBorder(0), s.EffectiveBorder(1), s.EffectiveBorder(2), s.EffectiveBorder(3) };
    const bool anyBorder = bw.Top > 0.f || bw.Right > 0.f || bw.Bottom > 0.f || bw.Left > 0.f;
    const bool hasBorderImage = !fx.BorderImage.Src.empty();

    // outer shadows: the first one listed is on top, so they are drawn last to first
    for (size_t i = fx.BoxShadow.size(); i-- > 0;)
        if (!fx.BoxShadow[i].Inset)
            p.BoxShadow(UiRect{ 0.f, 0.f, W, H }, radii, ToPaint(fx.BoxShadow[i]), op);

    // background colour and border
    UiBoxPaint paint;
    paint.Fill = s.BackgroundColor;
    paint.Fill.a *= op;
    paint.Radii = radii;
    if (!hasBorderImage)
    {
        paint.BorderWidth = bw;
        for (int i = 0; i < 4; ++i)
        {
            paint.BorderColor[i] = s.BorderColor[i];
            paint.BorderColor[i].a *= op;
        }
    }
    if ((s.BackgroundColor.a > 0.f && e.DrawsBoxBackground()) || (anyBorder && !hasBorderImage))
        p.Box(UiRect{ 0.f, 0.f, W, H }, paint);

    // background layers: the first one listed is on top
    if (!fx.BgImages.empty())
    {
        const UiEdges inner = hasBorderImage ? UiEdges() : bw;
        UiBoxPaint layer;
        layer.Fill = glm::vec4(1.f, 1.f, 1.f, op);
        layer.Radii = radii;
        layer.BorderWidth = inner;   // the layer fills the padding box only; the border colours stay transparent
        const Area area{ inner.Left, inner.Top, std::max(0.f, W - inner.Left - inner.Right), std::max(0.f, H - inner.Top - inner.Bottom) };
        const bool pixelated = p.Fx().Pixelated;
        for (size_t i = fx.BgImages.size(); i-- > 0;)
        {
            const UiBgImage& image = fx.BgImages[i];
            const UiBgSize size = fx.BgSizes.empty() ? UiBgSize() : fx.BgSizes[i % fx.BgSizes.size()];
            const UiBgPos position = fx.BgPositions.empty() ? UiBgPos() : fx.BgPositions[i % fx.BgPositions.size()];
            const UiBgRepeat repeat = fx.BgRepeats.empty() ? UiBgRepeat() : fx.BgRepeats[i % fx.BgRepeats.size()];
            if (image.Kind == UiBgImage::Type::Gradient)
                PaintGradientLayer(p, layer, image.Gradient, area, size, position, repeat);
            else
                PaintImageLayer(e, p, layer, image.Url, area, size, position, repeat, pixelated);
        }
    }

    if (hasBorderImage)
        PaintBorderImage(e, p, op, bw, W, H);

    // inset shadows sit on the padding box
    bool anyInset = false;
    for (const UiShadow& sh : fx.BoxShadow)
        anyInset = anyInset || sh.Inset;
    if (anyInset)
    {
        const UiRect padding{ bw.Left, bw.Top, std::max(0.f, W - bw.Left - bw.Right), std::max(0.f, H - bw.Top - bw.Bottom) };
        const UiCorners innerRadii = InnerRadii(radii, bw);
        for (size_t i = fx.BoxShadow.size(); i-- > 0;)
            if (fx.BoxShadow[i].Inset)
                p.BoxShadow(padding, innerRadii, ToPaint(fx.BoxShadow[i]), op);
    }
}

void UiBoxPainter::PaintOutline(UiElem& e, UiPainter& p, float op, const UiCorners& radii)
{
    const UiFxStyle& fx = e.Computed().Fx;
    if (!fx.OutlineSolid || fx.OutlineWidth <= 0.f || fx.OutlineColor.a <= 0.f)
        return;
    const float grow = fx.OutlineOffset + fx.OutlineWidth;
    const float W = e.BorderRect.W + 2.f * grow, H = e.BorderRect.H + 2.f * grow;
    if (W <= 0.f || H <= 0.f)
        return;
    UiBoxPaint paint;
    paint.Radii = UiCorners{ std::max(0.f, radii.TopLeft + grow), std::max(0.f, radii.TopRight + grow),
                             std::max(0.f, radii.BottomRight + grow), std::max(0.f, radii.BottomLeft + grow) };
    paint.BorderWidth = UiEdges{ fx.OutlineWidth, fx.OutlineWidth, fx.OutlineWidth, fx.OutlineWidth };
    for (glm::vec4& c : paint.BorderColor)
    {
        c = fx.OutlineColor;
        c.a *= op;
    }
    p.Box(UiRect{ -grow, -grow, W, H }, paint);
}

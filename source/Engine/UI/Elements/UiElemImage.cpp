#include "UiElemImage.h"

#include <AssetRegistry.h>
#include <Texture.hpp>
#include <UI/Render/UiImageAtlas.h>
#include <UI/Render/UiSvg.h>

#include <algorithm>

bool UiElemImage::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "src")
    {
        m_src = value;
        m_texture = nullptr;
        m_standalone = false;
        MarkLayoutDirty();
        return true;
    }
    return false;
}

bool UiElemImage::Fetch(UiAtlasImage& out)
{
    if (m_src.empty())
        return false;
    if (!m_standalone)
    {
        out = UiImageAtlas::Get(m_src);
        if (out.Valid || out.Pending)
            return out.Valid;
        // decoding failed (a missing file): the asset registry's own handling, as before
        m_standalone = true;
        m_texture = AssetRegistry::GetTextureFromFile(m_src);
    }
    if (m_texture == nullptr || !m_texture->valid)
        return false;
    out = UiAtlasImage();
    out.Texture = m_texture->getHandle();
    out.Width = out.TextureWidth = m_texture->width;
    out.Height = out.TextureHeight = m_texture->height;
    out.Valid = true;
    return true;
}

glm::vec2 UiElemImage::MeasureContent(float, float)
{
    UiAtlasImage image;
    if (Fetch(image))
        return glm::vec2((float)image.Width, (float)image.Height);
    if (!m_src.empty())
        MarkLayoutDirty();   // measure again next frame until the texture has loaded
    return glm::vec2(0.f);
}

void UiElemImage::PaintContent(UiPainter& painter, const UiRect& c)
{
    UiAtlasImage image;
    if (!Fetch(image))
    {
        if (!m_src.empty())
            MarkPaintDirty();
        return;
    }
    const float iw = (float)image.Width, ih = (float)image.Height;
    if (iw <= 0.f || ih <= 0.f || c.W <= 0.f || c.H <= 0.f)
        return;

    // The scaled image rectangle (centred on the content box), then the part of it inside the content box.
    float sw = c.W, sh = c.H;
    switch (Computed().ObjectFit)
    {
    case UiObjectFit::Fill: break;
    case UiObjectFit::Contain: { const float s = std::min(c.W / iw, c.H / ih); sw = iw * s; sh = ih * s; break; }
    case UiObjectFit::Cover: { const float s = std::max(c.W / iw, c.H / ih); sw = iw * s; sh = ih * s; break; }
    case UiObjectFit::None: sw = iw; sh = ih; break;
    case UiObjectFit::ScaleDown:
    {
        const float s = std::min(c.W / iw, c.H / ih);
        if (s < 1.f) { sw = iw * s; sh = ih * s; } else { sw = iw; sh = ih; }
        break;
    }
    }
    if (!m_standalone && UiSvg::IsSvgPath(m_src))
        image = UiImageAtlas::Get(m_src, std::max(sw, sh) * painter.PixelScale());   // a drawing is rasterised for the size it is shown at
    const UiBgPos& position = Computed().Fx.ObjectPosition;
    const float ix = c.X + position.X.Px + position.X.Percent * 0.01f * (c.W - sw);
    const float iy = c.Y + position.Y.Px + position.Y.Percent * 0.01f * (c.H - sh);
    const float x0 = std::max(ix, c.X), y0 = std::max(iy, c.Y);
    const float x1 = std::min(ix + sw, c.X + c.W), y1 = std::min(iy + sh, c.Y + c.H);
    if (x1 <= x0 || y1 <= y0)
        return;
    glm::vec4 uv((x0 - ix) / sw, (y0 - iy) / sh, (x1 - ix) / sw, (y1 - iy) / sh);
    // the image may be a part of an atlas page
    const glm::vec2 uvSize(image.Uv.z - image.Uv.x, image.Uv.w - image.Uv.y);
    uv = glm::vec4(image.Uv.x + uv.x * uvSize.x, image.Uv.y + uv.y * uvSize.y, image.Uv.x + uv.z * uvSize.x, image.Uv.y + uv.w * uvSize.y);
    const UiPainterFx saved = painter.Fx();
    if (saved.Pixelated)
    {
        UiPainterFx fx = saved;
        fx.PixelatedSize = glm::vec2((float)std::max(image.TextureWidth, 1), (float)std::max(image.TextureHeight, 1));
        painter.SetFx(fx);
    }
    painter.Image(UiRect{ x0, y0, x1 - x0, y1 - y0 }, image.Texture, uv, glm::vec4(1.f, 1.f, 1.f, m_paintOpacity));
    painter.SetFx(saved);
}

#include "UiElemImage.h"

#include <AssetRegistry.h>
#include <Texture.hpp>

#include <algorithm>

bool UiElemImage::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "src")
    {
        m_src = value;
        m_texture = value.empty() ? nullptr : AssetRegistry::GetTextureFromFile(value);
        MarkLayoutDirty();
        return true;
    }
    return false;
}

glm::vec2 UiElemImage::MeasureContent(float, float)
{
    if (m_texture != nullptr && m_texture->valid)
        return glm::vec2((float)m_texture->width, (float)m_texture->height);
    if (m_texture != nullptr)
        MarkLayoutDirty();   // measure again next frame until the texture has loaded
    return glm::vec2(0.f);
}

void UiElemImage::PaintContent(UiPainter& painter, const UiRect& c)
{
    if (m_texture == nullptr)
        return;
    if (!m_texture->valid)
    {
        MarkPaintDirty();
        return;
    }
    const float iw = (float)m_texture->width, ih = (float)m_texture->height;
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
    const float ix = c.X + (c.W - sw) * 0.5f, iy = c.Y + (c.H - sh) * 0.5f;
    const float x0 = std::max(ix, c.X), y0 = std::max(iy, c.Y);
    const float x1 = std::min(ix + sw, c.X + c.W), y1 = std::min(iy + sh, c.Y + c.H);
    if (x1 <= x0 || y1 <= y0)
        return;
    const glm::vec4 uv((x0 - ix) / sw, (y0 - iy) / sh, (x1 - ix) / sw, (y1 - iy) / sh);
    painter.Image(UiRect{ x0, y0, x1 - x0, y1 - y0 }, m_texture->getHandle(), uv, glm::vec4(1.f, 1.f, 1.f, m_paintOpacity));
}

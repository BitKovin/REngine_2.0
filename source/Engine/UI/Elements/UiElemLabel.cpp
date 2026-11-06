#include "UiElemLabel.h"

#include <AssetRegistry.h>
#include <Texture.hpp>
#include <UI/Menus/UiInputPrompts.h>

#include <cmath>

bool UiElemLabel::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "text")
    {
        SetResolvedText(value);
        return true;
    }
    if (name == "html")
    {
        const bool on = !(value.empty() || value == "0" || value == "false");
        if (on != m_html)
        {
            m_html = on;
            MarkLayoutDirty();
        }
        return true;
    }
    return false;
}

void UiElemLabel::SetResolvedText(const std::string& text)
{
    if (m_text == text)
        return;
    m_text = text;
    MarkLayoutDirty();
}

glm::vec2 UiElemLabel::MeasureContent(float availableWidth, float availableHeight)
{
    const UiComputedStyle& s = Computed();
    UiTextParams p;
    p.Text = m_text;
    p.Html = m_html;
    p.Family = s.FontFamily;
    p.Weight = s.FontWeight;
    p.Italic = s.FontItalic;
    p.FontSize = s.FontSize;
    p.LetterSpacing = s.LetterSpacing;
    p.LineHeight = s.LineHeight;
    p.WhiteSpace = s.WhiteSpace;
    p.Align = s.TextAlign;
    p.Overflow = s.TextOverflow;
    p.Transform = s.TextTransform;
    const bool widthKnown = s.Width.Kind != UiLenKind::FitChildren && !std::isnan(availableWidth);
    if (widthKnown && s.WhiteSpace == UiWhiteSpace::Normal)
        p.WrapWidth = availableWidth;
    if (s.TextOverflow != UiTextOverflow::Clip && !std::isnan(availableWidth))
    {
        p.WrapWidth = availableWidth;
        if (!std::isnan(availableHeight))
            p.MaxHeight = availableHeight;
    }
    if (!m_haveLayout || !(p == m_lastParams))
    {
        m_layout = UiLayoutText(p);
        m_lastParams = p;
        m_haveLayout = true;
        MarkPaintDirty();
    }
    return glm::vec2(m_layout.Width, m_layout.Height);
}

void UiElemLabel::PaintContent(UiPainter& painter, const UiRect& content)
{
    if (!m_haveLayout || (m_layout.Quads.empty() && m_layout.Icons.empty()))
        return;
    const glm::mat3& t = painter.Transform();
    float ox = content.X, oy = content.Y;
    if (t[0][0] == 1.f && t[1][1] == 1.f && t[0][1] == 0.f && t[1][0] == 0.f)
    {
        // Run-origin snapping: whole target pixels for the origin, so glyph edges look the same on every backend.
        ox = std::round(content.X + t[2][0]) - t[2][0];
        oy = std::round(content.Y + t[2][1]) - t[2][1];
    }
    std::vector<UiGlyphQuad> quads = m_layout.Quads;
    for (UiGlyphQuad& q : quads)
    {
        q.X += ox;
        q.Y += oy;
    }
    for (const UiTextRun& run : m_layout.Runs)
    {
        glm::vec4 color = run.HasColor ? run.Color : Computed().Color;
        color.a *= m_paintOpacity;
        painter.GlyphRun(quads.data() + run.First, run.Count, color, run.GlyphScale, run.Dilation);
    }
    for (const UiTextIcon& icon : m_layout.Icons)
    {
        const std::string path = icon.IsAction ? UiInputPrompts::Resolve(icon.Ref) : icon.Ref;
        if (path.empty())
            continue;
        Texture* texture = AssetRegistry::GetTextureFromFile(path);
        if (texture == nullptr)
            continue;
        if (!texture->valid)
        {
            MarkPaintDirty();   // paint again once the texture has loaded
            continue;
        }
        painter.Image(UiRect{ icon.X + ox, icon.Y + oy, icon.W, icon.H }, texture->getHandle(), glm::vec4(0.f, 0.f, 1.f, 1.f),
                      glm::vec4(1.f, 1.f, 1.f, m_paintOpacity));
    }
}

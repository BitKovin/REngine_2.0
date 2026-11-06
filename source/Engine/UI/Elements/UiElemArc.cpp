#include "UiElemArc.h"

#include <algorithm>
#include <cstdlib>

bool UiElemArc::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "start") { m_start = static_cast<float>(std::atof(value.c_str())); MarkPaintDirty(); return true; }
    if (name == "sweep") { m_sweep = static_cast<float>(std::atof(value.c_str())); MarkPaintDirty(); return true; }
    if (name == "thickness") { m_thickness = std::max(0.f, static_cast<float>(std::atof(value.c_str()))); MarkPaintDirty(); return true; }
    if (name == "round-caps") { m_roundCaps = value == "true" || value == "1"; MarkPaintDirty(); return true; }
    return false;
}

void UiElemArc::PaintContent(UiPainter& painter, const UiRect& content)
{
    constexpr float kRadians = 0.01745329252f;
    glm::vec4 color = Computed().Color;
    color.a *= m_paintOpacity;
    painter.Arc(content, m_thickness, m_start * kRadians, std::clamp(m_sweep, -360.f, 360.f) * kRadians, m_roundCaps, color);
}

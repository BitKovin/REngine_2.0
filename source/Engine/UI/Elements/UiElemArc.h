#pragma once
#include <UI/Elements/UiElem.h>

// A ring segment (progress ring, cooldown, radial meter) drawn with the arc primitive. Angles in degrees, clockwise from 12 o'clock.
class UiElemArc : public UiElem
{
public:
    UiElemArc() { TagName = "Arc"; }
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemArc>(); }

    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void PaintContent(UiPainter& painter, const UiRect& contentRect) override;

private:
    float m_start = 0.f, m_sweep = 360.f, m_thickness = 6.f;
    bool m_roundCaps = false;
};

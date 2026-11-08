#include "UiElemControls.h"

#include <UI/Elements/UiElemPanel.h>
#include <UI/UiSystem.h>

#include <algorithm>
#include <cstdlib>

namespace
{
    bool IsTrue(const std::string& v) { return !(v.empty() || v == "0" || v == "false" || v == "False"); }
}

// ── ToggleButton

UiElemToggleButton::UiElemToggleButton()
{
    TagName = "ToggleButton";
    m_binding.ReadWidget = [this]() { return Checked() ? 1.0 : 0.0; };
    m_binding.WriteWidget = [this](double v) { SetChecked(v != 0.0); };
}

void UiElemToggleButton::SetChecked(bool on)
{
    if (on == Checked())
        return;
    if (on) States |= kStateChecked; else States &= ~kStateChecked;
    MarkStyleDirty();
}

bool UiElemToggleButton::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "checked") { SetChecked(IsTrue(value)); m_binding.Resync(); return true; }
    if (name == "bind") { m_binding.SetBind(value); return true; }
    return UiElemButton::ApplyAttribute(name, value);
}

void UiElemToggleButton::OnActivate()
{
    SetChecked(!Checked());
    m_binding.Poll();
    UiSystem::QueueEvent(*this, Checked() ? "checked" : "unchecked");
}

void UiElemToggleButton::Update()
{
    UiElemButton::Update();
    m_binding.Poll();
}

// ── RadioButton

UiElemRadioButton::UiElemRadioButton()
{
    TagName = "RadioButton";
    m_binding.ReadWidget = [this]() { return m_hasOption ? (Checked() ? m_option : m_binding.LastSeen) : (Checked() ? 1.0 : 0.0); };
    m_binding.WriteWidget = [this](double v) { SetChecked(m_hasOption ? v == m_option : v != 0.0); };
}

void UiElemRadioButton::SetChecked(bool on)
{
    if (on == Checked())
        return;
    if (on) States |= kStateChecked; else States &= ~kStateChecked;
    MarkStyleDirty();
}

bool UiElemRadioButton::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "group") { m_group = value; return true; }
    if (name == "option") { m_option = std::atof(value.c_str()); m_hasOption = true; return true; }
    if (name == "checked") { SetChecked(IsTrue(value)); m_binding.Resync(); return true; }
    if (name == "bind") { m_binding.SetBind(value); return true; }
    return UiElemButton::ApplyAttribute(name, value);
}

void UiElemRadioButton::OnActivate()
{
    if (!Checked())
    {
        SetChecked(true);
        if (!m_group.empty())
        {
            std::vector<UiElem*> all;
            Root()->CollectElems(all);
            for (UiElem* e : all)
                if (e != this && e->TagName == "RadioButton")
                {
                    auto* other = static_cast<UiElemRadioButton*>(e);
                    if (other->m_group == m_group)
                        other->SetChecked(false);
                }
        }
        UiSystem::QueueEvent(*this, "checked");
    }
    m_binding.Poll();
}

void UiElemRadioButton::Update()
{
    UiElemButton::Update();
    m_binding.Poll();
}

// ── ProgressBar

UiElemProgressBar::UiElemProgressBar()
{
    TagName = "ProgressBar";
    auto fill = std::make_shared<UiElemPanel>();
    fill->Classes = { "fill" };
    m_fill = fill.get();
    AddChild(fill);
    ApplyFill();
}

void UiElemProgressBar::ApplyFill()
{
    if (m_fill == nullptr)
        return;
    const float percent = static_cast<float>(std::clamp(m_value, 0.0, 1.0) * 100.0);
    UiComputedStyle& s = m_fill->Style();
    UiLength along, across;
    along.Kind = UiLenKind::Percent;
    along.Percent = percent;
    across.Kind = UiLenKind::Percent;
    across.Percent = 100.f;
    s.Width = m_vertical ? across : along;
    s.Height = m_vertical ? along : across;
    s.VAlign = m_vertical ? UiVAlign::Bottom : UiVAlign::Top;
}

bool UiElemProgressBar::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "value")
    {
        const double v = std::atof(value.c_str());
        if (v != m_value)
        {
            m_value = v;
            ApplyFill();
        }
        return true;
    }
    if (name == "direction")
    {
        m_vertical = value == "vertical";
        ApplyFill();
        return true;
    }
    return false;
}

// ── CircularProgress

bool UiElemCircularProgress::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "value") { m_value = std::atof(value.c_str()); MarkPaintDirty(); return true; }
    if (name == "start-angle") { m_start = static_cast<float>(std::atof(value.c_str())); MarkPaintDirty(); return true; }
    if (name == "thickness") { m_thickness = std::max(0.f, static_cast<float>(std::atof(value.c_str()))); MarkPaintDirty(); return true; }
    return false;
}

void UiElemCircularProgress::PaintContent(UiPainter& painter, const UiRect& content)
{
    constexpr float kRadians = 0.01745329252f;
    glm::vec4 track = Computed().BackgroundColor;
    track.a *= m_paintOpacity;
    if (track.a > 0.f)
        painter.Arc(content, m_thickness, 0.f, 360.f * kRadians, false, track);
    glm::vec4 color = Computed().Color;
    color.a *= m_paintOpacity;
    const float sweep = static_cast<float>(std::clamp(m_value, 0.0, 1.0)) * 360.f;
    if (sweep > 0.f)
        painter.Arc(content, m_thickness, m_start * kRadians, sweep * kRadians, false, color);
}

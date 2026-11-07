#include "UiElemLegacyWidgets.h"

#include <Input.h>
#include <Time.hpp>
#include <UI/Markup/UiVariables.h>
#include <UI/UiCustomShaderImage.hpp>
#include <UI/UiSlider.hpp>
#include <UI/UiTextBox.hpp>
#include <UI/UiVideo.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <sstream>

namespace
{
    bool IsTrue(const std::string& v) { return !(v.empty() || v == "0" || v == "false" || v == "False"); }

}

// ── wrapper base

void UiElemLegacyWrapper::Attach(const std::shared_ptr<UiElement>& legacy)
{
    m_legacy = legacy;
    AddChild(legacy);
}

void UiElemLegacyWrapper::UpdateChildrenOffsets()
{
    if (m_legacy)
    {
        const UiRect content = ContentRectLocal();
        m_legacy->origin = glm::vec2(0.f);
        m_legacy->pivot = glm::vec2(0.f);
        m_legacy->position = glm::vec2(0.f);
        m_legacy->size = glm::vec2(content.W, content.H);
    }
    UiElem::UpdateChildrenOffsets();
}

// ── Slider

UiElemSlider::UiElemSlider()
{
    TagName = "Slider";
    m_natural = glm::vec2(300.f, 28.f);
    m_slider = std::make_shared<UiSlider>();
    Attach(m_slider);
    m_binding.ReadWidget = [this]() { return Value(); };
    m_binding.WriteWidget = [this](double v) { SetValue(v); };
}

double UiElemSlider::Value() const { return m_slider->Value; }
void UiElemSlider::SetValue(double v) { m_slider->Value = std::clamp(static_cast<float>(v), m_slider->MinValue, m_slider->MaxValue); }

bool UiElemSlider::ApplyAttribute(const std::string& name, const std::string& value)
{
    const float number = static_cast<float>(std::atof(value.c_str()));
    if (name == "min") { m_slider->MinValue = number; return true; }
    if (name == "max") { m_slider->MaxValue = number; return true; }
    if (name == "step") { m_slider->Step = number; return true; }
    if (name == "value") { SetValue(number); m_binding.Resync(); return true; }
    if (name == "bind") { m_binding.SetBind(value); return true; }
    return false;
}

void UiElemSlider::Tick()
{
    const UiComputedStyle& s = Computed();
    m_slider->BackgroundColor = UiCustomColor(s, "--track-color", glm::vec4(0.15f, 0.15f, 0.15f, 1.f));
    m_slider->color = UiCustomColor(s, "--fill-color", s.Color);
    const glm::vec4 handle = UiCustomColor(s, "--handle-color", s.Color);
    m_slider->HandleColor = handle;
    m_slider->HandleHoverColor = glm::vec4(glm::min(glm::vec3(handle) * 1.15f, glm::vec3(1.f)), handle.a);
    m_slider->HandleActiveColor = glm::vec4(glm::min(glm::vec3(handle) * 1.3f, glm::vec3(1.f)), handle.a);
    m_slider->visible = s.Visibility == UiVisibility::Visible;
}

void UiElemSlider::Update()
{
    UiElem::Update();
    m_binding.Poll();
}

// ── TextEntry

UiElemTextEntry::UiElemTextEntry()
{
    TagName = "TextEntry";
    m_natural = glm::vec2(300.f, 40.f);
    m_box = std::make_shared<UiTextBox>();
    Attach(m_box);
}

const std::string& UiElemTextEntry::Text() const { return m_box->Text; }
void UiElemTextEntry::SetText(const std::string& text) { m_box->SetText(text); }

bool UiElemTextEntry::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "text") { m_box->SetText(value); m_lastText = m_box->Text; SetVar("value", m_lastText); return true; }
    if (name == "placeholder") { m_box->Placeholder = value; return true; }
    if (name == "max-length") { m_box->MaxLength = std::atoi(value.c_str()); return true; }
    if (name == "password") { m_box->PasswordMode = IsTrue(value); return true; }
    if (name == "numeric") { m_box->NumericOnly = IsTrue(value); return true; }
    return false;
}

void UiElemTextEntry::Tick()
{
    const UiComputedStyle& s = Computed();
    m_box->TextColor = s.Color;
    m_box->CaretColor = s.Color;
    m_box->PlaceholderColor = glm::vec4(s.Color.r, s.Color.g, s.Color.b, s.Color.a * 0.4f);
    m_box->FontSize = s.FontSize;
    m_box->Color = UiCustomColor(s, "--field-color", glm::vec4(0.12f, 0.12f, 0.12f, 1.f));
    m_box->HoverColor = UiCustomColor(s, "--field-hover-color", glm::vec4(0.16f, 0.16f, 0.16f, 1.f));
    m_box->FocusColor = UiCustomColor(s, "--field-focus-color", glm::vec4(0.20f, 0.20f, 0.26f, 1.f));
    m_box->EditingColor = UiCustomColor(s, "--field-editing-color", glm::vec4(0.16f, 0.24f, 0.32f, 1.f));
}

void UiElemTextEntry::Update()
{
    UiElem::Update();
    if (m_box->Text != m_lastText)
    {
        m_lastText = m_box->Text;
        SetVar("value", m_lastText);
        Fire("valuechanged");
    }
}

// ── Movie

UiElemMovie::UiElemMovie()
{
    TagName = "Movie";
    m_video = std::make_shared<UiVideo>();
    Attach(m_video);
}

bool UiElemMovie::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "src") { m_video->VideoPath = value; m_video->videoInstance.reset(); return true; }
    if (name == "loop") { if (m_video->videoInstance) m_video->videoInstance->SetLoop(IsTrue(value)); return true; }
    // autoplay and muted have no field on the legacy video: playback starts on the first draw and the audio follows the sound volumes
    if (name == "autoplay" || name == "muted") return true;
    return false;
}

// ── ShaderPanel

UiElemShaderPanel::UiElemShaderPanel()
{
    TagName = "ShaderPanel";
    m_panel = std::make_shared<UiCustomShaderImage>();
    Attach(m_panel);
}

bool UiElemShaderPanel::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "shader") { m_panel->PixelShader = value; return true; }
    if (name.rfind("param-", 0) == 0)
    {
        std::istringstream in(value);
        glm::vec4 v(0.f);
        in >> v.x >> v.y >> v.z >> v.w;
        m_panel->Vector4s[name.substr(6)] = v;
        return true;
    }
    return false;
}

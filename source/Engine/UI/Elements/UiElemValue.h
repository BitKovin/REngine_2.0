#pragma once
#include <UI/Elements/UiElem.h>
#include <UI/Kit/UiSettings.h>

#include <functional>
#include <string>

// What every value widget (Slider, DropDown, ToggleButton, ...) shares: the "value" variable, the valuechanged event and
// bind="setting:<key>". The widget supplies how to read and write its own value; the owner calls Poll() from Update().
class UiValueBinding
{
public:
    explicit UiValueBinding(UiElem& owner) : m_owner(owner) {}

    std::function<double()> ReadWidget;
    std::function<void(double)> WriteWidget;

    // "setting:<key>" (or an empty string to unbind). Returns false for an unknown key.
    bool SetBind(const std::string& spec);
    bool IsBound() const { return m_entry != nullptr; }
    const UiSettings::Entry* Entry() const { return m_entry; }

    // Pushes an externally changed setting into the widget and reports a changed widget value (variable, event, setting).
    void Poll();
    // Forget what was seen and publish the widget's current value as the variable without firing the event.
    void Resync();
    double LastSeen = 0.0;

private:
    void Publish(double value);
    UiElem& m_owner;
    const UiSettings::Entry* m_entry = nullptr;
    double m_lastSetting = 0.0;
    bool m_started = false;
};

// A colour from a custom property of the element (--name: #rrggbb), or the fallback.
glm::vec4 UiCustomColor(const UiComputedStyle& style, const char* name, const glm::vec4& fallback);

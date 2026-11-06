#include "UiElemInputPrompt.h"

#include <UI/Menus/UiInputPrompts.h>

bool UiElemInputPrompt::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "action")
    {
        m_action = value;
        m_dirty = true;
        return true;
    }
    return UiElemImage::ApplyAttribute(name, value);
}

void UiElemInputPrompt::Tick()
{
    if (m_dirty || m_revision != UiInputPrompts::DeviceRevision())
        Refresh();
}

void UiElemInputPrompt::Refresh()
{
    m_dirty = false;
    m_revision = UiInputPrompts::DeviceRevision();
    const std::string path = m_action.empty() ? std::string() : UiInputPrompts::Resolve(m_action);
    if (path != Src())
        UiElemImage::ApplyAttribute("src", path);
    const bool collapse = path.empty();
    if (m_visibleOverrideCollapse != collapse && Attributes.find("visible") == Attributes.end())
    {
        m_visibleOverrideCollapse = collapse;
        MarkStyleDirty();
    }
    MarkLayoutDirty();
}

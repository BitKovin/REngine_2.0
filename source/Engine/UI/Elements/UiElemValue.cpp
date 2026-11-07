#include "UiElemValue.h"

#include <UI/Markup/UiMarkupLog.h>
#include <UI/Style/UiFxParse.h>

bool UiValueBinding::SetBind(const std::string& spec)
{
    m_entry = nullptr;
    m_started = false;
    if (spec.empty())
        return true;
    const std::string prefix = "setting:";
    if (spec.compare(0, prefix.size(), prefix) != 0)
    {
        UiMarkupLog::Error(m_owner.ScopeFile, m_owner.SrcLine, m_owner.SrcCol, "bind: expected 'setting:<key>', got '" + spec + "'");
        return false;
    }
    m_entry = UiSettings::Find(spec.substr(prefix.size()));
    if (m_entry == nullptr)
    {
        UiMarkupLog::Warning(m_owner.ScopeFile, m_owner.SrcLine, m_owner.SrcCol, "bind: unknown setting '" + spec.substr(prefix.size()) + "'");
        return false;
    }
    // show the setting at once (a sibling label may print {value} while the page is still being built)
    if (m_entry->Get && WriteWidget)
    {
        m_lastSetting = m_entry->Get();
        WriteWidget(m_lastSetting);
        m_started = true;
        Resync();
    }
    return true;
}

// The variable "value" is set on the widget and also on its parent, so a sibling label can show it: {value:0.00}.
void UiValueBinding::Publish(double value)
{
    m_owner.SetVar("value", value);
    if (UiElem* parent = m_owner.AsParentElem())
        parent->SetVar("value", value);
}

void UiValueBinding::Resync()
{
    if (!ReadWidget)
        return;
    LastSeen = ReadWidget();
    Publish(LastSeen);
}

void UiValueBinding::Poll()
{
    if (!ReadWidget || !WriteWidget)
        return;
    if (m_entry != nullptr && m_entry->Get)
    {
        const double setting = m_entry->Get();
        if (!m_started || setting != m_lastSetting)
        {
            m_started = true;
            m_lastSetting = setting;
            WriteWidget(setting);
            Resync();
            return;
        }
    }
    const double value = ReadWidget();
    if (!m_started)
    {
        m_started = true;
        Resync();
        return;
    }
    if (value == LastSeen)
        return;
    LastSeen = value;
    Publish(value);
    if (m_entry != nullptr && m_entry->Set && !m_entry->ReadOnly)
    {
        UiSettings::NoteChange(m_entry->Key, m_lastSetting);
        m_entry->Set(value);
        if (m_entry->Get)
            m_lastSetting = m_entry->Get();
    }
    m_owner.Fire("valuechanged");
}

glm::vec4 UiCustomColor(const UiComputedStyle& style, const char* name, const glm::vec4& fallback)
{
    const auto it = style.Custom.find(name);
    if (it == style.Custom.end())
        return fallback;
    std::vector<UiCssToken> tokens;
    std::string error;
    if (!UiSubstituteVars(it->second, style.Custom, tokens, error))
        return fallback;
    const auto parts = UiSplitComponents(tokens);
    if (parts.empty())
        return fallback;
    UiStyleParseContext ctx;
    ctx.CurrentColor = style.Color;
    glm::vec4 out;
    return UiParseColorComponent(parts[0], ctx, out, error) ? out : fallback;
}

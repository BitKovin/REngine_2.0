#include "UiElemButton.h"

#include <Input.h>
#include <UI/Elements/UiElemLabel.h>
#include <UI/Menus/UiMenuStack.h>

UiElemButton::UiElemButton()
{
    TagName = "Button";
    HitCheck = true;
    DisableFocus = false;
}

bool UiElemButton::ApplyAttribute(const std::string& name, const std::string&)
{
    return name == "hotkey" || name == "hotkey-label";   // read from Attributes by Tick and the action bar
}

void UiElemButton::Tick()
{
    const auto it = Attributes.find("hotkey");
    if (it == Attributes.end() || it->second.empty())
        return;
    InputAction* action = Input::GetAction(it->second);
    if (!action->Pressed() || Computed().Visibility != UiVisibility::Visible || (States & kStateDisabled))
        return;
    UiMenuStack* stack = Stack();
    const bool active = stack != nullptr ? PageOf() == stack->Top() : Root()->Computed().Visibility == UiVisibility::Visible;
    if (!active)
        return;
    Activate();
    action->CleanPressed();
}

UiElemTextButton::UiElemTextButton()
{
    TagName = "TextButton";
    auto label = std::make_shared<UiElemLabel>();
    m_label = label.get();
    AddChild(label);
}

bool UiElemTextButton::ApplyAttribute(const std::string& name, const std::string& value)
{
    if (name == "text" && m_label != nullptr)
    {
        m_label->SetResolvedText(value);
        return true;
    }
    return UiElemButton::ApplyAttribute(name, value);
}

void UiElemTextButton::OnBuilt()
{
    if (m_label != nullptr)
    {
        m_label->ScopeFile = ScopeFile;
        m_label->Instance = Instance;
    }
}

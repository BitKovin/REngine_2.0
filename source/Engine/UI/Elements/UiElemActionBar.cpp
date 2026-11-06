#include "UiElemActionBar.h"

#include <UI/Elements/UiElemButton.h>
#include <UI/Elements/UiElemInputPrompt.h>
#include <UI/Elements/UiElemLabel.h>
#include <UI/Menus/UiInputPrompts.h>
#include <UI/Menus/UiMenuStack.h>

namespace
{
    struct Entry { std::string Action, Text; bool Localise = false; };

    void CollectHotkeys(UiElem& e, std::vector<Entry>& out)
    {
        if (e.Computed().Visibility != UiVisibility::Visible)
            return;
        if (UiElemButton* button = dynamic_cast<UiElemButton*>(&e))
        {
            const auto hot = e.Attributes.find("hotkey");
            if (hot != e.Attributes.end() && !hot->second.empty() && !(e.States & kStateDisabled))
            {
                Entry entry;
                entry.Action = hot->second;
                const auto label = e.Attributes.find("hotkey-label");
                if (label != e.Attributes.end())
                {
                    entry.Text = label->second;
                    entry.Localise = true;
                }
                else if (UiElemTextButton* tb = dynamic_cast<UiElemTextButton*>(button); tb != nullptr && tb->LabelElem() != nullptr)
                    entry.Text = tb->LabelElem()->Text();
                out.push_back(entry);
            }
        }
        for (auto& c : e.children)
            if (UiElem::IsElem(c.get()))
                CollectHotkeys(static_cast<UiElem&>(*c), out);
    }
}

UiElemActionBar::UiElemActionBar()
{
    TagName = "ActionBar";
    Style().Flow = UiFlow::Right;
    Style().Gap = UiLength{ UiLenKind::Px, 24.f };
}

void UiElemActionBar::Tick()
{
    UiElem* page = UiMenus::Stack().Top();
    std::vector<Entry> entries;
    if (page != nullptr)
    {
        CollectHotkeys(*page, entries);
        if (page->ReadHostOptions().CloseOnBack)
            entries.push_back(Entry{ "ui_cancel", "${UI_BACK}", false });
    }
    std::string signature;
    for (const Entry& e : entries)
        signature += e.Action + "|" + e.Text + "\n";
    if (page == m_page && signature == m_signature && m_revision == UiInputPrompts::DeviceRevision())
        return;
    m_page = page;
    m_signature = signature;
    m_revision = UiInputPrompts::DeviceRevision();

    ClearChildren();
    for (const Entry& e : entries)
    {
        auto item = std::make_shared<UiElemPanel>();
        item->Style().Flow = UiFlow::Right;
        item->Style().Gap = UiLength{ UiLenKind::Px, 8.f };
        item->Style().VAlign = UiVAlign::Center;
        item->Classes.push_back("action-entry");
        item->Instance = Instance;
        item->ScopeFile = ScopeFile;
        AddChild(item);

        auto prompt = std::make_shared<UiElemInputPrompt>();
        prompt->Style().Width = UiLength{ UiLenKind::Px, 36.f };
        prompt->Style().Height = UiLength{ UiLenKind::Px, 36.f };
        prompt->Style().VAlign = UiVAlign::Center;
        prompt->Classes.push_back("action-prompt");
        item->AddChild(prompt);
        prompt->Attributes["action"] = e.Action;
        prompt->ApplyAttributeValue("action", e.Action, false);

        auto label = std::make_shared<UiElemLabel>();
        label->Style().VAlign = UiVAlign::Center;
        label->Classes.push_back("action-label");
        item->AddChild(label);
        label->Attributes["text"] = e.Text;
        label->ApplyAttributeValue("text", e.Text, false);
    }
}

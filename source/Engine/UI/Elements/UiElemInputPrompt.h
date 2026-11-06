#pragma once
#include <UI/Elements/UiElemImage.h>

// Image of the key, button or gesture that triggers an action on the device the player used last.
class UiElemInputPrompt : public UiElemImage
{
public:
    UiElemInputPrompt() { TagName = "InputPrompt"; }
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemInputPrompt>(); }

    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void Tick() override;

    const std::string& Action() const { return m_action; }

private:
    std::string m_action;
    uint32_t m_revision = 0;
    bool m_dirty = true;
    void Refresh();
};

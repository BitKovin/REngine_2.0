#pragma once
#include <UI/Elements/UiElemPanel.h>

// Row of "prompt + label" entries: one per visible hotkey button of the top page of the main menu stack, plus back.
class UiElemActionBar : public UiElemPanel
{
public:
    UiElemActionBar();
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemActionBar>(); }
    void Tick() override;

private:
    UiElem* m_page = nullptr;
    uint32_t m_revision = 0;
    std::string m_signature;
    void Rebuild(UiElem* page);
};

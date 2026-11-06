#pragma once
#include <UI/Elements/UiElem.h>

class UiElemLabel;

class UiElemButton : public UiElem
{
public:
    UiElemButton();
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemButton>(); }
    bool IsFocusable() const override { return true; }
    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void Tick() override;   // hotkey
};

class UiElemTextButton : public UiElemButton
{
public:
    UiElemTextButton();
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemTextButton>(); }
    bool ApplyAttribute(const std::string& name, const std::string& value) override;
    void OnBuilt() override;
    UiElemLabel* LabelElem() const { return m_label; }

private:
    UiElemLabel* m_label = nullptr;
};

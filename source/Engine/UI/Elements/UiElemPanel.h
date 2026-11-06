#pragma once
#include <UI/Elements/UiElem.h>

class UiElemPanel : public UiElem
{
public:
    UiElemPanel() { TagName = "Panel"; }
    static std::shared_ptr<UiElem> Create() { return std::make_shared<UiElemPanel>(); }
};

#pragma once

#include <UI/Elements/UiElemPanel.h>
#include <UI/Markup/UiTagRegistry.h>
#include <UI/Menus/UiMenuStack.h>
#include <EngineMain.h>

#include "Entities/Player/Player.hpp"

#include "UiSaveLoadMenu.h"
#include "UiSettingsMenu.h"

// Code-behind of ui/pause.xml: the markup describes the page, this class provides the actions it calls.
class StealthPauseMenu : public UiElemPanel
{
public:
    StealthPauseMenu()
    {
        LoadLayout("GameData/ui/pause.xml");

        On("OpenSettings", [this]() { OpenLegacy(std::make_shared<UiSettingsMenu>(MakeShowAgain())); });
        On("OpenSave", [this]() { OpenLegacy(std::make_shared<UiSaveLoadMenu>(MakeShowAgain(), UiSaveLoadMenu::Mode::Save)); });
        On("OpenLoad", [this]() { OpenLegacy(std::make_shared<UiSaveLoadMenu>(MakeShowAgain(), UiSaveLoadMenu::Mode::Load)); });

        // Saving is only possible while the player is alive.
        if (!(Player::Instance && Player::Instance->dead == false))
            if (UiElem* save = FindElem("psave"))
                save->SetAttribute("visible", "false");
    }

private:
    // The settings and save screens are still C++ menus on the viewport; the page hides while one is open.
    std::function<void()> MakeShowAgain()
    {
        std::weak_ptr<UiElement> weak = shared_from_this();
        return [weak]()
        {
            if (auto page = weak.lock())
                static_cast<UiElem&>(*page).SetAttribute("visible", "true");
        };
    }

    void OpenLegacy(std::shared_ptr<UiElement> menu)
    {
        EngineMain::MainInstance->Viewport.AddChild(menu);
        SetAttribute("visible", "false");
    }
};

UI_REGISTER_CODE_BEHIND("ui/pause.xml", StealthPauseMenu);

#pragma once

// SlasherGame's own settings section: [Gameplay] of the settings file (same name as before, so existing files still load),
// a UiSettings entry for the kit's bound widgets, and the gameplay tab of the kit settings page.

#include "GameplaySettingsData.h"

#include <UI/Kit/KitPages.h>
#include <UI/Kit/Settings/KitSettings.h>
#include <UI/Kit/UiSettings.h>

class GameplaySettings
{
public:
    static GameplaySettingsData& Data()
    {
        static GameplaySettingsData data;
        return data;
    }

    static void Register()
    {
        KitSettings::Section section;
        section.Name = "Gameplay";
        section.Serialize = []() { return Data().Serialize(); };
        section.ApplyLine = [](const std::string& key, const std::string& value) { Data().ApplyLine(key, value); };
        section.Apply = []() { Data().ApplyToEngine(); };
        section.Reset = []() { Data().ResetToDefaults(); };
        KitSettings::Instance().RegisterSection(section);

        UiSettings::Entry shake;
        shake.Key = "gameplay.camera_shake";
        shake.Type = UiSettings::Kind::Float;
        shake.Label = "${SETTINGS_GAME_CAMERA_SHAKE}";
        shake.Min = GameplaySettingsData::MinCameraShake;
        shake.Max = GameplaySettingsData::MaxCameraShake;
        shake.Step = 0.01;
        shake.Get = []() { return static_cast<double>(Data().CameraShake); };
        shake.Set = [](double v) { Data().CameraShake = static_cast<float>(v); };
        UiSettings::Register(shake);

        KitSettingsMenu::RegisterTab("${SETTINGS_MENU_GAMEPLAY}", "ui/game_settings.xml");
    }
};

#pragma once

// KitSettings: the settings every game of the engine kit shares (video, audio, input, UI) in one saveable tree.
// It is the old per-game GameSettings; games add their own sections through RegisterSection.
//
//   KitSettings::Instance().Video.Width = 2560;
//   KitSettings::Instance().ApplyAll();
//   KitSettings::Instance().SaveToFile();

#include <functional>
#include <string>
#include <vector>

#include "AudioSettingsData.h"
#include "InputSettingsData.h"
#include "VideoSettingsData.h"

class KitSettings
{
public:
    static KitSettings& Instance();

    VideoSettingsData Video;
    AudioSettingsData Audio;
    InputSettingsData Input;

    // A game section: `[Name]` in the file. The INI section name decides compatibility with existing settings files.
    struct Section
    {
        std::string Name;
        std::function<std::string()> Serialize;                                   // body: key=value lines
        std::function<void(const std::string& key, const std::string& value)> ApplyLine;
        std::function<void()> Apply;                                              // ApplyAll
        std::function<void()> Reset;                                              // ResetAllToDefaults
    };
    void RegisterSection(const Section& section);

    void ApplyAll() const;
    void ResetAllToDefaults();

    // Default save location, relative to the game's working directory.
    static std::string DefaultSavePath;

    bool SaveToFile(const std::string& path = DefaultSavePath) const;
    bool LoadFromFile(const std::string& path = DefaultSavePath);

    std::string Serialize() const;
    void Deserialize(const std::string& text);

    // Registers the kit's settings in UiSettings (video.*, audio.*, input.*). Call once after the window exists and before
    // the first kit page opens; the games' own settings register next to it.
    void RegisterUiSettings();

private:
    KitSettings() = default;
    std::vector<Section> m_sections;
};

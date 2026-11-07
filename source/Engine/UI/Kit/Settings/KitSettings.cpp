#include "KitSettings.h"

#include <FileSystem/FileSystem.h>
#include <Logger.hpp>
#include <UI/Kit/UiSettings.h>
#include <UI/UiSystem.h>

#include <algorithm>
#include <sstream>

std::string KitSettings::DefaultSavePath = "SaveData/GameSettings.cfg";

KitSettings& KitSettings::Instance()
{
    static KitSettings instance;
    return instance;
}

void KitSettings::RegisterSection(const Section& section)
{
    for (Section& s : m_sections)
        if (s.Name == section.Name)
        {
            s = section;
            return;
        }
    m_sections.push_back(section);
}

void KitSettings::ApplyAll() const
{
    Video.ApplyToEngine();
    Audio.ApplyToEngine();
    Input.ApplyToEngine();
    for (const Section& s : m_sections)
        if (s.Apply)
            s.Apply();
}

void KitSettings::ResetAllToDefaults()
{
    Video.ResetToDefaults();
    Audio.ResetToDefaults();
    Input.ResetToDefaults();
    for (const Section& s : m_sections)
        if (s.Reset)
            s.Reset();
}

std::string KitSettings::Serialize() const
{
    std::ostringstream ss;
    ss << "[Video]\n" << Video.Serialize() << "\n";
    ss << "[Audio]\n" << Audio.Serialize() << "\n";
    ss << "[Input.Sensitivity]\n" << Input.SerializeSensitivity() << "\n";
    ss << "[Input.Bindings]\n" << Input.SerializeBindings() << "\n";
    ss << "[UI]\n" << "TextScale=" << UiSystem::TextScale << "\n" << "ReducedMotion=" << (UiSystem::ReducedMotion() ? 1 : 0) << "\n\n";
    for (const Section& s : m_sections)
        if (s.Serialize)
            ss << "[" << s.Name << "]\n" << s.Serialize() << "\n";
    return ss.str();
}

void KitSettings::Deserialize(const std::string& text)
{
    std::istringstream stream(text);
    std::string line, section;
    while (std::getline(stream, line))
    {
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
            line.pop_back();
        const size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos)
            continue;
        const size_t end = line.find_last_not_of(" \t");
        line = line.substr(start, end - start + 1);
        if (line.empty() || line[0] == '#')
            continue;
        if (line.front() == '[' && line.back() == ']')
        {
            section = line.substr(1, line.size() - 2);
            continue;
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos)
            continue;
        const std::string key = line.substr(0, eq);
        const std::string value = line.substr(eq + 1);

        if (section == "Video") Video.ApplyLine(key, value);
        else if (section == "Audio") Audio.ApplyLine(key, value);
        else if (section == "Input.Sensitivity") Input.ApplySensitivityLine(key, value);
        else if (section == "Input.Bindings") Input.ApplyBindingsLine(key, value);
        else if (section == "UI")
        {
            try
            {
                if (key == "TextScale") UiSystem::SetTextScale(std::stof(value));
                else if (key == "ReducedMotion") UiSystem::SetReducedMotion(value == "1" || value == "true");
            }
            catch (...) { /* malformed line: keep the current value */ }
        }
        else
            for (const Section& s : m_sections)
                if (s.Name == section && s.ApplyLine)
                    s.ApplyLine(key, value);
    }
}

bool KitSettings::SaveToFile(const std::string& path) const
{
    return FileSystemEngine::WriteFile(path, Serialize());
}

bool KitSettings::LoadFromFile(const std::string& path)
{
    const std::string text = FileSystemEngine::ReadFile(path);
    if (text.empty())
        return false;
    Deserialize(text);
    return true;
}

namespace
{
    using UiSettings::Entry;
    using UiSettings::Kind;

    Entry Make(const char* key, Kind kind, const char* label, double min, double max, double step)
    {
        Entry e;
        e.Key = key;
        e.Type = kind;
        e.Label = label;
        e.Min = min;
        e.Max = max;
        e.Step = step;
        return e;
    }

    // The resolution and MSAA lists are only known at run time; the entry keeps the list it last handed out so that
    // Get and Set index the same list.
    std::vector<std::string> g_resolutions;
    std::vector<std::string> g_msaa;
}

void KitSettings::RegisterUiSettings()
{
    KitSettings& s = *this;

    Entry resolution = Make("video.resolution", Kind::Choice, "${SETTINGS_VIDEO_RESOLUTION}", 0, 0, 1);
    resolution.GetChoices = []() { g_resolutions = VideoSettingsData::ResolutionList(); return g_resolutions; };
    resolution.Get = []()
    {
        if (g_resolutions.empty())
            g_resolutions = VideoSettingsData::ResolutionList();
        const auto it = std::find(g_resolutions.begin(), g_resolutions.end(), VideoSettingsData::CurrentResolution());
        return it == g_resolutions.end() ? 0.0 : static_cast<double>(it - g_resolutions.begin());
    };
    resolution.Set = [&s](double v)
    {
        const int index = static_cast<int>(v);
        if (index < 0 || index >= static_cast<int>(g_resolutions.size()))
            return;
        const std::string& value = g_resolutions[index];
        const size_t x = value.find('x');
        if (x == std::string::npos)
            return;
        s.Video.Width = std::atoi(value.substr(0, x).c_str());
        s.Video.Height = std::atoi(value.substr(x + 1).c_str());
        s.Video.ApplyToEngine();
    };
    UiSettings::Register(resolution);

    Entry mode = Make("video.window_mode", Kind::Choice, "${SETTINGS_VIDEO_WINDOW_MODE}", 0, 2, 1);
    mode.Choices = { "${SETTINGS_VIDEO_WINDOWED}", "${SETTINGS_VIDEO_FULLSCREEN}", "${SETTINGS_VIDEO_BORDERLESS}" };
    mode.Get = []() { return static_cast<double>(VideoSettingsData::WindowModeIndex()); };
    mode.Set = [&s](double v)
    {
        const int index = static_cast<int>(v);
        s.Video.WindowMode = index == 0 ? "windowed" : (index == 1 ? "fullscreen" : "borderless");
        s.Video.ApplyToEngine();
    };
    UiSettings::Register(mode);

    Entry vsync = Make("video.vsync", Kind::Bool, "${SETTINGS_VIDEO_VSYNC}", 0, 1, 1);
    vsync.Get = [&s]() { return s.Video.VSync ? 1.0 : 0.0; };
    vsync.Set = [&s](double v) { s.Video.VSync = v != 0.0; s.Video.ApplyToEngine(); };
    UiSettings::Register(vsync);

    Entry msaa = Make("video.msaa", Kind::Choice, "${SETTINGS_VIDEO_MSAA}", 0, 3, 1);
    msaa.GetChoices = [&s]()
    {
        g_msaa = { "0", "2", "4", "8" };
        const std::string current = std::to_string(s.Video.MSAA);
        if (std::find(g_msaa.begin(), g_msaa.end(), current) == g_msaa.end())
            g_msaa.push_back(current);
        return g_msaa;
    };
    msaa.Get = [&s]()
    {
        if (g_msaa.empty())
            g_msaa = { "0", "2", "4", "8" };
        const auto it = std::find(g_msaa.begin(), g_msaa.end(), std::to_string(s.Video.MSAA));
        return it == g_msaa.end() ? 0.0 : static_cast<double>(it - g_msaa.begin());
    };
    msaa.Set = [&s](double v)
    {
        const int index = static_cast<int>(v);
        if (index < 0 || index >= static_cast<int>(g_msaa.size()))
            return;
        s.Video.MSAA = std::atoi(g_msaa[index].c_str());
        s.Video.ApplyToEngine();
    };
    UiSettings::Register(msaa);

    Entry fxaa = Make("video.fxaa", Kind::Bool, "${SETTINGS_VIDEO_FXAA}", 0, 1, 1);
    fxaa.Get = [&s]() { return s.Video.FXAA ? 1.0 : 0.0; };
    fxaa.Set = [&s](double v) { s.Video.FXAA = v != 0.0; s.Video.ApplyToEngine(); };
    UiSettings::Register(fxaa);

    Entry shadows = Make("video.dynamic_shadows", Kind::Bool, "${SETTINGS_VIDEO_DYNAMIC_SHADOWS}", 0, 1, 1);
    shadows.Get = [&s]() { return s.Video.DynamicShadows ? 1.0 : 0.0; };
    shadows.Set = [&s](double v) { s.Video.DynamicShadows = v != 0.0; s.Video.ApplyToEngine(); };
    UiSettings::Register(shadows);

    // Volumes are written straight to the sound manager, as the settings screen always did.
    struct Volume { const char* Key; const char* Label; float* Value; };
    const Volume volumes[] = {
        { "audio.master", "${SETTINGS_SOUND_MASTER_VOLUME}", &SoundManager::GlobalVolume },
        { "audio.sfx", "${SETTINGS_SOUND_SFX_VOLUME}", &SoundManager::SfxVolume },
        { "audio.music", "${SETTINGS_SOUND_MUSIC_VOLUME}", &SoundManager::MusicVolume },
    };
    for (const Volume& v : volumes)
    {
        Entry e = Make(v.Key, Kind::Float, v.Label, 0.0, 1.0, 0.01);
        float* value = v.Value;
        e.Get = [value]() { return static_cast<double>(*value); };
        e.Set = [value](double x) { *value = static_cast<float>(std::clamp(x, 0.0, 1.0)); };
        UiSettings::Register(e);
    }

    Entry mouse = Make("input.mouse_sensitivity", Kind::Float, "${SETTINGS_INPUT_MOUSE_SENSITIVITY}", InputSensitivitySettings::MinMouseSensitivity,
                       InputSensitivitySettings::MaxMouseSensitivity, 0.01);
    mouse.Get = [&s]() { return static_cast<double>(s.Input.Sensitivity.MouseSensitivity); };
    mouse.Set = [&s](double v) { s.Input.Sensitivity.MouseSensitivity = static_cast<float>(v); s.Input.Sensitivity.ApplyToEngine(); };
    UiSettings::Register(mouse);

    Entry pad = Make("input.gamepad_sensitivity", Kind::Float, "${SETTINGS_INPUT_GAMEPAD_SENSITIVITY}", InputSensitivitySettings::MinGamepadSensitivity,
                     InputSensitivitySettings::MaxGamepadSensitivity, 0.05);
    pad.Get = [&s]() { return static_cast<double>(s.Input.Sensitivity.GamepadLookSensitivity); };
    pad.Set = [&s](double v) { s.Input.Sensitivity.GamepadLookSensitivity = static_cast<float>(v); s.Input.Sensitivity.ApplyToEngine(); };
    UiSettings::Register(pad);

    Entry invert = Make("input.invert_y", Kind::Bool, "${SETTINGS_INPUT_INVERT_LOOK_Y}", 0, 1, 1);
    invert.Get = [&s]() { return s.Input.Sensitivity.InvertY ? 1.0 : 0.0; };
    invert.Set = [&s](double v) { s.Input.Sensitivity.InvertY = v != 0.0; s.Input.Sensitivity.ApplyToEngine(); };
    UiSettings::Register(invert);
}

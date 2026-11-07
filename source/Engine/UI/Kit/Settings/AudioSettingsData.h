#pragma once

// ---------------------------------------------------------------------------
// AudioSettingsData
//
// Placeholder module, same shape as VideoSettingsData/InputSettingsData, so
// GameSettings has a third nested module to prove out the "serialize the
// whole tree" pattern. ApplyToEngine() is intentionally a stub — wire it up
// to your real audio system (no audio engine headers were provided).
// ---------------------------------------------------------------------------

#include <string>
#include <sstream>
#include <algorithm>

#include <SoundSystem/SoundManager.hpp>

struct AudioSettingsData
{
    float MasterVolume = 1.0f;
    float MusicVolume  = 0.8f;
    float SfxVolume    = 1.0f;
    bool Loaded = false;   // the volumes came from a settings file; without one the engine defaults stay

    void ResetToDefaults() { *this = AudioSettingsData(); }

    void ApplyToEngine() const
    {
        if (!Loaded)
            return;
        SoundManager::GlobalVolume = MasterVolume;
        SoundManager::MusicVolume = MusicVolume;
        SoundManager::SfxVolume = SfxVolume;
    }

    void FromCurrentState()
    {
        MasterVolume = SoundManager::GlobalVolume;
        MusicVolume = SoundManager::MusicVolume;
        SfxVolume = SoundManager::SfxVolume;
        Loaded = true;
    }

    std::string Serialize() const
    {
        std::ostringstream ss;
        ss << "MasterVolume=" << MasterVolume << "\n";
        ss << "MusicVolume=" << MusicVolume << "\n";
        ss << "SfxVolume=" << SfxVolume << "\n";
        return ss.str();
    }

    void ApplyLine(const std::string& key, const std::string& value)
    {
        try
        {
            float v = std::stof(value);
            v = std::clamp(v, 0.0f, 1.0f);
            if (key == "MasterVolume")     { MasterVolume = v; Loaded = true; }
            else if (key == "MusicVolume") { MusicVolume = v; Loaded = true; }
            else if (key == "SfxVolume")   { SfxVolume = v; Loaded = true; }
        }
        catch (...) { /* malformed line — keep current value */ }
    }
};

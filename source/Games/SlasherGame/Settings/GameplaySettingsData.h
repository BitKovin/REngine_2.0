#pragma once

// ---------------------------------------------------------------------------
// GameplaySettingsData
//
// Gameplay and comfort options, same shape as the other settings modules
// (ResetToDefaults / ApplyToEngine / Serialize / ApplyLine). Nothing here is
// engine state: the game reads the values where they apply.
//
//   CameraShake   scales the camera's whole impact response - hit kicks,
//                 rumble and shakes queued through the engine
//                 (PlayerCamera, COMBAT_DESIGN.md §9.4). 0 turns it off.
// ---------------------------------------------------------------------------

#include <algorithm>
#include <sstream>
#include <string>

struct GameplaySettingsData
{
    float CameraShake = 1.0f;

    static constexpr float MinCameraShake = 0.0f;
    static constexpr float MaxCameraShake = 1.0f;

    void ResetToDefaults() { *this = GameplaySettingsData(); }

    void ApplyToEngine() const
    {
        // Read live by the game (PlayerCamera::Update); nothing to push.
    }

    std::string Serialize() const
    {
        std::ostringstream ss;
        ss << "CameraShake=" << CameraShake << "\n";
        return ss.str();
    }

    void ApplyLine(const std::string& key, const std::string& value)
    {
        try
        {
            if (key == "CameraShake")
                CameraShake = std::clamp(std::stof(value), MinCameraShake, MaxCameraShake);
        }
        catch (...) { /* malformed line - keep the current value */ }
    }
};

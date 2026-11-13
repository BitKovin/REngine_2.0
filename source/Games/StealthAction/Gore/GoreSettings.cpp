#include "GoreSettings.h"

#include <FileSystem/FileSystem.h>
#include <Logger.hpp>
#include <json.hpp>

namespace Gore
{
    GoreSettings& GoreSettings::Get()
    {
        static GoreSettings settings;
        return settings;
    }

    void GoreSettings::Load()
    {
        GoreSettings& s = Get();
        s = GoreSettings();

        const std::string text = FileSystemEngine::ReadFile("GameData/gore/settings.json");
        if (text.empty())
        {
            Logger::Warning("Gore: GameData/gore/settings.json missing, using defaults");
            return;
        }

        nlohmann::json j = nlohmann::json::parse(text, nullptr, false);
        if (j.is_discarded() || !j.is_object())
        {
            Logger::Error("Gore: GameData/gore/settings.json is not valid JSON, using defaults");
            return;
        }

        s.maxGibs = j.value("maxGibs", s.maxGibs);
        s.maxGibSpawnsPerFrame = j.value("maxGibSpawnsPerFrame", s.maxGibSpawnsPerFrame);
        s.maxParticlesPerFrame = j.value("maxParticlesPerFrame", s.maxParticlesPerFrame);
        s.freshBand = j.value("freshBand", s.freshBand);
        s.freeformWounds = j.value("freeformWounds", s.freeformWounds);

        const std::string level = j.value("level", std::string("full"));
        if (level == "reduced") s.level = Level::Reduced;
        else if (level == "minimal") s.level = Level::Minimal;
        else s.level = Level::Full;
    }
}

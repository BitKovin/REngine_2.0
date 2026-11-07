#pragma once
#include <UI/Preview/UiPreviewRegistry.h>

#include <map>
#include <string>

namespace UiKitPreviews
{
    // Widget, settings, kit and localisation metadata for the UI database (phase 4). Never contains setting values or translated text.
    using Registries = std::map<std::string, std::map<std::string, nlohmann::json>>;
    void FillRegistries(Registries& registries);   // "settings", "kitPages", "kitComponents", "themeTokens", "localeKeys"; also fills "locales"
    void AddRecipes();                             // widget gallery with representative states, kit settings page with isolated test settings
}

#pragma once
#include <UI/Preview/UiPreviewRegistry.h>

#include <map>
#include <string>

namespace UiPaintPreviews
{
    // Paint and motion metadata for the UI database (phase 3): effect descriptors, easing functions, animatable properties,
    // and the preview recipes of effect-bearing elements.
    using Registries = std::map<std::string, std::map<std::string, nlohmann::json>>;
    void FillRegistries(Registries& registries);        // "paintEffects", "animations", "easings"
    void AddRecipes();                                  // from the UI database refresh (recipes are rebuilt each time)
}

#pragma once
#include <UI/Preview/UiPreviewRegistry.h>

namespace UiMenuPreviews
{
    // Engine-owned preview recipes and the sample runtime provider for menus, events and input (phase 2).
    void RegisterProviders();                           // from UiSystem::Init
    void AddRecipes();                                  // from the UI database refresh (recipes are rebuilt each time)
}

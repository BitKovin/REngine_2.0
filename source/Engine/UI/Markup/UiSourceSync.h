#pragma once
#include <string>
#include <vector>

// Development builds only (not DISTRIBUTION): the game runs from a copy of GameData that the build made, so edits to the
// files in the source folder would never be seen until the next build. This looks at the source copy of the files the UI
// uses and copies the ones that changed over the running copy, so hot reload sees them.
namespace UiSourceSync
{
#ifndef DISTRIBUTION
    // `paths` are the files the UI depends on, as the engine names them ("GameData/ui/theme.css").
    // Returns the ones whose content was replaced.
    std::vector<std::string> Poll(const std::vector<std::string>& paths);
#endif
}

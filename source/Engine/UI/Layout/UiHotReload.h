#pragma once

#include "../Editor/UiEditorConfig.h"

#if UI_LAYOUT_HOT_RELOAD_ENABLED

#include <string>

// Core-runtime hot reload -- deliberately NOT part of the ImGui editor, so
// it works (and can be called from a console command) even in a dev build
// that never opens the editor UI. Gated end-to-end by
// UI_LAYOUT_HOT_RELOAD_ENABLED (see UiEditorConfig.h); this whole header's
// contents disappear in a DISTRIBUTION build.
namespace UiHotReload
{
    // Call once per frame (see INTEGRATION.md) -- cheap: just an mtime
    // check per live LoadedLayoutInstance, throttle further at the call
    // site if desired. Any instance whose file changed on disk gets
    // ReloadFromDisk()'d.
    void PollForChangedFiles();

    // Explicit trigger, for the `ui_reload` console command. Reloads every
    // live instance of `path` (there can be more than one, e.g. the same
    // styled-button layout used by several buttons on screen at once).
    // Returns how many instances were reloaded.
    int ReloadPath(const std::string& path);

    // Reloads every currently-live layout instance, regardless of path.
    // Returns how many instances were reloaded.
    int ReloadAll();
}

#endif // UI_LAYOUT_HOT_RELOAD_ENABLED

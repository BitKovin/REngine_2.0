#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cstdint>

#include "../Editor/UiEditorConfig.h"
#include "LayoutBlueprint.h"

class UiElement;

// Everything the runtime needs to know about one loaded .ui file instance:
// where it came from, its editable blueprint tree, fast lookup of its named
// children, and which single element owns it. Lives as long as its owner
// element does (owned via UiElement::OwnedLayout).
struct LoadedLayoutInstance
{
    std::string path;
    LayoutBlueprintNodePtr blueprintRoot;

    // Fast lookup for FindNamed<T>(). Scoped to THIS layout only -- a child
    // element that is itself a layout owner (e.g. a UiStyledButton placed
    // inside a menu) keeps its own "label" etc. in ITS OWN
    // LoadedLayoutInstance::namedElements, never merged up into this map.
    // To reach a name inside a child's own layout, find the child first,
    // then look inside its OwnedLayout.
    std::unordered_map<std::string, std::weak_ptr<UiElement>> namedElements;

    // The single element LoadLayoutFromFile() was called on. Every element
    // in this layout's subtree (including the owner itself) points back to
    // this instance via UiElement::MemberOfLayout.
    UiElement* ownerElement = nullptr;

#if UI_LAYOUT_HOT_RELOAD_ENABLED
    uint32_t lastLoadedModTime = 0;
#endif
};

using LoadedLayoutInstancePtr = std::shared_ptr<LoadedLayoutInstance>;

// Global weak-registry of every currently-live LoadedLayoutInstance. Exists
// independently of the ImGui editor (it's core runtime, not editor-only) so
// `ui_reload <path>` -- and, when UI_LAYOUT_HOT_RELOAD_ENABLED, automatic
// mtime polling -- can find and rebuild every instance of a given file
// without the game needing to track them itself.
class LoadedLayoutRegistry
{
public:
    static LoadedLayoutRegistry& Instance();

    void Register(const LoadedLayoutInstancePtr& instance);

    // Drops entries whose owner has been destroyed. Cheap; call before
    // iterating (both GetAll/GetByPath do this internally).
    void Prune();

    std::vector<LoadedLayoutInstancePtr> GetAll();
    std::vector<LoadedLayoutInstancePtr> GetByPath(const std::string& path);

private:
    std::vector<std::weak_ptr<LoadedLayoutInstance>> m_instances;
};

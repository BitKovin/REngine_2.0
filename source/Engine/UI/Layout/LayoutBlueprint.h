#pragma once

#include <string>
#include <vector>
#include <memory>

#include <json.hpp> // nlohmann::json

#include "../Reflection/PropertyReflection.h"

// One node of the EDITABLE blueprint tree -- pure data, no live UiElement
// involved. This is what the editor's tree view / details panel actually
// mutate; the live UiElement tree is rebuilt FROM this, not the other way
// around (see UiElement::Rebuild).
//
// The ROOT node of a .ui file describes the element LoadLayoutFromFile() is
// called ON (the "owner"): its own `properties` are applied onto that
// existing element, and `children` become that element's children. A layout
// file therefore always has exactly one owner/parent, by construction.
struct LayoutBlueprintNode
{
    std::string type; // a name registered in UiElementRegistry -- the BASE type
                       // to construct. For a layout-reference node (see
                       // `layoutPath` below) this is that file's own declared
                       // root type, e.g. "UiButton", not a special marker --
                       // there's no separate type-namespace for layout
                       // references, which is what lets any plain .ui file
                       // work as a placeable node with zero C++ registration.
    std::string name; // stable id, unique only within THIS node's own layout

    // Non-empty only for a node that is ITSELF a reference to another .ui
    // file (placed as a component/prefab-style instance rather than a plain
    // built-in type) -- see UiLayoutSerializer::BuildChildren. `type` above
    // still names the base C++ class to construct (matching that file's own
    // root type); `layoutPath` additionally tells BuildChildren to call
    // LoadLayoutFromFile(layoutPath) on the freshly constructed instance,
    // giving it that file's internal structure. `properties` on THIS node
    // are per-placement overrides applied AFTER the referenced file loads
    // (e.g. resize/reposition one particular placement without touching the
    // shared file), and `children` (normally empty for a reference node) can
    // still add further inline children on top if ever needed.
    std::string layoutPath;
                       // (see UiLayoutSerializer -- nested layouts, e.g. a
                       // styled button's internal "label", get their own
                       // independent LoadedLayoutInstance and namespace)

    // Only properties that differ from `type`'s default instance are kept
    // here (see UiElementRegistry::GetOrCreateDefaultInstance) -- keeps
    // files small and diffs meaningful. Values are stored pre-converted to
    // JSON (via PropertyKind, see UiLayoutSerializer) rather than as
    // PropertyValue, so a node can hold a property for a kind the running
    // binary doesn't currently recognize without losing it on the next save.
    nlohmann::json properties = nlohmann::json::object();

    std::vector<std::shared_ptr<LayoutBlueprintNode>> children;

    // Non-owning, for editor tree navigation only (selection, "select
    // parent", etc.) -- never serialized.
    LayoutBlueprintNode* parent = nullptr;
};

using LayoutBlueprintNodePtr = std::shared_ptr<LayoutBlueprintNode>;

// Deep-clones a node (and its subtree) with parent pointers relinked.
// Used for duplicate / paste in the editor.
LayoutBlueprintNodePtr CloneBlueprintNode(const LayoutBlueprintNodePtr& node);

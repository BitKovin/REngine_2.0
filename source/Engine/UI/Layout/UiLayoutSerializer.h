#pragma once

#include <string>
#include <memory>

#include "LayoutBlueprint.h"
#include "LoadedLayoutInstance.h"

class UiElement;

namespace UiLayoutSerializer
{
    // ── Pure data (no live elements) ────────────────────────────────────────
    LayoutBlueprintNodePtr ParseBlueprint(const std::string& jsonText);
    std::string            SerializeBlueprint(const LayoutBlueprintNodePtr& root);

    // Exposed publicly (rather than kept file-local) so UiEditor's property
    // widgets can convert a freshly-edited PropertyValue to/from the same
    // JSON representation a save/load would use, without duplicating the
    // per-PropertyKind switch.
    nlohmann::json PropertyValueToJson(const PropertyValue& value, PropertyKind kind);
    PropertyValue  PropertyValueFromJson(const nlohmann::json& json, PropertyKind kind);
    bool           PropertyValuesEqual(const PropertyValue& a, const PropertyValue& b);

    // ── Blueprint <-> live UiElement bridge ─────────────────────────────────

    // Applies `node`'s own properties onto an already-constructed `target`
    // (its concrete runtime type is looked up in UiElementRegistry by
    // `node.type` to resolve each PropertyKind). Used both for the layout's
    // root node (applied onto the owner) and for freshly-factoried children.
    void ApplyProperties(UiElement* target, const LayoutBlueprintNode& node);

    // Diffs `element`'s current property values against `typeName`'s
    // registered default instance and returns only what differs, ready to
    // drop into a LayoutBlueprintNode::properties field. Used when saving.
    nlohmann::json CaptureProperties(UiElement* element, const std::string& typeName);

    // Recursively instantiates `node`'s children as live children of
    // `parent` (building each child's own full subtree before AddChild-ing
    // it, so a single UpdateChildrenOffsetRecursive pass sees a complete
    // subtree). Every created element's MemberOfLayout is set to `layout`,
    // and non-empty names are registered into layout->namedElements.
    //
    // Does NOT recurse into a child's own nested layout, if that child's
    // constructor itself calls LoadLayoutFromFile (e.g. a UiStyledButton
    // placed inside a menu) -- that child builds and owns its own
    // LoadedLayoutInstance internally, with its own separate name
    // namespace. See LoadedLayoutInstance.h for why that isolation matters.
    void BuildChildren(UiElement* parent, const LayoutBlueprintNode& node, LoadedLayoutInstance* layout);
}

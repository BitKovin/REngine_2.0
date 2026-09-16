#include "LayoutBlueprint.h"

LayoutBlueprintNodePtr CloneBlueprintNode(const LayoutBlueprintNodePtr& node)
{
    if (!node)
        return nullptr;

    auto clone = std::make_shared<LayoutBlueprintNode>();
    clone->type       = node->type;
    clone->name       = node->name; // caller is responsible for re-uniquing this (see UiEditor::DuplicateNode)
    clone->layoutPath = node->layoutPath;
    clone->properties = node->properties; // nlohmann::json deep-copies on assignment

    clone->children.reserve(node->children.size());
    for (auto& child : node->children)
    {
        auto childClone = CloneBlueprintNode(child);
        childClone->parent = clone.get();
        clone->children.push_back(childClone);
    }

    return clone;
}

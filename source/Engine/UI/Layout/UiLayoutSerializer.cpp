#include "UiLayoutSerializer.h"
#include "../UiElement.h"
#include "../Reflection/UiElementRegistry.h"

#include "Logger.hpp"

// ── Public: PropertyValue <-> json ──────────────────────────────────────
// Exposed on the namespace (not file-local) because UiEditor's property
// widgets need the exact same conversion when a person edits a value in
// the inspector, so the in-memory blueprint always matches what Save()
// would write.

nlohmann::json UiLayoutSerializer::PropertyValueToJson(const PropertyValue& v, PropertyKind kind)
{
    switch (kind)
    {
    case PropertyKind::Bool:
        return std::get<bool>(v);
    case PropertyKind::Int:
    case PropertyKind::Enum:
        return std::get<int>(v);
    case PropertyKind::Float:
        return std::get<float>(v);
    case PropertyKind::String:
    case PropertyKind::AssetPath:
    case PropertyKind::LocKey:
        return std::get<std::string>(v);
    case PropertyKind::Vec2:
    {
        const glm::vec2 vec = std::get<glm::vec2>(v);
        return nlohmann::json::array({ vec.x, vec.y });
    }
    case PropertyKind::Vec4:
    case PropertyKind::Color:
    {
        const glm::vec4 vec = std::get<glm::vec4>(v);
        return nlohmann::json::array({ vec.x, vec.y, vec.z, vec.w });
    }
    }
    return nullptr;
}

PropertyValue UiLayoutSerializer::PropertyValueFromJson(const nlohmann::json& j, PropertyKind kind)
{
    switch (kind)
    {
    case PropertyKind::Bool:
        return PropertyValue{ j.get<bool>() };
    case PropertyKind::Int:
    case PropertyKind::Enum:
        return PropertyValue{ j.get<int>() };
    case PropertyKind::Float:
        return PropertyValue{ j.get<float>() };
    case PropertyKind::String:
    case PropertyKind::AssetPath:
    case PropertyKind::LocKey:
        return PropertyValue{ j.get<std::string>() };
    case PropertyKind::Vec2:
        return PropertyValue{ glm::vec2(j.at(0).get<float>(), j.at(1).get<float>()) };
    case PropertyKind::Vec4:
    case PropertyKind::Color:
        return PropertyValue{ glm::vec4(j.at(0).get<float>(), j.at(1).get<float>(),
                                         j.at(2).get<float>(), j.at(3).get<float>()) };
    }
    return PropertyValue{ 0.0f };
}

bool UiLayoutSerializer::PropertyValuesEqual(const PropertyValue& a, const PropertyValue& b)
{
    // bool/int/float/string/glm::vec2/glm::vec4 all provide operator==.
    return a == b;
}

// ── File-local: raw JSON <-> blueprint tree ─────────────────────────────

namespace
{
    LayoutBlueprintNodePtr ParseNode(const nlohmann::json& j, LayoutBlueprintNode* parent)
    {
        auto node = std::make_shared<LayoutBlueprintNode>();
        node->type   = j.value("type", std::string());
        node->name   = j.value("name", std::string());
        node->parent = parent;

        if (j.contains("properties") && j["properties"].is_object())
            node->properties = j["properties"];

        if (j.contains("children") && j["children"].is_array())
        {
            node->children.reserve(j["children"].size());
            for (const auto& childJson : j["children"])
                node->children.push_back(ParseNode(childJson, node.get()));
        }

        return node;
    }

    nlohmann::json NodeToJson(const LayoutBlueprintNode& node)
    {
        nlohmann::json j;
        j["type"] = node.type;
        j["name"] = node.name;

        if (!node.properties.empty())
            j["properties"] = node.properties;

        if (!node.children.empty())
        {
            nlohmann::json childrenJson = nlohmann::json::array();
            for (const auto& child : node.children)
                childrenJson.push_back(NodeToJson(*child));
            j["children"] = childrenJson;
        }

        return j;
    }
}

LayoutBlueprintNodePtr UiLayoutSerializer::ParseBlueprint(const std::string& jsonText)
{
    nlohmann::json j = nlohmann::json::parse(jsonText, nullptr, /*allow_exceptions*/ false);
    if (j.is_discarded())
    {
        Logger::Error("UiLayoutSerializer: failed to parse layout JSON (malformed file?)");
        return nullptr;
    }
    return ParseNode(j, nullptr);
}

std::string UiLayoutSerializer::SerializeBlueprint(const LayoutBlueprintNodePtr& root)
{
    if (!root)
        return "{}";
    return NodeToJson(*root).dump(2); // 2-space indent: readable + diffable in source control
}

// ── Blueprint <-> live UiElement bridge ─────────────────────────────────

void UiLayoutSerializer::ApplyProperties(UiElement* target, const LayoutBlueprintNode& node)
{
    if (!node.name.empty())
        target->name = node.name;

    for (const PropertyTable* table : UiElementRegistry::Instance().GetPropertyChain(node.type))
    {
        for (const PropertyInfo& info : table->properties)
        {
            auto it = node.properties.find(info.name);
            if (it == node.properties.end())
                continue;

            try
            {
                info.set(target, PropertyValueFromJson(it.value(), info.kind));
            }
            catch (const std::exception& e)
            {
                Logger::Log("UiLayoutSerializer: property '" + info.name + "' on '" + node.type +
                    "' ('" + node.name + "') failed to parse: " + e.what());
            }
        }
    }
}

nlohmann::json UiLayoutSerializer::CaptureProperties(UiElement* element, const std::string& typeName)
{
    nlohmann::json result = nlohmann::json::object();

    std::shared_ptr<UiElement> defaultInstance = UiElementRegistry::Instance().GetOrCreateDefaultInstance(typeName);
    if (!defaultInstance)
        return result;

    for (const PropertyTable* table : UiElementRegistry::Instance().GetPropertyChain(typeName))
    {
        for (const PropertyInfo& info : table->properties)
        {
            const PropertyValue liveValue    = info.get(element);
            const PropertyValue defaultValue = info.get(defaultInstance.get());

            if (!PropertyValuesEqual(liveValue, defaultValue))
                result[info.name] = PropertyValueToJson(liveValue, info.kind);
        }
    }

    return result;
}

void UiLayoutSerializer::BuildChildren(UiElement* parent, const LayoutBlueprintNode& node, LoadedLayoutInstance* layout)
{
    for (const auto& childNode : node.children)
    {
        std::shared_ptr<UiElement> child = UiElementRegistry::Instance().Create(childNode->type);
        if (!child)
            continue; // already logged by Create()

        // If constructing `child` already made IT the owner of its OWN
        // nested layout (e.g. a UiStyledButton loading its own .ui file
        // from its own constructor), its MemberOfLayout is already set to
        // that -- leave it alone. Picking that button should jump straight
        // into the button's own file, not this one. It's still correctly
        // reachable as a node of THIS layout via its parent chain / via
        // `layout->namedElements`; only the single MemberOfLayout pointer
        // prioritizes "what does this element own" over "what placed it".
        if (!child->MemberOfLayout)
            child->MemberOfLayout = layout;

        ApplyProperties(child.get(), *childNode);

        // No-op for composite types that already built their own subtree
        // in their own constructor (childNode->children is expected to be
        // empty for those, in a well-formed file); recurses normally for
        // plain container types (UiVerticalBox, UiCanvas, ...) whose
        // structure is described inline in this file.
        BuildChildren(child.get(), *childNode, layout);

        parent->AddChild(child);

        if (!childNode->name.empty())
        {
            if (layout->namedElements.count(childNode->name))
            {
                Logger::Log("UiLayoutSerializer: duplicate element name '" + childNode->name +
                    "' in layout '" + layout->path + "' -- FindNamed() will return whichever was built last");
            }
            layout->namedElements[childNode->name] = child;
        }
    }
}

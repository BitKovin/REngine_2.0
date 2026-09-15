#pragma once

#include <string>
#include <vector>
#include <functional>
#include <variant>
#include <glm.h>

class UiElement;

// Display/edit hint for the inspector AND the disambiguator for JSON
// (de)serialization -- e.g. Vec4 vs Color are both glm::vec4 under the
// hood, but need different ImGui widgets and are worth distinguishing in
// the property table declaration.
enum class PropertyKind
{
    Bool,
    Int,
    Float,
    String,
    Vec2,
    Vec4,
    Color,      // glm::vec4, shown as a color picker
    AssetPath,  // std::string, shown with an inline "browse" affordance
    LocKey,     // std::string, "${...}" convention (see PLAYER_HUD_STAMINA etc.)
    Enum        // stored as int, shown as a combo box using PropertyInfo::enumLabels
};

// Every serializable property value collapses to one of these. Deliberately
// NOT trying to cover std::function members (onClick, ...) or raw gameplay
// pointers (Player*, ...) -- those stay hand-authored C++ and are simply
// never registered as properties.
using PropertyValue = std::variant<bool, int, float, std::string, glm::vec2, glm::vec4>;

struct PropertyInfo
{
    std::string  name;
    PropertyKind kind = PropertyKind::Float;

    std::function<PropertyValue(UiElement*)>                   get;
    std::function<void(UiElement*, const PropertyValue&)>      set;

    // Only populated when kind == Enum: index -> display label.
    std::vector<std::string> enumLabels;
};

// Builds a PropertyInfo bound to a real member pointer on a concrete
// subclass, so a property table reads naturally:
//
//   Prop(&UiButton::Color, "Color", PropertyKind::Color)
//
// `elem` is always the actual UiElement the property table's owning type
// says it should be (UiElementRegistry never calls get/set on the wrong
// concrete type), so the static_cast here is safe.
template <typename Class, typename Member>
PropertyInfo Prop(Member Class::* memberPtr, std::string name, PropertyKind kind)
{
    PropertyInfo info;
    info.name = std::move(name);
    info.kind = kind;

    info.get = [memberPtr](UiElement* elem) -> PropertyValue
    {
        Class* self = static_cast<Class*>(elem);
        return PropertyValue{ self->*memberPtr };
    };

    info.set = [memberPtr](UiElement* elem, const PropertyValue& value)
    {
        Class* self = static_cast<Class*>(elem);
        if (const Member* v = std::get_if<Member>(&value))
            self->*memberPtr = *v;
    };

    return info;
}

// Same idea for `enum class` members, stored/edited as an int index into
// `labels` but kept type-safe on the C++ side.
template <typename Class, typename Member>
PropertyInfo PropEnum(Member Class::* memberPtr, std::string name, std::vector<std::string> labels)
{
    PropertyInfo info;
    info.name       = std::move(name);
    info.kind       = PropertyKind::Enum;
    info.enumLabels = std::move(labels);

    info.get = [memberPtr](UiElement* elem) -> PropertyValue
    {
        Class* self = static_cast<Class*>(elem);
        return PropertyValue{ static_cast<int>(self->*memberPtr) };
    };

    info.set = [memberPtr](UiElement* elem, const PropertyValue& value)
    {
        Class* self = static_cast<Class*>(elem);
        if (const int* v = std::get_if<int>(&value))
            self->*memberPtr = static_cast<Member>(*v);
    };

    return info;
}

// One type's OWN properties, plus a link to its base type's table so the
// inspector/serializer can walk the inheritance chain (UiButton -> UiElement)
// without every subclass re-declaring inherited properties.
struct PropertyTable
{
    std::string typeName;
    std::string baseTypeName; // empty for the UiElement root table
    std::vector<PropertyInfo> properties;
};

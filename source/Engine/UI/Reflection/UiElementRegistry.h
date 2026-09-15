#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <typeinfo>

#include "PropertyReflection.h"

class UiElement;

struct UiTypeInfo
{
    std::string typeName;
    std::function<std::shared_ptr<UiElement>()> factory;
    PropertyTable properties;
};

// Central registry of every UiElement-derived type that layouts are allowed
// to instantiate. Two things are registered together per type: how to
// construct it (factory), and what of it is editable/serializable
// (PropertyTable) -- see UiBuiltinReflection.cpp for the engine's own types
// and UI_REGISTER_ELEMENT below for game-code types.
//
// Only default-constructible types belong here. Composite widgets that need
// constructor arguments (WeaponSlots, UseIndicator(Player*), ...) are not,
// and are not meant to be -- they stay hand-authored C++, same as today.
class UiElementRegistry
{
public:
    static UiElementRegistry& Instance();

    template <typename T>
    void Register(const std::string& typeName, const std::string& baseTypeName, std::vector<PropertyInfo> ownProperties)
    {
        static_assert(std::is_default_constructible<T>::value,
            "UI layout element types must be default-constructible. Widgets that need "
            "constructor arguments (e.g. a Player*) are not meant to be loaded from a "
            "layout file -- keep constructing those by hand in C++.");

        UiTypeInfo info;
        info.typeName          = typeName;
        info.factory           = []() -> std::shared_ptr<UiElement> { return std::make_shared<T>(); };
        info.properties.typeName     = typeName;
        info.properties.baseTypeName = baseTypeName;
        info.properties.properties   = std::move(ownProperties);

        m_types[typeName] = std::move(info);
        m_typeIndexToName[std::type_index(typeid(T))] = typeName;
    }

    const UiTypeInfo* Find(const std::string& typeName) const;

    // Reverse lookup: registered type name for a live element's actual
    // runtime type (requires UiElement to be polymorphic, which it is).
    // Used to sanity-check a layout's root node type against the element
    // LoadLayoutFromFile was called on, and to let the editor capture/save
    // properties starting from an arbitrary already-live element. Returns
    // "" if that concrete type was never registered.
    std::string GetTypeNameForInstance(const UiElement* instance) const;

    std::shared_ptr<UiElement> Create(const std::string& typeName) const;

    // Root-first (UiElement, then UiButton, ...) so callers can render
    // inherited properties before a type's own.
    std::vector<const PropertyTable*> GetPropertyChain(const std::string& typeName) const;

    // A cached, never-mutated default instance of `typeName`, used to diff
    // a live element's current property values when saving (see
    // UiLayoutSerializer::CaptureProperties).
    std::shared_ptr<UiElement> GetOrCreateDefaultInstance(const std::string& typeName);

    std::vector<std::string> GetAllTypeNames() const;

    // Registers every engine-provided widget type (UiButton, UiText, UiImage,
    // ...). Implemented in UiBuiltinReflection.cpp. MUST be called explicitly
    // -- once, from EngineMain::Init() -- because these types live in the
    // engine's static library; see the long comment in UiElementRegistry.cpp
    // for why that means they can't reliably self-register.
    static void RegisterBuiltinTypes();

private:
    std::unordered_map<std::string, UiTypeInfo> m_types;
    std::unordered_map<std::string, std::shared_ptr<UiElement>> m_defaultInstances;
    std::unordered_map<std::type_index, std::string> m_typeIndexToName;
};

// Self-registration helper for GAME-CODE widget types only (see the note on
// RegisterBuiltinTypes above and the comment in the .cpp for why this is
// safe for game code compiled into the .exe but not for the engine lib).
struct UiAutoRegister
{
    explicit UiAutoRegister(const std::function<void()>& registerFn) { registerFn(); }
};

// Usage, in a game-code .cpp (NOT a header):
//
//   UI_REGISTER_ELEMENT(UiStyledButton, "UiButton",
//       Prop(&UiStyledButton::SomeExtraField, "SomeExtraField", PropertyKind::Float)
//   );
//
// Base type name is a string (not a template param) since the base's own
// PropertyTable is looked up by name at use time, not at registration time --
// registration order between a type and its base doesn't matter.
#define UI_REGISTER_ELEMENT(TypeName, BaseTypeName, ...)                          \
    static UiAutoRegister _uiAutoRegister_##TypeName(                             \
        std::function<void()>([]()                                               \
        {                                                                         \
            UiElementRegistry::Instance().Register<TypeName>(                     \
                #TypeName, BaseTypeName, { __VA_ARGS__ });                        \
        }))

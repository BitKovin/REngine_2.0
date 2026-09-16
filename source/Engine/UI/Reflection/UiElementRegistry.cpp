#include "UiElementRegistry.h"
#include "../UiElement.h"

#include <algorithm>

// Swap for whatever the engine's actual logging include/macro is.
#include "Logger.hpp"

/*
 * ───────────────────────────────────────────────────────────────────────
 * Why there are two registration paths (macro vs. manual list)
 * ───────────────────────────────────────────────────────────────────────
 * UI_REGISTER_ELEMENT expands to a namespace-scope object whose constructor
 * runs the registration as a side effect (`static UiAutoRegister x = ...;`).
 * That works reliably as long as the .cpp it lives in is actually linked
 * into the final binary.
 *
 * For a .cpp compiled straight into an executable project, that's
 * guaranteed -- the linker includes every object file from an exe's own
 * translation units regardless of whether anything else references a
 * symbol from it.
 *
 * For a .cpp compiled into the ENGINE, which ships as a static library
 * (.lib/.a), that guarantee disappears. Static-library linking pulls in
 * object files lazily, one unresolved-symbol reference at a time. A
 * translation unit whose only content is a self-registering global has no
 * symbol anything else needs to reference, so the linker is free to never
 * pull that .o out of the .lib at all -- and if it doesn't, the static
 * initializer simply never runs. No error, no warning: the type is just
 * silently missing from the registry at runtime.
 *
 * So: game-code widget types (compiled into the .exe) use
 * UI_REGISTER_ELEMENT freely, in a .cpp, wherever's convenient. Engine
 * built-in widget types are instead registered by an explicit, hand
 * maintained list of Register<T>(...) calls in RegisterBuiltinTypes()
 * below, which EngineMain::Init() calls directly -- a real call from a
 * real reachable function, not relying on a linker implementation detail.
 */

UiElementRegistry& UiElementRegistry::Instance()
{
    static UiElementRegistry instance;
    return instance;
}

const UiTypeInfo* UiElementRegistry::Find(const std::string& typeName) const
{
    auto it = m_types.find(typeName);
    return it != m_types.end() ? &it->second : nullptr;
}

std::shared_ptr<UiElement> UiElementRegistry::Create(const std::string& typeName) const
{
    const UiTypeInfo* info = Find(typeName);
    if (!info)
    {
        Logger::Log("UiElementRegistry: unknown element type '" + typeName +
            "' -- is it registered? (built-ins: RegisterBuiltinTypes; game types: UI_REGISTER_ELEMENT)");
        return nullptr;
    }
    return info->factory();
}

std::vector<const PropertyTable*> UiElementRegistry::GetPropertyChain(const std::string& typeName) const
{
    std::vector<const PropertyTable*> chain;

    std::string current = typeName;
    while (!current.empty())
    {
        const UiTypeInfo* info = Find(current);
        if (!info)
            break;

        chain.push_back(&info->properties);
        current = info->properties.baseTypeName;
    }

    std::reverse(chain.begin(), chain.end()); // root (UiElement) first, most-derived last
    return chain;
}

std::shared_ptr<UiElement> UiElementRegistry::GetOrCreateDefaultInstance(const std::string& typeName)
{
    auto it = m_defaultInstances.find(typeName);
    if (it != m_defaultInstances.end())
        return it->second;

    std::shared_ptr<UiElement> inst = Create(typeName);
    m_defaultInstances[typeName] = inst;
    return inst;
}

std::string UiElementRegistry::GetTypeNameForInstance(const UiElement* instance) const
{
    if (!instance)
        return {};

    auto it = m_typeIndexToName.find(std::type_index(typeid(*instance)));
    return it != m_typeIndexToName.end() ? it->second : std::string();
}

void UiElementRegistry::AssociateLayoutPath(const std::string& typeName, const std::string& layoutPath)
{
    m_layoutPathToTypeName[layoutPath] = typeName;
}

std::string UiElementRegistry::GetTypeNameForLayoutPath(const std::string& layoutPath) const
{
    auto it = m_layoutPathToTypeName.find(layoutPath);
    return it != m_layoutPathToTypeName.end() ? it->second : std::string();
}

std::vector<std::string> UiElementRegistry::GetAllTypeNames() const
{
    std::vector<std::string> names;
    names.reserve(m_types.size());
    for (auto& [name, info] : m_types)
        names.push_back(name);
    std::sort(names.begin(), names.end());
    return names;
}

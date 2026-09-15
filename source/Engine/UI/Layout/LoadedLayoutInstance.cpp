#include "LoadedLayoutInstance.h"
#include "../UiElement.h"
#include <algorithm>

LoadedLayoutRegistry& LoadedLayoutRegistry::Instance()
{
    static LoadedLayoutRegistry instance;
    return instance;
}

void LoadedLayoutRegistry::Register(const LoadedLayoutInstancePtr& instance)
{
    m_instances.push_back(instance);
}

void LoadedLayoutRegistry::Prune()
{
    m_instances.erase(
        std::remove_if(m_instances.begin(), m_instances.end(),
            [](const std::weak_ptr<LoadedLayoutInstance>& w) { return w.expired(); }),
        m_instances.end());
}

std::vector<LoadedLayoutInstancePtr> LoadedLayoutRegistry::GetAll()
{
    Prune();

    std::vector<LoadedLayoutInstancePtr> result;
    result.reserve(m_instances.size());
    for (auto& w : m_instances)
        if (auto locked = w.lock())
            result.push_back(locked);

    return result;
}

std::vector<LoadedLayoutInstancePtr> LoadedLayoutRegistry::GetByPath(const std::string& path)
{
    std::vector<LoadedLayoutInstancePtr> result;
    for (auto& instance : GetAll())
        if (instance->path == path)
            result.push_back(instance);

    return result;
}

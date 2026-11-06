#include "UiPreviewRegistry.h"

#include <UI/Markup/UiMarkupLog.h>
#include <UI/Preview/UiDatabase.h>

#include <algorithm>
#include <map>

namespace
{
    std::map<std::string, std::shared_ptr<IUiPreviewProvider>>& Providers()
    {
        static std::map<std::string, std::shared_ptr<IUiPreviewProvider>> m;
        return m;
    }
    std::map<std::string, UiPreviewDescriptor>& Recipes()
    {
        static std::map<std::string, UiPreviewDescriptor> m;
        return m;
    }
}

bool UiPreviewRegistry::RegisterProvider(std::shared_ptr<IUiPreviewProvider> provider)
{
    if (!provider)
        return false;
    const UiPreviewDescriptor d = provider->Describe();
    if (d.ProviderId.empty() || d.Id.empty())
    {
        UiMarkupLog::Error("<preview>", 0, 0, "preview provider needs a provider id and a descriptor id");
        return false;
    }
    Providers()[d.ProviderId] = std::move(provider);
    UiDatabase::MarkDirty("preview provider " + d.ProviderId);
    return true;
}

void UiPreviewRegistry::UnregisterProvider(const std::string& providerId)
{
    if (Providers().erase(providerId) > 0)
        UiDatabase::MarkDirty("preview provider removed " + providerId);
}

IUiPreviewProvider* UiPreviewRegistry::FindProvider(const std::string& providerId)
{
    const auto it = Providers().find(providerId);
    return it == Providers().end() ? nullptr : it->second.get();
}

void UiPreviewRegistry::RegisterRecipe(const UiPreviewDescriptor& descriptor)
{
    Recipes()[descriptor.Id] = descriptor;
}

void UiPreviewRegistry::ClearRecipes()
{
    Recipes().clear();
}

std::vector<UiPreviewDescriptor> UiPreviewRegistry::Descriptors()
{
    std::map<std::string, UiPreviewDescriptor> all = Recipes();
    for (const auto& [id, provider] : Providers())
    {
        UiPreviewDescriptor d = provider->Describe();
        all[d.Id] = d;
    }
    std::vector<UiPreviewDescriptor> out;
    for (auto& [id, d] : all)
        out.push_back(d);
    return out;
}

nlohmann::json UiPreviewRegistry::ToJson(const UiPreviewDescriptor& d)
{
    using nlohmann::json;
    json j = json::object();
    j["id"] = d.Id;
    j["name"] = d.Name;
    j["category"] = d.Category;
    j["description"] = d.Description;
    j["kind"] = d.Kind;
    if (!d.ProviderId.empty())
    {
        j["providerId"] = d.ProviderId;
        j["providerVersion"] = d.ProviderVersion;
    }
    json host = json::object();
    host["kind"] = d.HostKind;
    if (!d.HostLayer.empty())
        host["layer"] = d.HostLayer;
    j["host"] = host;
    json refs = json::array();
    for (const auto& r : d.SourceRefs)
        refs.push_back({ { "kind", r.Kind }, { "path", r.Path } });
    j["sourceRefs"] = refs;
    j["requiredCapabilities"] = d.RequiredCapabilities;
    json params = json::array();
    for (const auto& p : d.Parameters)
    {
        json pj = json::object();
        pj["name"] = p.Name;
        pj["type"] = p.Type;
        pj["default"] = p.Default;
        if (!p.Min.is_null()) pj["min"] = p.Min;
        if (!p.Max.is_null()) pj["max"] = p.Max;
        if (!p.Step.is_null()) pj["step"] = p.Step;
        if (!p.Choices.empty()) pj["choices"] = p.Choices;
        if (!p.Unit.empty()) pj["unit"] = p.Unit;
        if (!p.Help.empty()) pj["help"] = p.Help;
        params.push_back(pj);
    }
    j["parameters"] = params;
    json scenarios = json::array();
    for (const auto& s : d.Scenarios)
        scenarios.push_back({ { "id", s.Id }, { "label", s.Label }, { "values", s.Values } });
    j["scenarios"] = scenarios;
    json animationScenarios = json::array();
    for (const auto& s : d.AnimationScenarios)
        animationScenarios.push_back({ { "id", s.Id }, { "label", s.Label }, { "values", s.Values } });
    j["animationScenarios"] = animationScenarios;
    j["supportsLiveCapture"] = d.SupportsLiveCapture;
    return j;
}

#pragma once
#include <json.hpp>
#include <memory>
#include <string>
#include <vector>

class UiElem;

// Typed preview descriptors and provider registration (conventions §13.4, §13.5).
// Only descriptors are serialized into the UI database; provider callbacks stay compiled C++ code.

struct UiPreviewParameter
{
    std::string Name;
    std::string Type;                 // "float", "int", "bool", "string", "color", "vec2", "asset", "choice"
    nlohmann::json Default;
    nlohmann::json Min, Max, Step;    // null when unused
    std::vector<std::string> Choices;
    std::string Unit;
    std::string Help;
};

struct UiPreviewScenario
{
    std::string Id;
    std::string Label;
    nlohmann::json Values = nlohmann::json::object();
};

struct UiPreviewSourceRef
{
    std::string Kind;                 // "document", "component", "code"
    std::string Path;                 // GameData-relative for documents/components
};

struct UiPreviewDescriptor
{
    std::string Id;
    std::string Name;
    std::string Category;
    std::string Description;
    std::string Kind;                 // document, component, element, runtime, hosted, world-surface, scene-view
    std::string ProviderId;           // empty for engine-generated recipes
    int ProviderVersion = 1;
    std::vector<UiPreviewSourceRef> SourceRefs;
    std::string HostKind = "viewport";   // viewport, menu-stack, hud-layer, overlay, world-surface, loading-screen, scene-view
    std::string HostLayer;
    std::vector<std::string> RequiredCapabilities;
    std::vector<UiPreviewParameter> Parameters;
    std::vector<UiPreviewScenario> Scenarios;
    bool SupportsLiveCapture = false;
};

// Engine-owned adapter the provider receives. Phase 1 only has the plain viewport host.
class UiPreviewHost
{
public:
    std::string Kind = "viewport";
};

class IUiPreviewProvider
{
public:
    virtual ~IUiPreviewProvider() = default;
    virtual UiPreviewDescriptor Describe() const = 0;
    virtual bool CreatePreview(UiPreviewHost& host, const nlohmann::json& values, std::shared_ptr<UiElem>& outRoot, std::string& error) = 0;
    virtual bool ApplyValues(UiPreviewHost& host, UiElem& root, const nlohmann::json& values, std::string& error) = 0;
    virtual bool ApplyScenario(UiPreviewHost& host, UiElem& root, const std::string& scenarioId, std::string& error) = 0;
    virtual void DestroyPreview(UiPreviewHost& host, std::shared_ptr<UiElem>& root) = 0;
};

namespace UiPreviewRegistry
{
    // Providers are registered by the game's explicit UI registration path (no static initialisers).
    bool RegisterProvider(std::shared_ptr<IUiPreviewProvider> provider);
    void UnregisterProvider(const std::string& providerId);
    IUiPreviewProvider* FindProvider(const std::string& providerId);

    // Engine-generated recipes (documents, components, elements): descriptors without a provider.
    void RegisterRecipe(const UiPreviewDescriptor& descriptor);
    void ClearRecipes();

    // All descriptors, sorted by id (recipes and provider descriptors).
    std::vector<UiPreviewDescriptor> Descriptors();
    nlohmann::json ToJson(const UiPreviewDescriptor& descriptor);   // never contains callbacks or pointers
}

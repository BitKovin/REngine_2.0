#pragma once
#include <json.hpp>
#include <cstdint>
#include <string>

// The single owner of the generated UI database (`SaveData/UIStudio/uidb.json`, conventions §13).
// Registries publish into it; nothing else writes the file. The database stores serializable metadata only.
namespace UiDatabase
{
    constexpr int kFormatVersion = 1;
    constexpr int kPreviewProtocol = 1;

    // Early bootstrap (EngineMain::Init after UiElementRegistry::RegisterBuiltinTypes and Localisation::Initialize):
    // establishes the identity and writes a baseline snapshot from the registries available in this build.
    void InitBootstrap();
    // Games may register a stable id / display name before or after the bootstrap. The fallback is the executable name.
    void SetGameIdentity(const std::string& id, const std::string& displayName);

    // Registry changed. `reason` is diagnostic only and never serialized.
    void MarkDirty(const std::string& reason);
    uint64_t Revision();
    bool IsDirty();

    // Capabilities describe the actual build; they default to false.
    void SetCapability(const std::string& name, bool value);

    // Rebuilds every registry section from the live registries, then flushes (UiSystem::Init, hot reload).
    void RefreshFromRegistries();
    // Writes the file when the database is dirty (call from a safe main-thread point).
    void Flush();

    // Extension point for registries that do not live in the engine (later phases, games): typed JSON entries.
    void PublishEntry(const std::string& registry, const std::string& id, const nlohmann::json& entry);
    void RemoveEntry(const std::string& registry, const std::string& id);

    // Deterministic UTF-8 JSON of the current state.
    std::string Serialize();
    const std::string& LastWritten();

    // Database paths are GameData-relative, use '/', keep case and never contain absolute paths or traversal.
    // A leading "GameData/" is stripped. Returns false and sets `error` for anything else.
    bool NormalizePath(const std::string& in, std::string& normalized, std::string& error);

    // Output location (tests point it at an unwritable place).
    void SetOutputPath(const std::string& path);
    const std::string& OutputPath();
    bool LastWriteSucceeded();
    int WriteCount();

    // Unit tests of the serializer, path policy and preview descriptors. Returns false and fills `error` on failure.
    bool SelfTest(std::string& error);
}

#include "UiDatabase.h"

#include <FileSystem/FileSystem.h>
#include <Input.h>
#include <Logger.hpp>
#include <UI/Markup/UiDocument.h>
#include <UI/Menus/UiActions.h>
#include <UI/Menus/UiInputPrompts.h>
#include <UI/Preview/UiMenuPreviews.h>
#include <UI/Preview/UiKitPreviews.h>
#include <UI/Preview/UiPaintPreviews.h>
#include <UI/Markup/UiTagRegistry.h>
#include <UI/Preview/UiPreviewRegistry.h>
#include <UI/Reflection/UiElementRegistry.h>
#include <UI/Style/UiComputedStyle.h>
#include <UI/Style/UiSelector.h>
#include <UI/Test/UiTestRunner.h>

#include <algorithm>
#include <filesystem>
#include <map>
#include <tuple>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace
{
    using nlohmann::json;

    struct State
    {
        std::string GameId;
        std::string DisplayName;
        bool IdentityExplicit = false;
        std::map<std::string, bool> Capabilities;
        // registry name -> id -> entry
        std::map<std::string, std::map<std::string, json>> Registries;
        std::map<std::string, std::map<std::string, json>> Extra;
        std::map<std::string, json> Previews;
        uint64_t Revision = 0;
        bool Dirty = false;
        std::string OutputPath = "SaveData/UIStudio/uidb.json";
        std::string Last;
        bool WriteOk = true;
        int Writes = 0;
        bool WarnedWrite = false;
        bool Bootstrapped = false;
    };

    State& S()
    {
        static State s;
        return s;
    }

    const char* kRegistryNames[] = { "documents", "components", "tags", "cssProperties", "pseudoClasses", "actions", "inputActions", "hosts", "locales", "testCommands",
                                      "paintEffects", "animations", "easings", "settings", "kitPages", "kitComponents", "themeTokens", "localeKeys" };

    std::string ExecutableName()
    {
#ifdef _WIN32
        char buffer[MAX_PATH] = {};
        const DWORD n = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
        if (n > 0)
        {
            std::string p(buffer, n);
            const size_t slash = p.find_last_of("\\/");
            if (slash != std::string::npos) p = p.substr(slash + 1);
            const size_t dot = p.find_last_of('.');
            if (dot != std::string::npos) p = p.substr(0, dot);
            if (!p.empty()) return p;
        }
#endif
        return "game";
    }

    std::string KindName(PropertyKind k)
    {
        switch (k)
        {
        case PropertyKind::Bool: return "bool";
        case PropertyKind::Int: return "int";
        case PropertyKind::Float: return "float";
        case PropertyKind::String: return "string";
        case PropertyKind::Vec2: return "vec2";
        case PropertyKind::Vec4: return "vec4";
        case PropertyKind::Color: return "color";
        case PropertyKind::AssetPath: return "asset";
        case PropertyKind::LocKey: return "localisation-key";
        case PropertyKind::Enum: return "enum";
        }
        return "string";
    }

    json AttributeJson(const UiTagRegistry::AttributeInfo& a)
    {
        json j = json::object();
        j["name"] = a.Name;
        j["type"] = a.Type;
        j["default"] = a.Default;
        j["required"] = a.Required;
        j["description"] = a.Description;
        j["variables"] = a.Type == "string" || a.Type == "text" || a.Type == "path" || a.Type == "bool";
        return j;
    }

    void FillLegacyTags(State& s)
    {
        auto& registry = UiElementRegistry::Instance();
        std::vector<std::string> names = registry.GetAllTypeNames();
        std::sort(names.begin(), names.end());
        for (const std::string& name : names)
        {
            json j = json::object();
            j["name"] = name;
            j["category"] = "legacy";
            j["legacy"] = true;
            json attrs = json::array();
            for (const PropertyTable* table : registry.GetPropertyChain(name))
                for (const PropertyInfo& p : table->properties)
                    attrs.push_back({ { "name", p.name }, { "type", KindName(p.kind) } });
            j["attributes"] = attrs;
            s.Registries["tags"]["legacy:" + name] = j;
        }
    }

    void FillMarkup(State& s)
    {
        for (const auto& tag : UiTagRegistry::BuiltinInfos())
        {
            json j = json::object();
            j["name"] = tag.Name;
            j["category"] = tag.Category;
            j["description"] = tag.Description;
            j["legacy"] = false;
            json attrs = json::array();
            for (const auto& a : UiTagRegistry::StandardAttributes()) attrs.push_back(AttributeJson(a));
            for (const auto& a : tag.Attributes) attrs.push_back(AttributeJson(a));
            j["attributes"] = attrs;
            s.Registries["tags"]["builtin:" + tag.Name] = j;
        }

        for (const UiPropertyInfo& p : UiStyleProperties())
        {
            json j = json::object();
            j["name"] = p.Name;
            j["grammar"] = p.Grammar;
            j["inherited"] = p.Inherited;
            j["initial"] = p.Initial;
            j["invalidates"] = p.Invalidates;
            j["shorthand"] = !p.Longhand;
            s.Registries["cssProperties"][p.Name] = j;
        }
        for (const std::string& name : UiPseudoNames())
            s.Registries["pseudoClasses"][name] = json{ { "name", name } };

        // Globally registered components, plus those defined by documents that were loaded (imported component files).
        std::vector<const UiComponentDef*> componentDefs = UiComponentRegistry::All();
        std::vector<std::shared_ptr<UiDocumentDef>> keepAlive;
        for (const std::string& full : UiDocumentCache_Paths())
            if (const auto doc = UiDocumentCache_Get(full))
                if (doc->Component != nullptr)
                {
                    keepAlive.push_back(doc);
                    componentDefs.push_back(doc->Component.get());
                }
        for (const UiComponentDef* def : componentDefs)
        {
            std::string path, error;
            json j = json::object();
            j["tag"] = def->Tag;
            if (UiDatabase::NormalizePath(def->File, path, error))
                j["path"] = path;
            json params = json::array();
            for (size_t i = 0; i < def->Params.size(); ++i)
                params.push_back({ { "name", def->Params[i].first }, { "default", def->Params[i].second },
                                   { "required", i < def->ParamRequired.size() && def->ParamRequired[i] } });
            j["parameters"] = params;
            j["slots"] = def->Slots;
            s.Registries["components"][def->Tag] = j;
        }

        // Documents: file listing of the conventional folders; parsed details only for documents already loaded.
        static const char* kDirs[] = { "GameData/ui/", "GameData/ui/kit/", "GameData/ui/kit/tests/", "GameData/ui/components/" };
        for (const char* dir : kDirs)
        {
            for (const std::string& file : FileSystemEngine::GetFilesInPath(dir))
            {
                if (file.size() < 4 || file.compare(file.size() - 4, 4, ".xml") != 0)
                    continue;
                const std::string full = std::string(dir) + file;
                std::string path, error;
                if (!UiDatabase::NormalizePath(full, path, error))
                    continue;
                json j = json::object();
                j["path"] = path;
                s.Registries["documents"][path] = j;
            }
        }
        for (const std::string& full : UiDocumentCache_Paths())
        {
            std::string path, error;
            if (!UiDatabase::NormalizePath(full, path, error))
                continue;
            json j = json::object();
            j["path"] = path;
            if (const auto def = UiDocumentCache_Get(full))
            {
                json css = json::array();
                for (const std::string& f : def->SheetFiles)
                {
                    std::string cp;
                    if (UiDatabase::NormalizePath(f, cp, error)) css.push_back(cp);
                }
                j["styleSheets"] = css;
                json imports = json::array();
                for (const std::string& f : def->Imports)
                {
                    std::string ip;
                    if (UiDatabase::NormalizePath(f, ip, error)) imports.push_back(ip);
                }
                j["imports"] = imports;
                j["component"] = def->Component != nullptr;
            }
            s.Registries["documents"][path] = j;
        }
    }

    // Phase 2: events and actions, input actions with prompt metadata, hosts.
    void FillMenus(State& s)
    {
        for (const UiActionInfo& a : UiActions::Builtins())
            s.Registries["actions"][a.Name] = json{ { "name", a.Name }, { "kind", "action" }, { "arguments", a.Args }, { "description", a.Description }, { "builtin", true } };
        for (const UiEventInfo& e : UiActions::Events())
            s.Registries["actions"]["event:" + e.Name] = json{ { "name", e.Name }, { "kind", "event" }, { "attribute", "on" + e.Name }, { "description", e.Description } };
        s.Registries["actions"]["@resolution"] = json{ { "name", "@resolution" }, { "kind", "rules" },
            { "rules", json::array({ "The element and its UiElem ancestors are searched for a handler registered with On(name); the first one runs it",
                                     "Otherwise a built-in action of that name runs",
                                     "Otherwise a warning 'unhandled action' is logged once per element and name" }) } };

        for (const std::string& name : Input::GetActionNames())
        {
            InputAction* action = Input::GetAction(name);
            json keys = json::array(), buttons = json::array(), mouse = json::array();
            for (SDL_Scancode k : action->keys) keys.push_back(SDL_GetScancodeName(k));
            for (GamepadButton b : action->buttons) buttons.push_back((int)b);
            for (uint8_t m : action->mouseButtons) mouse.push_back((int)m);
            json prompts = json::object();
            if (UiInputPrompts::HasProvider())
            {
                // An empty path from the provider means no prompt can be resolved for that device.
                for (UiInputDevice d : { UiInputDevice::KeyboardMouse, UiInputDevice::Gamepad, UiInputDevice::Touch })
                    prompts[UiInputPrompts::DeviceClass(d)] = !UiInputPrompts::ResolveFor(name, d).empty();
            }
            s.Registries["inputActions"][name] = json{ { "name", name }, { "label", name }, { "keys", keys }, { "gamepadButtons", buttons }, { "mouseButtons", mouse },
                                                       { "promptResolvable", prompts } };
        }

        json options = json::array();
        for (const auto& o : { std::make_tuple("close-on-back", "true", "Back pops the page"), std::make_tuple("shows-below", "false", "Pages under it stay visible"),
                               std::make_tuple("pauses-game", "false", "The game is paused while the page is on the stack"),
                               std::make_tuple("hides-hud", "false", "The HUD is hidden while the page is on the stack") })
            options.push_back({ { "attribute", std::get<0>(o) }, { "default", std::get<1>(o) }, { "description", std::get<2>(o) } });
        s.Registries["hosts"]["menu-stack"] = json{ { "name", "menu-stack" }, { "description", "Modal stack of pages; any element can be a page" },
            { "options", options }, { "lifecycle", json::array({ "OnPushed", "OnShown", "OnHidden", "OnPopped", "OnBack" }) },
            { "back", json{ { "cancelAction", "ui_cancel" }, { "event", "cancel" }, { "emptyStack", "OnEmptyBack" } } },
            { "focusTrap", true }, { "topPageOnlyInteractive", true }, { "pauseHook", "OnPauseChanged" } };
        s.Registries["hosts"]["drag-drop"] = json{ { "name", "drag-drop" }, { "description", "Pointer and gamepad drag and drop between elements" },
            { "attributes", json::array({ "draggable", "droptarget" }) }, { "events", json::array({ "dragstart", "dragenter", "dragleave", "drop" }) },
            { "pseudoClass", "dragging" }, { "dragThresholdPx", 6 }, { "ghostOpacity", 0.7 },
            { "gamepad", json{ { "pickUp", "ui_confirm on a draggable" }, { "drop", "ui_confirm on a drop target" }, { "abort", "ui_cancel" } } },
            { "payload", "C++ only: UiElem::DragPayload (std::any), set in dragstart; never serialized, previews use test data" },
            { "requiresInput", json::array({ "pointer or gamepad focus" }) } };
        s.Registries["hosts"]["context-menu"] = json{ { "name", "context-menu" }, { "description", "oncontextmenu fires where the pointer is; the ContextMenu action pushes a page there" },
            { "event", "contextmenu" }, { "action", "ContextMenu" }, { "page", json{ { "host", "menu-stack" }, { "shows-below", true }, { "placedElementClass", "context-menu" } } },
            { "triggers", json::array({ "right mouse button (ui_context)", "touch hold 0.5 s", "ui_context action (gamepad focus)" }) },
            { "requiresInput", json::array({ "ui_context" }) } };
        s.Registries["hosts"]["viewport"] = json{ { "name", "viewport" }, { "description", "Plain parent: no lifecycle, no host options" } };
    }

    // Phase 3: paint effects, easing functions, animatable properties, and the @keyframes of every sheet that has been loaded.
    void FillPaint(State& s)
    {
        UiPaintPreviews::FillRegistries(s.Registries);
        UiKitPreviews::FillRegistries(s.Registries);

        std::vector<std::shared_ptr<UiStyleSheet>> sheets = UiStyleSheetCache::Cached();
        for (const std::string& full : UiDocumentCache_Paths())
            if (const auto doc = UiDocumentCache_Get(full))
                for (const auto& sheet : doc->Sheets)
                    sheets.push_back(sheet);
        for (const auto& sheet : sheets)
        {
            if (!sheet)
                continue;
            for (const UiKeyframes& k : sheet->Keyframes)
            {
                std::string path, error;
                std::string file = sheet->Path.rfind("<inline:", 0) == 0 ? "" : sheet->Path;
                json j = json::object();
                j["name"] = k.Name;
                j["kind"] = "keyframes";
                if (!file.empty() && UiDatabase::NormalizePath(file, path, error))
                    j["path"] = path;
                j["line"] = k.Line;
                json stops = json::array();
                std::map<std::string, std::vector<float>> tracks;
                for (const UiKeyframeStop& stop : k.Stops)
                {
                    json props = json::array();
                    for (const UiCssDeclaration& d : stop.Declarations)
                    {
                        props.push_back(d.Name);
                        tracks[d.Name].push_back(stop.Offset);
                    }
                    stops.push_back({ { "offset", stop.Offset }, { "properties", props } });
                }
                j["stops"] = stops;
                json trackList = json::array();
                for (const auto& [property, offsets] : tracks)
                    trackList.push_back({ { "property", property }, { "offsets", offsets } });
                j["tracks"] = trackList;
                s.Registries["animations"]["keyframes:" + k.Name] = j;
            }
        }
    }

    void FillTestCommands(State& s)
    {
        for (const std::string& name : UiTestRunner::CommandNames())
            s.Registries["testCommands"][name] = json{ { "name", name } };
    }

    void FillRecipes(State& s)
    {
        UiPreviewRegistry::ClearRecipes();
        for (const auto& [path, entry] : s.Registries["documents"])
        {
            UiPreviewDescriptor d;
            d.Id = "document:" + path;
            d.Name = path;
            d.Category = "Documents";
            d.Description = "Layout document";
            d.Kind = "document";
            d.SourceRefs.push_back({ "document", path });
            UiPreviewRegistry::RegisterRecipe(d);
        }
        for (const auto& [tag, entry] : s.Registries["components"])
        {
            UiPreviewDescriptor d;
            d.Id = "component:" + tag;
            d.Name = tag;
            d.Category = "Components";
            d.Description = "Component";
            d.Kind = "component";
            if (entry.contains("path")) d.SourceRefs.push_back({ "component", entry["path"].get<std::string>() });
            for (const auto& p : entry["parameters"])
            {
                UiPreviewParameter param;
                param.Name = p["name"].get<std::string>();
                param.Type = "string";
                param.Default = p["default"];
                d.Parameters.push_back(param);
            }
            UiPreviewRegistry::RegisterRecipe(d);
        }
        UiMenuPreviews::AddRecipes();
        UiPaintPreviews::AddRecipes();
        UiKitPreviews::AddRecipes();
        for (const auto& [key, entry] : s.Registries["tags"])
        {
            UiPreviewDescriptor d;
            d.Id = "element:" + key;
            d.Name = entry["name"].get<std::string>();
            d.Category = entry.value("legacy", false) ? "Legacy elements" : "Elements";
            d.Description = "Single element";
            d.Kind = "element";
            UiPreviewRegistry::RegisterRecipe(d);
        }
    }

    void Rebuild(State& s, bool markup)
    {
        s.Registries.clear();
        for (const char* name : kRegistryNames)
            s.Registries[name];
        FillLegacyTags(s);
        FillTestCommands(s);
        if (markup)
        {
            FillMarkup(s);
            FillMenus(s);
            FillPaint(s);
        }
        for (const auto& [registry, entries] : s.Extra)
            for (const auto& [id, entry] : entries)
                s.Registries[registry][id] = entry;
        if (markup)
            FillRecipes(s);
        else
            UiPreviewRegistry::ClearRecipes();
        s.Previews.clear();
        for (const UiPreviewDescriptor& d : UiPreviewRegistry::Descriptors())
            s.Previews[d.Id] = UiPreviewRegistry::ToJson(d);
    }

    bool WriteFileAtomic(State& s, const std::string& content)
    {
        const std::string tmp = s.OutputPath + ".tmp";
        if (!FileSystemEngine::WriteFile(tmp, content))
            return false;
        try
        {
            std::filesystem::rename(tmp, s.OutputPath);
            return true;
        }
        catch (...)
        {
            // Platforms where the VFS path is not a plain file path: write the final file directly.
            const bool ok = FileSystemEngine::WriteFile(s.OutputPath, content);
            try { std::filesystem::remove(tmp); } catch (...) {}
            return ok;
        }
    }
}

void UiDatabase::SetGameIdentity(const std::string& id, const std::string& displayName)
{
    State& s = S();
    s.GameId = id;
    s.DisplayName = displayName.empty() ? id : displayName;
    s.IdentityExplicit = true;
    MarkDirty("game identity");
}

void UiDatabase::InitBootstrap()
{
    State& s = S();
    if (!s.IdentityExplicit)
    {
        s.GameId = ExecutableName();
        s.DisplayName = s.GameId;
    }
    s.Bootstrapped = true;
    for (const char* cap : { "markup", "events", "hudLayers" })
        if (!s.Capabilities.count(cap))
            s.Capabilities[cap] = false;
    Rebuild(s, s.Capabilities["markup"]);
    s.Dirty = true;
    Flush();
}

void UiDatabase::MarkDirty(const std::string& reason)
{
    (void)reason;   // diagnostic only
    State& s = S();
    ++s.Revision;
    s.Dirty = true;
}

uint64_t UiDatabase::Revision() { return S().Revision; }
bool UiDatabase::IsDirty() { return S().Dirty; }

void UiDatabase::SetCapability(const std::string& name, bool value)
{
    State& s = S();
    if (s.Capabilities.count(name) && s.Capabilities[name] == value)
        return;
    s.Capabilities[name] = value;
    MarkDirty("capability " + name);
}

void UiDatabase::RefreshFromRegistries()
{
    State& s = S();
    Rebuild(s, s.Capabilities["markup"]);
    s.Dirty = true;
    Flush();
}

void UiDatabase::PublishEntry(const std::string& registry, const std::string& id, const nlohmann::json& entry)
{
    S().Extra[registry][id] = entry;
    MarkDirty("publish " + registry + "/" + id);
}

void UiDatabase::RemoveEntry(const std::string& registry, const std::string& id)
{
    State& s = S();
    const auto it = s.Extra.find(registry);
    if (it != s.Extra.end() && it->second.erase(id) > 0)
        MarkDirty("remove " + registry + "/" + id);
}

std::string UiDatabase::Serialize()
{
    State& s = S();
    json root = json::object();
    root["format"] = "engine-ui-database";
    root["formatVersion"] = kFormatVersion;
    root["engine"] = { { "version", "1" }, { "buildId", std::string(__DATE__) + " " + __TIME__ }, { "previewProtocol", kPreviewProtocol } };
    root["game"] = { { "id", s.GameId }, { "displayName", s.DisplayName } };
    root["roots"] = { { "gameData", "." }, { "ui", "ui/" } };
    json caps = json::object();
    for (const auto& [name, value] : s.Capabilities)
        caps[name] = value;
    root["capabilities"] = caps;
    json registries = json::object();
    for (const auto& [name, entries] : s.Registries)
    {
        json arr = json::array();
        for (const auto& [id, entry] : entries)
            arr.push_back(entry);
        registries[name] = arr;
    }
    root["registries"] = registries;
    json previews = json::array();
    for (const auto& [id, entry] : s.Previews)
        previews.push_back(entry);
    root["previewTargets"] = previews;
    return root.dump(2);
}

const std::string& UiDatabase::LastWritten() { return S().Last; }

void UiDatabase::Flush()
{
    State& s = S();
    if (!s.Dirty || !s.Bootstrapped)
        return;
    s.Dirty = false;
    const std::string content = Serialize();
    if (content == s.Last && s.WriteOk)
        return;
    s.Last = content;
    ++s.Writes;
    s.WriteOk = WriteFileAtomic(s, content);
    if (!s.WriteOk && !s.WarnedWrite)
    {
        s.WarnedWrite = true;
        Logger::Warning("[UI] cannot write UI database '%s'; keeping it in memory", s.OutputPath.c_str());
    }
    else if (s.WriteOk)
        s.WarnedWrite = false;
}

void UiDatabase::SetOutputPath(const std::string& path)
{
    State& s = S();
    s.OutputPath = path;
    s.Dirty = true;
    s.Last.clear();
}

const std::string& UiDatabase::OutputPath() { return S().OutputPath; }
bool UiDatabase::LastWriteSucceeded() { return S().WriteOk; }
int UiDatabase::WriteCount() { return S().Writes; }

bool UiDatabase::NormalizePath(const std::string& in, std::string& normalized, std::string& error)
{
    std::string p = in;
    for (char& c : p)
        if (c == '\\') c = '/';
    if (p.empty()) { error = "empty path"; return false; }
    if (p[0] == '/' || (p.size() > 1 && p[1] == ':')) { error = "absolute path '" + in + "'"; return false; }
    std::string out;
    size_t start = 0;
    while (start <= p.size())
    {
        size_t slash = p.find('/', start);
        if (slash == std::string::npos) slash = p.size();
        const std::string part = p.substr(start, slash - start);
        if (part == "..") { error = "path traversal in '" + in + "'"; return false; }
        if (!part.empty() && part != ".")
            out += (out.empty() ? "" : "/") + part;
        start = slash + 1;
    }
    if (out.rfind("GameData/", 0) == 0)
        out = out.substr(9);
    if (out.empty()) { error = "empty path"; return false; }
    normalized = out;
    return true;
}

bool UiDatabase::SelfTest(std::string& error)
{
    // path policy
    std::string n, e;
    if (!NormalizePath("GameData\\ui\\a.xml", n, e) || n != "ui/a.xml") { error = "normalize backslashes/GameData prefix: got '" + n + "'"; return false; }
    if (NormalizePath("../x", n, e)) { error = "traversal was accepted"; return false; }
    if (NormalizePath("GameData/ui/../../x", n, e)) { error = "embedded traversal was accepted"; return false; }
    if (NormalizePath("C:/x/y", n, e)) { error = "drive path was accepted"; return false; }
    if (NormalizePath("/etc/x", n, e)) { error = "rooted path was accepted"; return false; }

    // serializer: valid JSON, stable across repeated serializations
    const std::string a = Serialize();
    const std::string b = Serialize();
    if (a != b) { error = "serialization is not stable"; return false; }
    json parsed = json::parse(a, nullptr, false);
    if (parsed.is_discarded() || parsed.value("format", "") != "engine-ui-database") { error = "serialized JSON is invalid"; return false; }
    if (a.find("GameData\\") != std::string::npos || a.find("C:/") != std::string::npos)
    { error = "serialized JSON contains an absolute path"; return false; }

    // preview descriptor: registers and enumerates without callbacks
    UiPreviewDescriptor d;
    d.Id = "selftest.preview";
    d.Name = "Self test";
    d.Kind = "element";
    UiPreviewRegistry::RegisterRecipe(d);
    bool found = false;
    for (const auto& x : UiPreviewRegistry::Descriptors())
        if (x.Id == "selftest.preview") found = true;
    if (!found) { error = "preview descriptor was not enumerated"; return false; }
    const std::string text = UiPreviewRegistry::ToJson(d).dump();
    if (text.find("provider") != std::string::npos && !d.ProviderId.empty()) { error = "descriptor leaked provider data"; return false; }

    // unwritable destination must not fail
    State& s = S();
    const std::string savedPath = s.OutputPath;
    const std::string savedLast = s.Last;
    const bool savedWarned = s.WarnedWrite;
    s.OutputPath = "SaveData/UIStudio/uidb_blocked/bad<name>?.json";
    s.Dirty = true;
    s.Last.clear();
    Flush();
    const bool failedAsExpected = !s.WriteOk;
    s.OutputPath = savedPath;
    s.Dirty = true;
    s.Last.clear();
    s.WarnedWrite = savedWarned;
    Flush();
    UiPreviewRegistry::ClearRecipes();
    RefreshFromRegistries();
    if (!failedAsExpected) { error = "unwritable destination did not report a failure"; return false; }
    (void)savedLast;
    return true;
}

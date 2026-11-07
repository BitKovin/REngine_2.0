#include "UiKitPreviews.h"

#include <FileSystem/FileSystem.h>
#include <Localization/Localisation.h>
#include <UI/Kit/UiSettings.h>

#include <algorithm>

namespace
{
    using nlohmann::json;

    UiPreviewScenario Scenario(const char* id, const char* label, json values)
    {
        UiPreviewScenario s;
        s.Id = id;
        s.Label = label;
        s.Values = std::move(values);
        return s;
    }

    // File names (without extension) of the xml files in a GameData folder.
    std::vector<std::string> XmlNames(const std::string& dir)
    {
        std::vector<std::string> out;
        for (const std::string& file : FileSystemEngine::GetFilesInPath(dir))
        {
            const size_t dot = file.rfind(".xml");
            if (dot != std::string::npos && dot + 4 == file.size())
                out.push_back(file.substr(0, dot));
        }
        std::sort(out.begin(), out.end());
        return out;
    }

    // Names of the --custom properties a theme sheet defines (names only, never values).
    std::vector<std::string> TokenNames(const std::string& file)
    {
        std::vector<std::string> out;
        const std::string text = FileSystemEngine::ReadFile(file);
        size_t pos = 0;
        while ((pos = text.find("--", pos)) != std::string::npos)
        {
            size_t end = pos + 2;
            while (end < text.size() && (isalnum(static_cast<unsigned char>(text[end])) || text[end] == '-' || text[end] == '_'))
                ++end;
            size_t colon = end;
            while (colon < text.size() && text[colon] == ' ')
                ++colon;
            if (end > pos + 2 && colon < text.size() && text[colon] == ':')
                out.push_back(text.substr(pos, end - pos));
            pos = end;
        }
        std::sort(out.begin(), out.end());
        out.erase(std::unique(out.begin(), out.end()), out.end());
        return out;
    }

    UiPreviewParameter Param(const char* name, const char* type, json def, const char* help)
    {
        UiPreviewParameter p;
        p.Name = name;
        p.Type = type;
        p.Default = std::move(def);
        p.Help = help;
        return p;
    }
}

void UiKitPreviews::FillRegistries(Registries& r)
{
    // settings the markup can bind to: key, type, bounds, choice label keys. No values.
    for (const std::string& key : UiSettings::Keys())
    {
        const UiSettings::Entry* e = UiSettings::Find(key);
        if (!e)
            continue;
        json j = json::object();
        j["name"] = key;
        j["key"] = key;
        j["type"] = UiSettings::KindName(e->Type);
        j["readOnly"] = e->ReadOnly;
        if (!e->Label.empty())
            j["label"] = e->Label;
        if (e->Type == UiSettings::Kind::Float || e->Type == UiSettings::Kind::Int)
        {
            j["min"] = e->Min;
            j["max"] = e->Max;
            j["step"] = e->Step;
        }
        if (e->Type == UiSettings::Kind::Choice)
        {
            j["choiceKeys"] = e->Choices;                  // labels or ${KEYS} as registered; dynamic lists are only known at run time
            j["dynamicChoices"] = static_cast<bool>(e->GetChoices);
        }
        r["settings"][key] = j;
    }

    for (const std::string& n : XmlNames("GameData/ui/kit/pages"))
        r["kitPages"][n] = json{ { "name", n }, { "path", "GameData/ui/kit/pages/" + n + ".xml" } };
    for (const std::string& n : XmlNames("GameData/ui/kit/components"))
        r["kitComponents"][n] = json{ { "name", n }, { "path", "GameData/ui/kit/components/" + n + ".xml" } };
    for (const std::string& n : TokenNames("GameData/ui/theme.css"))
        r["themeTokens"][n] = json{ { "name", n }, { "kind", "custom-property" } };

    // locales: ids and key names only (the keys of a language that has been loaded; the active and fallback ones are)
    const std::string current = Localisation::GetCurrentLanguage();
    for (const std::string& code : Localisation::LoadedLanguages())
    {
        const std::vector<std::string> keys = Localisation::KeysOf(code);
        r["locales"][code] = json{ { "name", code }, { "id", code }, { "current", code == current }, { "keyCount", keys.size() } };
        r["localeKeys"][code] = json{ { "name", code }, { "id", code }, { "keys", keys } };
    }
}

void UiKitPreviews::AddRecipes()
{
    // The widget gallery: one document with every widget, scenarios set the values the widgets show.
    UiPreviewDescriptor widgets;
    widgets.Id = "engine.widgets.gallery";
    widgets.Name = "Widget gallery";
    widgets.Category = "Widgets";
    widgets.Description = "Slider, drop-down, text entry, toggle, radio group, progress bars and list views in representative states.";
    widgets.Kind = "document";
    widgets.HostKind = "viewport";
    widgets.RequiredCapabilities = { "widgets" };
    widgets.SourceRefs.push_back({ "document", "GameData/ui/kit/tests/p4_widgets.xml" });
    widgets.SourceRefs.push_back({ "code", "source/Engine/UI/Elements/UiElemLegacyWidgets.cpp" });
    widgets.SourceRefs.push_back({ "code", "source/Engine/UI/Elements/UiElemControls.cpp" });
    widgets.Parameters.push_back(Param("value", "float", 0.5, "Slider and progress value"));
    widgets.Parameters.push_back(Param("rows", "int", 40, "ListView row count"));
    widgets.Scenarios.push_back(Scenario("slider-min", "Slider at minimum", { { "value", 0.0 } }));
    widgets.Scenarios.push_back(Scenario("slider-mid", "Slider at middle", { { "value", 0.5 } }));
    widgets.Scenarios.push_back(Scenario("slider-max", "Slider at maximum", { { "value", 1.0 } }));
    widgets.Scenarios.push_back(Scenario("dropdown-open", "Drop-down with options", { { "open", true } }));
    widgets.Scenarios.push_back(Scenario("entry-empty", "Text entry empty", { { "entry", "" } }));
    widgets.Scenarios.push_back(Scenario("entry-filled", "Text entry filled", { { "entry", "Sample" } }));
    widgets.Scenarios.push_back(Scenario("toggle-on", "Toggle on", { { "checked", true } }));
    widgets.Scenarios.push_back(Scenario("toggle-off", "Toggle off", { { "checked", false } }));
    widgets.Scenarios.push_back(Scenario("progress-empty", "Progress empty", { { "value", 0.0 } }));
    widgets.Scenarios.push_back(Scenario("progress-full", "Progress full", { { "value", 1.0 } }));
    widgets.Scenarios.push_back(Scenario("list-empty", "List with no rows", { { "rows", 0 } }));
    widgets.Scenarios.push_back(Scenario("list-short", "List that fits", { { "rows", 3 } }));
    widgets.Scenarios.push_back(Scenario("list-long", "List that scrolls", { { "rows", 500 } }));
    UiPreviewRegistry::RegisterRecipe(widgets);

    // Settings kit: the real kit settings page. The fixture is an isolated test settings registry (test.*), never the user's settings.
    UiPreviewDescriptor settings;
    settings.Id = "engine.kit.settings";
    settings.Name = "Kit settings";
    settings.Category = "Kit";
    settings.Description = "The kit settings menu and a bound test page; the fixture registers isolated test.* settings and never touches the saved settings.";
    settings.Kind = "document";
    settings.HostKind = "menu-stack";
    settings.RequiredCapabilities = { "widgets", "settings" };
    settings.SourceRefs.push_back({ "document", "GameData/ui/kit/pages/settings.xml" });
    settings.SourceRefs.push_back({ "document", "GameData/ui/kit/tests/p4_bind.xml" });
    settings.Parameters.push_back(Param("pseudoloc", "bool", false, "Pseudo-localised text, to check that labels fit"));
    settings.Parameters.push_back(Param("textScale", "float", 1.0, "ui.text_scale"));
    settings.Scenarios.push_back(Scenario("video", "Video tab", { { "tab", "video" } }));
    settings.Scenarios.push_back(Scenario("sound", "Sound tab", { { "tab", "sound" } }));
    settings.Scenarios.push_back(Scenario("input", "Input tab", { { "tab", "input" } }));
    settings.Scenarios.push_back(Scenario("bound-test", "Bound test page", { { "document", "GameData/ui/kit/tests/p4_bind.xml" }, { "fixture", "test.*" } }));
    settings.Scenarios.push_back(Scenario("confirm-changes", "Back with changes", { { "document", "GameData/ui/kit/tests/p4_bind.xml" }, { "change", true } }));
    UiPreviewRegistry::RegisterRecipe(settings);
}

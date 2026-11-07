#include "UiLint.h"

#include <Console/Console.h>
#include <EngineMain.h>
#include <FileSystem/FileSystem.h>
#include <Localization/Localisation.h>
#include <Logger.hpp>
#include <UI/Elements/UiElem.h>
#include <UI/Markup/UiDocument.h>
#include <UI/Markup/UiMarkupLog.h>
#include <UI/Preview/UiDatabase.h>
#include <UI/Style/UiComputedStyle.h>
#include <UI/Style/UiCssParser.h>
#include <UI/Style/UiStyleResolver.h>

#include <SDL2/SDL.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <unordered_map>

namespace
{
    using Diagnostic = UiLint::Diagnostic;

    struct Lint
    {
        std::vector<Diagnostic> Items;
        std::set<std::string> Seen;
        std::set<std::string> LocaleKeys;
        std::unordered_map<std::string, std::vector<UiCssToken>> Custom;

        void Add(const char* severity, const std::string& code, const std::string& path, int line, int col, const std::string& element,
                 const std::string& message, const std::string& help = std::string())
        {
            const std::string key = std::string(severity) + "|" + code + "|" + path + "|" + std::to_string(line) + "|" + std::to_string(col) + "|" + message;
            if (!Seen.insert(key).second)
                return;
            Items.push_back({ severity, code, path, line, col, element, message, help });
        }
    };

    bool EndsWith(const std::string& s, const char* suffix)
    {
        const size_t n = std::char_traits<char>::length(suffix);
        return s.size() >= n && s.compare(s.size() - n, n, suffix) == 0;
    }

    // Every .xml and .css under a folder, recursively, sorted.
    void CollectFiles(const std::string& dir, std::vector<std::string>& xml, std::vector<std::string>& css)
    {
        for (const std::string& name : FileSystemEngine::GetFilesInPath(dir))
        {
            const std::string path = dir + "/" + name;
            if (EndsWith(name, ".xml"))
                xml.push_back(path);
            else if (EndsWith(name, ".css"))
                css.push_back(path);
            else if (name.find('.') == std::string::npos)
                CollectFiles(path, xml, css);
        }
    }

    bool Exists(const std::string& path)
    {
        return FileSystemEngine::GetFileModificationTime(path) != 0 || !FileSystemEngine::ReadFileBinary(path).empty();
    }

    // Static text without placeholders, so the value can be checked as it is.
    bool IsLiteralPath(const std::string& v) { return !v.empty() && v.find('{') == std::string::npos && v.find('$') == std::string::npos; }

    std::string Classify(const std::string& message)
    {
        auto has = [&](const char* s) { return message.find(s) != std::string::npos; };
        if (has("unknown tag")) return "markup.unknown-tag";
        if (has("unknown attribute")) return "markup.unknown-attribute";
        if (has("unknown property")) return "css.unknown-property";
        if (has("unknown action")) return "action.unknown";
        if (has("duplicate id")) return "markup.duplicate-id";
        if (has("cycle")) return "markup.import-cycle";
        if (has("cannot read") || has("cannot open") || has("not found")) return "resource.missing";
        if (has("selector")) return "css.selector";
        if (has("import")) return "markup.import";
        if (has("parse") || has("malformed") || has("XML")) return "markup.parse";
        return "ui.message";
    }

    void CollectCustom(const UiStyleSheet& sheet, Lint& lint)
    {
        for (const UiCssRule& rule : sheet.Rules)
            for (const UiCssDeclaration& decl : rule.Declarations)
                if (decl.Name.size() > 2 && decl.Name[0] == '-' && decl.Name[1] == '-')
                    lint.Custom[decl.Name] = decl.Value;
    }

    void CheckSheet(const UiStyleSheet& sheet, Lint& lint)
    {
        UiStyleParseContext ctx;
        for (const UiCssRule& rule : sheet.Rules)
            for (const UiCssDeclaration& decl : rule.Declarations)
            {
                if (decl.Name.size() > 2 && decl.Name[0] == '-' && decl.Name[1] == '-')
                    continue;
                if (!UiIsKnownProperty(decl.Name))
                {
                    lint.Add("error", "css.unknown-property", sheet.Path, decl.Line, decl.Col, decl.Name, "unknown property '" + decl.Name + "'",
                             "Check the spelling; the Properties list in Studio shows every property this engine supports.");
                    continue;
                }
                UiComputedStyle scratch;
                scratch.Custom = lint.Custom;
                std::string error;
                if (!UiStyleResolver::ApplyDeclaration(scratch, decl, ctx, error))
                    lint.Add("error", error.find("variable") != std::string::npos ? "css.variable" : "css.invalid-value", sheet.Path, decl.Line, decl.Col, decl.Name,
                             decl.Name + ": " + error);
            }
        for (const UiFontFaceRule& face : sheet.FontFaces)
            if (IsLiteralPath(face.Src) && !Exists(face.Src))
                lint.Add("error", "resource.missing", sheet.Path, face.Line, 0, face.Family, "font file not found: " + face.Src);
    }

    void CheckText(const std::string& text, const UiNodeDef& node, const std::string& file, Lint& lint)
    {
        size_t pos = 0;
        while ((pos = text.find("${", pos)) != std::string::npos)
        {
            const size_t end = text.find('}', pos);
            if (end == std::string::npos)
                break;
            const std::string key = text.substr(pos + 2, end - pos - 2);
            if (!key.empty() && !lint.LocaleKeys.count(key))
                lint.Add("warning", "localisation.missing-key", file, node.Line, node.Col, node.Tag, "localisation key '" + key + "' does not exist",
                         "Add it in Studio's Localisation panel, or fix the key name.");
            pos = end + 1;
        }
    }

    void CollectIds(const UiNodeDef& node, std::set<std::string>& ids)
    {
        for (const auto& [name, value] : node.Attributes)
            if (name == "id")
                ids.insert(value);
        for (const UiNodeDef& child : node.Children)
            CollectIds(child, ids);
    }

    void CheckNode(const UiNodeDef& node, const std::string& file, const std::set<std::string>& ids, std::set<std::string>& seenIds, Lint& lint)
    {
        for (const auto& [name, value] : node.Attributes)
        {
            CheckText(value, node, file, lint);
            if (name == "id" && !value.empty() && !seenIds.insert(value).second)
                lint.Add("warning", "markup.duplicate-id", file, node.Line, node.Col, node.Tag, "id '" + value + "' is used more than once in this document");
            if (name == "src" && node.Tag == "Image" && IsLiteralPath(value) && !Exists(value))
                lint.Add("error", "resource.missing", file, node.Line, node.Col, node.Tag, "image not found: " + value,
                         "Pick an existing image with the Assets panel.");
            if ((name == "nav-up" || name == "nav-down" || name == "nav-left" || name == "nav-right") && IsLiteralPath(value) && value.find('/') == std::string::npos &&
                !ids.count(value))
                lint.Add("warning", "nav.unresolved", file, node.Line, node.Col, node.Tag, name + " points at '" + value + "', which is not an id in this document");
        }
        CheckText(node.Text, node, file, lint);
        for (const UiNodeDef& child : node.Children)
            CheckNode(child, file, ids, seenIds, lint);
    }

    void CheckDocument(const std::string& path, Lint& lint)
    {
        UiDocumentCache_Invalidate(path);
        const std::shared_ptr<UiDocumentDef> def = UiDocumentCache_Get(path);
        if (!def)
            return;   // the parser already reported why
        for (const auto& sheet : def->Sheets)
            if (sheet)
                CheckSheet(*sheet, lint);

        std::set<std::string> ids;
        CollectIds(def->Top, ids);
        std::set<std::string> seen;
        CheckNode(def->Top, path, ids, seen, lint);
        for (const auto& [tag, snippet] : def->Snippets)
        {
            std::set<std::string> snippetIds, snippetSeen;
            CollectIds(snippet.Body, snippetIds);
            CheckNode(snippet.Body, path, snippetIds, snippetSeen, lint);
        }

        // Build it offscreen (not attached to the viewport) so builder errors surface. Components need arguments, so only layouts are built.
        if (!def->Component)
            UiElem::LoadNested(path);
    }

}

std::vector<UiLint::Diagnostic> UiLint::Run()
{
    Lint lint;
    for (const std::string& lang : { Localisation::GetCurrentLanguage(), std::string("en") })
        for (const std::string& key : Localisation::KeysOf(lang))
            lint.LocaleKeys.insert(key);

    std::vector<std::string> xml, css;
    CollectFiles("GameData/ui", xml, css);
    std::sort(xml.begin(), xml.end());
    std::sort(css.begin(), css.end());

    // Variables first, so var() checks see every sheet's custom properties.
    std::vector<std::shared_ptr<UiStyleSheet>> sheets;
    UiMarkupLog::SetSink([&](bool isError, const std::string& path, int line, int col, const std::string& message) {
        lint.Add(isError ? "error" : "warning", Classify(message), path, line, col, std::string(), message);
    });

    for (const std::string& path : css)
    {
        UiStyleSheetCache::Invalidate(path);
        if (auto sheet = UiStyleSheetCache::Get(path))
        {
            sheets.push_back(sheet);
            CollectCustom(*sheet, lint);
        }
    }
    for (const auto& sheet : sheets)
        CheckSheet(*sheet, lint);
    for (const std::string& path : xml)
        CheckDocument(path, lint);

    UiMarkupLog::SetSink(nullptr);

    std::vector<Diagnostic>& items = lint.Items;
    std::sort(items.begin(), items.end(), [](const Diagnostic& a, const Diagnostic& b) {
        if (a.Path != b.Path) return a.Path < b.Path;
        if (a.Line != b.Line) return a.Line < b.Line;
        if (a.Column != b.Column) return a.Column < b.Column;
        if (a.Code != b.Code) return a.Code < b.Code;
        return a.Message < b.Message;
    });
    return items;
}

nlohmann::json UiLint::ToJson(const std::vector<Diagnostic>& diagnostics)
{
    nlohmann::json list = nlohmann::json::array();
    int errors = 0, warnings = 0;
    for (const Diagnostic& d : diagnostics)
    {
        nlohmann::json j = { { "severity", d.Severity }, { "code", d.Code }, { "path", d.Path }, { "line", d.Line }, { "column", d.Column },
                             { "element", d.Element }, { "message", d.Message } };
        if (!d.Help.empty())
            j["help"] = d.Help;
        list.push_back(j);
        errors += d.Severity == "error";
        warnings += d.Severity == "warning";
    }
    return { { "errors", errors }, { "warnings", warnings }, { "diagnostics", list } };
}

std::string UiLint::ToText(const std::vector<Diagnostic>& diagnostics)
{
    std::string out;
    int errors = 0;
    for (const Diagnostic& d : diagnostics)
    {
        out += d.Path + ":" + std::to_string(d.Line) + ":" + std::to_string(d.Column) + ": " + d.Severity + " [" + d.Code + "] " + d.Message + "\n";
        errors += d.Severity == "error";
    }
    out += errors == 0 ? "LINT PASS\n" : "LINT FAIL " + std::to_string(errors) + "\n";
    return out;
}

void UiLint::WriteReports(const std::vector<Diagnostic>& diagnostics)
{
    FileSystemEngine::WriteFile("SaveData/ui_lint.txt", ToText(diagnostics));
    FileSystemEngine::WriteFile("SaveData/ui_lint.json", ToJson(diagnostics).dump(2));
}

namespace
{
    void RequestQuit()
    {
        SDL_Event e{};
        e.type = SDL_QUIT;
        SDL_PushEvent(&e);
    }

    // Written next to the target and renamed, so a reader never sees half a file.
    bool WriteAtomic(const std::string& path, const std::string& text)
    {
        namespace fs = std::filesystem;
        const std::string temp = path + ".tmp";
        {
            std::ofstream file(temp, std::ios::binary | std::ios::trunc);
            if (!file)
                return false;
            file << text;
        }
        std::error_code ec;
        fs::rename(temp, path, ec);
        if (ec)
        {
            fs::remove(path, ec);
            fs::rename(temp, path, ec);
        }
        return !ec;
    }

    void CmdLint(const std::vector<std::string>&)
    {
        const auto items = UiLint::Run();
        UiLint::WriteReports(items);
        const nlohmann::json j = UiLint::ToJson(items);
        Logger::Log("[UiLint] %d errors, %d warnings (SaveData/ui_lint.txt)", j["errors"].get<int>(), j["warnings"].get<int>());
    }
}

bool UiLint::HandleLaunchArguments()
{
    if (EngineMain::MainInstance == nullptr)
        return false;
    const auto& args = EngineMain::MainInstance->Arguments;
    bool ran = false;

    const auto schema = args.find("ui_schema");
    if (schema != args.end() && !schema->second.empty())
    {
        UiDatabase::RefreshFromRegistries();
        if (WriteAtomic(schema->second[0], UiDatabase::Serialize()))
            Logger::Log("[UiSchema] wrote %s", schema->second[0].c_str());
        else
            Logger::Error("[UiSchema] cannot write %s", schema->second[0].c_str());
        ran = true;
    }

    if (args.find("ui_lint") != args.end())
    {
        const auto items = Run();
        WriteReports(items);
        const nlohmann::json j = ToJson(items);
        Logger::Log("[UiLint] %d errors, %d warnings (SaveData/ui_lint.txt)", j["errors"].get<int>(), j["warnings"].get<int>());
        ran = true;
    }

    if (ran)
        RequestQuit();
    return ran;
}

void UiLint::RegisterConsoleCommands()
{
    Console::Get().RegisterCommand({ "ui_lint", "ui_lint - checks every document and style sheet under GameData/ui and writes SaveData/ui_lint.txt/.json", CmdLint });
}

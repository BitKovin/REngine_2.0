#include "UiDocument.h"

#include <FileSystem/FileSystem.h>
#include <UI/Markup/UiMarkupLog.h>
#include <UI/Preview/UiDatabase.h>
#include <includedLibraries/pugixml/pugixml.hpp>

#include <algorithm>
#include <map>

namespace
{
    std::map<std::string, std::shared_ptr<UiDocumentDef>>& Cache()
    {
        static std::map<std::string, std::shared_ptr<UiDocumentDef>> cache;
        return cache;
    }

    std::map<std::string, std::shared_ptr<UiDocumentDef>>& Components()
    {
        static std::map<std::string, std::shared_ptr<UiDocumentDef>> components;
        return components;
    }

    struct Ctx
    {
        const std::string& Path;
        const std::string& Text;
    };

    void Position(const Ctx& c, const pugi::xml_node& node, int& line, int& col)
    {
        const ptrdiff_t off = node.offset_debug();
        if (off < 0)
        {
            line = 0;
            col = 0;
            return;
        }
        // offset_debug points at the element name, one past the '<'
        UiMarkupLog::LineColumn(c.Text, off > 0 ? (size_t)off - 1 : 0, line, col);
    }

    std::string Trim(const std::string& s)
    {
        const size_t a = s.find_first_not_of(" \t\r\n");
        if (a == std::string::npos) return "";
        const size_t b = s.find_last_not_of(" \t\r\n");
        return s.substr(a, b - a + 1);
    }

    UiNodeDef ConvertNode(const Ctx& c, const pugi::xml_node& node)
    {
        UiNodeDef def;
        def.Tag = node.name();
        Position(c, node, def.Line, def.Col);
        for (const pugi::xml_attribute& a : node.attributes())
            def.Attributes.emplace_back(a.name(), a.value());
        def.Text = Trim(node.child_value());
        for (const pugi::xml_node& child : node.children())
            if (child.type() == pugi::node_element)
                def.Children.push_back(ConvertNode(c, child));
        return def;
    }

    // "label, hint='none'" -> params with defaults; entries without '=' are required.
    void ParseParams(const std::string& spec, UiComponentDef& def)
    {
        std::vector<std::string> entries;
        std::string cur;
        char quote = 0;
        for (char ch : spec)
        {
            if (quote != 0)
            {
                cur += ch;
                if (ch == quote) quote = 0;
            }
            else if (ch == '\'' || ch == '"') { quote = ch; cur += ch; }
            else if (ch == ',') { entries.push_back(cur); cur.clear(); }
            else cur += ch;
        }
        entries.push_back(cur);
        for (const std::string& raw : entries)
        {
            const std::string entry = Trim(raw);
            if (entry.empty()) continue;
            const size_t eq = entry.find('=');
            if (eq == std::string::npos)
            {
                def.Params.emplace_back(entry, "");
                def.ParamRequired.push_back(true);
                continue;
            }
            std::string value = Trim(entry.substr(eq + 1));
            if (value.size() >= 2 && (value.front() == '\'' || value.front() == '"') && value.back() == value.front())
                value = value.substr(1, value.size() - 2);
            def.Params.emplace_back(Trim(entry.substr(0, eq)), value);
            def.ParamRequired.push_back(false);
        }
    }

    void CollectSlots(const UiNodeDef& node, std::vector<std::string>& slots)
    {
        if (node.Tag == "Children")
        {
            std::string name;
            for (const auto& a : node.Attributes)
                if (a.first == "slot") name = a.second;
            if (std::find(slots.begin(), slots.end(), name) == slots.end())
                slots.push_back(name);
        }
        for (const UiNodeDef& child : node.Children)
            CollectSlots(child, slots);
    }

    std::shared_ptr<UiDocumentDef> Parse(const std::string& path)
    {
        const std::string text = FileSystemEngine::ReadFile(path);
        if (text.empty())
        {
            UiMarkupLog::Error(path, 0, 0, "cannot read file");
            return nullptr;
        }

        pugi::xml_document xml;
        const pugi::xml_parse_result result = xml.load_string(text.c_str(), pugi::parse_default | pugi::parse_ws_pcdata_single);
        if (!result)
        {
            int line = 0, col = 0;
            UiMarkupLog::LineColumn(text, (size_t)result.offset, line, col);
            UiMarkupLog::Error(path, line, col, result.description());
            return nullptr;
        }

        const Ctx ctx{ path, text };
        auto def = std::make_shared<UiDocumentDef>();
        def->Path = path;
        def->SourceText = text;
        def->ModTime = FileSystemEngine::GetFileModificationTime(path);

        const pugi::xml_node root = xml.document_element();
        if (std::string(root.name()) != "root")
        {
            int line, col;
            Position(ctx, root, line, col);
            UiMarkupLog::Error(path, line, col, "the document element must be <root>");
            return nullptr;
        }

        bool haveTop = false;
        for (const pugi::xml_node& child : root.children())
        {
            if (child.type() != pugi::node_element)
                continue;
            const std::string name = child.name();
            int line, col;
            Position(ctx, child, line, col);

            if (name == "imports")
            {
                for (const pugi::xml_node& imp : child.children("import"))
                {
                    const std::string src = imp.attribute("src").value();
                    if (src.empty()) { int l, k; Position(ctx, imp, l, k); UiMarkupLog::Error(path, l, k, "<import> needs src"); }
                    else def->Imports.push_back(src);
                }
            }
            else if (name == "styles")
            {
                for (const pugi::xml_node& item : child.children())
                {
                    if (item.type() != pugi::node_element)
                        continue;
                    int l, k;
                    Position(ctx, item, l, k);
                    const std::string itemName = item.name();
                    if (itemName == "include")
                    {
                        const std::string src = item.attribute("src").value();
                        if (src.empty()) { UiMarkupLog::Error(path, l, k, "<include> needs src"); continue; }
                        if (auto sheet = UiStyleSheetCache::Get(src))
                        {
                            def->Sheets.push_back(sheet);
                            def->SheetFiles.push_back(src);
                        }
                    }
                    else if (itemName == "style")
                    {
                        auto sheet = UiParseStyleSheet(item.child_value(), path, l - 1);
                        sheet->Path = "<inline:" + path + ":" + std::to_string(l) + ">";
                        def->Sheets.push_back(sheet);
                    }
                    else
                        UiMarkupLog::Error(path, l, k, "unexpected <" + itemName + "> inside <styles>");
                }
            }
            else if (name == "snippets")
            {
                for (const pugi::xml_node& item : child.children("snippet"))
                {
                    int l, k;
                    Position(ctx, item, l, k);
                    UiComponentDef snippet;
                    snippet.Tag = item.attribute("name").value();
                    snippet.File = path;
                    ParseParams(item.attribute("params").value(), snippet);
                    int elementChildren = 0;
                    for (const pugi::xml_node& body : item.children())
                        if (body.type() == pugi::node_element)
                        {
                            ++elementChildren;
                            if (elementChildren == 1) snippet.Body = ConvertNode(ctx, body);
                        }
                    if (snippet.Tag.empty() || elementChildren != 1)
                        UiMarkupLog::Error(path, l, k, "<snippet> needs a name and exactly one element");
                    else
                    {
                        CollectSlots(snippet.Body, snippet.Slots);
                        def->Snippets[snippet.Tag] = std::move(snippet);
                    }
                }
            }
            else
            {
                if (haveTop)
                {
                    UiMarkupLog::Error(path, line, col, "only one top element is allowed inside <root>");
                    continue;
                }
                haveTop = true;
                if (name == "Component")
                {
                    auto comp = std::make_shared<UiComponentDef>();
                    comp->Tag = child.attribute("tag").value();
                    comp->File = path;
                    ParseParams(child.attribute("params").value(), *comp);
                    int elementChildren = 0;
                    for (const pugi::xml_node& body : child.children())
                        if (body.type() == pugi::node_element)
                        {
                            ++elementChildren;
                            if (elementChildren == 1) comp->Body = ConvertNode(ctx, body);
                        }
                    if (comp->Tag.empty() || elementChildren != 1)
                    {
                        UiMarkupLog::Error(path, line, col, "<Component> needs tag=\"...\" and exactly one element");
                        continue;
                    }
                    CollectSlots(comp->Body, comp->Slots);
                    def->Top = comp->Body;
                    def->Component = comp;
                }
                else
                    def->Top = ConvertNode(ctx, child);
            }
        }
        if (!haveTop)
        {
            UiMarkupLog::Error(path, 0, 0, "the document has no top element");
            return nullptr;
        }
        return def;
    }
}

std::shared_ptr<UiDocumentDef> UiDocumentCache_Get(const std::string& path)
{
    auto& cache = Cache();
    const auto it = cache.find(path);
    if (it != cache.end())
        return it->second;
    auto def = Parse(path);
    cache[path] = def;
    UiDatabase::MarkDirty("document " + path);
    return def;
}

void UiDocumentCache_Invalidate(const std::string& path)
{
    Cache().erase(path);
    UiDatabase::MarkDirty("document invalidated " + path);
}

std::vector<std::string> UiDocumentCache_Paths()
{
    std::vector<std::string> out;
    for (const auto& [path, def] : Cache())
        if (def)
            out.push_back(path);
    return out;
}

void UiComponentRegistry::RegisterFile(const std::string& path)
{
    auto def = UiDocumentCache_Get(path);
    if (!def)
        return;
    if (!def->Component)
    {
        UiMarkupLog::Warning(path, 0, 0, "not a component file");
        return;
    }
    const std::string& tag = def->Component->Tag;
    const auto existing = Components().find(tag);
    if (existing != Components().end() && existing->second->Path != path)
    {
        UiMarkupLog::Error(path, 0, 0, "component tag '" + tag + "' is already defined by " + existing->second->Path);
        return;
    }
    Components()[tag] = def;
    UiDatabase::MarkDirty("component " + tag);
}

void UiComponentRegistry::Discover()
{
    Components().clear();
    for (const char* dir : { "GameData/ui/kit/components/", "GameData/ui/components/" })
    {
        std::vector<std::string> files = FileSystemEngine::GetFilesInPath(dir);
        std::sort(files.begin(), files.end());
        for (const std::string& name : files)
            if (name.size() > 4 && name.compare(name.size() - 4, 4, ".xml") == 0)
                RegisterFile(std::string(dir) + name);
    }
    UiDatabase::MarkDirty("component discovery");
}

const UiComponentDef* UiComponentRegistry::Find(const std::string& tag)
{
    const auto it = Components().find(tag);
    return it == Components().end() ? nullptr : it->second->Component.get();
}

std::vector<const UiComponentDef*> UiComponentRegistry::All()
{
    std::vector<const UiComponentDef*> out;
    for (const auto& [tag, def] : Components())
        out.push_back(def->Component.get());
    return out;
}

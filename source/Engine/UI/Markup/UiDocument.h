#pragma once
#include <UI/Style/UiCssParser.h>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class UiElem;

struct UiNodeDef
{
    std::string Tag;
    std::vector<std::pair<std::string, std::string>> Attributes;   // in file order
    std::string Text;                 // trimmed text content (for Label, <style>)
    std::vector<UiNodeDef> Children;
    int Line = 0, Col = 0;
};

struct UiComponentDef
{
    std::string Tag;                  // from tag="..."
    std::string File;
    std::vector<std::pair<std::string, std::string>> Params;   // name, default
    std::vector<bool> ParamRequired;
    UiNodeDef Body;                   // the single element inside <Component>
    std::vector<std::string> Slots;   // named slots found in the body (the default slot is "")
};

struct UiDocumentDef
{
    std::string Path;
    std::string SourceText;
    uint32_t ModTime = 0;
    std::vector<std::string> Imports;                                   // component files
    std::vector<std::shared_ptr<UiStyleSheet>> Sheets;                  // <include> and <style>, in order
    std::vector<std::string> SheetFiles;                                // .css files included (for hot reload)
    std::map<std::string, UiComponentDef> Snippets;
    std::shared_ptr<UiComponentDef> Component;                          // set when the top element is <Component>
    UiNodeDef Top;                                                      // the top element (or Component body)
};

// Parse once, cached. Returns nullptr after logging on failure.
std::shared_ptr<UiDocumentDef> UiDocumentCache_Get(const std::string& path);
void UiDocumentCache_Invalidate(const std::string& path);
std::vector<std::string> UiDocumentCache_Paths();

// The id scope of one document instance (a loaded layout or a component instance).
class UiDocumentInstance
{
public:
    std::unordered_map<std::string, std::weak_ptr<UiElem>> Ids;
    std::shared_ptr<UiDocumentDef> Def;
    std::string Path;
};

namespace UiComponentRegistry
{
    void Discover();          // scans GameData/ui/kit/components/ and GameData/ui/components/ (flat, *.xml)
    const UiComponentDef* Find(const std::string& tag);   // global tags
    void RegisterFile(const std::string& path);           // used by Discover and by imports
    std::vector<const UiComponentDef*> All();             // sorted by tag
}

#pragma once
#include <UI/Markup/UiDocument.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

class UiElem;

namespace UiBuilder
{
    struct Context
    {
        UiDocumentInstance* Instance = nullptr;
        std::string File;
        std::shared_ptr<UiDocumentDef> Def;                  // definition of File (snippets, imports)
        const std::vector<UiNodeDef>* SlotChildren = nullptr; // children of the component usage node
        const Context* SlotContext = nullptr;                // context those children are built in
        std::string ComponentTag;                            // for slot errors
        std::vector<std::string>* Stack = nullptr;           // component tags being built (cycle check)
    };

    // Applies the top node of a layout onto `element` and builds its children (UiElem::LoadLayout).
    bool BuildLayout(UiElem& element, const std::shared_ptr<UiDocumentDef>& def, const std::shared_ptr<UiDocumentInstance>& instance);
    // Builds the snippet (or component, or built-in tag) `name` as a child of `parent`, in the document scope `owner` came from.
    // Returns the new element, or nullptr after logging.
    std::shared_ptr<UiElem> BuildSnippet(UiElem& owner, UiElem& parent, const std::string& name);
    // Files a loaded layout depends on (the layout, imported components, style sheets), for hot reload.
    void CollectDependencies(const std::shared_ptr<UiDocumentDef>& def, std::vector<std::string>& out);
}

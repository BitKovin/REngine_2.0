#include "UiBuilder.h"

#include <UI/Elements/UiElem.h>
#include <UI/Layout/LayoutBlueprint.h>
#include <UI/Layout/UiLayoutSerializer.h>
#include <UI/Markup/UiMarkupLog.h>
#include <UI/Markup/UiTagRegistry.h>
#include <UI/Reflection/UiElementRegistry.h>
#include <UI/UiSystem.h>

#include <algorithm>

namespace
{
    using UiBuilder::Context;

    const char* kStandardAttributes[] = { "id", "class", "style", "visible", "enabled", "selected", "hittest", "default-focus",
                                          "close-on-back", "shows-below", "pauses-game", "hides-hud", "confirm-changes", "nav-up", "nav-down", "nav-left", "nav-right", "draggable", "droptarget" };

    bool IsStandard(const std::string& name)
    {
        for (const char* s : kStandardAttributes)
            if (name == s)
                return true;
        // Event attributes (onactivate, onpicked, ...) are accepted on component usages too.
        return name.size() > 2 && name[0] == 'o' && name[1] == 'n';
    }

    struct Resolved
    {
        enum Kind { None, Builtin, Component, Legacy } K = None;
        UiTagRegistry::Factory Factory = nullptr;
        const UiComponentDef* Comp = nullptr;
        std::shared_ptr<UiDocumentDef> CompDoc;
    };

    Resolved ResolveTag(const Context& c, const std::string& tag)
    {
        Resolved r;
        if (UiTagRegistry::Factory f = UiTagRegistry::FindBuiltin(tag))
        {
            r.K = Resolved::Builtin;
            r.Factory = f;
            return r;
        }
        if (c.Def)
        {
            const auto it = c.Def->Snippets.find(tag);
            if (it != c.Def->Snippets.end())
            {
                r.K = Resolved::Component;
                r.Comp = &it->second;
                r.CompDoc = c.Def;
                return r;
            }
            for (const std::string& import : c.Def->Imports)
            {
                auto doc = UiDocumentCache_Get(import);
                if (doc && doc->Component && doc->Component->Tag == tag)
                {
                    r.K = Resolved::Component;
                    r.Comp = doc->Component.get();
                    r.CompDoc = doc;
                    return r;
                }
            }
        }
        if (const UiComponentDef* global = UiComponentRegistry::Find(tag))
        {
            r.K = Resolved::Component;
            r.Comp = global;
            r.CompDoc = UiDocumentCache_Get(global->File);
            return r;
        }
        if (UiElementRegistry::Instance().Find(tag) != nullptr)
            r.K = Resolved::Legacy;
        return r;
    }

    void ApplyNodeAttributes(UiElem& e, const UiNodeDef& node, const std::string& file, bool skipStandardForComponent = false)
    {
        (void)skipStandardForComponent;
        e.SrcLine = node.Line;
        e.SrcCol = node.Col;
        bool hasText = false;
        for (const auto& [name, value] : node.Attributes)
            if (name == "text")
                hasText = true;
        for (const auto& [name, value] : node.Attributes)
        {
            if (name == "slot")
                continue;
            e.Attributes[name] = value;
            if (!e.ApplyAttributeValue(name, value, true))
                UiMarkupLog::Warning(file, node.Line, node.Col, "unknown attribute '" + name + "' on <" + node.Tag + ">");
        }
        if (!hasText && node.Tag == "Label" && !node.Text.empty())
        {
            e.Attributes["text"] = node.Text;
            e.ApplyAttributeValue("text", node.Text, true);
        }
    }

    void BuildNode(UiElem& parent, const UiNodeDef& node, const Context& c);
    std::shared_ptr<UiElem> BuildNodeReturning(UiElem& parent, const UiNodeDef& node, const Context& c, const std::map<std::string, std::string>* preVars);

    void BuildChildren(UiElem& parent, const std::vector<UiNodeDef>& nodes, const Context& c)
    {
        for (const UiNodeDef& n : nodes)
            BuildNode(parent, n, c);
    }

    void BuildSlot(UiElem& parent, const UiNodeDef& node, const Context& c)
    {
        std::string slot;
        for (const auto& [name, value] : node.Attributes)
            if (name == "slot")
                slot = value;
        if (c.SlotChildren == nullptr || c.SlotContext == nullptr)
            return;
        for (const UiNodeDef& child : *c.SlotChildren)
        {
            std::string childSlot;
            for (const auto& [name, value] : child.Attributes)
                if (name == "slot")
                    childSlot = value;
            if (childSlot == slot)
                BuildNode(parent, child, *c.SlotContext);
        }
    }

    std::shared_ptr<UiElem> BuildComponent(UiElem& parent, const UiNodeDef& node, const Context& c, const Resolved& r)
    {
        const UiComponentDef& def = *r.Comp;
        std::vector<std::string> localStack;
        std::vector<std::string>& stack = c.Stack != nullptr ? *c.Stack : localStack;
        if (std::find(stack.begin(), stack.end(), def.Tag) != stack.end())
        {
            std::string chain;
            for (const std::string& s : stack) chain += s + " -> ";
            UiMarkupLog::Error(c.File, node.Line, node.Col, "component cycle: " + chain + def.Tag);
            return nullptr;
        }

        // Slots: every child must target a slot the component declares.
        for (const UiNodeDef& child : node.Children)
        {
            std::string slot;
            for (const auto& [name, value] : child.Attributes)
                if (name == "slot")
                    slot = value;
            if (std::find(def.Slots.begin(), def.Slots.end(), slot) == def.Slots.end())
                UiMarkupLog::Error(c.File, child.Line, child.Col, "component '" + def.Tag + "' has no slot '" + slot + "'");
        }

        // Parameters: defaults, then the usage node's values.
        std::map<std::string, std::string> vars;
        std::vector<bool> given(def.Params.size(), false);
        for (size_t i = 0; i < def.Params.size(); ++i)
            vars[def.Params[i].first] = def.Params[i].second;
        std::vector<std::pair<std::string, std::string>> standard;
        for (const auto& [name, value] : node.Attributes)
        {
            bool isParam = false;
            for (size_t i = 0; i < def.Params.size(); ++i)
                if (def.Params[i].first == name)
                {
                    vars[name] = value;
                    given[i] = true;
                    isParam = true;
                }
            if (isParam)
                continue;
            if (IsStandard(name))
                standard.emplace_back(name, value);
            else if (name != "slot")
                UiMarkupLog::Error(c.File, node.Line, node.Col, "component '" + def.Tag + "' has no parameter '" + name + "'");
        }
        for (size_t i = 0; i < def.Params.size(); ++i)
            if (i < def.ParamRequired.size() && def.ParamRequired[i] && !given[i])
                UiMarkupLog::Error(c.File, node.Line, node.Col, "component '" + def.Tag + "' needs parameter '" + def.Params[i].first + "'");

        auto instance = std::make_shared<UiDocumentInstance>();
        instance->Def = r.CompDoc;
        instance->Path = def.File;

        Context inner;
        inner.Instance = instance.get();
        inner.File = def.File;
        inner.Def = r.CompDoc;
        inner.SlotChildren = &node.Children;
        inner.SlotContext = &c;
        inner.ComponentTag = def.Tag;
        stack.push_back(def.Tag);
        inner.Stack = &stack;
        std::shared_ptr<UiElem> top = BuildNodeReturning(parent, def.Body, inner, &vars);
        stack.pop_back();
        if (!top)
            return nullptr;

        top->ComponentTag = def.Tag;
        top->SrcLine = node.Line;
        top->SrcCol = node.Col;
        top->OwnedInstance = instance;
        top->InstanceRoot = true;
        top->Instance = c.Instance;
        std::string usageId;
        for (const auto& [name, value] : standard)
        {
            if (name == "class")
            {
                std::string cls;
                for (char ch : value + " ")
                {
                    if (ch == ' ' || ch == '\t' || ch == '\n') { if (!cls.empty()) { top->Classes.push_back(cls); cls.clear(); } }
                    else cls += ch;
                }
                top->MarkStyleDirty();
                continue;
            }
            top->Attributes[name] = value;
            top->ApplyAttributeValue(name, value, true);
            if (name == "id")
                usageId = value;
        }
        if (!usageId.empty() && c.Instance != nullptr)
            c.Instance->Ids[usageId] = top;
        return top;
    }

    std::shared_ptr<UiElem> BuildNodeReturning(UiElem& parent, const UiNodeDef& node, const Context& c, const std::map<std::string, std::string>* preVars)
    {
        if (node.Tag == "Children")
        {
            BuildSlot(parent, node, c);
            return nullptr;
        }

        const Resolved r = ResolveTag(c, node.Tag);
        switch (r.K)
        {
        case Resolved::Component:
            return BuildComponent(parent, node, c, r);
        case Resolved::Builtin:
        {
            std::shared_ptr<UiElem> e = r.Factory();
            e->ScopeFile = c.File;
            e->Instance = c.Instance;
            if (preVars != nullptr)
                e->m_vars = *preVars;
            parent.AddChild(e);   // attach first: attribute substitution walks up the ancestors
            ApplyNodeAttributes(*e, node, c.File);
            e->OnBuilt();
            BuildChildren(*e, node.Children, c);
            return e;
        }
        case Resolved::Legacy:
        {
            std::shared_ptr<UiElement> legacy = UiElementRegistry::Instance().Create(node.Tag);
            if (!legacy)
            {
                UiMarkupLog::Error(c.File, node.Line, node.Col, "cannot create legacy element '" + node.Tag + "'");
                return nullptr;
            }
            if (!node.Children.empty())
                UiMarkupLog::Error(c.File, node.Line, node.Col, "legacy element <" + node.Tag + "> cannot have children in markup");
            LayoutBlueprintNode bp;
            bp.type = node.Tag;
            for (const auto& [name, value] : node.Attributes)
            {
                if (IsStandard(name) || name == "slot")
                    continue;
                const bool jsonLike = !value.empty() && (value[0] == '[' || value[0] == '{' || value[0] == '-' || (value[0] >= '0' && value[0] <= '9') ||
                                                         value == "true" || value == "false");
                nlohmann::json parsed = jsonLike ? nlohmann::json::parse(value, nullptr, false) : nlohmann::json();
                bp.properties[name] = (jsonLike && !parsed.is_discarded()) ? parsed : nlohmann::json(value);
            }
            UiLayoutSerializer::ApplyProperties(legacy.get(), bp);
            parent.AddChild(legacy);
            return nullptr;
        }
        case Resolved::None:
            UiMarkupLog::Error(c.File, node.Line, node.Col, "unknown tag '" + node.Tag + "'");
            return nullptr;
        }
        return nullptr;
    }

    void BuildNode(UiElem& parent, const UiNodeDef& node, const Context& c)
    {
        BuildNodeReturning(parent, node, c, nullptr);
    }

    void CollectDocument(const std::shared_ptr<UiDocumentDef>& def, std::vector<std::string>& out, int depth)
    {
        if (!def || depth > 8)
            return;
        if (std::find(out.begin(), out.end(), def->Path) != out.end())
            return;
        out.push_back(def->Path);
        for (const std::string& f : def->SheetFiles)
            if (std::find(out.begin(), out.end(), f) == out.end())
                out.push_back(f);
        for (const std::string& import : def->Imports)
            CollectDocument(UiDocumentCache_Get(import), out, depth + 1);
    }
}

bool UiBuilder::BuildLayout(UiElem& element, const std::shared_ptr<UiDocumentDef>& def, const std::shared_ptr<UiDocumentInstance>& instance)
{
    Context c;
    c.Instance = instance.get();
    c.File = def->Path;
    c.Def = def;
    std::vector<std::string> stack;
    c.Stack = &stack;
    ApplyNodeAttributes(element, def->Top, def->Path);
    BuildChildren(element, def->Top.Children, c);
    element.OnBuilt();
    return true;
}

void UiBuilder::CollectDependencies(const std::shared_ptr<UiDocumentDef>& def, std::vector<std::string>& out)
{
    CollectDocument(def, out, 0);
    // components used from the global registry are tracked through their own documents when they are built
    for (const UiComponentDef* comp : UiComponentRegistry::All())
        if (std::find(out.begin(), out.end(), comp->File) == out.end())
            out.push_back(comp->File);
}

bool UiElem::LoadLayout(const std::string& path)
{
    UiSystem::AssertUiThread();
    std::shared_ptr<UiDocumentDef> def = UiDocumentCache_Get(path);
    if (!def)
        return false;
    if (def->Component)
    {
        UiMarkupLog::Error(path, 0, 0, "cannot load a component file as a layout");
        return false;
    }
    if (def->Top.Tag != TagName)
    {
        UiMarkupLog::Error(path, def->Top.Line, def->Top.Col, "layout top element <" + def->Top.Tag + "> does not match element <" + TagName + ">");
        return false;
    }
    auto instance = std::make_shared<UiDocumentInstance>();
    instance->Def = def;
    instance->Path = path;
    OwnedInstance = instance;
    Instance = instance.get();
    ScopeFile = path;
    InstanceRoot = true;
    m_layoutPath = path;
    m_dependencies.clear();
    UiBuilder::CollectDependencies(def, m_dependencies);
    UiBuilder::BuildLayout(*this, def, instance);
    MarkStyleDirty();
    return true;
}

std::shared_ptr<UiElem> UiBuilder::BuildSnippet(UiElem& owner, UiElem& parent, const std::string& name)
{
    UiSystem::AssertUiThread();
    UiBuilder::Context c;
    c.Instance = owner.OwnedInstance ? owner.OwnedInstance.get() : owner.Instance;
    c.File = owner.ScopeFile;
    c.Def = c.Instance != nullptr ? c.Instance->Def : nullptr;
    if (!c.Def)
        c.Def = UiDocumentCache_Get(owner.ScopeFile);
    std::vector<std::string> stack;
    c.Stack = &stack;
    UiNodeDef node;
    node.Tag = name;
    node.Line = owner.SrcLine;
    node.Col = owner.SrcCol;
    return BuildNodeReturning(parent, node, c, nullptr);
}

std::shared_ptr<UiElem> UiElem::LoadNested(const std::string& path)
{
    UiSystem::AssertUiThread();
    std::shared_ptr<UiElem> element;
    if (UiTagRegistry::Factory behind = UiTagRegistry::FindCodeBehind(path))
    {
        element = behind();
    }
    else
    {
        std::shared_ptr<UiDocumentDef> def = UiDocumentCache_Get(path);
        if (!def)
            return nullptr;
        if (def->Component)
        {
            UiMarkupLog::Error(path, 0, 0, "cannot load a component file as a layout");
            return nullptr;
        }
        UiTagRegistry::Factory factory = UiTagRegistry::FindBuiltin(def->Top.Tag);
        if (!factory)
        {
            UiMarkupLog::Error(path, def->Top.Line, def->Top.Col, "the top element <" + def->Top.Tag + "> cannot be a layout root");
            return nullptr;
        }
        element = factory();
        if (!element->LoadLayout(path))
            return nullptr;
    }
    if (element && !element->Id.empty() && element->Instance != nullptr)
        element->Instance->Ids[element->Id] = element;
    return element;
}

std::shared_ptr<UiElem> UiElem::Load(const std::string& path)
{
    UiSystem::AssertUiThread();
    std::shared_ptr<UiElem> element;
    if (UiTagRegistry::Factory behind = UiTagRegistry::FindCodeBehind(path))
    {
        element = behind();
    }
    else
    {
        std::shared_ptr<UiDocumentDef> def = UiDocumentCache_Get(path);
        if (!def)
            return nullptr;
        if (def->Component)
        {
            UiMarkupLog::Error(path, 0, 0, "cannot load a component file as a layout");
            return nullptr;
        }
        UiTagRegistry::Factory factory = UiTagRegistry::FindBuiltin(def->Top.Tag);
        if (!factory)
        {
            UiMarkupLog::Error(path, def->Top.Line, def->Top.Col, "the top element <" + def->Top.Tag + "> cannot be a layout root");
            return nullptr;
        }
        element = factory();
        if (!element->LoadLayout(path))
            return nullptr;
    }
    if (element && !element->Id.empty() && element->Instance != nullptr)
        element->Instance->Ids[element->Id] = element;
    if (element)
        UiSystem::RegisterRoot(element);
    return element;
}

#include "UiActions.h"

#include <Logger.hpp>
#include <SoundSystem/SoundManager.hpp>
#include <UI/Elements/UiElem.h>
#include <UI/Markup/UiMarkupLog.h>
#include <UI/Menus/UiMenuStack.h>
#include <UI/UiNavigation.h>

#include <SDL2/SDL.h>

#include <algorithm>
#include <cctype>

namespace
{
    bool IsIdentStart(char c) { return std::isalpha((unsigned char)c) || c == '_'; }
    bool IsIdentChar(char c) { return std::isalnum((unsigned char)c) || c == '_' || c == '-'; }

    void SkipSpace(const std::string& s, size_t& i)
    {
        while (i < s.size() && std::isspace((unsigned char)s[i]))
            ++i;
    }

    UiMenuStack& StackOf(UiElem& e)
    {
        UiMenuStack* own = e.Stack();
        return own != nullptr ? *own : UiMenus::Stack();
    }

    UiElem* FindTarget(UiElem& from, const std::string& selector)
    {
        std::string id = selector;
        if (!id.empty() && id[0] == '#')
            id.erase(0, 1);
        // The top element of a component instance is declared in the usage's document, so ids are looked up from its parent.
        UiElem* scope = &from;
        if (!from.ComponentTag.empty() && from.AsParentElem() != nullptr)
            scope = from.AsParentElem();
        return scope->FindElem(id);
    }

    const std::string& Arg(const UiActionCall& c, size_t i)
    {
        static const std::string empty;
        return i < c.Args.size() ? c.Args[i] : empty;
    }

    bool NeedArgs(UiElem& from, const UiActionCall& c, size_t n)
    {
        if (c.Args.size() >= n)
            return true;
        UiMarkupLog::Warning(from.ScopeFile, from.SrcLine, from.SrcCol, "action '" + c.Name + "' needs " + std::to_string(n) + " argument(s)");
        return false;
    }

    bool RunBuiltin(UiElem& from, const UiActionCall& call)
    {
        const std::string& n = call.Name;
        if (n == "Back") { StackOf(from).Back(); return true; }
        if (n == "Pop") { StackOf(from).Pop(); return true; }
        if (n == "Clear") { StackOf(from).Clear(); return true; }
        if (n == "Push") { if (NeedArgs(from, call, 1)) StackOf(from).Push(UiActions::NormalizeLayoutPath(Arg(call, 0))); return true; }
        if (n == "Replace") { if (NeedArgs(from, call, 1)) StackOf(from).Replace(UiActions::NormalizeLayoutPath(Arg(call, 0))); return true; }
        if (n == "PushMenu") { if (NeedArgs(from, call, 1)) UiMenus::Stack().Push(UiActions::NormalizeLayoutPath(Arg(call, 0))); return true; }
        if (n == "Quit")
        {
            SDL_Event quit;
            SDL_zero(quit);
            quit.type = SDL_QUIT;
            SDL_PushEvent(&quit);
            return true;
        }
        if (n == "PlaySound")
        {
            if (NeedArgs(from, call, 1))
            {
                auto sound = SoundManager::GetSoundFromPath(Arg(call, 0));
                if (sound)
                {
                    sound->Type = SoundType::UI;
                    sound->Is2D = true;
                    sound->Play();
                }
            }
            return true;
        }
        if (n == "AddClass" || n == "RemoveClass" || n == "ToggleClass")
        {
            if (NeedArgs(from, call, 2))
                if (UiElem* target = FindTarget(from, Arg(call, 0)))
                {
                    if (n == "AddClass") target->AddClass(Arg(call, 1));
                    else if (n == "RemoveClass") target->RemoveClass(Arg(call, 1));
                    else target->ToggleClass(Arg(call, 1));
                }
            return true;
        }
        if (n == "SetFocus")
        {
            if (NeedArgs(from, call, 1))
                if (UiElem* target = FindTarget(from, Arg(call, 0)))
                    UiNavigation::SetFocus(target);
            return true;
        }
        if (n == "Raise")
        {
            if (!NeedArgs(from, call, 1))
                return true;
            UiElem* instanceRoot = &from;
            while (instanceRoot != nullptr && instanceRoot->ComponentTag.empty())
                instanceRoot = instanceRoot->AsParentElem();
            if (instanceRoot == nullptr)
            {
                UiMarkupLog::Warning(from.ScopeFile, from.SrcLine, from.SrcCol, "Raise('" + Arg(call, 0) + "') outside a component instance");
                return true;
            }
            const std::vector<UiActionCall>* calls = instanceRoot->EventCalls(Arg(call, 0));
            if (calls == nullptr)
                return true;
            const std::vector<UiActionCall> copy = *calls;
            UiElem* usageParent = instanceRoot->AsParentElem();
            for (const UiActionCall& c : copy)
                UiActions::Dispatch(usageParent != nullptr ? *usageParent : *instanceRoot, usageParent, c);
            return true;
        }
        return false;
    }

    void ValidateElem(UiElem& e)
    {
        const bool already = e.m_validated;
        e.m_validated = true;
        for (const auto& [event, calls] : e.m_eventCalls)
            if (!already)
            for (const UiActionCall& c : calls)
            {
                bool found = UiActions::IsBuiltin(c.Name);
                for (UiElem* p = &e; p != nullptr && !found; p = p->AsParentElem())
                    found = p->FindHandler(c.Name) != nullptr;
                if (!found && !e.WarnedAction(c.Name))
                    UiMarkupLog::Warning(e.ScopeFile, e.SrcLine, e.SrcCol, "unhandled action '" + c.Name + "' in on" + event);
            }
        for (auto& child : e.children)
            if (UiElem::IsElem(child.get()))
                ValidateElem(static_cast<UiElem&>(*child));
    }
}

bool UiActions::Parse(const std::string& text, std::vector<UiActionCall>& out, std::string& error)
{
    out.clear();
    size_t i = 0;
    while (true)
    {
        SkipSpace(text, i);
        if (i >= text.size())
            return true;
        if (text[i] == ';') { ++i; continue; }
        if (!IsIdentStart(text[i]))
        {
            error = "expected an action name at '" + text.substr(i, 12) + "'";
            return false;
        }
        UiActionCall call;
        while (i < text.size() && IsIdentChar(text[i]))
            call.Name += text[i++];
        SkipSpace(text, i);
        if (i < text.size() && text[i] == '(')
        {
            ++i;
            while (true)
            {
                SkipSpace(text, i);
                if (i >= text.size()) { error = "missing ')' after " + call.Name; return false; }
                if (text[i] == ')') { ++i; break; }
                std::string arg;
                if (text[i] == '\'' || text[i] == '"')
                {
                    const char quote = text[i++];
                    while (i < text.size() && text[i] != quote)
                        arg += text[i++];
                    if (i >= text.size()) { error = "unterminated string in " + call.Name; return false; }
                    ++i;
                }
                else
                {
                    while (i < text.size() && text[i] != ',' && text[i] != ')' && !std::isspace((unsigned char)text[i]))
                        arg += text[i++];
                    if (arg.empty()) { error = "empty argument in " + call.Name; return false; }
                }
                call.Args.push_back(arg);
                SkipSpace(text, i);
                if (i < text.size() && text[i] == ',') { ++i; continue; }
                if (i < text.size() && text[i] == ')') { ++i; break; }
                error = "expected ',' or ')' in " + call.Name;
                return false;
            }
        }
        out.push_back(std::move(call));
        SkipSpace(text, i);
        if (i < text.size())
        {
            if (text[i] != ';') { error = "expected ';' after " + out.back().Name; return false; }
            ++i;
        }
    }
}

void UiActions::Dispatch(UiElem& sender, UiElem* searchFrom, const UiActionCall& call)
{
    for (UiElem* e = searchFrom != nullptr ? searchFrom : &sender; e != nullptr; e = e->AsParentElem())
    {
        const UiElem::Handler* h = e->FindHandler(call.Name);
        if (h == nullptr)
            continue;
        if (h->Guarded && h->Guard.expired())
            return;
        const UiElem::ActionFn fn = h->Fn;   // the handler may re-register itself
        fn(sender, call.Args);
        return;
    }
    if (RunBuiltin(sender, call))
        return;
    if (!sender.WarnedAction(call.Name))
        UiMarkupLog::Warning(sender.ScopeFile, sender.SrcLine, sender.SrcCol, "unhandled action '" + call.Name + "'");
}

const std::vector<UiActionInfo>& UiActions::Builtins()
{
    static const std::vector<UiActionInfo> list = {
        { "AddClass", "selector, class", "Adds a class to the element found by #id or id path" },
        { "Back", "", "Runs back on the element's menu stack (the main stack when it is on none)" },
        { "Clear", "", "Pops every page of the element's menu stack" },
        { "Pop", "", "Pops the top page of the element's menu stack" },
        { "PlaySound", "path", "Plays a 2D UI sound" },
        { "Push", "path", "Pushes a layout on the element's menu stack (the main stack when it is on none)" },
        { "PushMenu", "path", "Pushes a layout on the main menu stack" },
        { "Quit", "", "Asks the engine to quit" },
        { "Raise", "event", "Fires the on<event> attribute of the enclosing component instance, handled from its parent" },
        { "RemoveClass", "selector, class", "Removes a class from the element found by #id or id path" },
        { "Replace", "path", "Replaces the top page of the element's menu stack with a layout" },
        { "SetFocus", "selector", "Moves navigation focus to the element found by #id or id path" },
        { "ToggleClass", "selector, class", "Toggles a class on the element found by #id or id path" },
    };
    return list;
}

bool UiActions::IsBuiltin(const std::string& name)
{
    for (const UiActionInfo& info : Builtins())
        if (info.Name == name)
            return true;
    return false;
}

const std::vector<UiEventInfo>& UiActions::Events()
{
    static const std::vector<UiEventInfo> list = {
        { "activate", "The element was activated by confirm, a click or a touch" },
        { "blur", "The element lost navigation focus" },
        { "cancel", "Back was requested on this page, before the stack handles it" },
        { "focus", "The element received navigation focus" },
        { "load", "Once, after the element's first layout" },
        { "mouseout", "The pointer left the element" },
        { "mouseover", "The pointer entered the element" },
    };
    return list;
}

bool UiActions::IsKnownEvent(const std::string& name)
{
    for (const UiEventInfo& e : Events())
        if (e.Name == name)
            return true;
    return false;
}

void UiActions::Validate(UiElem& root)
{
    ValidateElem(root);
}

std::string UiActions::NormalizeLayoutPath(const std::string& path)
{
    if (path.rfind("GameData/", 0) == 0 || path.empty())
        return path;
    return "GameData/" + path;
}

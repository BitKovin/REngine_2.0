#include "UiSelector.h"

#include <UI/Elements/UiElem.h>
#include <UI/Markup/UiMarkupLog.h>
#include <UI/UiNavigation.h>

#include <algorithm>

namespace
{
    bool PseudoFromName(const std::string& name, UiPseudo& out)
    {
        static const struct { const char* Name; UiPseudo Value; } kTable[] = {
            { "hover", UiPseudo::Hover }, { "active", UiPseudo::Active }, { "focus", UiPseudo::Focus },
            { "focus-within", UiPseudo::FocusWithin }, { "disabled", UiPseudo::Disabled }, { "selected", UiPseudo::Selected },
            { "root", UiPseudo::Root }, { "entering", UiPseudo::Entering }, { "leaving", UiPseudo::Leaving },
            { "checked", UiPseudo::Checked } };
        for (const auto& e : kTable)
            if (name == e.Name) { out = e.Value; return true; }
        return false;
    }

    bool HasFocusWithin(const UiElement* e)
    {
        for (const UiElement* f = UiNavigation::Focused; f; f = f->parent)
            if (f == e)
                return true;
        return false;
    }

    bool CompoundMatches(const UiCompound& c, const UiElem& e)
    {
        if (!c.Tag.empty() && c.Tag != e.TagName && c.Tag != e.ComponentTag)
            return false;
        if (!c.Id.empty() && c.Id != e.Id)
            return false;
        for (const std::string& cls : c.Classes)
            if (!e.HasClass(cls))
                return false;
        for (UiPseudo p : c.Pseudos)
        {
            switch (p)
            {
            case UiPseudo::Hover: if (!(e.States & kStateHover)) return false; break;
            case UiPseudo::Active: if (!(e.States & kStateActive)) return false; break;
            case UiPseudo::Focus: if (UiNavigation::Focused != &e) return false; break;
            case UiPseudo::FocusWithin: if (!HasFocusWithin(&e)) return false; break;
            case UiPseudo::Disabled: if (!(e.States & kStateDisabled)) return false; break;
            case UiPseudo::Selected: if (!(e.States & kStateSelected)) return false; break;
            case UiPseudo::Root: if (!e.InstanceRoot) return false; break;
            case UiPseudo::Entering: if (!(e.States & kStateEntering)) return false; break;
            case UiPseudo::Leaving: if (!(e.States & kStateLeaving)) return false; break;
            case UiPseudo::Checked: if (!(e.States & kStateChecked)) return false; break;
            }
        }
        return true;
    }

    // Parent as a UiElem, or null when the parent chain leaves the markup tree.
    const UiElem* ParentElem(const UiElem& e)
    {
        return e.parent != nullptr ? e.AsParentElem() : nullptr;
    }

    bool MatchFrom(const UiSelector& s, int part, const UiElem& e)
    {
        if (!CompoundMatches(s.Parts[part], e))
            return false;
        if (part == 0)
            return true;
        const UiCombinator comb = s.Combinators[part - 1];
        const UiElem* p = ParentElem(e);
        if (comb == UiCombinator::Child)
            return p != nullptr && MatchFrom(s, part - 1, *p);
        for (; p != nullptr; p = ParentElem(*p))
            if (MatchFrom(s, part - 1, *p))
                return true;
        return false;
    }
}

const std::vector<std::string>& UiPseudoNames()
{
    static const std::vector<std::string> names = { "hover", "active", "focus", "focus-within", "disabled", "selected", "root", "entering", "leaving", "checked" };
    return names;
}

bool UiParseSelectorList(const std::vector<UiCssToken>& prelude, const std::string& path, std::vector<UiSelector>& out)
{
    // Split on commas first.
    std::vector<std::vector<UiCssToken>> groups(1);
    for (const UiCssToken& t : prelude)
    {
        if (t.Type == UiCssTok::Eof)
            break;
        if (t.Type == UiCssTok::Comma)
            groups.emplace_back();
        else
            groups.back().push_back(t);
    }

    bool ok = true;
    for (auto& group : groups)
    {
        while (!group.empty() && group.front().Type == UiCssTok::Whitespace) group.erase(group.begin());
        while (!group.empty() && group.back().Type == UiCssTok::Whitespace) group.pop_back();
        if (group.empty())
        {
            UiMarkupLog::Error(path, prelude.empty() ? 0 : prelude.front().Line, prelude.empty() ? 0 : prelude.front().Col, "empty selector");
            ok = false;
            continue;
        }

        UiSelector selector;
        UiCompound current;
        bool haveCompound = false, pendingChild = false, dropSelector = false, error = false;
        auto flush = [&]() {
            if (!haveCompound)
                return;
            if (!selector.Parts.empty())
                selector.Combinators.push_back(pendingChild ? UiCombinator::Child : UiCombinator::Descendant);
            selector.Parts.push_back(current);
            current = UiCompound();
            haveCompound = false;
            pendingChild = false;
        };

        for (size_t i = 0; i < group.size() && !error; ++i)
        {
            const UiCssToken& t = group[i];
            switch (t.Type)
            {
            case UiCssTok::Whitespace:
                flush();
                break;
            case UiCssTok::Delim:
                if (t.Text == ">")
                {
                    flush();
                    pendingChild = true;
                }
                else if (t.Text == "*")
                    haveCompound = true;
                else if (t.Text == ".")
                {
                    if (i + 1 < group.size() && group[i + 1].Type == UiCssTok::Ident)
                    {
                        current.Classes.push_back(group[++i].Text);
                        haveCompound = true;
                    }
                    else
                    {
                        UiMarkupLog::Error(path, t.Line, t.Col, "expected a class name after '.'");
                        error = true;
                    }
                }
                else
                {
                    UiMarkupLog::Error(path, t.Line, t.Col, "unsupported selector token '" + t.Text + "'");
                    error = true;
                }
                break;
            case UiCssTok::Ident:
                current.Tag = t.Text;
                haveCompound = true;
                break;
            case UiCssTok::Hash:
                current.Id = t.Text;
                haveCompound = true;
                break;
            case UiCssTok::Colon:
                if (i + 1 < group.size() && group[i + 1].Type == UiCssTok::Ident)
                {
                    const UiCssToken& name = group[++i];
                    UiPseudo pseudo;
                    if (PseudoFromName(name.Text, pseudo))
                        current.Pseudos.push_back(pseudo);
                    else
                    {
                        UiMarkupLog::Warning(path, name.Line, name.Col, "unknown pseudo-class ':" + name.Text + "'; the selector is ignored");
                        dropSelector = true;
                    }
                    haveCompound = true;
                }
                else
                {
                    UiMarkupLog::Error(path, t.Line, t.Col, "unsupported selector (pseudo-element or functional pseudo-class)");
                    error = true;
                }
                break;
            default:
                UiMarkupLog::Error(path, t.Line, t.Col, "unsupported selector syntax");
                error = true;
                break;
            }
        }
        flush();
        if (error || selector.Parts.empty() || pendingChild)
        {
            if (!error)
                UiMarkupLog::Error(path, group.front().Line, group.front().Col, "incomplete selector");
            ok = false;
            continue;
        }
        if (dropSelector)
            continue;
        int ids = 0, classes = 0, tags = 0;
        for (const UiCompound& c : selector.Parts)
        {
            ids += c.Id.empty() ? 0 : 1;
            classes += (int)c.Classes.size() + (int)c.Pseudos.size();
            tags += c.Tag.empty() ? 0 : 1;
        }
        selector.Specificity = ids * 10000 + classes * 100 + tags;
        out.push_back(std::move(selector));
    }
    return ok;
}

bool UiSelectorMatches(const UiSelector& selector, const UiElem& element)
{
    if (selector.Parts.empty())
        return false;
    return MatchFrom(selector, (int)selector.Parts.size() - 1, element);
}

#include "UiStateSnapshot.h"

#ifndef DISTRIBUTION

#include <UI/Elements/UiElem.h>
#include <UI/UiNavigation.h>

#include <string>
#include <unordered_map>

namespace
{
    using Located = std::vector<std::pair<UiElem*, std::string>>;

    // Every element of the tree with its key: "/Panel#settings/Frame#content/Label[1]".
    void Walk(UiElem& e, const std::string& key, Located& out)
    {
        out.emplace_back(&e, key);
        std::map<std::string, int> counts;
        for (const auto& child : e.children)
        {
            if (!UiElem::IsElem(child.get()))
                continue;
            UiElem& c = static_cast<UiElem&>(*child);
            if (c.InternalOverlay)
                continue;
            const int n = counts[c.TagName]++;
            Walk(c, key + "/" + c.TagName + (c.Id.empty() ? "[" + std::to_string(n) + "]" : "#" + c.Id), out);
        }
    }

    struct Lookup
    {
        std::unordered_map<std::string, UiElem*> ByKey;
        std::unordered_map<std::string, std::vector<UiElem*>> ById;

        explicit Lookup(UiElem& root)
        {
            Located all;
            Walk(root, "", all);
            for (auto& [elem, key] : all)
            {
                ByKey.emplace(key, elem);
                if (!elem->Id.empty())
                    ById[elem->Id].push_back(elem);
            }
        }

        // Same place first; if the element moved, an id that is unique in the new tree finds it.
        UiElem* Find(const std::string& key, const std::string& id) const
        {
            const auto k = ByKey.find(key);
            if (k != ByKey.end())
                return k->second;
            if (!id.empty())
            {
                const auto i = ById.find(id);
                if (i != ById.end() && i->second.size() == 1)
                    return i->second.front();
            }
            return nullptr;
        }
    };
}

UiStateSnapshot UiStateSnapshot::Capture(UiElem& root)
{
    UiStateSnapshot snap;
    Located all;
    Walk(root, "", all);
    for (auto& [e, key] : all)
    {
        Entry entry;
        entry.Key = key;
        entry.Id = e->Id;
        bool interesting = false;

        for (const auto& [name, origin] : e->m_runtimeAttrs)
        {
            const auto now = e->Attributes.find(name);
            if (now == e->Attributes.end())
                continue;
            entry.RuntimeAttrs[name] = origin;
            entry.AttrValues[name] = now->second;
            interesting = true;
        }
        for (const std::string& name : e->m_runtimeVars)
        {
            const auto v = e->m_vars.find(name);
            if (v == e->m_vars.end())
                continue;
            entry.Vars[name] = v->second;
            const auto n = e->m_numVars.find(name);
            if (n != e->m_numVars.end())
                entry.NumVars[name] = n->second;
            interesting = true;
        }
        if (!e->CodeClasses.empty())
        {
            entry.CodeClasses = e->CodeClasses;
            interesting = true;
        }
        if ((e->States & kStateChecked) != 0)
        {
            entry.Checked = true;
            interesting = true;
        }
        if (e->IsScrollContainer() && (e->ScrollOffset != glm::vec2(0.f) || e->ScrollTarget != glm::vec2(0.f)))
        {
            entry.Scrolls = true;
            entry.Scroll = e->ScrollOffset;
            entry.ScrollTarget = e->ScrollTarget;
            interesting = true;
        }
        if (e->CaptureValue(entry.Number, entry.Text))
        {
            entry.HasValue = true;
            interesting = true;
        }
        if (UiNavigation::Focused == static_cast<UiElement*>(e))
        {
            entry.Focused = true;
            interesting = true;
        }
        if (interesting)
            snap.m_entries.push_back(std::move(entry));
    }
    return snap;
}

void UiStateSnapshot::Restore(UiElem& root) const
{
    // 1. attributes the page's code had set (the content of a Frame, the selected tab): they can create new elements,
    //    which later entries refer to, so repeat until nothing new is found
    std::vector<bool> done(m_entries.size(), false);
    for (int round = 0; round < 8; ++round)
    {
        const Lookup lookup(root);
        bool created = false;
        for (size_t i = 0; i < m_entries.size(); ++i)
        {
            const Entry& entry = m_entries[i];
            if (done[i])
                continue;
            UiElem* e = lookup.Find(entry.Key, entry.Id);
            if (e == nullptr)
                continue;
            done[i] = true;
            for (const auto& [name, origin] : entry.RuntimeAttrs)
            {
                // the file was edited to say something else about this attribute: the file wins
                const auto now = e->Attributes.find(name);
                const bool markupUnchanged = (now != e->Attributes.end()) == origin.first && (now == e->Attributes.end() || now->second == origin.second);
                if (!markupUnchanged)
                    continue;
                e->SetAttribute(name, entry.AttrValues.at(name));
                created = true;
            }
        }
        if (!created)
            break;
    }

    // 2. the rest
    const Lookup lookup(root);
    UiElem* focus = nullptr;
    for (const Entry& entry : m_entries)
    {
        UiElem* e = lookup.Find(entry.Key, entry.Id);
        if (e == nullptr)
            continue;
        for (const auto& [name, value] : entry.Vars)
        {
            const auto n = entry.NumVars.find(name);
            if (n != entry.NumVars.end())
                e->SetVar(name, n->second);
            else
                e->SetVar(name, value);
        }
        for (const std::string& cls : entry.CodeClasses)
            e->SetClass(cls, true);
        if (entry.Checked)
        {
            e->States |= kStateChecked;
            e->MarkStyleDirty();
        }
        if (entry.Scrolls)
        {
            e->ScrollOffset = entry.Scroll;
            e->ScrollTarget = entry.ScrollTarget;
        }
        if (entry.HasValue)
            e->RestoreValue(entry.Number, entry.Text);
        if (entry.Focused && e->IsFocusable())
            focus = e;
    }
    if (focus != nullptr)
        UiNavigation::SetFocus(focus);
}

#endif

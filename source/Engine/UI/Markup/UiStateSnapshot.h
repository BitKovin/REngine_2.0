#pragma once
#include <glm/glm.hpp>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

class UiElem;

// Development builds only: what the user has done to a page (values typed or dragged, toggles, scroll, focus, and
// whatever the page's code set at run time) so that hot reload can rebuild the page from the changed files and put it back.
// Elements are matched by id when they have one, otherwise by their place among siblings of the same tag, so edits that
// add, remove or move elements keep the state of everything that still exists.
#ifndef DISTRIBUTION
class UiStateSnapshot
{
public:
    static UiStateSnapshot Capture(UiElem& root);
    void Restore(UiElem& root) const;

private:
    struct Entry
    {
        std::string Key;                                   // path from the root
        std::string Id;
        std::map<std::string, std::pair<bool, std::string>> RuntimeAttrs;   // name -> markup value it replaced
        std::map<std::string, std::string> AttrValues;     // name -> value now
        std::map<std::string, std::string> Vars;
        std::map<std::string, double> NumVars;
        std::vector<std::string> CodeClasses;
        bool Checked = false;
        bool Scrolls = false;
        glm::vec2 Scroll = glm::vec2(0.f), ScrollTarget = glm::vec2(0.f);
        bool HasValue = false;
        double Number = 0.0;
        std::string Text;
        bool Focused = false;
    };
    std::vector<Entry> m_entries;
};
#endif

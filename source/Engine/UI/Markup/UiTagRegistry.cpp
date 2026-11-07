#include "UiTagRegistry.h"

#include <UI/Elements/UiElem.h>
#include <UI/Menus/UiActions.h>
#include <UI/Preview/UiDatabase.h>

#include <algorithm>
#include <map>

namespace
{
    std::map<std::string, UiTagRegistry::Factory>& Builtins()
    {
        static std::map<std::string, UiTagRegistry::Factory> m;
        return m;
    }
    std::map<std::string, UiTagRegistry::TagInfo>& Infos()
    {
        static std::map<std::string, UiTagRegistry::TagInfo> m;
        return m;
    }
    std::map<std::string, UiTagRegistry::Factory>& CodeBehinds()
    {
        static std::map<std::string, UiTagRegistry::Factory> m;
        return m;
    }
}

void UiTagRegistry::RegisterBuiltin(const std::string& tag, Factory f, const TagInfo& info)
{
    Builtins()[tag] = f;
    Infos()[tag] = info;
    UiDatabase::MarkDirty("tag " + tag);
}

UiTagRegistry::Factory UiTagRegistry::FindBuiltin(const std::string& tag)
{
    const auto it = Builtins().find(tag);
    return it == Builtins().end() ? nullptr : it->second;
}

const std::vector<UiTagRegistry::TagInfo>& UiTagRegistry::BuiltinInfos()
{
    static std::vector<TagInfo> sorted;
    sorted.clear();
    for (const auto& [name, info] : Infos())
        sorted.push_back(info);
    return sorted;
}

const std::vector<UiTagRegistry::AttributeInfo>& UiTagRegistry::StandardAttributes()
{
    static const std::vector<AttributeInfo> attrs = {
        { "id", "string", "", false, "Element id, unique inside its document instance" },
        { "class", "string", "", false, "Space separated style classes" },
        { "style", "string", "", false, "Inline declarations, e.g. width: 100px; height: 20px" },
        { "visible", "bool", "true", false, "false collapses the element" },
        { "enabled", "bool", "true", false, "false disables focus and activation" },
        { "selected", "bool", "false", false, "Sets the :selected state" },
        { "hittest", "bool", "false", false, "true makes the element take pointer hits" },
        { "default-focus", "bool", "false", false, "The element a menu page focuses first (first in document order)" },
        { "close-on-back", "bool", "true", false, "Host option of a menu page: back pops it" },
        { "shows-below", "bool", "false", false, "Host option of a menu page: pages under it stay visible" },
        { "pauses-game", "bool", "false", false, "Host option of a menu page: the game is paused while it is on the stack" },
        { "hides-hud", "bool", "false", false, "Host option of a menu page: the HUD is hidden while it is on the stack" },
        { "confirm-changes", "bool", "false", false, "Menu page: back with changed bound settings asks to apply or discard them first" },
        { "nav-up", "string", "", false, "#id of the element focus moves to on up (overrides spatial navigation)" },
        { "nav-down", "string", "", false, "#id of the element focus moves to on down" },
        { "nav-left", "string", "", false, "#id of the element focus moves to on left" },
        { "nav-right", "string", "", false, "#id of the element focus moves to on right" },
        { "onactivate", "actions", "", false, "Calls run when the element is activated, e.g. Push('ui/a.xml'); PlaySound('x.wav')" },
        { "onmouseover", "actions", "", false, "Calls run when the pointer enters the element" },
        { "onmouseout", "actions", "", false, "Calls run when the pointer leaves the element" },
        { "onfocus", "actions", "", false, "Calls run when the element receives navigation focus" },
        { "onblur", "actions", "", false, "Calls run when the element loses navigation focus" },
        { "oncancel", "actions", "", false, "Calls run when back is requested on this page" },
        { "onload", "actions", "", false, "Calls run once after the element's first layout" },
    };
    return attrs;
}

void UiTagRegistry::RegisterCodeBehind(const std::string& layoutPath, Factory f)
{
    CodeBehinds()[UiActions::NormalizeLayoutPath(layoutPath)] = f;
}

UiTagRegistry::Factory UiTagRegistry::FindCodeBehind(const std::string& layoutPath)
{
    const auto it = CodeBehinds().find(UiActions::NormalizeLayoutPath(layoutPath));
    return it == CodeBehinds().end() ? nullptr : it->second;
}

#pragma once
#include <json.hpp>
#include <string>

class UiElem;

// Read-only runtime inspection for Studio and the in-game console: a JSON tree, hit testing and computed values.
// Everything runs on the main thread and reads the live tree after UiSystem::Update (conventions section 2).
namespace UiInspect
{
    // Size of the UI viewport in UI units ({"w":..,"h":..}); the UI target stays 1080 units tall (decision D10).
    nlohmann::json ViewportJson();
    // Stable reference to an element: source file/line/column, DOM path and scoped id.
    nlohmann::json ElementRef(const UiElem& element);
    // Every live root with children, layout rectangles in UI units and state flags.
    nlohmann::json TreeJson(int maxDepth);
    // Deepest visible element under a UI-unit point, with its ancestor chain. {"hit":false} when nothing is there.
    nlohmann::json PickAt(float x, float y);
    // Rectangles, transform, states and computed values of one element ("id", "scope/id" or a DOM path).
    nlohmann::json ComputedJson(const std::string& idOrPath);
    // Finds an element by id or by DOM path (as written by ElementRef). nullptr when there is no match.
    UiElem* Resolve(const std::string& idOrPath);

    // ui_dump_json [depth], ui_pick_at <x> <y>, ui_computed <id-or-path>
    void RegisterConsoleCommands();
}

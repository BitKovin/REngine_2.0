#pragma once
#include <glm/glm.hpp>
#include <string>

class UiElem;
class UiMenuStack;

// Drag and drop and context menus (phase 7.5).
//
// An element with draggable="true" starts a drag once the pointer moved 6 px from its press (a gamepad picks it up with confirm).
// Drop targets are elements with droptarget="true". Events (attributes on<event>, or UiElem::On): dragstart (on the source),
// dragenter / dragleave / drop (on the target). While a drag runs the source has the :dragging state, and a ghost of it follows
// the pointer. In a handler, UiDragDrop::Source() is the dragged element and Source()->DragPayload the C++ payload.
//
// oncontextmenu fires on the nearest element (starting at the one under the pointer) that has the event, for a right mouse
// button, a 0.5 s touch hold, or the ui_context action; PointerPos() is where.
namespace UiDragDrop
{
    constexpr float kDragThresholdPx = 6.f;
    constexpr float kLongPressSeconds = 0.5f;

    bool Active();
    UiElem* Source();
    UiElem* Target();
    glm::vec2 PointerPos();      // where the last drag move / context menu request happened (UI units)
    glm::vec2 GhostOffset();     // how far the ghost is from the source
    int DropCount();             // drops since start (tests)

    // Called by UiSystem
    void Press(int touchId, UiElem& pressed, glm::vec2 position);
    bool Move(int touchId, glm::vec2 position);      // true: a drag runs, the pressed element does not get the move
    bool Release(int touchId, glm::vec2 position);   // true: the release was a drag drop or a long press, buttons must not activate
    void Tick(float dt);                             // long press, ui_context, gamepad ghost
    bool NavConfirm(UiElem& focused);                // true: consumed (picked up or dropped)
    bool NavCancel();                                // true: consumed (a gamepad drag was aborted)
    // The ContextMenu action: pushes the page and places its element with class context-menu where the request happened.
    UiElem* PushContextMenu(UiMenuStack& stack, const std::string& layoutPath);
    void Reset();                                    // level change
    void RecordGhost();                              // after layout: records the ghost for DrawGhost
    void DrawGhost();                                // in the late UI pass, over everything
}

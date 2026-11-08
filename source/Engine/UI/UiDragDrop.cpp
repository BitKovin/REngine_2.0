#include "UiDragDrop.h"

#include <EngineMain.h>
#include <Input.h>
#include <UI/Elements/UiElem.h>
#include <UI/Elements/UiElemPanel.h>
#include <UI/Menus/UiInputPrompts.h>
#include <UI/Menus/UiMenuStack.h>
#include <UI/UiNavigation.h>
#include <UI/UiSystem.h>

#include <cmath>
#include <cstdio>
#include <vector>
#include <memory>

namespace
{
    struct State
    {
        // A press that may become a drag or a long press.
        int PressId = -1;
        std::weak_ptr<UiElem> Pressed;
        std::weak_ptr<UiElem> Candidate;   // the draggable ancestor of the pressed element
        glm::vec2 PressPos = glm::vec2(0.f);
        float Held = 0.f;
        bool LongFired = false;     // the hold was settled (a long press fired, or the pointer moved)
        bool LongContext = false;   // a long press fired a context menu

        bool Active = false;
        bool ByNav = false;
        std::weak_ptr<UiElem> Source;
        std::weak_ptr<UiElem> Target;
        glm::vec2 Pos = glm::vec2(0.f);
        glm::vec2 Origin = glm::vec2(0.f);   // where the drag started (pointer position, or the centre of the picked-up element)

        glm::vec2 ContextPos = glm::vec2(0.f);
        int Drops = 0;
    };
    State g_s;
    std::shared_ptr<UiElem> g_ghost;   // an unattached root the ghost is recorded into

    bool AttrTrue(const UiElem& e, const char* name)
    {
        const auto it = e.Attributes.find(name);
        return it != e.Attributes.end() && (it->second == "true" || it->second == "1");
    }

    std::shared_ptr<UiElem> Shared(UiElem& e)
    {
        try { return std::static_pointer_cast<UiElem>(e.shared_from_this()); }
        catch (...) { return nullptr; }
    }

    // `from` or the nearest ancestor that satisfies the attribute.
    UiElem* Climb(UiElem* from, const char* attribute)
    {
        for (UiElem* e = from; e != nullptr; e = e->AsParentElem())
            if (AttrTrue(*e, attribute) && !(e->States & kStateDisabled))
                return e;
        return nullptr;
    }

    // `from` or the nearest ancestor that has the event (an attribute or a code handler).
    UiElem* ClimbEvent(UiElem* from, const std::string& event)
    {
        for (UiElem* e = from; e != nullptr; e = e->AsParentElem())
            if (e->EventCalls(event) != nullptr || e->FindHandler(event) != nullptr)
                return e;
        return nullptr;
    }

    UiElem* HitElem(glm::vec2 position)
    {
        const std::shared_ptr<UiElement> hit = EngineMain::Viewport.GetHitElementUnderPosition(position);
        UiElement* e = hit.get();
        while (e != nullptr && !UiElem::IsElem(e))
            e = e->parent;
        return static_cast<UiElem*>(e);
    }

    glm::vec2 Centre(const UiElem& e)
    {
        return UiElement::TransformPoint(e.AbsoluteTransform, glm::vec2(e.BorderRect.W, e.BorderRect.H) * 0.5f);
    }

    void SetDragging(UiElem& e, bool on)
    {
        if (on) e.States |= kStateDragging; else e.States &= ~kStateDragging;
        e.MarkStyleDirty();
    }

    void EndDrag()
    {
        if (auto source = g_s.Source.lock())
        {
            SetDragging(*source, false);
            source->Fire("dragend");   // the payload is still there for the handler
            source->DragPayload.reset();
        }
        g_s.Active = false;
        g_s.ByNav = false;
        g_s.Source.reset();
        g_s.Target.reset();
    }

    void SetTarget(UiElem* target)
    {
        const std::shared_ptr<UiElem> old = g_s.Target.lock();
        if (old.get() == target)
            return;
        g_s.Target.reset();
        if (old)
            old->Fire("dragleave");
        if (target != nullptr)
        {
            g_s.Target = Shared(*target);
            target->Fire("dragenter");
        }
    }

    void Begin(UiElem& source, bool byNav, glm::vec2 origin)
    {
        g_s.Active = true;
        g_s.ByNav = byNav;
        g_s.Source = Shared(source);
        g_s.Origin = origin;
        g_s.Pos = origin;
        SetDragging(source, true);
        source.Fire("dragstart");
    }

    // Drops on `target` (may be null: nothing happens) and ends the drag.
    void Drop(UiElem* target)
    {
        if (target != nullptr)
        {
            // keep the source alive through the handlers
            const std::shared_ptr<UiElem> keep = g_s.Source.lock();
            const std::shared_ptr<UiElem> hold = Shared(*target);
            SetTarget(nullptr);   // dragleave comes first, so a style that marks the hovered target is cleared before drop runs
            ++g_s.Drops;
            target->Fire("drop");
        }
        EndDrag();
    }

    void FireContext(UiElem* from, glm::vec2 position)
    {
        UiElem* target = ClimbEvent(from, "contextmenu");
        if (target == nullptr)
            return;
        g_s.ContextPos = position;
        target->Fire("contextmenu");
    }
}

namespace UiDragDrop
{
    bool Active() { return g_s.Active; }
    UiElem* Source() { return g_s.Source.lock().get(); }
    UiElem* Target() { return g_s.Target.lock().get(); }
    glm::vec2 PointerPos() { return g_s.ContextPos; }
    int DropCount() { return g_s.Drops; }

    glm::vec2 GhostOffset() { return g_s.Pos - g_s.Origin; }

    void Press(int touchId, UiElem& pressed, glm::vec2 position)
    {
        g_s.PressId = touchId;
        g_s.Pressed = Shared(pressed);
        g_s.Candidate.reset();
        if (UiElem* draggable = Climb(&pressed, "draggable"))
            g_s.Candidate = Shared(*draggable);
        g_s.PressPos = position;
        g_s.Held = 0.f;
        g_s.LongFired = false;
        g_s.LongContext = false;
    }

    bool Move(int touchId, glm::vec2 position)
    {
        if (g_s.Active && !g_s.ByNav && touchId == g_s.PressId)
        {
            g_s.Pos = position;
            SetTarget(Climb(HitElem(position), "droptarget"));
            return true;
        }
        if (touchId != g_s.PressId || g_s.Active)
            return false;
        if (glm::length(position - g_s.PressPos) < kDragThresholdPx)
            return false;
        g_s.LongFired = true;   // moving cancels a long press
        if (auto candidate = g_s.Candidate.lock())
        {
            Begin(*candidate, false, g_s.PressPos);
            g_s.Pos = position;
            SetTarget(Climb(HitElem(position), "droptarget"));
            return true;
        }
        return false;
    }

    bool Release(int touchId, glm::vec2 position)
    {
        if (touchId != g_s.PressId)
            return false;
        g_s.PressId = -1;
        const bool longPress = g_s.LongContext;
        g_s.Pressed.reset();
        g_s.Candidate.reset();
        if (g_s.Active && !g_s.ByNav)
        {
            g_s.Pos = position;
            Drop(Climb(HitElem(position), "droptarget"));
            return true;
        }
        return longPress;
    }

    void Tick(float dt)
    {
        // Touch long press: oncontextmenu at the press position.
        if (g_s.PressId > 1 && !g_s.LongFired && !g_s.Active)
        {
            g_s.Held += dt;
            if (g_s.Held >= kLongPressSeconds)
            {
                g_s.LongFired = true;
                g_s.LongContext = true;
                if (auto pressed = g_s.Pressed.lock())
                    FireContext(pressed.get(), g_s.PressPos);
            }
        }

        // ui_context (right mouse button, gamepad Y): the element under the pointer, or the focused one when navigating.
        if (Input::GetAction("ui_context")->Pressed() && !g_s.Active)
        {
            UiElem* from = nullptr;
            glm::vec2 position(0.f);
            // The pointer decides unless the last input came from a gamepad; a pointer over nothing that has the event falls back to focus.
            const auto mouse = Input::TouchActions.find(1);
            if (UiInputPrompts::LastDevice() != UiInputDevice::Gamepad && mouse != Input::TouchActions.end())
            {
                position = mouse->second.position;
                from = ClimbEvent(HitElem(position), "contextmenu");
            }
            if (from == nullptr && UiNavigation::Focused != nullptr && UiElem::IsElem(UiNavigation::Focused))
            {
                from = static_cast<UiElem*>(UiNavigation::Focused);
                position = Centre(*from);
            }
            if (from != nullptr)
                FireContext(from, position);
        }

        // Gamepad drag: the ghost sits on the focused element, and entering / leaving follows focus.
        if (g_s.Active && g_s.ByNav)
        {
            if (!g_s.Source.lock())
            {
                EndDrag();
                return;
            }
            UiElement* focused = UiNavigation::Focused;
            if (focused != nullptr && UiElem::IsElem(focused))
            {
                g_s.Pos = Centre(static_cast<UiElem&>(*focused));
                SetTarget(Climb(static_cast<UiElem*>(focused), "droptarget"));
            }
        }
        else if (g_s.Active && !g_s.Source.lock())
            EndDrag();
    }

    bool NavConfirm(UiElem& focused)
    {
        if (!g_s.Active)
        {
            UiElem* draggable = Climb(&focused, "draggable");
            if (draggable == nullptr)
                return false;
            Begin(*draggable, true, Centre(*draggable));
            return true;
        }
        if (!g_s.ByNav)
            return false;
        UiElem* target = Climb(&focused, "droptarget");
        if (target == nullptr)
            return true;   // confirm on something that takes no drops: the item stays in hand
        Drop(target);
        return true;
    }

    bool NavCancel()
    {
        if (!g_s.Active || !g_s.ByNav)
            return false;
        SetTarget(nullptr);
        EndDrag();
        return true;
    }

    UiElem* PushContextMenu(UiMenuStack& stack, const std::string& layoutPath)
    {
        UiElem* page = stack.Push(layoutPath);
        if (page == nullptr)
            return nullptr;
        std::vector<UiElem*> open{ page };
        UiElem* menu = nullptr;
        while (!open.empty() && menu == nullptr)
        {
            UiElem* e = open.back();
            open.pop_back();
            if (e->HasClass("context-menu"))
                menu = e;
            for (auto& child : e->children)
                if (UiElem::IsElem(child.get()))
                    open.push_back(static_cast<UiElem*>(child.get()));
        }
        if (menu == nullptr)
            return page;
        const glm::vec2 size = EngineMain::Viewport.GetSize();
        const glm::vec2 at = glm::clamp(g_s.ContextPos, glm::vec2(0.f), glm::max(size - glm::vec2(8.f), glm::vec2(0.f)));
        char style[96];
        std::snprintf(style, sizeof(style), "x: %.0fpx; y: %.0fpx", at.x, at.y);
        menu->SetAttribute("style", style);
        return page;
    }

    void Reset()
    {
        g_s = State();
        g_ghost.reset();
    }

    void RecordGhost()
    {
        const std::shared_ptr<UiElem> source = g_s.Source.lock();
        if (!g_s.Active || !source)
        {
            if (g_ghost)
                g_ghost->m_frame.reset();
            return;
        }
        if (!g_ghost)
        {
            g_ghost = std::make_shared<UiElemPanel>();
            g_ghost->TagName = "Panel";
        }
        UiSystem::RecordSubtree(*source, *g_ghost, 0.7f, GhostOffset());
    }

    void DrawGhost()
    {
        if (g_s.Active && g_ghost)
            g_ghost->Draw();
    }
}

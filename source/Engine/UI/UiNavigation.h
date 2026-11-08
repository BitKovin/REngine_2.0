#pragma once

#include "UiElement.h"
#include "../Input.h"

#include <vector>
#include <cfloat>

#include <imgui/imgui.h>

#include <EngineMain.h>

// ---------------------------------------------------------------------------
// UiNavigation
//
// SETUP
//   Call UiNavigation::Update(root) once per frame AFTER the UI tree Update()
//   so worldMatrix / topLeft / bottomRight are valid.
//
// INPUT ACTIONS
//   ui_up  ui_down  ui_left  ui_right  ui_confirm  ui_cancel
//
// HOVER → FOCUS
//   Any focusable element (HitCheck=true, visible=true) that has active
//   TouchEvents immediately becomes focused. DisableFocus is ignored here —
//   it only restricts keyboard/gamepad navigation.
//
// FOCUS TRAP
//   The last visible FocusTrap in a top-down tree search has priority.
//
// SPATIAL NAVIGATION
//   Neighbour candidates are scored by their world-space centres, computed via
//   element.worldMatrix so the layout correctly handles rotated subtrees.
// ---------------------------------------------------------------------------

class UiNavigation
{
public:
    static inline UiElement* Focused = nullptr;

    static inline bool MouseNavigation = false;

    // ── Focus control ─────────────────────────────────────────────────────────
    static void SetFocus(UiElement* element)
    {
        if (element && (!element->IsVisible() || !element->HitCheck || element->DisableFocus))
            element = nullptr; // refuse to focus anything invalid, regardless of caller

        if (Focused == element) return;

        if (Focused)
        {
            Focused->IsFocused = false;
            Focused->OnUnfocused();
        }

        Focused = element;

        if (Focused)
        {
            s_lastFocused = Focused;
            Focused->IsFocused = true;
            Focused->OnFocused();
        }
    }

    static void ClearFocus() { SetFocus(nullptr); }

    static void FocusFirst(UiElement* root)
    {
        s_root = root;
        std::vector<UiElement*> candidates;
        Collect(candidates);
        if (!candidates.empty()) SetFocus(candidates[0]);
    }

    // ── Per-frame update ──────────────────────────────────────────────────────
    static void Update(UiElement* root)
    {
        s_root = root;

        auto& io = ImGui::GetIO();
        if ((io.WantCaptureKeyboard || io.WantCaptureMouse) && EngineMain::MainInstance->DebugUiEnabled) 
            return;

        if (Focused && (!Focused->IsVisible() || !Focused->HitCheck || Focused->DisableFocus))
            SetFocus(nullptr);

        if (Input::IsScrenTouched || length(Input::MouseDelta) > 0)
        {
            // the last input decides how focus is shown: the pointer moved, so only what is under it has focus
            MouseNavigation = true;
            UpdateHoverFocus();
        }

        // pages and scripts that set a focus while the pointer is in charge leave nothing focused
        if (MouseNavigation && Focused && Focused != s_hoverFocus && !Focused->HoldsFocus())
            ClearFocus();

        if (!Focused && !MouseNavigation)
            FocusFirst(root);

        if (Input::GetAction("ui_up")->Pressed())    Navigate(UiNavDir::Up);
        if (Input::GetAction("ui_down")->Pressed())  Navigate(UiNavDir::Down);
        if (Input::GetAction("ui_left")->Pressed())  Navigate(UiNavDir::Left);
        if (Input::GetAction("ui_right")->Pressed()) Navigate(UiNavDir::Right);

        if (Input::GetAction("ui_confirm")->Pressed() && Focused)
            Focused->OnNavConfirm();

        if (Input::GetAction("ui_cancel")->Pressed())
        {
            UiElement* trap = GetActiveTrap(root);
            if (trap) trap->OnNavCancel();
        }
    }

    static void LateUpdate()
    {
        if (Focused && (!Focused->IsVisible() || !Focused->HitCheck || Focused->DisableFocus))
            SetFocus(nullptr);
    }

private:
    static inline UiElement* s_root = nullptr;
    static inline UiElement* s_hoverFocus = nullptr;
    static inline UiElement* s_lastFocused = nullptr;

    // ── Hover → focus ─────────────────────────────────────────────────────────
    // DisableFocus is intentionally ignored — it only gates navigation controls,
    // not pointer-driven focus transfer.
    static void UpdateHoverFocus()
    {
        if (Focused && Focused->HoldsFocus())
        {
            s_hoverFocus = Focused;   // a text field being edited keeps the focus until it ends the edit itself
            return;
        }
        std::vector<UiElement*> candidates;
        CollectHover(candidates);

        for (UiElement* el : candidates)
        {
            if (!el->TouchEvents.empty() && el->DisableFocus == false)
            {
                SetFocus(el);
                s_hoverFocus = el;
                return;
            }
        }

        // the pointer is over nothing focusable: nothing is focused
        s_hoverFocus = nullptr;
        ClearFocus();
    }

    // ── Navigation ────────────────────────────────────────────────────────────
    static void Navigate(UiNavDir dir)
    {
        if (!Focused)
        {
            // directional input takes over from the pointer: bring the focus back where it was, or to the first element
            MouseNavigation = false;
            std::vector<UiElement*> candidates;
            Collect(candidates);
            if (std::find(candidates.begin(), candidates.end(), s_lastFocused) != candidates.end())
                SetFocus(s_lastFocused);
            else if (!candidates.empty())
                SetFocus(candidates[0]);
            return;
        }

        MouseNavigation = false;

        if (Focused->OnNav(dir)) return;

        if (UiElement* manual = GetManualOverride(Focused, dir))
        {
            SetFocus(manual);
            return;
        }

        UiElement* neighbour = FindNeighbour(Focused, dir);
        if (neighbour) SetFocus(neighbour);
    }

    // ── Navigation candidate collection (respects DisableFocus) ──────────────
    static void Collect(std::vector<UiElement*>& out)
    {
        if (!s_root) return;
        UiElement* trap = GetActiveTrap(s_root);
        if (trap) CollectInTrap(trap, out);
        else      CollectGlobal(s_root, out);
    }

    static void CollectGlobal(UiElement* node, std::vector<UiElement*>& out)
    {
        for (auto& child : node->children)
        {
            if (!child->IsVisible())  continue;
            if (child->DisableFocus)  continue;
            if (child->FocusTrap)     continue;
            if (child->HitCheck)      out.push_back(child.get());
            CollectGlobal(child.get(), out);
        }
    }

    static void CollectInTrap(UiElement* trap, std::vector<UiElement*>& out)
    {
        for (auto& child : trap->children)
        {
            if (!child->IsVisible())  continue;
            if (child->DisableFocus)  continue;
            if (child->HitCheck)      out.push_back(child.get());
            if (!child->FocusTrap)    CollectInTrap(child.get(), out);
        }
    }

    // ── Hover candidate collection (ignores DisableFocus) ────────────────────
    static void CollectHover(std::vector<UiElement*>& out)
    {
        if (!s_root) return;
        UiElement* trap = GetActiveTrap(s_root);
        if (trap) CollectHoverInTrap(trap, out);
        else      CollectHoverGlobal(s_root, out);
    }

    static void CollectHoverGlobal(UiElement* node, std::vector<UiElement*>& out)
    {
        for (auto& child : node->children)
        {
            if (!child->IsVisible()) continue;
            if (child->FocusTrap)    continue;
            if (child->HitCheck)     out.push_back(child.get());
            CollectHoverGlobal(child.get(), out);
        }
    }

    static void CollectHoverInTrap(UiElement* trap, std::vector<UiElement*>& out)
    {
        for (auto& child : trap->children)
        {
            if (!child->IsVisible()) continue;
            if (child->HitCheck)     out.push_back(child.get());
            if (!child->FocusTrap)   CollectHoverInTrap(child.get(), out);
        }
    }

    // ── Spatial neighbour ─────────────────────────────────────────────────────
    static UiElement* FindNeighbour(UiElement* from, UiNavDir dir)
    {
        std::vector<UiElement*> candidates;
        Collect(candidates);
        candidates.erase(
            std::remove(candidates.begin(), candidates.end(), from),
            candidates.end());

        if (candidates.empty()) return nullptr;

        // Axis-aligned boxes: a candidate is in the direction when its centre is; the gap between the facing edges is the
        // primary cost, and the perpendicular distance between the two extents (zero when they overlap) the secondary one, so a
        // wide control straight below is preferred over a narrow one that is merely closer to the centre.
        const Box fb = BoxOf(from);
        const vec2 fc((fb.x0 + fb.x1) * 0.5f, (fb.y0 + fb.y1) * 0.5f);
        UiElement* best = nullptr;
        float      bestScore = FLT_MAX;

        for (UiElement* c : candidates)
        {
            const Box cb = BoxOf(c);
            const float dx = (cb.x0 + cb.x1) * 0.5f - fc.x;
            const float dy = (cb.y0 + cb.y1) * 0.5f - fc.y;

            bool  valid = false;
            float gap = 0.f, offset = 0.f, centreOffset = 0.f;

            switch (dir)
            {
            case UiNavDir::Up:    valid = dy < 0.f; gap = fb.y0 - cb.y1; offset = Separation(fb.x0, fb.x1, cb.x0, cb.x1); centreOffset = std::abs(dx); break;
            case UiNavDir::Down:  valid = dy > 0.f; gap = cb.y0 - fb.y1; offset = Separation(fb.x0, fb.x1, cb.x0, cb.x1); centreOffset = std::abs(dx); break;
            case UiNavDir::Left:  valid = dx < 0.f; gap = fb.x0 - cb.x1; offset = Separation(fb.y0, fb.y1, cb.y0, cb.y1); centreOffset = std::abs(dy); break;
            case UiNavDir::Right: valid = dx > 0.f; gap = cb.x0 - fb.x1; offset = Separation(fb.y0, fb.y1, cb.y0, cb.y1); centreOffset = std::abs(dy); break;
            }

            if (!valid) continue;

            const float score = std::max(gap, 0.f) + offset * 3.f + centreOffset * 0.02f;
            if (score < bestScore) { bestScore = score; best = c; }
        }

        return best;
    }

    // ── Active trap ───────────────────────────────────────────────────────────
    static UiElement* GetActiveTrap(UiElement* root)
    {
        UiElement* last = nullptr;
        FindLastTrap(root, last);
        return last;
    }

    static void FindLastTrap(UiElement* node, UiElement*& last)
    {
        for (auto& child : node->children)
        {
            if (!child->visible) continue;
            if (child->FocusTrap) last = child.get();
            FindLastTrap(child.get(), last);
        }
    }

    // ── Utilities ─────────────────────────────────────────────────────────────
    static UiElement* GetManualOverride(UiElement* el, UiNavDir dir)
    {
        std::weak_ptr<UiElement>* field = nullptr;
        switch (dir)
        {
        case UiNavDir::Up:    field = &el->NavUp;    break;
        case UiNavDir::Down:  field = &el->NavDown;  break;
        case UiNavDir::Left:  field = &el->NavLeft;  break;
        case UiNavDir::Right: field = &el->NavRight; break;
        }
        if (!field) return nullptr;
        auto locked = field->lock();
        return (locked && locked->visible && locked->HitCheck && !locked->DisableFocus)
            ? locked.get() : nullptr;
    }

    struct Box { float x0, y0, x1, y1; };

    static Box BoxOf(UiElement* el)
    {
        const vec2 s = el->GetSize();
        const vec2 p[4] = { UiElement::TransformPoint(el->worldMatrix, vec2(0.f, 0.f)), UiElement::TransformPoint(el->worldMatrix, vec2(s.x, 0.f)),
                            UiElement::TransformPoint(el->worldMatrix, vec2(0.f, s.y)), UiElement::TransformPoint(el->worldMatrix, s) };
        Box b{ p[0].x, p[0].y, p[0].x, p[0].y };
        for (const vec2& v : p)
        {
            b.x0 = std::min(b.x0, v.x); b.x1 = std::max(b.x1, v.x);
            b.y0 = std::min(b.y0, v.y); b.y1 = std::max(b.y1, v.y);
        }
        return b;
    }

    // Distance between two intervals; zero when they overlap.
    static float Separation(float a0, float a1, float b0, float b1)
    {
        return std::max(0.f, std::max(a0 - b1, b0 - a1));
    }

    // Returns the world-space centre of an element using its worldMatrix.
    // This is accurate even when the element (or any of its ancestors) is rotated.
    static vec2 Center(UiElement* el)
    {
        return UiElement::TransformPoint(el->worldMatrix, el->GetSize() * 0.5f);
    }
};
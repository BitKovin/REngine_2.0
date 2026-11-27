#include "UiTooltips.h"

#include "UiHost.h"

#include <EngineMain.h>
#include <Input.h>
#include <UI/Markup/UiTagRegistry.h>
#include <UI/UiNavigation.h>
#include <UI/UiSystem.h>

#include <algorithm>

namespace
{
    constexpr float kHoverDelay = 0.5f, kFocusDelay = 0.8f;

    std::weak_ptr<UiElem> g_target;
    bool g_fromFocus = false;
    float g_timer = 0.f;
    std::shared_ptr<UiElem> g_tip;
    bool g_tipPositioned = false;

    UiElem* ElemOf(UiElement* e)
    {
        while (e != nullptr && !UiElem::IsElem(e))
            e = e->parent;
        return static_cast<UiElem*>(e);
    }

    UiElem* WithTooltip(UiElem* e)
    {
        for (; e != nullptr; e = e->AsParentElem())
            if (e->Attributes.count("tooltip") != 0 || e->Attributes.count("tooltip-layout") != 0)
                return e;
        return nullptr;
    }

    void Remove()
    {
        if (g_tip && g_tip->parent != nullptr)
            g_tip->parent->RemoveChild(g_tip);
        g_tip.reset();
        g_tipPositioned = false;
    }

    void Build(UiElem& target)
    {
        UiElemPanel& overlay = UiHost::Overlay();
        const auto layout = target.Attributes.find("tooltip-layout");
        std::shared_ptr<UiElem> tip;
        if (layout != target.Attributes.end())
        {
            tip = UiElem::Load(layout->second);
            if (tip)
                UiSystem::UnregisterRoot(tip);
        }
        else
        {
            tip = std::make_shared<UiElemPanel>();
            if (UiTagRegistry::Factory label = UiTagRegistry::FindBuiltin("Label"))
            {
                std::shared_ptr<UiElem> text = label();
                text->SetAttribute("text", target.Attributes["tooltip"]);
                tip->AddChild(text);
            }
        }
        if (!tip)
            return;
        tip->AddClass("tooltip");
        {
            // out of sight until it has a size and a place (a visibility override equal to the default is not applied by the resolver)
            UiTransformFn away;
            away.Type = UiTransformFnType::Translate;
            away.A.Kind = UiLenKind::Px; away.A.Px = -20000.f;
            away.B.Kind = UiLenKind::Px; away.B.Px = -20000.f;
            tip->SetTransformOverride({ away });
        }
        std::vector<UiElem*> all;
        tip->CollectElems(all);
        for (UiElem* e : all)
            e->HitCheck = false;   // the pointer never lands on the tooltip
        overlay.AddChild(tip);
        g_tip = tip;
        g_tipPositioned = false;
    }

    void Place(UiElem& tip, glm::vec2 anchor, bool belowAnchor)
    {
        if (tip.BorderRect.W <= 0.f || tip.BorderRect.H <= 0.f)
            return;   // not laid out yet
        const glm::vec2 viewport = EngineMain::Viewport.GetSize();
        const glm::vec2 size(tip.BorderRect.W, tip.BorderRect.H);
        glm::vec2 topLeft = belowAnchor ? anchor + glm::vec2(0.f, 8.f) : anchor + glm::vec2(16.f, 22.f);
        topLeft.x = std::clamp(topLeft.x, 0.f, std::max(0.f, viewport.x - size.x));
        topLeft.y = std::clamp(topLeft.y, 0.f, std::max(0.f, viewport.y - size.y));
        // the layout position of the tooltip inside the overlay; the translate moves it from there
        const UiElem* parent = tip.AsParentElem();
        const glm::mat3 parentAbs = parent != nullptr ? parent->AbsoluteTransform : glm::mat3(1.f);
        const glm::vec2 layoutTopLeft = glm::vec2(parentAbs * glm::vec3(tip.BorderRect.X, tip.BorderRect.Y, 1.f));
        const glm::vec2 shift = topLeft - layoutTopLeft;
        UiTransformFn move;
        move.Type = UiTransformFnType::Translate;
        move.A.Kind = UiLenKind::Px; move.A.Px = shift.x;
        move.B.Kind = UiLenKind::Px; move.B.Px = shift.y;
        tip.SetTransformOverride({ move });
        if (!g_tipPositioned)
        {
            g_tipPositioned = true;
            tip.MarkPaintDirty();
        }
    }
}

void UiTooltips::Update(float realDt)
{
    // who is the tooltip about: what is under the pointer while the pointer is in charge, else the focused element
    UiElem* hoverElem = nullptr;
    glm::vec2 pointer(0.f);
    // the pointer is in charge once it moved, until a navigation key or button is used
    static glm::vec2 s_lastPointer(-1.f);
    static bool s_pointerActive = false;
    if (Input::TouchActions.count(1) != 0)
    {
        const glm::vec2 now = Input::TouchActions[1].position;
        if (now != s_lastPointer)
        {
            s_lastPointer = now;
            s_pointerActive = true;
        }
    }
    for (const char* action : { "ui_up", "ui_down", "ui_left", "ui_right", "ui_confirm" })
        if (Input::GetAction(action)->Pressed())
            s_pointerActive = false;
    const bool pointerMode = (s_pointerActive || UiNavigation::MouseNavigation) && Input::TouchActions.count(1) != 0;
    if (pointerMode)
    {
        pointer = Input::TouchActions[1].position;
        const std::shared_ptr<UiElement> hit = EngineMain::Viewport.GetHitElementUnderPosition(pointer);
        hoverElem = WithTooltip(ElemOf(hit.get()));
    }
    else
        hoverElem = WithTooltip(ElemOf(UiNavigation::Focused));

    const std::shared_ptr<UiElem> current = g_target.lock();
    if (hoverElem == nullptr || hoverElem != current.get() || pointerMode == g_fromFocus)
    {
        Remove();
        g_timer = 0.f;
        g_fromFocus = !pointerMode;
        g_target.reset();
        if (hoverElem != nullptr)
            if (auto self = hoverElem->weak_from_this().lock())
                g_target = std::static_pointer_cast<UiElem>(self);
        return;
    }

    g_timer += realDt;
    if (!g_tip && g_timer >= (g_fromFocus ? kFocusDelay : kHoverDelay))
        Build(*current);
    if (g_tip)
    {
        if (g_fromFocus)
        {
            const UiRect r = current->ContentRectScreen();
            Place(*g_tip, glm::vec2(r.X, r.Y + r.H), true);
        }
        else
            Place(*g_tip, pointer, false);
    }
}

void UiTooltips::Reset()
{
    g_tip.reset();   // the overlay was cleared with the viewport
    g_tipPositioned = false;
    g_target.reset();
    g_timer = 0.f;
}

bool UiTooltips::IsShowing() { return g_tip != nullptr; }

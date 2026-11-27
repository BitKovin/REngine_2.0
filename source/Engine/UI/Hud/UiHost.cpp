#include "UiHost.h"

#include <EngineMain.h>
#include <UI/Menus/UiMenuStack.h>
#include <UI/UiSystem.h>

#include <algorithm>

namespace
{
    std::shared_ptr<UiElemPanel> g_hud;
    std::shared_ptr<UiElemPanel> g_overlay;

    std::shared_ptr<UiElemPanel> MakeLayer(const char* tag, const char* id)
    {
        auto layer = std::make_shared<UiElemPanel>();
        layer->TagName = tag;
        layer->Id = id;
        layer->HitCheck = false;   // the layer itself is never hit; its pages are
        layer->Style().Width = UiLength{ UiLenKind::Percent, 0.f, 100.f };
        layer->Style().Height = UiLength{ UiLenKind::Percent, 0.f, 100.f };
        return layer;
    }

    // The viewport's child list is the truth: ClearChildren() leaves the old `parent` pointers behind.
    bool IsChildOfViewport(const UiElement& e)
    {
        for (const auto& child : EngineMain::Viewport.children)
            if (child.get() == &e)
                return true;
        return false;
    }

    void AttachLayer(UiElement& layer)
    {
        if (IsChildOfViewport(layer))
            return;
        try { EngineMain::Viewport.AddChild(std::static_pointer_cast<UiElement>(layer.shared_from_this())); } catch (...) {}
    }

    UiElem* AddTo(UiElemPanel& layer, const std::string& layoutPath)
    {
        std::shared_ptr<UiElem> page = UiElem::Load(layoutPath);
        if (!page)
            return nullptr;
        UiSystem::UnregisterRoot(page);   // a child of the layer: the layer's style (hud-hidden) covers it, the layer paints it
        layer.AddChild(page);
        return page.get();
    }
}

void UiHost::Init()
{
    if (g_hud)
        return;
    g_hud = MakeLayer("HudLayer", "hud");
    g_overlay = MakeLayer("OverlayLayer", "overlay");
    g_overlay->useLateDraw = true;   // draws after everything, also the legacy elements that are added later
    EngineMain::Viewport.AddChild(g_hud);
    UiSystem::RegisterRoot(g_hud);
    UiMenus::Init();                 // the menu stack goes in between
    EngineMain::Viewport.AddChild(g_overlay);
    UiSystem::RegisterRoot(g_overlay);
}

void UiHost::Shutdown()
{
    for (auto* layer : { &g_hud, &g_overlay })
        if (*layer)
        {
            (*layer)->ClearChildren();
            if ((*layer)->parent != nullptr)
                (*layer)->parent->RemoveChild(*layer);
            layer->reset();
        }
}

void UiHost::Update()
{
    if (!g_hud)
        return;
    g_hud->SetClass("hud-hidden", UiMenus::Stack().HidesHud());
}

void UiHost::Attach()
{
    if (!g_hud)
        return;
    AttachLayer(*g_hud);
    UiMenuStack& stack = UiMenus::Stack();
    if (!IsChildOfViewport(stack))
        try { EngineMain::Viewport.AddChild(std::static_pointer_cast<UiElement>(stack.shared_from_this())); } catch (...) {}
    AttachLayer(*g_overlay);
}

void UiHost::Reset()
{
    if (!g_hud)
        return;
    g_hud->ClearChildren();
    g_overlay->ClearChildren();
    g_hud->SetClass("hud-hidden", false);
}

UiElemPanel& UiHost::Hud()
{
    if (!g_hud)
        Init();
    return *g_hud;
}

UiElemPanel& UiHost::Overlay()
{
    if (!g_overlay)
        Init();
    return *g_overlay;
}

UiElem* UiHost::AddHud(const std::string& layoutPath) { return AddTo(Hud(), layoutPath); }
UiElem* UiHost::AddOverlay(const std::string& layoutPath) { return AddTo(Overlay(), layoutPath); }

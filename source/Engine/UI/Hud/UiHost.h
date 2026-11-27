#pragma once
#include <UI/Elements/UiElemPanel.h>

#include <memory>
#include <string>

// The three full-viewport layers of the main screen, in draw order: HUD, menus (UiMenus::Stack()), overlay.
// (Named UiHost because UiLayers is the offscreen layer code of phase 7.)
// Each layer is a Panel root that holds any element. HUD pages go into Hud() as children (LoadNested), so the layer's own style
// (hud-hidden, opacity, transitions) applies to them as one picture.
namespace UiHost
{
    void Init();                              // from UiSystem::Init: creates Hud and Overlay, attaches Hud, Menus, Overlay to the viewport in that order
    void Shutdown();
    void Update();                            // from UiSystem::Update: hides the HUD while a page with hides-hud is on the menu stack
    void Reset();                             // level change: the HUD and overlay contents are dropped silently, the layers are attached again
    void Attach();                            // puts the three layers under the viewport again when they are not (ResetViewport)

    UiElemPanel& Hud();
    UiElemPanel& Overlay();

    // Loads a layout as a HUD page (a child of the HUD layer, not a root of its own). Returns nullptr on error.
    UiElem* AddHud(const std::string& layoutPath);
    UiElem* AddOverlay(const std::string& layoutPath);
}

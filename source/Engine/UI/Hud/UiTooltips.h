#pragma once

// Tooltips (phase 5). Any element can carry   tooltip="${KEY}"   (a localised text) or   tooltip-layout="path.xml"   (any layout).
// After 0.5 s of pointer hover, or 0.8 s of gamepad / keyboard focus, the tooltip is built in the overlay layer (class `tooltip`),
// below-right of the pointer (below the element for focus) and kept inside the viewport. It is removed when hover or focus leaves.
// One tooltip at a time. Real time, so it also works while the game is paused.
namespace UiTooltips
{
    void Update(float realDt);   // from UiSystem::Update, after UiHost::Update
    void Reset();                // level change: dropped silently
    bool IsShowing();
}

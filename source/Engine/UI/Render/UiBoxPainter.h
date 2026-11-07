#pragma once
#include <UI/Render/UiPainter.h>
#include <UI/Style/UiFxStyle.h>

class UiElem;

// Paints what the phase 3 style properties add to an element's own box: shadows, background layers, border image, outline,
// and the effects (filter, mask, clip-path, blend mode, image rendering) that apply to the element and everything below it.
namespace UiBoxPainter
{
    struct Scope
    {
        UiPainterFx Saved;
        bool ClipPushed = false;
    };

    // Starts the element's effects on top of the painter's current ones. The painter's transform must already be the element's.
    // leaveBlendToLayer: the element is a layer, and mix-blend-mode applies to its composite quad instead of to what is inside.
    Scope Begin(UiElem& e, UiPainter& p, bool leaveBlendToLayer = false);
    void End(UiPainter& p, const Scope& scope);

    // Outer shadows, background colour and border, background layers, border image, inset shadows. `opacity` is the effective opacity.
    void PaintBox(UiElem& e, UiPainter& p, float opacity, const UiCorners& radii);
    // backdrop-filter: the blurred scene behind the box (before its background), with the colour functions of the property applied.
    void PaintBackdrop(UiElem& e, UiPainter& p, float opacity, const UiCorners& radii);
    // The outline, drawn after the element's content and children.
    void PaintOutline(UiElem& e, UiPainter& p, float opacity, const UiCorners& radii);

    // 4 x 5 colour matrix (row major) of a style's filters and wash-color; false when it is the identity.
    bool FilterMatrix(const UiFxStyle& fx, float out[20]);
}

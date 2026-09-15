// Explicit, hand-maintained registration for every engine-provided UI
// element type. Called once from EngineMain::Init() -- see the big comment
// in UiElementRegistry.cpp for why engine built-ins can't rely on
// UI_REGISTER_ELEMENT self-registration the way game-code widgets can.
//
// When you add a new default-constructible widget class to the engine,
// add one Register<T>(...) call for it here. Nothing else needs to change.

#include "UiElementRegistry.h"
#include "PropertyReflection.h"

#include "../UiElement.h"
#include "../UiCanvas.hpp"
#include "../UiContentBox.hpp"
#include "../UiVerticalBox.hpp"
#include "../UiHorizontalBox.hpp"
#include "../UiButton.hpp"
#include "../UiText.hpp"
#include "../UiImage.hpp"

void UiElementRegistry::RegisterBuiltinTypes()
{
    UiElementRegistry& registry = UiElementRegistry::Instance();

    // ── UiElement itself: the root of every property chain ─────────────────
    // Registered (it's concretely constructible) purely so every other
    // type's GetPropertyChain("...") walk actually finds these base
    // properties at the root. Not intended to be a common pick in the
    // editor's "add child" list -- it draws nothing on its own.
    registry.Register<UiElement>("UiElement", "",
    {
        Prop(&UiElement::position, "Position", PropertyKind::Vec2),
        Prop(&UiElement::size,     "Size",     PropertyKind::Vec2),
        Prop(&UiElement::origin,   "Origin",   PropertyKind::Vec2),
        Prop(&UiElement::pivot,    "Pivot",    PropertyKind::Vec2),
        Prop(&UiElement::rotation, "Rotation", PropertyKind::Float),

        Prop(&UiElement::color,               "Color",              PropertyKind::Color),
        Prop(&UiElement::inheritParentColor,  "InheritParentColor", PropertyKind::Bool),
        Prop(&UiElement::visible,             "Visible",            PropertyKind::Bool),

        Prop(&UiElement::HitCheck,            "HitCheck",           PropertyKind::Bool),
        Prop(&UiElement::DisableFocus,        "DisableFocus",       PropertyKind::Bool),
        Prop(&UiElement::LimitHitTestToBounds,"LimitHitTestToBounds", PropertyKind::Bool),
        Prop(&UiElement::FocusTrap,           "FocusTrap",          PropertyKind::Bool),

        Prop(&UiElement::RectPosition,        "RectPosition",       PropertyKind::Vec2),
        Prop(&UiElement::RectSize,            "RectSize",           PropertyKind::Vec2),
        Prop(&UiElement::NineSliceEnabled,    "NineSliceEnabled",   PropertyKind::Bool),

        Prop(&UiElement::shadowEnabled,  "ShadowEnabled",  PropertyKind::Bool),
        Prop(&UiElement::shadowColor,    "ShadowColor",    PropertyKind::Color),
        Prop(&UiElement::shadowOffset,   "ShadowOffset",   PropertyKind::Vec2),
        Prop(&UiElement::shadowSoftness, "ShadowSoftness", PropertyKind::Float),
        Prop(&UiElement::shadowSpread,   "ShadowSpread",   PropertyKind::Float),

        Prop(&UiElement::outlineEnabled, "OutlineEnabled", PropertyKind::Bool),
        Prop(&UiElement::outlineColor,   "OutlineColor",   PropertyKind::Color),
        Prop(&UiElement::outlineWidth,   "OutlineWidth",   PropertyKind::Float),

        Prop(&UiElement::glowEnabled,    "GlowEnabled",    PropertyKind::Bool),
        Prop(&UiElement::glowColor,      "GlowColor",      PropertyKind::Color),
        Prop(&UiElement::glowRadius,     "GlowRadius",     PropertyKind::Float),
        Prop(&UiElement::glowIntensity,  "GlowIntensity",  PropertyKind::Float),
    });

    // ── Containers ───────────────────────────────────────────────────────
    registry.Register<UiCanvas>("UiCanvas", "UiElement",
    {
        Prop(&UiCanvas::ScaleToParent, "ScaleToParent", PropertyKind::Bool),
    });

    registry.Register<UiContentBox>("UiContentBox", "UiElement", {}); // no own properties, just a base

    registry.Register<UiVerticalBox>("UiVerticalBox", "UiContentBox",
    {
        Prop(&UiVerticalBox::ContentDistance, "ContentDistance", PropertyKind::Float),
    });

    registry.Register<UiHorizontalBox>("UiHorizontalBox", "UiContentBox",
    {
        Prop(&UiHorizontalBox::ContentDistance, "ContentDistance", PropertyKind::Float),
    });

    // ── Leaf widgets ─────────────────────────────────────────────────────
    registry.Register<UiButton>("UiButton", "UiElement",
    {
        Prop(&UiButton::ImagePath,     "ImagePath",     PropertyKind::AssetPath),
        Prop(&UiButton::Color,         "Color",         PropertyKind::Color),
        Prop(&UiButton::HoverColor,    "HoverColor",    PropertyKind::Color),
        Prop(&UiButton::OnlyTouch,     "OnlyTouch",     PropertyKind::Bool),
        Prop(&UiButton::OnlyNotPaused, "OnlyNotPaused", PropertyKind::Bool),
        // onClick / onNavConfirm are std::function -- intentionally not
        // registered; wire those up in C++ after FindNamed<UiButton>(...).
    });

    registry.Register<UiText>("UiText", "UiElement",
    {
        // `text` follows the engine's existing "${LOC_KEY}" convention
        // (Localisation::LocalizeString runs on it at draw time) as well as
        // plain literal text -- LocKey just hints the inspector to that.
        Prop(&UiText::text,      "Text",      PropertyKind::LocKey),
        Prop(&UiText::fontSize,  "FontSize",  PropertyKind::Float),
        Prop(&UiText::textColor, "TextColor", PropertyKind::Color),
    });

    registry.Register<UiImage>("UiImage", "UiElement",
    {
        Prop(&UiImage::ImagePath, "ImagePath", PropertyKind::AssetPath),
    });
}

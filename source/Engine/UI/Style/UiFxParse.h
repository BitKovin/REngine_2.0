#pragma once
#include <UI/Style/UiComputedStyle.h>

// Parsing of the phase 3 painting and motion properties into UiComputedStyle::Fx.

// Parsers of UiComputedStyle.cpp that the effect properties reuse.
bool UiParseColorComponent(const std::vector<UiCssToken>& comp, const UiStyleParseContext& ctx, glm::vec4& out, std::string& err);
bool UiParseLengthComponent(const std::vector<UiCssToken>& comp, const UiStyleParseContext& ctx, UiLength& out, std::string& err, bool allowPercent);

// Property table entries (published into the UI database).
const std::vector<UiPropertyInfo>& UiFxProperties();

// Applies one phase 3 property or shorthand. `handled` is false when `name` is not one of them. On failure `error` is set and the style is unchanged.
bool UiFxApplyProperty(UiComputedStyle& s, const std::string& name, const std::vector<UiCssToken>& value, const UiStyleParseContext& ctx,
                       std::string& error, bool& handled);

// Sounds of a style rule. sound-enter plays when the rule starts to match the element (a pseudo-class or class turns on, the way
// an animation starts), sound-exit when it stops matching; sound-volume of the same rule scales both. They are not inherited and
// not cascaded: every rule that declares them plays its own, so the sound belongs to the state (Button:hover, Button:click).
struct UiRuleSounds
{
    std::string Enter, Exit;
    float Volume = 1.f;
};
bool UiFxIsRuleSoundProperty(const std::string& name);
// Reads sound-enter, sound-exit or sound-volume into `into`; false with `error` when the value is not valid.
bool UiFxParseRuleSound(const std::string& property, const std::vector<UiCssToken>& value, UiRuleSounds& into, std::string& error);

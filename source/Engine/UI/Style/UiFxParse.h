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

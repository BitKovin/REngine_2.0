#pragma once
#include <string>

class UiElem;
struct UiComputedStyle;
struct UiCssDeclaration;
struct UiStyleParseContext;

namespace UiStyleResolver
{
    // Resolves the computed style of every element of a root (cascade, inheritance, var()).
    // The caller marks the root layout- and paint-dirty afterwards.
    void ResolveRoot(UiElem& root);
    // Forget which style warnings were already logged (hot reload).
    void ResetWarnings();
    // Applies one declaration (var(), shorthand expansion and value parsing) to `style`; used for keyframes. False + error when invalid.
    bool ApplyDeclaration(UiComputedStyle& style, const UiCssDeclaration& decl, const UiStyleParseContext& ctx, std::string& error);
}

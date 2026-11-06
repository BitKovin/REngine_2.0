#pragma once
#include <string>

class UiElem;

namespace UiStyleResolver
{
    // Resolves the computed style of every element of a root (cascade, inheritance, var()).
    // The caller marks the root layout- and paint-dirty afterwards.
    void ResolveRoot(UiElem& root);
    // Forget which style warnings were already logged (hot reload).
    void ResetWarnings();
}

#pragma once
#include <glm.h>

class UiElem;

namespace UiFlowLayout
{
    // Lays out a root inside its container (size in UI units) and sets every element's BorderRect,
    // AbsoluteTransform (from `base`), and the legacy fields (by calling UpdateOffsets()).
    void LayoutRoot(UiElem& root, glm::vec2 containerSize, const glm::mat3& base);
}

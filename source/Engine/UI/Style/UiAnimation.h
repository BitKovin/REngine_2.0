#pragma once
#include <UI/Style/UiComputedStyle.h>
#include <vector>

class UiElem;

// Transitions and @keyframes animations. Real time: it keeps running while the game is paused.
// The element's m_computed holds the DISPLAYED values; the last resolved CSS values are kept as the target, and the running
// transitions and animations overwrite the animated properties of every freshly resolved style (the "overlay").
struct UiAnimState;
struct UiAnimStateDeleter { void operator()(UiAnimState* state) const; };

// The matrix of a transform function list (translate percentages resolve against the box size bw x bh).
glm::mat3 UiTransformListMatrix(const std::vector<UiTransformFn>& list, float bw, float bh);

namespace UiAnimation
{
    // Called by the style resolver for every element with the fresh target style, before it becomes the displayed one:
    // starts transitions (compared with the displayed values) and animations, then writes their current values into `target`.
    void OnStyleResolved(UiElem& e, UiComputedStyle& target);

    // Advances every running transition and animation by `seconds` and writes the values into the elements' displayed styles.
    // Marks the roots dirty (paint, layout or transform patch) as needed.
    void Tick(float seconds);

    // True while a transition or animation is running on the element or any element below it (a page that is leaving waits for this).
    bool IsAnimating(const UiElem& e);

    // reduced motion: every transition duration is 0 and infinite animations do not run
    bool ReducedMotion();
    void SetReducedMotion(bool reduced);

    // The motion clock can be paused and scrubbed: a deterministic timeline for previews and tests.
    void SetPaused(bool paused);
    bool Paused();
    // Moves every transition and animation at or below `root` to `seconds` since it started and writes the values.
    void Seek(UiElem& root, float seconds);

    // For the UI database: animatable property names, easing function names.
    std::vector<const char*> AnimatableProperties();
    std::vector<const char*> EasingFunctions();
    // Number of elements with running motion (tests).
    int RunningCount();
}

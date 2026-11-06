#include "UiPaintPreviews.h"

#include <UI/Style/UiAnimation.h>

namespace
{
    using nlohmann::json;

    // Every effect runs in the engine's own shaders on every rendering backend; the ones that need offscreen layers are listed as unavailable.
    void Effect(UiPaintPreviews::Registries& r, const std::string& name, const char* category, const char* kind, const char* description,
                const char* parameters, bool available = true, const char* reason = "")
    {
        json j = json::object();
        j["name"] = name;
        j["category"] = category;
        j["kind"] = kind;
        j["description"] = description;
        j["parameters"] = parameters;
        j["available"] = available;
        j["requires"] = available ? json::array() : json::array({ "layers" });
        j["requiresLayer"] = !available;
        if (!available)
            j["unavailableReason"] = reason;
        j["backends"] = json::array({ "dx11", "dx12", "vulkan", "opengl", "gles300" });
        r["paintEffects"][name] = j;
    }

    UiPreviewScenario Scenario(const char* id, const char* label, json values)
    {
        UiPreviewScenario s;
        s.Id = id;
        s.Label = label;
        s.Values = std::move(values);
        return s;
    }
}

void UiPaintPreviews::FillRegistries(Registries& r)
{
    Effect(r, "Arc", "paint", "element", "Ring segment drawn with the arc primitive (progress rings, cooldowns)", "start, sweep, thickness, round-caps");
    Effect(r, "linear-gradient()", "gradient", "function", "Linear colour ramp", "[<angle> | to <side-or-corner>,] <color-stop>#");
    Effect(r, "radial-gradient()", "gradient", "function", "Radial colour ramp", "[circle | ellipse] [<size>] [at <position>], <color-stop>#");
    Effect(r, "conic-gradient()", "gradient", "function", "Colour ramp around a centre, clockwise from 12 o'clock", "[from <angle>] [at <position>], <color-stop>#");
    Effect(r, "repeating-linear-gradient()", "gradient", "function", "Repeating linear gradient", "same as linear-gradient()");
    Effect(r, "repeating-radial-gradient()", "gradient", "function", "Repeating radial gradient", "same as radial-gradient()");
    Effect(r, "repeating-conic-gradient()", "gradient", "function", "Repeating conic gradient", "same as conic-gradient()");
    Effect(r, "background-image", "image", "property", "Image layers (atlas images and gradients), tiled, sized and positioned", "url() | gradient, per layer");
    Effect(r, "border-image", "image", "property", "Nine-slice border from an atlas image (stretch or round)", "<source> <slice> [/ <width>] <repeat>");
    Effect(r, "box-shadow", "shadow", "property", "Outer and inset shadows with blur and spread (own shader program)", "[inset] <x> <y> [blur] [spread] <color>, ...");
    Effect(r, "outline", "shadow", "property", "Outline outside the border box with an offset", "<width> || <style> || <color>; outline-offset");
    Effect(r, "text-shadow", "text", "property", "Shadow behind text, optionally blurred", "<x> <y> [blur] <color>, ...");
    Effect(r, "text-stroke", "text", "property", "Outline around glyphs, drawn under the fill", "<width> <color>");
    for (const char* f : { "brightness()", "contrast()", "saturate()", "grayscale()", "sepia()", "hue-rotate()", "invert()" })
        Effect(r, f, "filter", "function", "Colour matrix filter (applied to the element and everything below it)", "<number | percentage>");
    Effect(r, "wash-color", "filter", "property", "Tint over everything: the colour matrix lerps to the colour by its alpha", "<color>");
    Effect(r, "mask-image", "mask", "property", "Linear or radial gradient mask (alpha), evaluated per pixel", "linear-gradient() | radial-gradient()");
    Effect(r, "clip-path", "clip", "property", "inset(), circle() and ellipse() clip shapes", "inset() | circle() | ellipse()");
    for (const char* m : { "plus-lighter", "multiply", "screen" })
        Effect(r, std::string("mix-blend-mode: ") + m, "blend", "value", "Blend function of the element's primitives with what is behind", m);
    Effect(r, "transform", "transform", "property", "2D transform of the element and its subtree; layout ignores it, hit testing follows it",
           "translate() | rotate() | scale() | skew() list; transform-origin");
    Effect(r, "image-rendering: pixelated", "image", "value", "Nearest-neighbour sampling of images", "auto | pixelated");
    Effect(r, "object-position", "image", "property", "Position of an Image's content inside its box", "<position>");
    Effect(r, "transition", "motion", "property", "Interpolates changed properties over time", "<property> <duration> <timing-function> <delay>");
    Effect(r, "animation", "motion", "property", "Plays @keyframes", "<name> <duration> <timing-function> <delay> <iteration-count> <direction> <fill-mode>");

    const char* layers = "needs an offscreen layer pass (not available until layers are implemented); the engine does not approximate it";
    Effect(r, "blur()", "filter", "function", "Gaussian blur filter", "<length>", false, layers);
    Effect(r, "drop-shadow()", "filter", "function", "Shadow that follows the alpha of the content", "<x> <y> [blur] <color>", false, layers);
    Effect(r, "backdrop-filter", "filter", "property", "Filters what is behind the element", "<filter-list>", false, layers);
    Effect(r, "opacity (group)", "layer", "property", "Opacity of an element as a whole (overlapping children do not show through)", "<number>", false, layers);

    // easing functions and animatable properties
    for (const char* e : UiAnimation::EasingFunctions())
        r["easings"][e] = json{ { "name", e }, { "kind", "easing" } };
    for (const char* p : UiAnimation::AnimatableProperties())
        r["animations"][std::string("property:") + p] = json{ { "name", p }, { "kind", "animatable-property" } };
    json rules = json::object();
    rules["time"] = "real time; also runs while the game is paused";
    rules["reducedMotion"] = "transitions are instant and infinite animations do not run";
    rules["pages"] = "pages in a menu stack start from :entering and stay until the :leaving transition ends (2 s at most)";
    r["animations"]["@rules"] = json{ { "name", "@rules" }, { "kind", "rules" }, { "rules", rules } };
}

void UiPaintPreviews::AddRecipes()
{
    // One recipe for the effect families: the same element in its baseline state and with an effect, drawn by the real painter and shaders.
    UiPreviewDescriptor effects;
    effects.Id = "engine.effects.element";
    effects.Name = "Effect-bearing element";
    effects.Category = "Painting";
    effects.Description = "A box with the style of each effect family applied, drawn by the engine painter and shaders.";
    effects.Kind = "element";
    effects.HostKind = "viewport";
    effects.RequiredCapabilities = { "painting" };
    effects.SourceRefs.push_back({ "code", "source/Engine/UI/Render/UiBoxPainter.cpp" });
    effects.Scenarios.push_back(Scenario("baseline", "Baseline", { { "style", "" } }));
    effects.Scenarios.push_back(Scenario("transformed", "Transformed", { { "style", "transform: rotate(15deg) scale(1.1)" } }));
    effects.Scenarios.push_back(Scenario("shadowed", "Shadowed", { { "style", "box-shadow: 0 8px 16px 2px rgba(0,0,0,0.6), inset 0 0 8px #ffffff" } }));
    effects.Scenarios.push_back(Scenario("filtered", "Filtered", { { "style", "filter: grayscale(0.7) brightness(1.2)" } }));
    effects.Scenarios.push_back(Scenario("clipped", "Clipped", { { "style", "clip-path: circle(40% at 50% 50%)" } }));
    effects.Scenarios.push_back(Scenario("masked", "Masked", { { "style", "mask-image: linear-gradient(to bottom, #000, transparent)" } }));
    effects.Scenarios.push_back(Scenario("animated", "Animated", { { "style", "transition: transform 0.4s ease" }, { "timeline", "enter" } }));
    UiPreviewRegistry::RegisterRecipe(effects);

    // A deterministic animation timeline: Studio pauses the motion clock (anim_pause) and scrubs it (anim_seek); the interpolation is UiAnimation's.
    UiPreviewDescriptor timeline;
    timeline.Id = "engine.animation.timeline";
    timeline.Name = "Animation timeline";
    timeline.Category = "Painting";
    timeline.Description = "Pause and scrub transitions and @keyframes animations of any document with the engine's own animation engine.";
    timeline.Kind = "element";
    timeline.HostKind = "viewport";
    timeline.RequiredCapabilities = { "painting", "motion" };
    timeline.SourceRefs.push_back({ "code", "source/Engine/UI/Style/UiAnimation.cpp" });
    UiPreviewParameter time;
    time.Name = "time";
    time.Type = "float";
    time.Default = 0.0;
    time.Min = 0.0;
    time.Max = 60.0;
    time.Step = 0.01;
    time.Unit = "s";
    time.Help = "Position on the timeline; applied with anim_seek while the motion clock is paused";
    timeline.Parameters.push_back(time);
    UiPreviewParameter paused;
    paused.Name = "paused";
    paused.Type = "bool";
    paused.Default = true;
    paused.Help = "anim_pause: stops the real-time motion clock";
    timeline.Parameters.push_back(paused);
    timeline.Scenarios.push_back(Scenario("start", "Start", { { "time", 0.0 }, { "paused", true } }));
    timeline.AnimationScenarios.push_back(Scenario("start", "Start", { { "time", 0.0 } }));
    timeline.AnimationScenarios.push_back(Scenario("mid-point", "Mid-point", { { "time", 0.5 }, { "relative", true } }));
    timeline.AnimationScenarios.push_back(Scenario("end", "End", { { "time", 1.0 }, { "relative", true } }));
    timeline.AnimationScenarios.push_back(Scenario("interrupted", "Interrupted at mid-point", { { "time", 0.5 }, { "relative", true }, { "interrupt", true } }));
    timeline.AnimationScenarios.push_back(Scenario("retargeted", "Re-targeted while running", { { "time", 0.5 }, { "relative", true }, { "retarget", true } }));
    UiPreviewRegistry::RegisterRecipe(timeline);
}

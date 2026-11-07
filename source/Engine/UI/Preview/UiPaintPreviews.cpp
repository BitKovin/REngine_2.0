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

    // Offscreen layers (phase 7): a layer is a rectangle of a 4096 px RGBA8 atlas page; the effects run as passes before the UI pass.
    Effect(r, "blur()", "filter", "function", "Gaussian blur of the element and everything inside it (9 taps, downsampled above sigma 4)", "<length>");
    Effect(r, "drop-shadow()", "filter", "function", "Shadow that follows the alpha of the content; offset 0 is a glow", "<x> <y> [blur] <color>");
    Effect(r, "isolation: isolate", "layer", "value", "Renders the element and its subtree as one picture, so opacity and blend apply to the group", "auto | isolate");
    Effect(r, "opacity (group)", "layer", "property", "Opacity below 1 with isolation: isolate, and on every fading page: overlapping children do not show through", "<number>");
    Effect(r, "cache: layer", "layer", "property", "Keeps the rendered layer until its content changes (static content only; texture contents are not watched)", "none | layer");
    Effect(r, "backdrop-filter", "filter", "property",
           "Blurs the scene colour behind the element (levels of sigma 4, 8 and 16; the nearest one is used) and applies the colour functions; the UI under it is not blurred",
           "blur(<length>) | brightness() | contrast() | saturate() | grayscale() | sepia() | hue-rotate() | invert()");
    r["paintEffects"]["backdrop-filter"]["requiresScene"] = true;
    r["paintEffects"]["layer-limits"] = json{ { "name", "layer-limits" }, { "kind", "limits" }, { "category", "layer" },
        { "description", "Layer atlas limits and unsupported combinations" },
        { "atlasPageSize", 4096 }, { "maxPages", 4 }, { "cacheFreeAfterSeconds", 3 }, { "viewBudgetPerFrame", 96 },
        { "unsupported", json::array({ "stencil clips inside a layer are ignored", "external (legacy) elements inside a layer are skipped",
                                       "cache: layer does not notice a texture whose pixels change" }) },
        { "requiresPreviewTarget", "offscreen render targets; backdrop-filter also needs the scene colour (scene views and the main UI pass have it, loading screens and world-space UI do not)" } };

    r["paintEffects"]["svg-images"] = json{ { "name", "svg-images" }, { "kind", "limits" }, { "category", "image" },
        { "description", "SVG files are rasterised by nanosvg for the size they are drawn at (Image, background-image, border-image)" },
        { "rasterBuckets", json::array({ 16, 32, 64, 128, 256, 512 }) }, { "unit", "longer side in device pixels; each bucket is cached separately" },
        { "supported", json::array({ "paths, shapes, solid and gradient fills, strokes, transforms, groups" }) },
        { "unsupported", json::array({ "text, filters, masks, CSS styling, animation, external images" }) },
        { "layoutSize", "the drawing's own width and height (96 dpi px); the raster size never changes layout" } };

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
    effects.RequiredCapabilities = { "painting", "layers" };
    effects.SourceRefs.push_back({ "code", "source/Engine/UI/Render/UiBoxPainter.cpp" });
    effects.Scenarios.push_back(Scenario("baseline", "Baseline", { { "style", "" } }));
    effects.Scenarios.push_back(Scenario("transformed", "Transformed", { { "style", "transform: rotate(15deg) scale(1.1)" } }));
    effects.Scenarios.push_back(Scenario("shadowed", "Shadowed", { { "style", "box-shadow: 0 8px 16px 2px rgba(0,0,0,0.6), inset 0 0 8px #ffffff" } }));
    effects.Scenarios.push_back(Scenario("filtered", "Filtered", { { "style", "filter: grayscale(0.7) brightness(1.2)" } }));
    effects.Scenarios.push_back(Scenario("clipped", "Clipped", { { "style", "clip-path: circle(40% at 50% 50%)" } }));
    effects.Scenarios.push_back(Scenario("masked", "Masked", { { "style", "mask-image: linear-gradient(to bottom, #000, transparent)" } }));
    effects.Scenarios.push_back(Scenario("animated", "Animated", { { "style", "transition: transform 0.4s ease" }, { "timeline", "enter" } }));
    effects.Scenarios.push_back(Scenario("blurred", "Blurred", { { "style", "filter: blur(4px)" } }));
    effects.Scenarios.push_back(Scenario("drop-shadow", "Drop shadow", { { "style", "filter: drop-shadow(8px 8px 4px rgba(0,0,0,0.7))" } }));
    effects.Scenarios.push_back(Scenario("glow", "Glow", { { "style", "filter: drop-shadow(0px 0px 16px #00ff00)" } }));
    effects.Scenarios.push_back(Scenario("group-opacity", "Group opacity", { { "style", "opacity: 0.5; isolation: isolate" } }));
    effects.Scenarios.push_back(Scenario("backdrop", "Backdrop blur", { { "style", "backdrop-filter: blur(8px)" }, { "needsScene", true } }));
    effects.Scenarios.push_back(Scenario("cached", "Cached layer", { { "style", "cache: layer; box-shadow: 0 4px 12px 0 #000000" } }));
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

    UiPreviewDescriptor svg;
    svg.Id = "engine.image.svg";
    svg.Name = "SVG icon";
    svg.Category = "Painting";
    svg.Description = "The same drawing as an Image and as a background-image at several sizes; each is rasterised for the size it is shown at.";
    svg.Kind = "element";
    svg.HostKind = "viewport";
    svg.RequiredCapabilities = { "painting", "svg" };
    svg.SourceRefs.push_back({ "document", "GameData/ui/kit/tests/p7_svg.xml" });
    svg.SourceRefs.push_back({ "asset", "GameData/ui/kit/tests/p7_icon.svg" });
    svg.SourceRefs.push_back({ "code", "source/Engine/UI/Render/UiSvg.cpp" });
    svg.Scenarios.push_back(Scenario("sizes", "Three sizes", { { "style", "" } }));
    UiPreviewRegistry::RegisterRecipe(svg);

    // Drag and drop and context menus. The fixture documents carry test data only; no inventory or game object is ever the payload.
    UiPreviewDescriptor drag;
    drag.Id = "engine.interaction.drag-drop";
    drag.Name = "Drag and drop";
    drag.Category = "Menus/Input";
    drag.Description = "An item dragged between two slots by pointer and by gamepad (pick up, move focus, drop, abort); the handlers only change classes.";
    drag.Kind = "element";
    drag.HostKind = "viewport";
    drag.RequiredCapabilities = { "events", "dragDrop" };
    drag.SourceRefs.push_back({ "document", "GameData/ui/kit/tests/p7_dragdrop.xml" });
    drag.SourceRefs.push_back({ "code", "source/Engine/UI/UiDragDrop.cpp" });
    drag.Scenarios.push_back(Scenario("idle", "Idle", { { "state", "idle" } }));
    drag.Scenarios.push_back(Scenario("pointer-drag", "Pointer drag over a slot", { { "state", "dragging" }, { "input", json::array({ "mouse_move", "mouse_down", "mouse_move", "mouse_move" }) } }));
    drag.Scenarios.push_back(Scenario("pointer-drop", "Pointer drop", { { "state", "dropped" }, { "input", json::array({ "mouse_move", "mouse_down", "mouse_move", "mouse_move", "mouse_up" }) } }));
    drag.Scenarios.push_back(Scenario("gamepad-drop", "Gamepad pick up and drop", { { "state", "dropped" }, { "device", "gamepad" }, { "input", json::array({ "ui_confirm", "ui_right", "ui_confirm" }) } }));
    drag.Scenarios.push_back(Scenario("gamepad-abort", "Gamepad abort", { { "state", "idle" }, { "device", "gamepad" }, { "input", json::array({ "ui_confirm", "ui_right", "ui_cancel" }) } }));
    UiPreviewRegistry::RegisterRecipe(drag);

    UiPreviewDescriptor context;
    context.Id = "engine.interaction.context-menu";
    context.Name = "Context menu";
    context.Category = "Menus/Input";
    context.Description = "A page pushed on the menu stack at the pointer by oncontextmenu (right click, touch hold or ui_context).";
    context.Kind = "element";
    context.HostKind = "menu-stack";
    context.RequiredCapabilities = { "events", "menus", "contextMenu" };
    context.SourceRefs.push_back({ "document", "GameData/ui/kit/tests/p7_ctx.xml" });
    context.SourceRefs.push_back({ "code", "source/Engine/UI/UiDragDrop.cpp" });
    context.Scenarios.push_back(Scenario("closed", "Closed", { { "state", "idle" } }));
    context.Scenarios.push_back(Scenario("at-pointer", "At the pointer", { { "state", "open" }, { "input", json::array({ "mouse_move", "ui_context" }) } }));
    UiPreviewRegistry::RegisterRecipe(context);
}

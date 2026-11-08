#include "UiSystem.h"
#include <UI/Render/UiGpu.h>

#include <Console/Console.h>
#include <FileSystem/FileSystem.h>
#include <EngineMain.h>
#include <Input.h>
#include <Logger.hpp>
#include <UI/Elements/UiElem.h>
#include <UI/UiScrollRegion.hpp>
#include <UI/Elements/UiElemArc.h>
#include <UI/Elements/UiElemActionBar.h>
#include <UI/Elements/UiElemButton.h>
#include <UI/Elements/UiElemControls.h>
#include <UI/Elements/UiElemDropDown.h>
#include <UI/Elements/UiElemFrame.h>
#include <UI/Elements/UiElemLegacyWidgets.h>
#include <UI/Elements/UiElemTextInput.h>
#include <UI/Style/UiSelector.h>
#include <UI/Elements/UiElemInputPrompt.h>
#include <UI/Elements/UiElemImage.h>
#include <UI/Elements/UiElemLabel.h>
#include <UI/Elements/UiElemPanel.h>
#include <UI/Flow/UiFlowLayout.h>
#include <UI/Kit/KitPages.h>
#include <UI/Kit/UiSettings.h>
#include <UI/Markup/UiBuilder.h>
#include <UI/Markup/UiDocument.h>
#include <UI/Markup/UiMarkupLog.h>
#include <UI/Markup/UiSourceSync.h>
#include <UI/Markup/UiStateSnapshot.h>
#include <UI/Markup/UiTagRegistry.h>
#include <UI/Menus/UiActions.h>
#include <UI/Menus/UiInputPrompts.h>
#include <UI/Menus/UiMenuStack.h>
#include <UI/Preview/UiDatabase.h>
#include <UI/Studio/UiInspect.h>
#include <UI/Studio/UiLint.h>
#include <UI/Preview/UiMenuPreviews.h>
#include <UI/Preview/UiPreviewRegistry.h>
#include <UI/Reflection/UiElementRegistry.h>
#include <UI/Render/UiBoxPainter.h>
#include <UI/Render/UiFontLibrary.h>
#include <UI/Render/UiImageAtlas.h>
#include <UI/Render/UiLayers.h>
#include <UI/Style/UiAnimation.h>
#include <UI/Style/UiStyleResolver.h>
#include <UI/UiManager.h>
#include <UI/Test/UiTestRunner.h>
#include <UI/UiDragDrop.h>
#include <UI/UiNavigation.h>

#include <SDL2/SDL.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <mutex>
#include <sstream>

namespace
{
    std::thread::id g_mainThread;
    std::atomic<bool> g_gameThreadKnown{ false };
    std::thread::id g_gameThread;
    bool g_initialised = false;
    std::vector<std::weak_ptr<UiElem>> g_roots;
#ifndef DISTRIBUTION
    std::map<std::string, uint32_t> g_modTimes;   // development builds: files the live pages were built from
    uint64_t g_lastPollMs = 0;
#endif
    int g_frameCounter = 0;
    std::string g_previewPath;
    bool g_previewLoaded = false;
    UiElement* g_lastFocus = nullptr;          // compared only, never dereferenced (the element may be gone)
    std::weak_ptr<UiElem> g_lastFocusElem;     // the markup element that held the focus at the last change

    std::vector<std::pair<std::weak_ptr<UiElem>, std::string>> g_queuedEvents;
    struct Capture { std::weak_ptr<UiElem> Elem; glm::vec2 Last = glm::vec2(0.f); };
    std::map<int, Capture> g_captures;
    int g_clickReleaseIn = -1;            // frames until the scripted click is released

    // Test-only element that counts the pointer events it receives (capture test).
    class UiTestCapturePanel : public UiElemPanel
    {
    public:
        UiTestCapturePanel()
        {
            LoadLayout("GameData/ui/kit/tests/p2_capture.xml");
            HitCheck = true;
        }
        void OnPointerDown(glm::vec2) override { ++Downs; }
        void OnPointerMove(glm::vec2) override { ++Moves; }
        void OnPointerUp(glm::vec2) override { ++Ups; }
        void OnScroll(float delta) override { Scroll += delta; }
        int Downs = 0, Moves = 0, Ups = 0;
        float Scroll = 0.f;
    };

    std::mutex g_pendingMutex;
    std::string g_pendingGameThreadTest;
    std::shared_ptr<UiElem> g_lastGameThreadRoot;

    // ── registration of built-in tags with their metadata (published into the UI database)
    void RegisterBuiltins()
    {
        using UiTagRegistry::AttributeInfo;
        using UiTagRegistry::TagInfo;
        UiTagRegistry::RegisterBuiltin("Panel", &UiElemPanel::Create, TagInfo{ "Panel", "layout", "Container with a box (background, border, radius)", {} });
        UiTagRegistry::RegisterBuiltin("Label", &UiElemLabel::Create, TagInfo{ "Label", "text", "Text",
            { AttributeInfo{ "text", "text", "", false, "Text to show; the element's text content is used when the attribute is absent" },
              AttributeInfo{ "html", "bool", "false", false, "true interprets <b> <i> <color=#rrggbb> <size=NN> <icon=action:NAME> <icon=img:PATH>" } } });
        UiTagRegistry::RegisterBuiltin("Arc", &UiElemArc::Create, TagInfo{ "Arc", "paint", "Ring segment (progress, cooldown); the colour is the text colour",
            { AttributeInfo{ "start", "number", "0", false, "Start angle in degrees, clockwise from 12 o'clock" },
              AttributeInfo{ "sweep", "number", "360", false, "Sweep in degrees (negative = counter-clockwise)" },
              AttributeInfo{ "thickness", "number", "6", false, "Ring thickness in px" },
              AttributeInfo{ "round-caps", "bool", "false", false, "Round ends" } } });
        UiTagRegistry::RegisterBuiltin("InputPrompt", &UiElemInputPrompt::Create, TagInfo{ "InputPrompt", "input", "Image of the key or button that triggers an action on the device used last",
            { AttributeInfo{ "action", "string", "", true, "Input action name" } } });
        UiTagRegistry::RegisterBuiltin("ActionBar", &UiElemActionBar::Create, TagInfo{ "ActionBar", "input", "Prompt and label of every hotkey button of the top menu page, plus back", {} });
        UiTagRegistry::RegisterBuiltin("Image", &UiElemImage::Create, TagInfo{ "Image", "image", "Texture drawn with object-fit",
            { AttributeInfo{ "src", "path", "", true, "GameData-relative texture path" } } });
        const AttributeInfo hotkey{ "hotkey", "string", "", false, "Input action that activates the button while its page is on top" };
        const AttributeInfo hotkeyLabel{ "hotkey-label", "text", "", false, "Text shown for the hotkey in an ActionBar (default: the button text)" };
        UiTagRegistry::RegisterBuiltin("Button", &UiElemButton::Create, TagInfo{ "Button", "input", "Focusable, activatable container", { hotkey, hotkeyLabel } });
        const AttributeInfo bind{ "bind", "string", "", false, "setting:<key>: the widget edits that engine setting and follows it" };
        const AttributeInfo value{ "value", "number", "0", false, "Initial value" };
        UiTagRegistry::RegisterBuiltin("Slider", &UiElemSlider::Create, TagInfo{ "Slider", "input", "Draggable value slider (wraps the legacy slider); events: valuechanged; variable: value",
            { AttributeInfo{ "min", "number", "0", false, "Smallest value" }, AttributeInfo{ "max", "number", "1", false, "Largest value" },
              AttributeInfo{ "step", "number", "0", false, "Snap increment (0 = continuous)" }, value, bind } });
        UiTagRegistry::RegisterBuiltin("DropDown", &UiElemDropDown::Create, TagInfo{ "DropDown", "input", "Drop-down list: a header and a list that opens above everything else (classes .dropdown-text, .dropdown-arrow, .dropdown-list, .dropdown-item; states :open via .open, :selected on the row); events: valuechanged; variable: value (the index)",
            { AttributeInfo{ "options", "text", "", false, "Items separated by |, localised" }, AttributeInfo{ "selected-index", "number", "-1", false, "Index of the selected item" },
              AttributeInfo{ "placeholder", "text", "", false, "Shown while nothing is selected" }, AttributeInfo{ "max-visible", "number", "6", false, "Rows shown before the list scrolls" }, bind } });
        const AttributeInfo textAttr{ "text", "text", "", false, "Initial text" };
        const AttributeInfo placeholderAttr{ "placeholder", "text", "", false, "Shown while empty" };
        const std::vector<AttributeInfo> textInputAttrs = {
            textAttr, placeholderAttr,
            AttributeInfo{ "max-length", "number", "0", false, "Most characters (code points); 0 = unlimited" },
            AttributeInfo{ "password", "bool", "false", false, "Mask the characters (no copy or cut)" },
            AttributeInfo{ "numeric", "bool", "false", false, "Digits, one leading '-' and one '.' only" },
            AttributeInfo{ "integer", "bool", "false", false, "Digits and one leading '-' only" },
            AttributeInfo{ "allowed-chars", "text", "", false, "Only these characters can be typed or pasted" },
            AttributeInfo{ "read-only", "bool", "false", false, "Shown and focusable, never edited" },
            AttributeInfo{ "select-on-focus", "bool", "false", false, "Starting to edit selects the whole text" },
            AttributeInfo{ "blur-on-submit", "bool", "false", false, "Enter also takes the navigation focus away" } };
        std::vector<AttributeInfo> entryAttrs = textInputAttrs;
        entryAttrs.push_back(AttributeInfo{ "min-rows", "number", "1", false, "Least lines the box grows from" });
        UiTagRegistry::RegisterBuiltin("TextEntry", &UiElemTextEntry::Create, TagInfo{ "TextEntry", "input", "Single-line text field: caret, selection, clipboard, undo, scrolls with the caret; width: auto grows with the text. States: :focus, class .editing. Events: input, valuechanged, change, submit, editstart, editend, editcancel, inputrejected; variable: value (the text); custom properties --caret-color, --selection-color, --placeholder-color",
            textInputAttrs });
        std::vector<AttributeInfo> multilineAttrs = textInputAttrs;
        multilineAttrs.push_back(AttributeInfo{ "min-rows", "number", "1", false, "Least lines the box grows from (height: auto)" });
        multilineAttrs.push_back(AttributeInfo{ "max-rows", "number", "0", false, "Most lines the box grows to, then it scrolls; 0 = no limit" });
        multilineAttrs.push_back(AttributeInfo{ "wrap", "bool", "true", false, "Wrap long lines (white-space: pre turns it off)" });
        multilineAttrs.push_back(AttributeInfo{ "scrollbar", "bool", "true", false, "Show the scrollbar (class .scrollbar / .thumb)" });
        UiTagRegistry::RegisterBuiltin("MultilineTextEntry", &UiElemMultilineTextEntry::Create, TagInfo{ "MultilineTextEntry", "input", "Multi-line text field: Enter inserts a line break, Ctrl+Enter submits; wraps at the box width, height: auto grows with the text (min-rows, max-rows) and then scrolls. Events and variable as TextEntry",
            multilineAttrs });
        UiTagRegistry::RegisterBuiltin("Movie", &UiElemMovie::Create, TagInfo{ "Movie", "image", "Video (wraps the legacy video)",
            { AttributeInfo{ "src", "path", "", true, "GameData-relative video path" }, AttributeInfo{ "loop", "bool", "false", false, "Restart at the end" },
              AttributeInfo{ "autoplay", "bool", "true", false, "Accepted; the video starts when first drawn" }, AttributeInfo{ "muted", "bool", "false", false, "Accepted; the legacy video has no mute field" } } });
        UiTagRegistry::RegisterBuiltin("ShaderPanel", &UiElemShaderPanel::Create, TagInfo{ "ShaderPanel", "image", "Panel drawn by a custom pixel shader (wraps the legacy shader image)",
            { AttributeInfo{ "shader", "path", "", true, "Shader name" }, AttributeInfo{ "param-<name>", "text", "", false, "vec4 uniform as 'x y z w'" } } });
        UiTagRegistry::RegisterBuiltin("ToggleButton", &UiElemToggleButton::Create, TagInfo{ "ToggleButton", "input", "Button that toggles :checked; events: valuechanged; variable: value (0/1)",
            { AttributeInfo{ "checked", "bool", "false", false, "Initial state" }, bind, hotkey, hotkeyLabel } });
        UiTagRegistry::RegisterBuiltin("RadioButton", &UiElemRadioButton::Create, TagInfo{ "RadioButton", "input", "One of a group; checking it unchecks the others of the group; events: valuechanged",
            { AttributeInfo{ "group", "string", "", false, "Radios with the same group exclude each other inside one root" }, AttributeInfo{ "checked", "bool", "false", false, "Initial state" },
              AttributeInfo{ "option", "number", "", false, "With bind: the setting value this radio selects" }, bind } });
        UiTagRegistry::RegisterBuiltin("ProgressBar", &UiElemProgressBar::Create, TagInfo{ "ProgressBar", "paint", "Track with a .fill child whose size is value x content",
            { AttributeInfo{ "value", "number", "0", false, "0..1" }, AttributeInfo{ "direction", "string", "horizontal", false, "horizontal or vertical" } } });
        UiTagRegistry::RegisterBuiltin("CircularProgress", &UiElemCircularProgress::Create, TagInfo{ "CircularProgress", "paint", "Ring: background-color is the track, color the fill",
            { AttributeInfo{ "value", "number", "0", false, "0..1" }, AttributeInfo{ "start-angle", "number", "0", false, "Degrees clockwise from 12 o'clock" },
              AttributeInfo{ "thickness", "number", "8", false, "Ring thickness in px" } } });
        UiTagRegistry::RegisterBuiltin("Frame", &UiElemFrame::Create, TagInfo{ "Frame", "layout", "Builds another layout (src) or a snippet of this file (snippet) inside itself",
            { AttributeInfo{ "src", "path", "", false, "Layout file; changing it rebuilds the content" }, AttributeInfo{ "snippet", "string", "", false, "Snippet or component tag of this file" } } });
        UiTagRegistry::RegisterBuiltin("ListView", &UiElemListView::Create, TagInfo{ "ListView", "layout", "Scrolling virtualised list: only the visible rows are built from the snippet and recycled; needs overflow: scroll",
            { AttributeInfo{ "snippet", "string", "", true, "Row snippet; rows get the variable index" }, AttributeInfo{ "count", "number", "0", false, "Number of items" },
              AttributeInfo{ "layout", "string", "list", false, "list or grid" }, AttributeInfo{ "item-height", "number", "", true, "Row height in px" },
              AttributeInfo{ "columns", "number", "1", false, "Columns of a grid" } } });
        UiTagRegistry::RegisterBuiltin("TextButton", &UiElemTextButton::Create, TagInfo{ "TextButton", "input", "Button with a text label",
            { AttributeInfo{ "text", "text", "", false, "Button text" }, hotkey, hotkeyLabel } });
    }

    bool g_pseudoLoc = false;
    float g_lastUiScale = 1.f;

    // ── helpers
    bool IsUnderViewport(const UiElement* e)
    {
        for (const UiElement* p = e->parent; p != nullptr; p = p->parent)
            if (p == &EngineMain::Viewport)
                return true;
        return false;
    }

    float g_frameDt = 0.f;   // real seconds of the last frame (at most 0.1)

    void TickAll(UiElem& e)
    {
        if (!e.m_navResolved)
            e.ResolveNavOverrides();
        e.Tick();
        e.TickScroll(g_frameDt);
        // Tick may rebuild the children (ActionBar), so the list is read afterwards.
        const auto children = e.children;
        for (auto& c : children)
            if (UiElem::IsElem(c.get()))
                TickAll(static_cast<UiElem&>(*c));
    }

    glm::vec2 ToLocal(const UiElem& e, glm::vec2 p)
    {
        return UiElement::TransformPoint(glm::inverse(e.AbsoluteTransform), p);
    }

    // The deepest scroll container under a point, for pointers over areas without a hit-testable element (the empty
    // parts of a list). Lists that are covered by a hit-testable element of another page are not reached through this.
    UiElem* ScrollContainerAt(glm::vec2 point)
    {
        UiElem* best = nullptr;
        for (auto& w : g_roots)
        {
            auto root = w.lock();
            if (!root || !IsUnderViewport(root.get()) || root->Computed().Visibility != UiVisibility::Visible)
                continue;
            std::vector<UiElem*> all;
            root->CollectElems(all);
            for (UiElem* e : all)
            {
                if (!e->IsScrollContainer() || e->Computed().Visibility != UiVisibility::Visible || !e->IsVisible())
                    continue;
                const glm::vec2 local = ToLocal(*e, point);
                if (local.x >= 0.f && local.y >= 0.f && local.x <= e->BorderRect.W && local.y <= e->BorderRect.H)
                    best = e;   // pre-order: deeper containers come later
            }
        }
        return best;
    }

    // Position in the viewport's child list of the root that holds `e` (later = drawn and hit above); -1 when it is not under the viewport.
    int ViewportIndexOf(const UiElement* e)
    {
        while (e != nullptr && e->parent != &EngineMain::Viewport)
            e = e->parent;
        if (e == nullptr)
            return -1;
        const auto& kids = EngineMain::Viewport.children;
        for (size_t i = 0; i < kids.size(); ++i)
            if (kids[i].get() == e)
                return static_cast<int>(i);
        return -1;
    }

    // Pointer events with capture: the element that got the press keeps receiving move and release.
    void ProcessPointers()
    {
        for (auto& [id, touch] : Input::TouchActions)
        {
            if (touch.pressed)
            {
                g_captures.erase(id);
                const std::shared_ptr<UiElement> hit = EngineMain::Viewport.GetHitElementUnderPosition(touch.position);
                if (hit && UiElem::IsElem(hit.get()))
                {
                    const auto element = std::static_pointer_cast<UiElem>(hit);
                    g_captures[id] = Capture{ element, touch.position };
                    element->OnPointerDown(ToLocal(*element, touch.position));
                    UiDragDrop::Press(id, *element, touch.position);
                }
                else
                {
                    // A press on nothing, or on a legacy element (a HUD touch area) that lies below the menu holding the container:
                    // the scroll container gets it (the scrollbar and the empty parts of a list).
                    UiElem* container = ScrollContainerAt(touch.position);
                    if (container != nullptr && hit && ViewportIndexOf(container) <= ViewportIndexOf(hit.get()))
                        container = nullptr;
                    if (container != nullptr)
                        if (auto self = container->weak_from_this().lock())
                        {
                            const auto element = std::static_pointer_cast<UiElem>(self);
                            g_captures[id] = Capture{ element, touch.position };
                            element->OnPointerDown(ToLocal(*element, touch.position));
                        }
                }
                continue;
            }
            const auto found = g_captures.find(id);
            if (found == g_captures.end())
                continue;
            const std::shared_ptr<UiElem> element = found->second.Elem.lock();
            if (!element)
            {
                g_captures.erase(found);
                continue;
            }
            if (touch.position != found->second.Last)
            {
                found->second.Last = touch.position;
                if (!UiDragDrop::Move(id, touch.position))   // a drag in progress keeps the move from scrolling the list under it
                    element->OnPointerMove(ToLocal(*element, touch.position));
            }
            if (touch.released)
            {
                element->OnPointerUp(ToLocal(*element, touch.position));
                if (UiDragDrop::Release(id, touch.position))
                    UiElem::SuppressActivate = true;   // a drop or a long press is not a click
                g_captures.erase(id);
            }
        }
        if (Input::MouseScrollDelta != 0.f)
        {
            const auto mouse = Input::TouchActions.find(1);
            if (mouse != Input::TouchActions.end())
            {
                // A legacy child inside a scroll container passes the wheel to the markup element that holds it.
                std::shared_ptr<UiElement> hit = EngineMain::Viewport.GetHitElementUnderPosition(mouse->second.position);
                UiElement* target = hit.get();
                bool ownsWheel = false;   // an open drop-down list (a legacy scroll region) scrolls itself, never the page behind it
                while (target != nullptr && !UiElem::IsElem(target))
                {
                    if (dynamic_cast<UiScrollRegion*>(target) != nullptr)
                        ownsWheel = true;
                    target = target->parent;
                }
                UiElem* nearest = target != nullptr ? static_cast<UiElem&>(*target).NearestScrollContainer() : nullptr;
                if (nearest == nullptr)
                    nearest = ScrollContainerAt(mouse->second.position);
                if (ownsWheel)
                    nearest = nullptr;
                if (target != nullptr)
                    static_cast<UiElem&>(*target).Fire("wheel");
                if (nearest != nullptr)
                    nearest->OnScroll(Input::MouseScrollDelta);
            }
        }
    }

    void FlushQueuedEvents()
    {
        if (g_queuedEvents.empty())
            return;
        auto queued = std::move(g_queuedEvents);
        g_queuedEvents.clear();
        for (auto& [weak, event] : queued)
            if (auto element = weak.lock())
                element->FireSilent(event);
    }

    // Fires "load" for elements that were laid out for the first time.
    void FireLoadEvents(UiElem& root)
    {
        std::vector<UiElem*> all;
        root.CollectElems(all);
        std::vector<std::shared_ptr<UiElem>> pending;
        for (UiElem* e : all)
            if (!e->m_loadFired)
            {
                e->m_loadFired = true;
                if (e->EventCalls("load") != nullptr)
                    try { pending.push_back(std::static_pointer_cast<UiElem>(e->shared_from_this())); } catch (...) {}
            }
        for (auto& e : pending)
            e->Fire("load");
    }

    std::vector<std::shared_ptr<UiElem>> LiveRoots()
    {
        std::vector<std::shared_ptr<UiElem>> out;
        for (auto& w : g_roots)
            if (auto r = w.lock())
                out.push_back(r);
        return out;
    }

#ifndef DISTRIBUTION
    void RememberModTimes(const UiElem& root)
    {
        for (const std::string& path : root.m_dependencies)
            if (!g_modTimes.count(path))
                g_modTimes[path] = FileSystemEngine::GetFileModificationTime(path);
    }
#else
    void RememberModTimes(const UiElem&) {}
#endif

    // ── paint recording (task 1.13)
    struct ClipInfo { bool Valid = false; glm::mat3 Matrix = glm::mat3(1.f); glm::vec2 Size = glm::vec2(0.f); };

    // The offscreen layer an element needs (phase 7): blur() / drop-shadow() filters, isolation: isolate, cache: layer, and a page
    // that is fading (its children must fade as one picture, not each on its own).
    bool BuildLayerSpec(UiElem& e, const UiComputedStyle& s, UiRender::LayerSpec& spec)
    {
        const UiFxStyle& fx = s.Fx;
        for (const UiFilterFn& f : fx.Filter)
        {
            UiRender::LayerEffect effect;
            if (f.Type == UiFilterType::Blur && f.Amount > 0.f)
            {
                effect.K = UiRender::LayerEffect::Kind::Blur;
                effect.Sigma = f.Amount;
            }
            else if (f.Type == UiFilterType::DropShadow)
            {
                effect.K = UiRender::LayerEffect::Kind::DropShadow;
                effect.Sigma = f.Shadow.Blur * 0.5f;
                effect.Offset = glm::vec2(f.Shadow.X, f.Shadow.Y);
                effect.Color = f.Shadow.Color;
            }
            else
                continue;
            spec.Effects.push_back(effect);
        }
        spec.Cache = fx.CacheLayer;
        spec.Id = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(&e));
        if (!spec.Effects.empty() || fx.Isolate || fx.CacheLayer)
            return true;
        return s.Opacity < 0.999f && e.PageOf() == &e;
    }

    void Paint(UiElem& e, UiPainter& p, UiElem& root, float parentOpacity, const ClipInfo& clip)
    {
        const UiComputedStyle& s = e.Computed();
        if (s.Visibility != UiVisibility::Visible)
            return;
        UiRender::LayerSpec layerSpec;
        const bool layer = BuildLayerSpec(e, s, layerSpec);
        const float groupOpacity = parentOpacity * s.Opacity;   // a layer applies this to its composite; inside, everything starts at 1
        const float op = layer ? 1.f : groupOpacity;
        const float W = e.BorderRect.W, H = e.BorderRect.H;
        p.SetTransform(e.AbsoluteTransform);

        UiCorners radii;
        {
            const float side = std::min(W, H);
            float r[4];
            for (int i = 0; i < 4; ++i)
                r[i] = std::clamp(s.Radius[i].Resolve(side), 0.f, side * 0.5f);
            radii = UiCorners{ r[0], r[1], r[2], r[3] };
        }

        if (layer)
            p.PushLayer(layerSpec);
        const UiBoxPainter::Scope effects = UiBoxPainter::Begin(e, p, layer);   // filter, mask, clip-path, blend mode for this subtree
        UiBoxPainter::PaintBackdrop(e, p, op, radii);
        UiBoxPainter::PaintBox(e, p, op, radii);

        e.m_paintOpacity = op;
        e.PaintContent(p, e.ContentRectLocal());

        ClipInfo childClip = clip;
        const bool clips = s.Overflow != UiOverflow::Visible;
        if (clips)
        {
            p.SetTransform(e.AbsoluteTransform);
            p.PushClip(UiRect{ 0.f, 0.f, W, H }, radii);
            childClip.Valid = true;
            childClip.Matrix = e.AbsoluteTransform;
            childClip.Size = glm::vec2(W, H);
        }

        // paint order: stable by z-index
        struct Entry { int Z; UiElement* Child; std::shared_ptr<UiElement> Owner; };
        std::vector<Entry> order;
        for (auto& c : e.children)
            order.push_back({ UiElem::IsElem(c.get()) ? (static_cast<UiElem*>(c.get())->InternalOverlay ? (1 << 20) : static_cast<UiElem*>(c.get())->Computed().ZIndex) : 0, c.get(), c });
        std::stable_sort(order.begin(), order.end(), [](const Entry& a, const Entry& b) { return a.Z < b.Z; });
        for (const Entry& entry : order)
        {
            if (UiElem::IsElem(entry.Child))
                Paint(static_cast<UiElem&>(*entry.Child), p, root, op, childClip);
            else
            {
                root.m_chunk.Barrier();
                root.m_legacyMarkers.push_back({ root.m_chunk.Items().size(), entry.Owner, childClip.Matrix, childClip.Size, childClip.Valid });
            }
        }
        if (clips)
            p.PopClip();
        p.SetTransform(e.AbsoluteTransform);
        UiBoxPainter::PaintOutline(e, p, op, radii);
        UiBoxPainter::End(p, effects);
        if (layer)
            p.PopLayer(groupOpacity, static_cast<UiBlendMode>(static_cast<int>(s.Fx.MixBlend)));
    }

    void Record(UiElem& root)
    {
        root.m_chunk.Clear();
        root.m_chunk.SetLocalPrims(true);
        root.m_legacyMarkers.clear();
        UiPainter painter(root.m_chunk);
        root.m_paintDirty = false;   // before painting: an image that is not loaded yet asks for another pass
        Paint(root, painter, root, 1.f, ClipInfo());
        UiElem::UiFrameData data{ root.m_chunk, root.m_legacyMarkers };
        root.m_frame = std::make_shared<const UiElem::UiFrameData>(std::move(data));
    }

    // ── hot reload (task 1.14). Development builds only: a shipped game does not watch its files.
#ifndef DISTRIBUTION
    void ReloadRoot(const std::shared_ptr<UiElem>& old)
    {
        UiElement* parent = old->parent;
        UiMenuStack* stack = UiElem::IsElem(parent) ? static_cast<UiElem*>(parent)->AsMenuStack() : nullptr;
        const std::string path = old->m_layoutPath;
        const UiStateSnapshot state = UiStateSnapshot::Capture(*old);      // what the user did on the page, put back below
        auto fresh = UiElem::Load(path);
        if (!fresh)
        {
            Logger::Warning("[UI] reload of %s failed; keeping the previous version", path.c_str());
            return;
        }
        if (stack != nullptr)
            UiSystem::UnregisterRoot(fresh);
        if (stack != nullptr)
            stack->SwapPage(old.get(), fresh);
        else if (parent != nullptr)
        {
            parent->AddChild(fresh);
            parent->RemoveChild(old);
            // Roots are painted from the registry, not from the parent: an old page that the game still holds
            // would keep drawing under the new one, and every reload would add another copy (glows get brighter).
            UiSystem::UnregisterRoot(old);
        }
        state.Restore(*fresh);
        RememberModTimes(*fresh);
        Logger::Log("[UI] reloaded %s", path.c_str());
    }

    void PollHotReload()
    {
        const uint64_t now = SDL_GetTicks64();
        if (now - g_lastPollMs < 500)
            return;
        g_lastPollMs = now;

        // The running copy of GameData is made by the build: bring edits of the source files over first.
        std::vector<std::string> tracked;
        for (const auto& entry : g_modTimes)
            tracked.push_back(entry.first);
        std::vector<std::string> changed = UiSourceSync::Poll(tracked);
        for (auto& [path, stamp] : g_modTimes)
        {
            if (std::find(changed.begin(), changed.end(), path) != changed.end())
            {
                stamp = FileSystemEngine::GetFileModificationTime(path);
                continue;
            }
            const uint32_t current = FileSystemEngine::GetFileModificationTime(path);
            if (current != 0 && current != stamp)
            {
                stamp = current;
                changed.push_back(path);
            }
        }
        if (changed.empty())
            return;

        bool componentChanged = false;
        for (const std::string& path : changed)
        {
            if (path.size() > 4 && path.compare(path.size() - 4, 4, ".css") == 0)
                UiStyleSheetCache::Invalidate(path);
            else
            {
                UiDocumentCache_Invalidate(path);
                if (UiComponentRegistry::Find(path) != nullptr) componentChanged = true;
            }
        }
        // Style sheets are embedded into parsed documents, so every document that includes a changed sheet is parsed again.
        for (const std::string& path : changed)
            if (path.size() > 4 && path.compare(path.size() - 4, 4, ".css") == 0)
                for (const std::string& doc : UiDocumentCache_Paths())
                {
                    const auto def = UiDocumentCache_Get(doc);
                    if (def && std::find(def->SheetFiles.begin(), def->SheetFiles.end(), path) != def->SheetFiles.end())
                        UiDocumentCache_Invalidate(doc);
                }
        for (const UiComponentDef* comp : UiComponentRegistry::All())
            if (std::find(changed.begin(), changed.end(), comp->File) != changed.end())
                componentChanged = true;
        UiStyleResolver::ResetWarnings();
        if (componentChanged)
            UiComponentRegistry::Discover();

        std::vector<std::shared_ptr<UiElem>> reloadable = LiveRoots();
        for (int i = 0; i < UiMenus::Stack().Depth(); ++i)
            if (UiElem* page = UiMenus::Stack().PageAt(i))
                try { reloadable.push_back(std::static_pointer_cast<UiElem>(page->shared_from_this())); } catch (...) {}
        for (size_t i = 0; i < reloadable.size(); ++i)
        {
            const std::shared_ptr<UiElem>& root = reloadable[i];
            bool duplicate = false;
            for (size_t j = 0; j < i; ++j)
                duplicate = duplicate || reloadable[j] == root;
            if (duplicate)
                continue;
            bool affected = false;
            for (const std::string& dep : root->m_dependencies)
                if (std::find(changed.begin(), changed.end(), dep) != changed.end())
                    affected = true;
            if (affected && !root->m_layoutPath.empty())
                ReloadRoot(root);
        }
        UiDatabase::MarkDirty("hot reload");
        UiDatabase::RefreshFromRegistries();
    }
#else
    void PollHotReload() {}
#endif

    // ── console commands
    void CmdLoad(const std::vector<std::string>& args)
    {
        if (args.empty()) { Logger::Log("usage: ui_load <layout path>"); return; }
        auto root = UiElem::Load(args[0]);
        if (root)
            EngineMain::Viewport.AddChild(root);
    }

    void CmdUnloadAll(const std::vector<std::string>&)
    {
        for (auto& root : LiveRoots())
            if (root->parent != nullptr)
                root->parent->RemoveChild(root);
        g_roots.clear();
    }

    void Dump(const UiElement& e, int depth, int maxDepth, std::ostringstream& out)
    {
        std::string indent(depth * 2, ' ');
        if (UiElem::IsElem(&e))
        {
            const UiElem& el = static_cast<const UiElem&>(e);
            out << indent << "<" << el.TagName;
            if (!el.Id.empty()) out << "#" << el.Id;
            for (const std::string& c : el.Classes) out << "." << c;
            for (const std::string& c : el.CodeClasses) out << "." << c;
            out << ">";
            out << " [";
            const char* sep = "";
            auto flag = [&](bool on, const char* name) { if (on) { out << sep << name; sep = " "; } };
            flag(el.States & kStateHover, "hover");
            flag(el.States & kStateActive, "active");
            flag(UiNavigation::Focused == &el, "focus");
            flag(el.States & kStateDisabled, "disabled");
            flag(el.States & kStateSelected, "selected");
            out << "]";
            const glm::vec2 p = UiElement::TransformPoint(el.AbsoluteTransform, glm::vec2(0.f));
            char buffer[128];
            std::snprintf(buffer, sizeof(buffer), " rect=(%g,%g,%g,%g)", p.x, p.y, el.BorderRect.W, el.BorderRect.H);
            out << buffer;
            if (el.HitCheck) out << " hit";
            if (el.DisableFocus) out << " nofocus";
            if (!const_cast<UiElem&>(el).IsVisible()) out << " invisible";
            if (el.Computed().Visibility == UiVisibility::Hidden) out << " hidden";
            if (el.Computed().Visibility == UiVisibility::Collapse) out << " collapsed";
            out << "\n";
        }
        else
        {
            out << indent << "<legacy " << UiElementRegistry::Instance().GetTypeNameForInstance(&e) << ">\n";
        }
        if (depth >= maxDepth)
            return;
        for (const auto& c : e.children)
            Dump(*c, depth + 1, maxDepth, out);
    }

    void CmdDump(const std::vector<std::string>& args)
    {
        const int maxDepth = args.empty() ? 64 : std::atoi(args[0].c_str());
        std::ostringstream out;
        out << "focused: " << (UiNavigation::Focused == nullptr ? "none" : UiElementRegistry::Instance().GetTypeNameForInstance(UiNavigation::Focused))
            << (UiElem::IsElem(UiNavigation::Focused) ? " (markup)" : "");
        if (UiNavigation::Focused != nullptr)
        {
            UiElement* f = UiNavigation::Focused;
            out << " rect=(" << f->topLeft.x << "," << f->topLeft.y << "," << f->size.x << "," << f->size.y << ") visible=" << f->visible << " parentType=" << (f->parent ? UiElementRegistry::Instance().GetTypeNameForInstance(f->parent) : std::string("-"));
        }
        out << "\n";
        for (auto& root : LiveRoots())
            Dump(*root, 0, maxDepth, out);
        Logger::Log("[UiDump]\n%s", out.str().c_str());
    }

    void CmdFocus(const std::vector<std::string>& args)
    {
        if (args.empty()) { Logger::Log("usage: ui_focus <id>"); return; }
        UiElem* e = UiSystem::FindById(args[0]);
        if (e == nullptr) { Logger::Warning("[UI] ui_focus: no element '%s'", args[0].c_str()); return; }
        // A wrapper (Slider, DropDown, ...) is not a focus target itself: its first focusable descendant is.
        UiElement* target = e;
        if (!target->HitCheck || target->DisableFocus)
        {
            std::vector<UiElement*> queue{ e };
            for (size_t i = 0; i < queue.size(); ++i)
            {
                for (auto& c : queue[i]->children)
                    queue.push_back(c.get());
                if (i > 0 && queue[i]->HitCheck && !queue[i]->DisableFocus && queue[i]->IsVisible())
                {
                    target = queue[i];
                    break;
                }
            }
        }
        UiNavigation::MouseNavigation = false;   // a script's focus is a navigation focus
        UiNavigation::SetFocus(target);
        Logger::Log("[UI] ui_focus %s: focused=%d visible=%d hit=%d disableFocus=%d", args[0].c_str(), UiNavigation::Focused == e ? 1 : 0,
                    e->IsVisible() ? 1 : 0, e->HitCheck ? 1 : 0, e->DisableFocus ? 1 : 0);
    }

    void CmdPush(const std::vector<std::string>& args)
    {
        if (args.empty()) { Logger::Log("usage: ui_push <layout path>"); return; }
        UiMenus::Push(UiActions::NormalizeLayoutPath(args[0]));
    }

    void CmdPop(const std::vector<std::string>&) { UiMenus::Pop(); }
    void CmdClearStack(const std::vector<std::string>&) { UiMenus::Clear(); }
    void CmdBack(const std::vector<std::string>&) { UiMenus::Stack().Back(); }

    void CmdReducedMotion(const std::vector<std::string>& args)
    {
        UiAnimation::SetReducedMotion(args.empty() || args[0] != "0");
        Logger::Log(std::string("ui_reduced_motion: ") + (UiAnimation::ReducedMotion() ? "on" : "off"));
    }

    void CmdStack(const std::vector<std::string>&)
    {
        std::ostringstream out;
        UiMenuStack& stack = UiMenus::Stack();
        out << "depth " << stack.Depth() << " paused " << (stack.PausesGame() ? 1 : 0) << "\n";
        for (int i = stack.Depth() - 1; i >= 0; --i)
        {
            UiElem* page = stack.PageAt(i);
            out << "  " << i << ": " << page->m_layoutPath << " visible=" << (page->Computed().Visibility == UiVisibility::Visible ? 1 : 0) << "\n";
        }
        Logger::Log("[UiStack] %s", out.str().c_str());
    }

    void CmdGameThreadTest(const std::vector<std::string>& args)
    {
        if (args.empty()) { Logger::Log("usage: ui_gamethread_test <layout path>"); return; }
        UiSystem::RequestGameThreadTest(args[0]);
    }

    void CmdDbInfo(const std::vector<std::string>&)
    {
        Logger::Log("[UiDb] revision %llu writes %d path %s lastWriteOk %d bytes %d", (unsigned long long)UiDatabase::Revision(),
                    UiDatabase::WriteCount(), UiDatabase::OutputPath().c_str(), UiDatabase::LastWriteSucceeded() ? 1 : 0,
                    (int)UiDatabase::LastWritten().size());
    }

    void CmdDbPath(const std::vector<std::string>& args)
    {
        if (args.empty()) { Logger::Log("usage: ui_db_path <file>"); return; }
        UiDatabase::SetOutputPath(args[0]);
        UiDatabase::Flush();
        CmdDbInfo({});
    }

    // ── test runner commands (task 1.15)
    UiElem* Lookup(const std::string& idPath)
    {
        const size_t slash = idPath.find('/');
        if (slash == std::string::npos)
            return UiSystem::FindById(idPath);
        UiElem* first = UiSystem::FindById(idPath.substr(0, slash));
        return first == nullptr ? nullptr : first->FindElem(idPath.substr(slash + 1));
    }

    std::vector<std::string> Words(const std::string& s)
    {
        std::vector<std::string> out;
        std::istringstream in(s);
        std::string w;
        while (in >> w) out.push_back(w);
        return out;
    }

    std::string RestAfter(const std::string& s, size_t words)
    {
        size_t pos = 0;
        for (size_t i = 0; i < words; ++i)
        {
            pos = s.find_first_not_of(" \t", pos);
            if (pos == std::string::npos) return "";
            pos = s.find_first_of(" \t", pos);
            if (pos == std::string::npos) return "";
        }
        pos = s.find_first_not_of(" \t", pos);
        return pos == std::string::npos ? "" : s.substr(pos);
    }

    bool TestExpectRect(const std::string& args, std::string& error)
    {
        const auto w = Words(args);
        if (w.size() < 5) { error = "usage: expect_rect <id> <x> <y> <w> <h> [tol]"; return false; }
        UiElem* e = Lookup(w[0]);
        if (!e) { error = "no element '" + w[0] + "'"; return false; }
        const float tol = w.size() > 5 ? (float)std::atof(w[5].c_str()) : 0.5f;
        const glm::vec2 p = UiElement::TransformPoint(e->AbsoluteTransform, glm::vec2(0.f));
        const float actual[4] = { p.x, p.y, e->BorderRect.W, e->BorderRect.H };
        const char* names[4] = { "x", "y", "w", "h" };
        for (int i = 0; i < 4; ++i)
        {
            const float want = (float)std::atof(w[1 + i].c_str());
            if (std::fabs(actual[i] - want) > tol)
            {
                char buffer[200];
                std::snprintf(buffer, sizeof(buffer), "expect_rect #%s %s: expected %g, got %g (rect %g %g %g %g)", w[0].c_str(), names[i], want, actual[i],
                              actual[0], actual[1], actual[2], actual[3]);
                error = buffer;
                return false;
            }
        }
        return true;
    }

    bool TestExpect(const std::string& args, std::string& error)
    {
        const auto w = Words(args);
        if (w.size() < 4) { error = "usage: expect <id> <prop> <op> <value>"; return false; }
        UiElem* e = Lookup(w[0]);
        if (!e) { error = "no element '" + w[0] + "'"; return false; }
        const std::string& prop = w[1];
        const std::string& op = w[2];
        const std::string value = RestAfter(args, 3);

        UiElemLabel* label = dynamic_cast<UiElemLabel*>(e);
        if (label == nullptr)
            if (auto* tb = dynamic_cast<UiElemTextButton*>(e)) label = tb->LabelElem();

        bool isNumber = true;
        double number = 0.0;
        std::string text;
        const glm::vec2 p = UiElement::TransformPoint(e->AbsoluteTransform, glm::vec2(0.f));
        if (prop == "x") number = p.x;
        else if (prop == "y") number = p.y;
        else if (prop == "w") number = e->BorderRect.W;
        else if (prop == "h") number = e->BorderRect.H;
        else if (prop == "lines") { if (!label) { error = "lines needs a Label"; return false; } number = label->Lines(); }
        else if (prop == "focus")
        {
            // a wrapper (Slider, DropDown, ...) counts as focused while its legacy widget is
            number = 0;
            for (UiElement* f = UiNavigation::Focused; f != nullptr && !UiElem::IsElem(f); f = f->parent)
                if (f->parent == e) number = 1;
            if (UiNavigation::Focused == e) number = 1;
        }
        else if (prop == "hover") number = (e->States & kStateHover) ? 1 : 0;
        else if (prop == "active") number = (e->States & kStateActive) ? 1 : 0;
        else if (prop == "disabled") number = (e->States & kStateDisabled) ? 1 : 0;
        else if (prop == "selected") number = (e->States & kStateSelected) ? 1 : 0;
        else if (prop == "visible") number = e->Computed().Visibility == UiVisibility::Visible ? 1 : 0;
        else if (prop == "activations") number = e->ActivateCount;
        else if (prop.rfind("pulse_", 0) == 0)   // pulse_<name>: 1 while the pulse pseudo-class (:click, :denied, ...) matches
        {
            const std::vector<UiPulseInfo>& pulses = UiPulses();
            const auto found = std::find_if(pulses.begin(), pulses.end(), [&](const UiPulseInfo& p) { return prop == std::string("pulse_") + p.Name; });
            if (found == pulses.end()) { error = "unknown pulse '" + prop + "'"; return false; }
            number = e->PulseActive(static_cast<int>(found - pulses.begin())) ? 1 : 0;
        }
        else if (prop == "opacity_now") number = e->Computed().Opacity;                          // the displayed value, also while it animates
        else if (prop == "animating") number = UiAnimation::IsAnimating(*e) ? 1 : 0;
        else if (prop == "font_size") number = e->Computed().FontSize;
        else if (prop == "line_width") { if (!label) { error = "line_width needs a Label"; return false; } number = label->TextWidth(); }
        else if (prop == "downs" || prop == "moves" || prop == "ups")
        {
            auto* capture = dynamic_cast<UiTestCapturePanel*>(e);
            if (!capture) { error = prop + " needs the capture panel"; return false; }
            number = prop == "downs" ? capture->Downs : (prop == "moves" ? capture->Moves : capture->Ups);
        }
        else if (prop == "value")
        {
            if (auto* sl = dynamic_cast<UiElemSlider*>(e)) number = sl->Value();
            else if (auto* dd = dynamic_cast<UiElemDropDown*>(e)) number = dd->Selected();
            else if (auto* tg = dynamic_cast<UiElemToggleButton*>(e)) number = tg->Checked() ? 1 : 0;
            else if (auto* rb = dynamic_cast<UiElemRadioButton*>(e)) number = rb->Checked() ? 1 : 0;
            else if (auto* pb = dynamic_cast<UiElemProgressBar*>(e)) number = pb->Value();
            else if (auto* cp = dynamic_cast<UiElemCircularProgress*>(e)) number = cp->Value();
            else { error = "value needs a Slider, DropDown, ToggleButton, RadioButton, ProgressBar or CircularProgress"; return false; }
        }
        else if (prop == "checked")
        {
            if (auto* tg = dynamic_cast<UiElemToggleButton*>(e)) number = tg->Checked() ? 1 : 0;
            else if (auto* rb = dynamic_cast<UiElemRadioButton*>(e)) number = rb->Checked() ? 1 : 0;
            else number = (e->States & kStateChecked) ? 1 : 0;
        }
        else if (prop == "fill_w" || prop == "fill_h")
        {
            auto* pb = dynamic_cast<UiElemProgressBar*>(e);
            if (!pb || pb->Fill() == nullptr) { error = prop + " needs a ProgressBar"; return false; }
            number = prop == "fill_w" ? pb->Fill()->BorderRect.W : pb->Fill()->BorderRect.H;
        }
        else if (prop == "scroll_x") number = e->ScrollOffset.x;
        else if (prop == "scroll_y") number = e->ScrollOffset.y;
        else if (prop == "scroll_max_y") number = e->MaxScroll().y;
        else if (prop == "children")
        {
            size_t n = 0;
            for (auto& c : e->children)
                if (UiElem::IsElem(c.get()) && !static_cast<UiElem&>(*c).InternalOverlay) ++n;
            number = static_cast<double>(n);
        }
        else if (prop == "rows") { auto* lv = dynamic_cast<UiElemListView*>(e); if (!lv) { error = "rows needs a ListView"; return false; } number = lv->RowCount(); }
        else if (prop == "entry_caret" || prop == "entry_selection" || prop == "entry_lines" || prop == "entry_editing")
        {
            auto* te = dynamic_cast<UiElemTextInput*>(e);
            if (!te) { error = prop + " needs a TextEntry or MultilineTextEntry"; return false; }
            if (prop == "entry_caret") number = static_cast<double>(te->Caret());
            else if (prop == "entry_selection") number = static_cast<double>(te->SelectionEnd() - te->SelectionStart());
            else if (prop == "entry_lines") number = te->LineCount();
            else number = te->IsEditing() ? 1 : 0;
        }
        else if (prop == "entry_selected")
        {
            auto* te = dynamic_cast<UiElemTextInput*>(e);
            if (!te) { error = "entry_selected needs a TextEntry or MultilineTextEntry"; return false; }
            isNumber = false;
            text = te->SelectedText();
        }
        else if (prop == "entry_text")
        {
            auto* te = dynamic_cast<UiElemTextInput*>(e);
            if (!te) { error = "entry_text needs a TextEntry or MultilineTextEntry"; return false; }
            isNumber = false;
            text = te->Text();
        }
        else if (prop == "src")
        {
            auto* image = dynamic_cast<UiElemImage*>(e);
            if (!image) { error = "src needs an Image"; return false; }
            isNumber = false;
            text = image->Src();
        }
        else if (prop == "text")
        {
            if (!label) { error = "text needs a Label"; return false; }
            isNumber = false;
            text = label->DisplayText();
        }
        else { error = "unknown property '" + prop + "'"; return false; }

        bool pass = false;
        std::string actual;
        if (isNumber)
        {
            const double want = std::atof(value.c_str());
            char buffer[64];
            std::snprintf(buffer, sizeof(buffer), "%g", number);
            actual = buffer;
            if (op == "==") pass = std::fabs(number - want) <= 0.5;
            else if (op == "!=") pass = std::fabs(number - want) > 0.5;
            else if (op == "<") pass = number < want;
            else if (op == "<=") pass = number <= want + 0.001;
            else if (op == ">") pass = number > want;
            else if (op == ">=") pass = number >= want - 0.001;
            else { error = "operator '" + op + "' is not valid for numbers"; return false; }
        }
        else
        {
            actual = text;
            if (op == "==") pass = text == value;
            else if (op == "!=") pass = text != value;
            else if (op == "contains") pass = text.find(value) != std::string::npos;
            else { error = "operator '" + op + "' is not valid for text"; return false; }
        }
        if (!pass)
        {
            error = "expect #" + w[0] + " " + prop + " " + op + " " + value + ": got " + actual;
            return false;
        }
        return true;
    }

    // Mouse simulation (Input::SimulatedMouse). Positions are UI units.
    bool TestMouseMove(const std::string& args, std::string& error)
    {
        const auto w = Words(args);
        if (w.size() < 2) { error = "usage: mouse_move <x> <y>"; return false; }
        TouchEvent t;
        t.position = glm::vec2((float)std::atof(w[0].c_str()), (float)std::atof(w[1].c_str()));
        if (Input::SimulatedMouse.has_value())
            t.delta = t.position - Input::SimulatedMouse->position;
        Input::SimulatedMouse = t;
        UiTestRunner::WaitFrames(2);
        return true;
    }

    bool TestMouseDown(const std::string&, std::string& error)
    {
        if (!Input::SimulatedMouse.has_value()) { error = "mouse_down needs mouse_move first"; return false; }
        Input::SimulatedMouse->pressed = true;
        UiTestRunner::WaitFrames(2);
        return true;
    }

    bool TestMouseUp(const std::string&, std::string& error)
    {
        if (!Input::SimulatedMouse.has_value()) { error = "mouse_up needs mouse_move first"; return false; }
        Input::SimulatedMouse->released = true;
        UiTestRunner::WaitFrames(2);
        return true;
    }

    bool TestClick(const std::string& args, std::string& error)
    {
        if (!TestMouseMove(args, error))
            return false;
        Input::SimulatedMouse->pressed = true;
        g_clickReleaseIn = 3;
        UiTestRunner::WaitFrames(8);
        return true;
    }

    bool TestMouseOff(const std::string&, std::string&)
    {
        Input::SimulatedMouse.reset();
        UiTestRunner::WaitFrames(2);
        return true;
    }

    bool TestSimDevice(const std::string& args, std::string& error)
    {
        if (args == "gamepad") UiInputPrompts::SetLastDevice(UiInputDevice::Gamepad);
        else if (args == "mouse") UiInputPrompts::SetLastDevice(UiInputDevice::KeyboardMouse);
        else if (args == "touch") UiInputPrompts::SetLastDevice(UiInputDevice::Touch);
        else { error = "usage: sim_device gamepad|mouse|touch"; return false; }
        UiTestRunner::WaitFrames(2);
        return true;
    }

    // Installs a provider that needs no game code: white.png for keyboard/mouse, cat.jpg for gamepad, nothing for touch and for actions called none*.
    bool TestPromptProvider(const std::string&, std::string&)
    {
        UiInputPrompts::SetProvider([](const std::string& action, UiInputDevice device) -> std::string {
            if (action.rfind("none", 0) == 0) return "";
            if (device == UiInputDevice::KeyboardMouse) return "GameData/textures/generic/white.png";
            if (device == UiInputDevice::Gamepad) return "GameData/textures/generic/cat.jpg";
            return "";
        });
        return true;
    }

    bool CompareNumber(double actual, const std::string& op, const std::string& value, const std::string& what, std::string& error)
    {
        const double want = std::atof(value.c_str());
        bool pass;
        if (op == "==") pass = std::fabs(actual - want) <= 0.5;
        else if (op == "!=") pass = std::fabs(actual - want) > 0.5;
        else if (op == "<") pass = actual < want;
        else if (op == ">") pass = actual > want;
        else if (op == "<=") pass = actual <= want;
        else if (op == ">=") pass = actual >= want;
        else { error = "operator '" + op + "' is not valid for numbers"; return false; }
        if (!pass)
            error = "expect " + what + " " + op + " " + value + ": got " + std::to_string(actual);
        return pass;
    }

    bool TestExpectEngine(const std::string& args, std::string& error)
    {
        const auto w = Words(args);
        if (w.size() < 3 || w[0] != "paused") { error = "usage: expect_engine paused <op> <value>"; return false; }
        const double paused = EngineMain::MainInstance != nullptr && EngineMain::MainInstance->Paused ? 1 : 0;
        return CompareNumber(paused, w[1], w[2], "engine paused", error);
    }

    bool TestExpectStack(const std::string& args, std::string& error)
    {
        const auto w = Words(args);
        if (w.size() < 3) { error = "usage: expect_stack depth <op> <n> | top contains <text>"; return false; }
        UiMenuStack& stack = UiMenus::Stack();
        if (w[0] == "depth")
            return CompareNumber(stack.Depth(), w[1], w[2], "stack depth", error);
        if (w[0] == "top")
        {
            const std::string top = stack.Top() == nullptr ? "" : stack.Top()->m_layoutPath;
            const std::string value = RestAfter(args, 2);
            const bool pass = w[1] == "contains" ? top.find(value) != std::string::npos : (w[1] == "==" ? top == value : false);
            if (!pass) error = "expect_stack top " + w[1] + " " + value + ": got '" + top + "'";
            return pass;
        }
        error = "unknown stack property '" + w[0] + "'";
        return false;
    }

    bool TestExpectClass(const std::string& args, std::string& error)
    {
        const auto w = Words(args);
        if (w.size() < 3) { error = "usage: expect_class <id> <class> <0/1>"; return false; }
        UiElem* e = w[0] == "@stack" ? &UiMenus::Stack() : Lookup(w[0]);
        if (!e) { error = "no element '" + w[0] + "'"; return false; }
        const bool has = e->HasClass(w[1]);
        if (has != (w[2] != "0")) { error = "expect_class #" + w[0] + " ." + w[1] + ": element " + (has ? "has" : "lacks") + " it"; return false; }
        return true;
    }

    bool TestExpectExists(const std::string& args, std::string& error)
    {
        if (Lookup(args) == nullptr) { error = "no element '" + args + "'"; return false; }
        return true;
    }

    bool TestExpectMissing(const std::string& args, std::string& error)
    {
        if (Lookup(args) != nullptr) { error = "element '" + args + "' exists"; return false; }
        return true;
    }

    bool TestExpectErrors(const std::string& args, std::string& error)
    {
        const int want = std::atoi(args.c_str());
        if (UiMarkupLog::ErrorCount() != want) { error = "expected " + std::to_string(want) + " UI errors, got " + std::to_string(UiMarkupLog::ErrorCount()); return false; }
        return true;
    }

    // backdrop_scene <image path | off>: an image stands in for the scene colour behind the UI, so a backdrop test does not depend on the level.
    bool TestBackdropScene(const std::string& args, std::string& error)
    {
        std::istringstream in(args);
        std::string path;
        in >> path;
        if (path.empty()) { error = "usage: backdrop_scene <image path | off>"; return false; }
        UiLayers::SetSceneOverride(path == "off" ? std::string() : UiActions::NormalizeLayoutPath(path));
        return true;
    }

    uint32_t g_layerMarkRendered = 0, g_layerMarkCached = 0;
    bool TestLayerMark(const std::string&, std::string&)
    {
        g_layerMarkRendered = UiLayers::TotalRendered();
        g_layerMarkCached = UiLayers::TotalCached();
        return true;
    }

    // expect_layers <rendered|cached> <op> <n>: layer renders (or cache hits) since the last layer_mark
    bool TestExpectLayers(const std::string& args, std::string& error)
    {
        std::istringstream in(args);
        std::string what, op;
        long want = 0;
        in >> what >> op >> want;
        const long got = what == "cached" ? static_cast<long>(UiLayers::TotalCached() - g_layerMarkCached)
                                          : static_cast<long>(UiLayers::TotalRendered() - g_layerMarkRendered);
        bool pass = op == "==" ? got == want : op == ">=" ? got >= want : op == "<=" ? got <= want : op == ">" ? got > want : op == "<" ? got < want : false;
        if (!pass)
            error = "expect_layers " + what + " " + op + " " + std::to_string(want) + ": got " + std::to_string(got);
        return pass;
    }

    bool TestExpectWarnings(const std::string& args, std::string& error)
    {
        const int want = std::atoi(args.c_str());
        if (UiMarkupLog::WarningCount() != want) { error = "expected " + std::to_string(want) + " UI warnings, got " + std::to_string(UiMarkupLog::WarningCount()); return false; }
        return true;
    }

    bool TestExpectRoots(const std::string& args, std::string& error)
    {
        const int want = std::atoi(args.c_str());
        int attached = 0;
        for (auto& r : LiveRoots())
            if (IsUnderViewport(r.get()) && r->AsMenuStack() == nullptr)   // the main menu stack always exists
                ++attached;
        if (attached != want) { error = "expected " + std::to_string(want) + " attached roots, got " + std::to_string(attached); return false; }
        return true;
    }

    bool TestSetVar(const std::string& args, std::string& error)
    {
        const auto w = Words(args);
        if (w.size() < 2) { error = "usage: set_var <id> <name> <value>"; return false; }
        UiElem* e = Lookup(w[0]);
        if (!e) { error = "no element '" + w[0] + "'"; return false; }
        e->SetVar(w[1], RestAfter(args, 2));
        return true;
    }

    bool TestSetClass(const std::string& args, std::string& error)
    {
        const auto w = Words(args);
        if (w.size() < 3) { error = "usage: set_class <id> <class> <0/1>"; return false; }
        UiElem* e = Lookup(w[0]);
        if (!e) { error = "no element '" + w[0] + "'"; return false; }
        e->SetClass(w[1], w[2] != "0");
        return true;
    }

    bool TestAnimPause(const std::string& args, std::string&)
    {
        UiAnimation::SetPaused(args != "0");
        return true;
    }

    // anim_seek <id> <seconds>: moves the transitions and animations of the element and everything below it to that time.
    bool TestAnimSeek(const std::string& args, std::string& error)
    {
        const auto w = Words(args);
        if (w.size() < 2) { error = "usage: anim_seek <id> <seconds>"; return false; }
        UiElem* e = Lookup(w[0]);
        if (!e) { error = "no element '" + w[0] + "'"; return false; }
        UiAnimation::Seek(*e, static_cast<float>(std::atof(w[1].c_str())));
        UiTestRunner::WaitFrames(2);
        return true;
    }

    // Makes an element a legacy focus trap, so a test menu is not competing with menus of the running game. Phase 2 replaces this.
    bool TestFocusTrap(const std::string& args, std::string& error)
    {
        const auto w = Words(args);
        if (w.size() < 1) { error = "usage: focus_trap <id>"; return false; }
        UiElem* e = Lookup(w[0]);
        if (!e) { error = "no element '" + w[0] + "'"; return false; }
        e->FocusTrap = true;
        return true;
    }

    // Reads SaveData/UIStudio/uidb.json and checks it is valid JSON of the expected format.
    // ui_overflow_report: every Label whose text does not fit its box (not wrapping: wider; wrapping: taller), or sticks out of a
    // fixed-size parent. Run it with -ui_pseudoloc on the kit pages.
    int RunOverflowReport()
    {
        int found = 0, checked = 0;
        for (auto& w : g_roots)
        {
            auto root = w.lock();
            if (!root || !IsUnderViewport(root.get()))
                continue;
            std::vector<UiElem*> all;
            root->CollectElems(all);
            for (UiElem* e : all)
            {
                UiElemLabel* label = dynamic_cast<UiElemLabel*>(e);
                if (label == nullptr || !e->IsVisible() || e->Computed().Visibility != UiVisibility::Visible || label->Text().empty())
                    continue;
                bool hidden = false;
                for (UiElem* p = e; p != nullptr; p = p->AsParentElem())
                    if (p->Computed().Visibility != UiVisibility::Visible) hidden = true;
                if (hidden)
                    continue;
                ++checked;
                const UiRect box = e->ContentRectLocal();
                const bool wraps = e->Computed().WhiteSpace == UiWhiteSpace::Normal || e->Computed().WhiteSpace == UiWhiteSpace::PreWrap;
                std::string why;
                char buffer[200];
                if (!wraps && label->TextWidth() > box.W + 0.5f)
                {
                    std::snprintf(buffer, sizeof(buffer), "text %.0f px wide, box %.0f px", label->TextWidth(), box.W);
                    why = buffer;
                }
                else if (wraps && label->TextHeight() > box.H + 0.5f && e->Computed().Height.Kind != UiLenKind::FitChildren)
                {
                    std::snprintf(buffer, sizeof(buffer), "text %.0f px tall over %d lines, box %.0f px", label->TextHeight(), label->Lines(), box.H);
                    why = buffer;
                }
                else if (UiElem* parent = e->AsParentElem())
                {
                    const UiRect pc = parent->ContentRectLocal();
                    const bool fixedW = parent->Computed().Width.Kind != UiLenKind::FitChildren;
                    const bool fixedH = parent->Computed().Height.Kind != UiLenKind::FitChildren;
                    if (fixedW && e->BorderRect.X + e->BorderRect.W > pc.X + pc.W + 0.5f)
                        why = "sticks out of its parent on the right";
                    else if (fixedH && e->BorderRect.Y + e->BorderRect.H > pc.Y + pc.H + 0.5f)
                        why = "sticks out of its parent at the bottom";
                }
                if (!why.empty())
                {
                    ++found;
                    Logger::Warning("[UI] %s:%d:%d: warning: overflow: label '%s' %s", e->ScopeFile.c_str(), e->SrcLine, e->SrcCol, label->DisplayText().c_str(), why.c_str());
                }
            }
        }
        Logger::Log("[UI] ui_overflow_report: %d overflowing of %d labels", found, checked);
        return found;
    }

    void CmdOverflowReport(const std::vector<std::string>&) { RunOverflowReport(); }

    bool TestExpectNoOverflow(const std::string&, std::string& error)
    {
        const int found = RunOverflowReport();
        if (found != 0)
        {
            error = "expect_no_overflow: " + std::to_string(found) + " label(s) overflow (see the log)";
            return false;
        }
        return true;
    }

    // type <text>: writes into the text input buffer, as typing would.
    bool TestType(const std::string& args, std::string&)
    {
        Input::TextInputBuffer += RestAfter("x " + args, 1);
        UiTestRunner::WaitFrames(2);
        return true;
    }

    // key <combo>: one key press for the text field that is being edited, e.g. key ctrl+a, key shift+left, key backspace.
    bool TestKey(const std::string& args, std::string& error)
    {
        const auto w = Words(args);
        if (w.empty()) { error = "usage: key <[ctrl+][shift+]name>"; return false; }
        Input::TextKeyEvent k{ SDL_SCANCODE_UNKNOWN, false, false, false };
        std::string rest = w[0];
        for (;;)
        {
            if (rest.rfind("ctrl+", 0) == 0) { k.Ctrl = true; rest.erase(0, 5); }
            else if (rest.rfind("shift+", 0) == 0) { k.Shift = true; rest.erase(0, 6); }
            else if (rest.rfind("alt+", 0) == 0) { k.Alt = true; rest.erase(0, 4); }
            else break;
        }
        static const std::pair<const char*, SDL_Scancode> names[] = {
            { "left", SDL_SCANCODE_LEFT }, { "right", SDL_SCANCODE_RIGHT }, { "up", SDL_SCANCODE_UP }, { "down", SDL_SCANCODE_DOWN },
            { "home", SDL_SCANCODE_HOME }, { "end", SDL_SCANCODE_END }, { "pageup", SDL_SCANCODE_PAGEUP }, { "pagedown", SDL_SCANCODE_PAGEDOWN },
            { "backspace", SDL_SCANCODE_BACKSPACE }, { "delete", SDL_SCANCODE_DELETE }, { "insert", SDL_SCANCODE_INSERT },
            { "a", SDL_SCANCODE_A }, { "c", SDL_SCANCODE_C }, { "x", SDL_SCANCODE_X }, { "v", SDL_SCANCODE_V }, { "y", SDL_SCANCODE_Y }, { "z", SDL_SCANCODE_Z } };
        for (const auto& n : names)
            if (rest == n.first)
                k.Scancode = n.second;
        if (k.Scancode == SDL_SCANCODE_UNKNOWN) { error = "key: unknown key '" + rest + "'"; return false; }
        Input::TextKeyEvents.push_back(k);
        UiTestRunner::WaitFrames(2);
        return true;
    }

    // clipboard <text>: puts text on the system clipboard.
    bool TestClipboard(const std::string& args, std::string&)
    {
        SDL_SetClipboardText(RestAfter("x " + args, 1).c_str());
        return true;
    }

    // wheel <delta>: one mouse wheel event at the simulated mouse position (positive = up).
    bool TestWheel(const std::string& args, std::string& error)
    {
        const auto w = Words(args);
        if (w.empty()) { error = "usage: wheel <delta>"; return false; }
        Input::MouseScrollDelta = static_cast<float>(std::atof(w[0].c_str()));
        UiTestRunner::WaitFrames(2);
        return true;
    }

    // test_settings: registers isolated settings (test.slider, test.toggle, test.choice, test.radio) backed by plain variables.
    double g_testSettingValues[4] = { 0.5, 0.0, 1.0, 0.0 };
    bool TestRegisterSettings(const std::string&, std::string&)
    {
        auto reg = [](const char* key, UiSettings::Kind kind, int slot, double min, double max, double step, std::vector<std::string> choices) {
            UiSettings::Entry e;
            e.Key = key;
            e.Type = kind;
            e.Min = min; e.Max = max; e.Step = step;
            e.Choices = std::move(choices);
            e.Get = [slot]() { return g_testSettingValues[slot]; };
            e.Set = [slot](double v) { g_testSettingValues[slot] = v; };
            UiSettings::Register(e);
        };
        reg("test.slider", UiSettings::Kind::Float, 0, 0.0, 1.0, 0.1, {});
        reg("test.toggle", UiSettings::Kind::Bool, 1, 0.0, 1.0, 1.0, {});
        reg("test.choice", UiSettings::Kind::Choice, 2, 0.0, 2.0, 1.0, { "Low", "Medium", "High" });
        reg("test.radio", UiSettings::Kind::Choice, 3, 0.0, 2.0, 1.0, { "A", "B", "C" });
        return true;
    }

    bool TestSetSetting(const std::string& args, std::string& error)
    {
        const auto w = Words(args);
        if (w.size() < 2) { error = "usage: set_setting <key> <value>"; return false; }
        const UiSettings::Entry* e = UiSettings::Find(w[0]);
        if (!e || !e->Set) { error = "no setting '" + w[0] + "'"; return false; }
        e->Set(std::atof(w[1].c_str()));
        return true;
    }

    bool TestExpectSetting(const std::string& args, std::string& error)
    {
        const auto w = Words(args);
        if (w.size() < 3) { error = "usage: expect_setting <key> <op> <value>"; return false; }
        const UiSettings::Entry* e = UiSettings::Find(w[0]);
        if (!e || !e->Get) { error = "no setting '" + w[0] + "'"; return false; }
        return CompareNumber(e->Get(), w[1], w[2], "setting " + w[0], error);
    }

    bool TestUidbFile(const std::string& args, std::string& error)
    {
        const std::string text = FileSystemEngine::ReadFile(UiDatabase::OutputPath());
        if (text.empty()) { error = "the UI database file is missing or empty"; return false; }
        const nlohmann::json j = nlohmann::json::parse(text, nullptr, false);
        if (j.is_discarded() || j.value("format", "") != "engine-ui-database") { error = "the UI database file is not valid"; return false; }
        for (const std::string& need : Words(args))
        {
            // "tags:Panel" -> registry entry with that name/tag/path exists; "cap:markup" -> capability true
            const size_t colon = need.find(':');
            const std::string key = need.substr(0, colon), value = colon == std::string::npos ? "" : need.substr(colon + 1);
            if (key == "cap")
            {
                if (!j["capabilities"].value(value, false)) { error = "capability '" + value + "' is false"; return false; }
                continue;
            }
            if (key == "nocap")
            {
                if (!j["capabilities"].contains(value) || j["capabilities"].value(value, true)) { error = "capability '" + value + "' is not reported as false"; return false; }
                continue;
            }
            if (key == "unavailable")
            {
                // "unavailable:blur()" -> a paint effect that is listed, flagged unavailable and tied to the layers capability
                bool listed = false;
                for (const auto& entry : j["registries"]["paintEffects"])
                    if (entry.value("name", "") == value)
                    {
                        listed = true;
                        if (entry.value("available", true) || !entry.value("requiresLayer", false)) { error = "effect '" + value + "' is not flagged unavailable"; return false; }
                    }
                if (!listed) { error = "effect '" + value + "' is not listed"; return false; }
                continue;
            }
            if (key == "preview")
            {
                bool listed = false;
                for (const auto& entry : j["previewTargets"])
                    if (entry.value("id", "") == value)
                        listed = true;
                if (!listed) { error = "no preview target '" + value + "'"; return false; }
                continue;
            }
            bool found = false;
            for (const auto& entry : j["registries"][key])
                if (entry.value("name", "") == value || entry.value("tag", "") == value || entry.value("path", "") == value)
                    found = true;
            if (!found) { error = "registry " + key + " has no entry '" + value + "'"; return false; }
        }
        return true;
    }

    bool TestUidbSelfTest(const std::string&, std::string& error)
    {
        return UiDatabase::SelfTest(error);
    }
}

void UiSystem::Init()
{
    g_mainThread = std::this_thread::get_id();
    UiFontLibrary::Init();
    RegisterBuiltins();
    UiComponentRegistry::Discover();
    UiTagRegistry::RegisterCodeBehind("GameData/ui/kit/tests/p2_capture.xml", []() -> std::shared_ptr<UiElem> { return std::make_shared<UiTestCapturePanel>(); });
    UiMenus::Init();

    Console& console = Console::Get();
    console.RegisterCommand({ "ui_load", "ui_load <layout> - loads a markup layout into the viewport", CmdLoad });
    console.RegisterCommand({ "ui_unload_all", "Removes every markup root", CmdUnloadAll });
    console.RegisterCommand({ "ui_dump", "ui_dump [depth] - prints the markup trees", CmdDump });
    UiInspect::RegisterConsoleCommands();
    UiLint::RegisterConsoleCommands();
    console.RegisterCommand({ "ui_overflow_report", "Logs every Label whose text overflows its box (run with -ui_pseudoloc)", CmdOverflowReport });
    console.RegisterCommand({ "ui_focus", "ui_focus <id> - focuses a markup element", CmdFocus });
    console.RegisterCommand({ "ui_push", "ui_push <layout> - pushes a page on the main menu stack", CmdPush });
    console.RegisterCommand({ "ui_clear_stack", "Pops every page of the main menu stack", CmdClearStack });
    console.RegisterCommand({ "ui_pop", "Pops the top page of the main menu stack", CmdPop });
    console.RegisterCommand({ "ui_back", "Runs back on the main menu stack", CmdBack });
    console.RegisterCommand({ "ui_stack", "Prints the main menu stack", CmdStack });
    console.RegisterCommand({ "ui_reduced_motion", "ui_reduced_motion [0|1] - transitions are instant and infinite animations stop", CmdReducedMotion });
    console.RegisterCommand({ "ui_gamethread_test", "ui_gamethread_test <layout> - uses the UI API from the game thread", CmdGameThreadTest });
    console.RegisterCommand({ "ui_db_info", "Prints the UI database state", CmdDbInfo });
    console.RegisterCommand({ "ui_db_path", "ui_db_path <file> - changes the UI database output file", CmdDbPath });

    UiTestRunner::RegisterCommand("anim_pause", TestAnimPause);
    UiTestRunner::RegisterCommand("anim_seek", TestAnimSeek);
    UiTestRunner::RegisterCommand("expect_no_overflow", TestExpectNoOverflow);
    UiTestRunner::RegisterCommand("type", TestType);
    UiTestRunner::RegisterCommand("key", TestKey);
    UiTestRunner::RegisterCommand("clipboard", TestClipboard);
    UiTestRunner::RegisterCommand("wheel", TestWheel);
    UiTestRunner::RegisterCommand("test_settings", TestRegisterSettings);
    UiTestRunner::RegisterCommand("set_setting", TestSetSetting);
    UiTestRunner::RegisterCommand("expect_setting", TestExpectSetting);
    UiTestRunner::RegisterCommand("expect_rect", TestExpectRect);
    UiTestRunner::RegisterCommand("expect", TestExpect);
    UiTestRunner::RegisterCommand("expect_exists", TestExpectExists);
    UiTestRunner::RegisterCommand("expect_missing", TestExpectMissing);
    UiTestRunner::RegisterCommand("expect_ui_errors", TestExpectErrors);
    UiTestRunner::RegisterCommand("expect_ui_warnings", TestExpectWarnings);
    UiTestRunner::RegisterCommand("layer_mark", TestLayerMark);
    UiTestRunner::RegisterCommand("backdrop_scene", TestBackdropScene);
    UiTestRunner::RegisterCommand("expect_layers", TestExpectLayers);
    UiTestRunner::RegisterCommand("expect_roots", TestExpectRoots);
    UiTestRunner::RegisterCommand("set_var", TestSetVar);
    UiTestRunner::RegisterCommand("set_class", TestSetClass);
    UiTestRunner::RegisterCommand("focus_trap", TestFocusTrap);
    UiTestRunner::RegisterCommand("mouse_move", TestMouseMove);
    UiTestRunner::RegisterCommand("mouse_down", TestMouseDown);
    UiTestRunner::RegisterCommand("mouse_up", TestMouseUp);
    UiTestRunner::RegisterCommand("click", TestClick);
    UiTestRunner::RegisterCommand("mouse_off", TestMouseOff);
    UiTestRunner::RegisterCommand("sim_device", TestSimDevice);
    UiTestRunner::RegisterCommand("prompt_provider_test", TestPromptProvider);
    UiTestRunner::RegisterCommand("expect_engine", TestExpectEngine);
    UiTestRunner::RegisterCommand("expect_stack", TestExpectStack);
    UiTestRunner::RegisterCommand("expect_class", TestExpectClass);
    UiTestRunner::RegisterCommand("expect_uidb", TestUidbFile);
    UiTestRunner::RegisterCommand("uidb_selftest", TestUidbSelfTest);

    if (EngineMain::MainInstance != nullptr)
    {
        const auto found = EngineMain::MainInstance->Arguments.find("ui_preview");
        if (found != EngineMain::MainInstance->Arguments.end() && !found->second.empty())
            g_previewPath = found->second[0];
    }

    if (EngineMain::MainInstance != nullptr && EngineMain::MainInstance->Arguments.find("ui_pseudoloc") != EngineMain::MainInstance->Arguments.end())
        g_pseudoLoc = true;

    // Settings of the UI itself (bound by the kit pages, saved with the other settings by the games).
    {
        UiSettings::Entry scale;
        scale.Key = "ui.text_scale"; scale.Type = UiSettings::Kind::Float; scale.Min = 0.75; scale.Max = 1.5; scale.Step = 0.05;
        scale.Label = "${SETTINGS_UI_TEXT_SCALE}";
        scale.Get = []() { return static_cast<double>(UiSystem::TextScale); };
        scale.Set = [](double v) { UiSystem::SetTextScale(static_cast<float>(v)); };
        UiSettings::Register(scale);
        UiSettings::Entry uiScale;
        uiScale.Key = "ui.scale"; uiScale.Type = UiSettings::Kind::Float; uiScale.Min = 0.75; uiScale.Max = 1.5; uiScale.Step = 0.05;
        uiScale.Label = "${SETTINGS_UI_SCALE}";
        uiScale.Get = []() { return static_cast<double>(UiManager::UiScale); };
        uiScale.Set = [](double v) { UiManager::UiScale = static_cast<float>(std::clamp(v, 0.5, 2.0)); };
        UiSettings::Register(uiScale);
        UiSettings::Entry motion;
        motion.Key = "ui.reduced_motion"; motion.Type = UiSettings::Kind::Bool;
        motion.Label = "${SETTINGS_UI_REDUCED_MOTION}";
        motion.Get = []() { return UiSystem::ReducedMotion() ? 1.0 : 0.0; };
        motion.Set = [](double v) { UiSystem::SetReducedMotion(v != 0.0); };
        UiSettings::Register(motion);
    }

    KitInit();

    g_initialised = true;
    UiMenuPreviews::RegisterProviders();
    UiDatabase::SetCapability("markup", true);
    UiDatabase::SetCapability("events", true);
    UiDatabase::SetCapability("menus", true);
    UiDatabase::SetCapability("inputPrompts", true);
    UiDatabase::SetCapability("painting", true);
    UiDatabase::SetCapability("motion", true);
    UiDatabase::SetCapability("layers", true);    // offscreen layers: blur, drop-shadow, group opacity, cache: layer (backdrop-filter is a later step)
    UiDatabase::SetCapability("svg", true);       // .svg in Image, background-image and border-image, rasterised at power-of-two sizes
    UiDatabase::SetCapability("dragDrop", true);
    UiDatabase::SetCapability("contextMenu", true);
    UiDatabase::RefreshFromRegistries();
}

void UiSystem::SetTextScale(float scale)
{
    scale = std::clamp(scale, 0.75f, 1.5f);
    if (scale == TextScale)
        return;
    TextScale = scale;
    for (auto& w : g_roots)
        if (auto root = w.lock())
            root->MarkStyleDirty();
}

bool UiSystem::ReducedMotion() { return UiAnimation::ReducedMotion(); }
void UiSystem::SetReducedMotion(bool reduced) { UiAnimation::SetReducedMotion(reduced); }
bool UiSystem::PseudoLoc() { return g_pseudoLoc; }

std::string UiSystem::PseudoLocalise(const std::string& text)
{
    // a..z and A..Z mapped to accented Latin letters (52 mappings)
    static const uint32_t lower[26] = { 0x00e1, 0x0180, 0x010b, 0x010f, 0x00e9, 0x0192, 0x0121, 0x0127, 0x00ed, 0x0135, 0x0137, 0x0142, 0x1e3f,
                                          0x0144, 0x00f3, 0x1e57, 0x024b, 0x0155, 0x015b, 0x0165, 0x00fa, 0x1e7d, 0x1e83, 0x1e8b, 0x00fd, 0x017e };
    static const uint32_t upper[26] = { 0x00c1, 0x1e02, 0x010a, 0x010e, 0x00c9, 0x1e1e, 0x0120, 0x0126, 0x00cd, 0x0134, 0x0136, 0x0139, 0x1e3e,
                                          0x0143, 0x00d3, 0x1e56, 0x024a, 0x0154, 0x015a, 0x0164, 0x00da, 0x1e7c, 0x1e82, 0x1e8a, 0x00dd, 0x0179 };
    std::string out = "[";
    int braces = 0, tags = 0, letters = 0;
    for (char c : text)
    {
        if (c == '{') ++braces;
        if (c == '<') ++tags;
        const bool skip = braces > 0 || tags > 0;   // {variables} and <tags> stay as they are
        if (!skip && c >= 'a' && c <= 'z') { out += UiEncodeUtf8({ lower[c - 'a'] }); ++letters; }
        else if (!skip && c >= 'A' && c <= 'Z') { out += UiEncodeUtf8({ upper[c - 'A'] }); ++letters; }
        else
        {
            out += c;
            if (!skip && (static_cast<unsigned char>(c) & 0xC0) != 0x80) ++letters;
        }
        if (c == '}' && braces > 0) --braces;
        if (c == '>' && tags > 0) --tags;
    }
    out.append(static_cast<size_t>(std::ceil(static_cast<float>(letters) * 0.35f)), '~');
    out += "]";
    return out;
}

void UiSystem::Shutdown()
{
    UiMenus::Shutdown();
    g_roots.clear();
    g_lastGameThreadRoot.reset();
    UiFontLibrary::Shutdown();
    g_initialised = false;
}

bool UiSystem::IsInitialised() { return g_initialised; }
std::thread::id UiSystem::MainThread() { return g_mainThread; }

int UiSystem::RootCount()
{
    return (int)LiveRoots().size();
}

void UiSystem::NoteGameThread()
{
    if (!g_gameThreadKnown.load())
    {
        g_gameThread = std::this_thread::get_id();
        g_gameThreadKnown.store(true);
    }

    std::string path;
    {
        std::lock_guard<std::mutex> lock(g_pendingMutex);
        path.swap(g_pendingGameThreadTest);
    }
    if (path.empty())
        return;

    // Exercises the whole game-thread API (R3): load, attach, variables, classes and removal of last frame's root.
    auto root = UiElem::Load(path);
    if (root)
    {
        EngineMain::Viewport.AddChild(root);
        root->SetVar("test", "1");
        root->SetClass("gamethread", true);
        root->SetAttribute("class", "gamethread");
    }
    if (g_lastGameThreadRoot)
    {
        EngineMain::Viewport.RemoveChild(g_lastGameThreadRoot);
        g_lastGameThreadRoot.reset();
    }
    g_lastGameThreadRoot = root;
}

void UiSystem::RequestGameThreadTest(const std::string& path)
{
    std::lock_guard<std::mutex> lock(g_pendingMutex);
    g_pendingGameThreadTest = path;
}

void UiSystem::AssertUiThread()
{
#ifndef NDEBUG
    const std::thread::id self = std::this_thread::get_id();
    if (g_initialised && self != g_mainThread && !(g_gameThreadKnown.load() && self == g_gameThread))
    {
        Logger::Error("[UI] the UI API was called from a thread other than the main or game thread");
        assert(false && "UI API used from the wrong thread");
    }
#endif
}

void UiSystem::RegisterRoot(const std::shared_ptr<UiElem>& root)
{
    g_roots.push_back(root);
    root->m_needsScan = true;
    RememberModTimes(*root);
}

void UiSystem::UnregisterRoot(const std::shared_ptr<UiElem>& root)
{
    g_roots.erase(std::remove_if(g_roots.begin(), g_roots.end(), [&](const std::weak_ptr<UiElem>& w) { return w.lock() == root; }), g_roots.end());
}

void UiSystem::QueueEvent(UiElem& element, const std::string& event)
{
    element.TriggerPulse(event);   // at once, so the reaction does not wait for the next frame
    if (!element.HasSubscribers(event))
        return;
    try { g_queuedEvents.emplace_back(std::static_pointer_cast<UiElem>(element.shared_from_this()), event); } catch (...) {}
}

int UiSystem::FrameIndex() { return g_frameCounter; }

void UiSystem::ResetViewport()
{
    UiMenus::Stack().Reset();   // forced and silent: the pause menu and every page above it go with the level
    EngineMain::Viewport.ClearChildren();
    UiDragDrop::Reset();
    g_captures.clear();
    g_queuedEvents.clear();
    UiNavigation::Focused = nullptr;   // dropped, not moved: the elements are gone, nothing is told (no OnUnfocused, no blur / focusout)
    g_lastFocus = nullptr;
    g_lastFocusElem.reset();
    UiMenuStack& stack = UiMenus::Stack();
    if (stack.parent != &EngineMain::Viewport || std::find_if(EngineMain::Viewport.children.begin(), EngineMain::Viewport.children.end(),
            [&](const std::shared_ptr<UiElement>& c) { return c.get() == &stack; }) == EngineMain::Viewport.children.end())
    {
        try { EngineMain::Viewport.AddChild(std::static_pointer_cast<UiElement>(stack.shared_from_this())); } catch (...) {}
    }
}

std::vector<std::shared_ptr<UiElem>> UiSystem::Roots() { return LiveRoots(); }

UiElem* UiSystem::FindById(const std::string& id)
{
    auto inInstance = [&](UiElem& scope) -> UiElem* {
        if (!scope.OwnedInstance)
            return nullptr;
        const auto it = scope.OwnedInstance->Ids.find(id);
        if (it != scope.OwnedInstance->Ids.end())
            if (auto e = it->second.lock())
                return e.get();
        return nullptr;
    };
    for (auto& w : g_roots)
    {
        auto root = w.lock();
        if (!root)
            continue;
        if (UiMenuStack* stack = root->AsMenuStack())
        {
            for (int i = stack->Depth() - 1; i >= 0; --i)
                if (UiElem* found = inInstance(*stack->PageAt(i)))
                    return found;
            for (int i = 0; i < stack->LeavingCount(); ++i)
                if (UiElem* found = inInstance(*stack->LeavingAt(i)))
                    return found;
            continue;
        }
        if (UiElem* found = inInstance(*root))
            return found;
    }
    // Documents loaded into a Frame have their own scope: look through the elements of every root and page.
    auto nested = [&](UiElem& scope) -> UiElem* {
        std::vector<UiElem*> all;
        scope.CollectElems(all);
        for (UiElem* e : all)
            if (e != &scope && dynamic_cast<UiElemFrame*>(e->AsParentElem()) != nullptr)
                if (UiElem* found = inInstance(*e))
                    return found;
        return nullptr;
    };
    for (auto& w : g_roots)
    {
        auto root = w.lock();
        if (!root)
            continue;
        if (UiMenuStack* stack = root->AsMenuStack())
        {
            for (int i = stack->Depth() - 1; i >= 0; --i)
                if (UiElem* found = nested(*stack->PageAt(i)))
                    return found;
            continue;
        }
        if (UiElem* found = nested(*root))
            return found;
    }
    return nullptr;
}

void UiSystem::Update()
{
    if (!g_initialised)
        return;
    UiGpu::CpuTimer cpuTimer(UiGpu::kCpuUpdate);
    ++g_frameCounter;

    if (!g_previewPath.empty() && !g_previewLoaded && g_frameCounter >= 10)
    {
        g_previewLoaded = true;
        auto root = UiElem::Load(g_previewPath);
        if (root)
            EngineMain::Viewport.AddChild(root);
    }

    PollHotReload();

    // Input actions and prompt providers are registered by the game after UiSystem::Init; publish them once they exist.
    if (g_frameCounter % 15 == 0)
    {
        static size_t lastActionCount = 0;
        static bool lastProvider = false;
        static size_t lastDocuments = 0;
        const size_t actionCount = Input::GetActionNames().size();
        const bool provider = UiInputPrompts::HasProvider();
        const size_t documents = UiDocumentCache_Paths().size();   // newly loaded documents bring @keyframes with them
        if (actionCount != lastActionCount || provider != lastProvider || documents != lastDocuments)
        {
            lastActionCount = actionCount;
            lastProvider = provider;
            lastDocuments = documents;
            UiDatabase::RefreshFromRegistries();
        }
    }

    UiMenus::Stack().ClearEntering();
    FlushQueuedEvents();

    if (g_clickReleaseIn > 0 && --g_clickReleaseIn == 0)
    {
        g_clickReleaseIn = -1;
        if (Input::SimulatedMouse.has_value())
            Input::SimulatedMouse->released = true;
    }

    ProcessPointers();
    UiMenus::Update();

    // Motion runs on real time, also while the game is paused; a long frame never jumps an animation by more than 0.1 s.
    {
        static uint64_t lastTicks = 0;
        const uint64_t now = SDL_GetTicks64();
        const float dt = lastTicks == 0 ? 0.f : std::min(static_cast<float>(now - lastTicks) * 0.001f, 0.1f);
        lastTicks = now;
        g_frameDt = dt;
        UiAnimation::Tick(dt);
        UiDragDrop::Tick(dt);
    }

    // Focus is read by selectors, so a change of focus restyles every root.
    UiElement* const focusNow = UiNavigation::Focused;
    const bool focusChanged = focusNow != g_lastFocus;
    if (focusChanged)
    {
        g_lastFocus = focusNow;
        // a legacy widget inside a markup widget (the slider handle) reports to the markup element that holds it
        auto elemOf = [](UiElement* e) -> UiElem* {
            while (e != nullptr && !UiElem::IsElem(e))
                e = e->parent;
            return static_cast<UiElem*>(e);
        };
        const std::shared_ptr<UiElem> lostHold = g_lastFocusElem.lock();   // null when the old element was destroyed with its page
        UiElem* const lost = lostHold.get();
        UiElem* const gained = elemOf(focusNow);
        g_lastFocusElem.reset();
        if (gained != nullptr)
            if (auto self = gained->weak_from_this().lock())
                g_lastFocusElem = std::static_pointer_cast<UiElem>(self);
        if (lost != nullptr && lost != gained)
        {
            lost->Fire("blur");
            for (UiElem* p = lost; p != nullptr; p = p->AsParentElem())
                p->Fire("focusout");
        }
        if (gained != nullptr && gained != lost && focusNow == UiNavigation::Focused)
        {
            gained->Fire("focus");
            for (UiElem* p = gained; p != nullptr; p = p->AsParentElem())
                p->Fire("focusin");
        }
    }

    std::vector<std::shared_ptr<UiElem>> roots = LiveRoots();
    g_roots.erase(std::remove_if(g_roots.begin(), g_roots.end(), [](const std::weak_ptr<UiElem>& w) { return w.expired(); }), g_roots.end());

    for (auto& root : roots)
    {
        if (!IsUnderViewport(root.get()))
        {
            if (root->parent != nullptr && root->m_attachWarned++ == 0)
                UiMarkupLog::Warning(root->ScopeFile, 0, 0, "markup roots must be attached under the viewport in phase 1; this one is not updated");
            continue;
        }
        TickAll(*root);

        // Exactly one input-device class per root, so styles can show keyboard, gamepad or touch variants.
        {
            const char* wanted = UiInputPrompts::DeviceClass(UiInputPrompts::LastDevice());
            for (const char* cls : { "input-mouse", "input-gamepad", "input-touch" })
                root->SetClass(cls, std::string(cls) == wanted);
        }

        glm::vec2 size;
        glm::mat3 base(1.f);
        if (root->parent == &EngineMain::Viewport)
            size = EngineMain::Viewport.GetSize();
        else
        {
            size = root->parent->bottomRight - root->parent->topLeft;
            base = root->parent->worldMatrix;
        }
        if (size != root->m_lastContainer || base != root->m_lastBase)
        {
            root->m_lastContainer = size;
            root->m_lastBase = base;
            root->m_layoutDirty = true;
        }
        if (focusChanged)
            root->m_styleDirty = true;

        // Flags are cleared before the work, so elements that ask for another pass while measuring (an image whose
        // texture is still loading) keep it requested.
        if (root->m_styleDirty)
        {
            root->m_styleDirty = false;
            UiStyleResolver::ResolveRoot(*root);
            root->m_layoutDirty = true;
            root->m_paintDirty = true;
        }
        if (root->m_layoutDirty)
        {
            root->m_layoutDirty = false;
            UiFlowLayout::LayoutRoot(*root, size, base);
            root->m_paintDirty = true;
        }
        else if (root->m_transformDirty)
        {
            UiFlowLayout::PatchTransforms(*root, base);   // only a transform changed: no measuring
            root->m_paintDirty = true;
        }
        root->m_transformDirty = false;
        if (root->m_needsScan)
        {
            root->m_needsScan = false;
            UiActions::Validate(*root);
            FireLoadEvents(*root);
        }
    }
    // Focus moved by gamepad or keyboard: every scroll container above it brings it into view.
    if (focusChanged && focusNow != nullptr && focusNow == UiNavigation::Focused)
        for (UiElement* p = focusNow->parent; p != nullptr; p = p->parent)
            if (UiElem::IsElem(p) && static_cast<UiElem*>(p)->IsScrollContainer())
                static_cast<UiElem*>(p)->ScrollIntoView(*focusNow, 16.f);
    UiImageAtlas::Update();
}

void UiSystem::Finalize()
{
    if (!g_initialised)
        return;
    UiGpu::CpuTimer cpuTimer(UiGpu::kCpuFinalize);
    for (auto& w : g_roots)
    {
        auto root = w.lock();
        if (!root || !IsUnderViewport(root.get()))
            continue;
        // Changes made after Update in this frame (a legacy menu closing, a pointer handler) are resolved before recording,
        // so the frame never shows a tree that is missing them (no one-frame flash).
        if (root->m_styleDirty || root->m_layoutDirty)
        {
            glm::vec2 size;
            glm::mat3 base(1.f);
            if (root->parent == &EngineMain::Viewport)
                size = EngineMain::Viewport.GetSize();
            else
            {
                size = root->parent->bottomRight - root->parent->topLeft;
                base = root->parent->worldMatrix;
            }
            if (root->m_styleDirty)
            {
                root->m_styleDirty = false;
                UiStyleResolver::ResolveRoot(*root);
                root->m_layoutDirty = true;
                root->m_paintDirty = true;
            }
            if (root->m_layoutDirty)
            {
                root->m_layoutDirty = false;
                UiFlowLayout::LayoutRoot(*root, size, base);
                root->m_paintDirty = true;
            }
        }
        if (root->m_paintDirty)
            Record(*root);
    }
    UiDragDrop::RecordGhost();
    UiFontLibrary::FlushUploads();
    UiDatabase::Flush();
}

void UiSystem::RecordSubtree(UiElem& source, UiElem& host, float opacity, glm::vec2 offset)
{
    // Every element paints with its AbsoluteTransform, so the subtree is moved for the length of the recording.
    std::vector<std::pair<UiElem*, glm::mat3>> saved;
    std::vector<UiElem*> stack{ &source };
    glm::mat3 shift(1.f);
    shift[2] = glm::vec3(offset, 1.f);
    while (!stack.empty())
    {
        UiElem* e = stack.back();
        stack.pop_back();
        saved.emplace_back(e, e->AbsoluteTransform);
        e->AbsoluteTransform = shift * e->AbsoluteTransform;
        for (auto& child : e->children)
            if (UiElem::IsElem(child.get()))
                stack.push_back(static_cast<UiElem*>(child.get()));
    }
    host.m_chunk.Clear();
    host.m_chunk.SetLocalPrims(true);
    host.m_legacyMarkers.clear();
    UiPainter painter(host.m_chunk);
    UiRender::LayerSpec spec;
    spec.Id = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(&host));
    painter.PushLayer(spec);
    Paint(source, painter, host, 1.f, ClipInfo());
    painter.PopLayer(opacity, UiBlendMode::Normal);
    UiElem::UiFrameData data{ host.m_chunk, host.m_legacyMarkers };
    host.m_frame = std::make_shared<const UiElem::UiFrameData>(std::move(data));
    for (auto& [e, m] : saved)
        e->AbsoluteTransform = m;
}

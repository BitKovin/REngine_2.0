#include "UiSystem.h"

#include <Console/Console.h>
#include <FileSystem/FileSystem.h>
#include <EngineMain.h>
#include <Input.h>
#include <Logger.hpp>
#include <UI/Elements/UiElem.h>
#include <UI/Elements/UiElemActionBar.h>
#include <UI/Elements/UiElemButton.h>
#include <UI/Elements/UiElemInputPrompt.h>
#include <UI/Elements/UiElemImage.h>
#include <UI/Elements/UiElemLabel.h>
#include <UI/Elements/UiElemPanel.h>
#include <UI/Flow/UiFlowLayout.h>
#include <UI/Markup/UiBuilder.h>
#include <UI/Markup/UiDocument.h>
#include <UI/Markup/UiMarkupLog.h>
#include <UI/Markup/UiTagRegistry.h>
#include <UI/Menus/UiActions.h>
#include <UI/Menus/UiInputPrompts.h>
#include <UI/Menus/UiMenuStack.h>
#include <UI/Preview/UiDatabase.h>
#include <UI/Preview/UiMenuPreviews.h>
#include <UI/Preview/UiPreviewRegistry.h>
#include <UI/Reflection/UiElementRegistry.h>
#include <UI/Render/UiFontLibrary.h>
#include <UI/Style/UiStyleResolver.h>
#include <UI/Test/UiTestRunner.h>
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
    std::map<std::string, uint32_t> g_modTimes;
    uint64_t g_lastPollMs = 0;
    int g_frameCounter = 0;
    std::string g_previewPath;
    bool g_previewLoaded = false;
    UiElement* g_lastFocus = nullptr;

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
        UiTagRegistry::RegisterBuiltin("InputPrompt", &UiElemInputPrompt::Create, TagInfo{ "InputPrompt", "input", "Image of the key or button that triggers an action on the device used last",
            { AttributeInfo{ "action", "string", "", true, "Input action name" } } });
        UiTagRegistry::RegisterBuiltin("ActionBar", &UiElemActionBar::Create, TagInfo{ "ActionBar", "input", "Prompt and label of every hotkey button of the top menu page, plus back", {} });
        UiTagRegistry::RegisterBuiltin("Image", &UiElemImage::Create, TagInfo{ "Image", "image", "Texture drawn with object-fit",
            { AttributeInfo{ "src", "path", "", true, "GameData-relative texture path" } } });
        const AttributeInfo hotkey{ "hotkey", "string", "", false, "Input action that activates the button while its page is on top" };
        const AttributeInfo hotkeyLabel{ "hotkey-label", "text", "", false, "Text shown for the hotkey in an ActionBar (default: the button text)" };
        UiTagRegistry::RegisterBuiltin("Button", &UiElemButton::Create, TagInfo{ "Button", "input", "Focusable, activatable container", { hotkey, hotkeyLabel } });
        UiTagRegistry::RegisterBuiltin("TextButton", &UiElemTextButton::Create, TagInfo{ "TextButton", "input", "Button with a text label",
            { AttributeInfo{ "text", "text", "", false, "Button text" }, hotkey, hotkeyLabel } });
    }

    // ── helpers
    bool IsUnderViewport(const UiElement* e)
    {
        for (const UiElement* p = e->parent; p != nullptr; p = p->parent)
            if (p == &EngineMain::Viewport)
                return true;
        return false;
    }

    void TickAll(UiElem& e)
    {
        if (!e.m_navResolved)
            e.ResolveNavOverrides();
        e.Tick();
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
                element->OnPointerMove(ToLocal(*element, touch.position));
            }
            if (touch.released)
            {
                element->OnPointerUp(ToLocal(*element, touch.position));
                g_captures.erase(id);
            }
        }
        if (Input::MouseScrollDelta != 0.f)
        {
            const auto mouse = Input::TouchActions.find(1);
            if (mouse != Input::TouchActions.end())
            {
                const std::shared_ptr<UiElement> hit = EngineMain::Viewport.GetHitElementUnderPosition(mouse->second.position);
                if (hit && UiElem::IsElem(hit.get()))
                    static_cast<UiElem&>(*hit).OnScroll(Input::MouseScrollDelta);
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
                element->Fire(event);
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

    void RememberModTimes(const UiElem& root)
    {
        for (const std::string& path : root.m_dependencies)
            if (!g_modTimes.count(path))
                g_modTimes[path] = FileSystemEngine::GetFileModificationTime(path);
    }

    // ── paint recording (task 1.13)
    struct ClipInfo { bool Valid = false; glm::mat3 Matrix = glm::mat3(1.f); glm::vec2 Size = glm::vec2(0.f); };

    void Paint(UiElem& e, UiPainter& p, UiElem& root, float parentOpacity, const ClipInfo& clip)
    {
        const UiComputedStyle& s = e.Computed();
        if (s.Visibility != UiVisibility::Visible)
            return;
        const float op = parentOpacity * s.Opacity;
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

        bool anyBorder = false;
        for (int i = 0; i < 4; ++i)
            anyBorder = anyBorder || s.EffectiveBorder(i) > 0.f;
        if (s.BackgroundColor.a > 0.f || anyBorder)
        {
            UiBoxPaint paint;
            paint.Fill = s.BackgroundColor;
            paint.Fill.a *= op;
            paint.Radii = radii;
            paint.BorderWidth = UiEdges{ s.EffectiveBorder(0), s.EffectiveBorder(1), s.EffectiveBorder(2), s.EffectiveBorder(3) };
            for (int i = 0; i < 4; ++i)
            {
                paint.BorderColor[i] = s.BorderColor[i];
                paint.BorderColor[i].a *= op;
            }
            p.Box(UiRect{ 0.f, 0.f, W, H }, paint);
        }

        e.m_paintOpacity = op;
        e.PaintContent(p, e.ContentRectLocal());

        ClipInfo childClip = clip;
        const bool clips = s.Overflow == UiOverflow::Clip;
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
            order.push_back({ UiElem::IsElem(c.get()) ? static_cast<UiElem*>(c.get())->Computed().ZIndex : 0, c.get(), c });
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

    // ── hot reload (task 1.14)
    void ReloadRoot(const std::shared_ptr<UiElem>& old)
    {
        UiElement* parent = old->parent;
        UiMenuStack* stack = UiElem::IsElem(parent) ? static_cast<UiElem*>(parent)->AsMenuStack() : nullptr;
        const std::string path = old->m_layoutPath;
        auto fresh = UiElem::Load(path);
        if (!fresh)
        {
            Logger::Warning("[UI] reload of %s failed; keeping the previous version", path.c_str());
            return;
        }
        if (stack != nullptr)
            UiSystem::UnregisterRoot(fresh);
        for (const auto& [name, value] : old->m_vars)
        {
            const auto num = old->m_numVars.find(name);
            if (num != old->m_numVars.end()) fresh->SetVar(name, num->second);
            else fresh->SetVar(name, value);
        }
        for (const std::string& cls : old->CodeClasses)
            fresh->SetClass(cls, true);
        if (stack != nullptr)
            stack->SwapPage(old.get(), fresh);
        else if (parent != nullptr)
        {
            parent->AddChild(fresh);
            parent->RemoveChild(old);
        }
        RememberModTimes(*fresh);
        Logger::Log("[UI] reloaded %s", path.c_str());
    }

    void PollHotReload()
    {
        const uint64_t now = SDL_GetTicks64();
        if (now - g_lastPollMs < 500)
            return;
        g_lastPollMs = now;

        std::vector<std::string> changed;
        for (auto& [path, stamp] : g_modTimes)
        {
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
        for (auto& root : reloadable)
        {
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
        UiNavigation::SetFocus(e);
        Logger::Log("[UI] ui_focus %s: focused=%d visible=%d hit=%d disableFocus=%d", args[0].c_str(), UiNavigation::Focused == e ? 1 : 0,
                    e->IsVisible() ? 1 : 0, e->HitCheck ? 1 : 0, e->DisableFocus ? 1 : 0);
    }

    void CmdPush(const std::vector<std::string>& args)
    {
        if (args.empty()) { Logger::Log("usage: ui_push <layout path>"); return; }
        UiMenus::Push(UiActions::NormalizeLayoutPath(args[0]));
    }

    void CmdPop(const std::vector<std::string>&) { UiMenus::Pop(); }
    void CmdBack(const std::vector<std::string>&) { UiMenus::Stack().Back(); }

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
        else if (prop == "focus") number = UiNavigation::Focused == e ? 1 : 0;
        else if (prop == "hover") number = (e->States & kStateHover) ? 1 : 0;
        else if (prop == "active") number = (e->States & kStateActive) ? 1 : 0;
        else if (prop == "disabled") number = (e->States & kStateDisabled) ? 1 : 0;
        else if (prop == "selected") number = (e->States & kStateSelected) ? 1 : 0;
        else if (prop == "visible") number = e->Computed().Visibility == UiVisibility::Visible ? 1 : 0;
        else if (prop == "activations") number = e->ActivateCount;
        else if (prop == "font_size") number = e->Computed().FontSize;
        else if (prop == "line_width") { if (!label) { error = "line_width needs a Label"; return false; } number = label->TextWidth(); }
        else if (prop == "downs" || prop == "moves" || prop == "ups")
        {
            auto* capture = dynamic_cast<UiTestCapturePanel*>(e);
            if (!capture) { error = prop + " needs the capture panel"; return false; }
            number = prop == "downs" ? capture->Downs : (prop == "moves" ? capture->Moves : capture->Ups);
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
    console.RegisterCommand({ "ui_focus", "ui_focus <id> - focuses a markup element", CmdFocus });
    console.RegisterCommand({ "ui_push", "ui_push <layout> - pushes a page on the main menu stack", CmdPush });
    console.RegisterCommand({ "ui_pop", "Pops the top page of the main menu stack", CmdPop });
    console.RegisterCommand({ "ui_back", "Runs back on the main menu stack", CmdBack });
    console.RegisterCommand({ "ui_stack", "Prints the main menu stack", CmdStack });
    console.RegisterCommand({ "ui_gamethread_test", "ui_gamethread_test <layout> - uses the UI API from the game thread", CmdGameThreadTest });
    console.RegisterCommand({ "ui_db_info", "Prints the UI database state", CmdDbInfo });
    console.RegisterCommand({ "ui_db_path", "ui_db_path <file> - changes the UI database output file", CmdDbPath });

    UiTestRunner::RegisterCommand("expect_rect", TestExpectRect);
    UiTestRunner::RegisterCommand("expect", TestExpect);
    UiTestRunner::RegisterCommand("expect_exists", TestExpectExists);
    UiTestRunner::RegisterCommand("expect_missing", TestExpectMissing);
    UiTestRunner::RegisterCommand("expect_ui_errors", TestExpectErrors);
    UiTestRunner::RegisterCommand("expect_ui_warnings", TestExpectWarnings);
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

    g_initialised = true;
    UiMenuPreviews::RegisterProviders();
    UiDatabase::SetCapability("markup", true);
    UiDatabase::SetCapability("events", true);
    UiDatabase::SetCapability("menus", true);
    UiDatabase::SetCapability("inputPrompts", true);
    UiDatabase::RefreshFromRegistries();
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
    if (element.EventCalls(event) == nullptr)
        return;
    try { g_queuedEvents.emplace_back(std::static_pointer_cast<UiElem>(element.shared_from_this()), event); } catch (...) {}
}

int UiSystem::FrameIndex() { return g_frameCounter; }

void UiSystem::ResetViewport()
{
    EngineMain::Viewport.ClearChildren();
    g_captures.clear();
    g_queuedEvents.clear();
    UiMenuStack& stack = UiMenus::Stack();
    if (stack.parent != &EngineMain::Viewport || std::find_if(EngineMain::Viewport.children.begin(), EngineMain::Viewport.children.end(),
            [&](const std::shared_ptr<UiElement>& c) { return c.get() == &stack; }) == EngineMain::Viewport.children.end())
    {
        try { EngineMain::Viewport.AddChild(std::static_pointer_cast<UiElement>(stack.shared_from_this())); } catch (...) {}
    }
}

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
            continue;
        }
        if (UiElem* found = inInstance(*root))
            return found;
    }
    return nullptr;
}

void UiSystem::Update()
{
    if (!g_initialised)
        return;
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
        const size_t actionCount = Input::GetActionNames().size();
        const bool provider = UiInputPrompts::HasProvider();
        if (actionCount != lastActionCount || provider != lastProvider)
        {
            lastActionCount = actionCount;
            lastProvider = provider;
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

    // Focus is read by selectors, so a change of focus restyles every root.
    UiElement* const focusNow = UiNavigation::Focused;
    const bool focusChanged = focusNow != g_lastFocus;
    if (focusChanged)
    {
        UiElement* const before = g_lastFocus;
        g_lastFocus = focusNow;
        if (UiElem::IsElem(before)) static_cast<UiElem*>(before)->Fire("blur");
        if (UiElem::IsElem(focusNow) && focusNow == UiNavigation::Focused) static_cast<UiElem*>(focusNow)->Fire("focus");
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
        if (root->m_needsScan)
        {
            root->m_needsScan = false;
            UiActions::Validate(*root);
            FireLoadEvents(*root);
        }
    }
}

void UiSystem::Finalize()
{
    if (!g_initialised)
        return;
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
    UiFontLibrary::FlushUploads();
    UiDatabase::Flush();
}

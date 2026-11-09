#include "UiElem.h"

#include <EngineMain.h>
#include <Input.h>
#include <Logger.hpp>
#include <UI/Markup/UiDocument.h>
#include <UI/Markup/UiMarkupLog.h>
#include <UI/Elements/UiElemPanel.h>
#include <UI/Markup/UiVariables.h>
#include <UI/Menus/UiMenuStack.h>
#include <UI/UiDragDrop.h>
#include <UI/UiNavigation.h>
#include <UI/UiSystem.h>

#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <mutex>
#include <unordered_set>

namespace
{
    // Leaked on purpose: elements owned by other statics are destroyed after this file's statics.
    std::mutex& LiveMutex()
    {
        static std::mutex* mutex = new std::mutex();
        return *mutex;
    }
    std::unordered_set<const UiElement*>& Live()
    {
        static std::unordered_set<const UiElement*>* live = new std::unordered_set<const UiElement*>();
        return *live;
    }

    bool IsFalsey(const std::string& v)
    {
        return v.empty() || v == "0" || v == "false" || v == "False" || v == "FALSE";
    }

    std::vector<std::string> Split(const std::string& s)
    {
        std::vector<std::string> out;
        std::string cur;
        for (char c : s)
        {
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
            {
                if (!cur.empty()) { out.push_back(cur); cur.clear(); }
            }
            else cur += c;
        }
        if (!cur.empty()) out.push_back(cur);
        return out;
    }
}

UiElem::UiElem()
{
    HitCheck = false;
    DisableFocus = false;   // legacy navigation skips the whole subtree of a DisableFocus element; HitCheck keeps containers from being candidates
    {
        std::lock_guard<std::mutex> lock(LiveMutex());
        Live().insert(this);
    }
}

UiElem::~UiElem()
{
    std::lock_guard<std::mutex> lock(LiveMutex());
    Live().erase(this);
}

bool UiElem::IsElem(const UiElement* element)
{
    if (element == nullptr)
        return false;
    std::lock_guard<std::mutex> lock(LiveMutex());
    return Live().count(element) != 0;
}

size_t UiElem::LiveCount()
{
    std::lock_guard<std::mutex> lock(LiveMutex());
    return Live().size();
}

const UiElem* UiElem::AsParentElem() const
{
    return IsElem(parent) ? static_cast<const UiElem*>(parent) : nullptr;
}

UiElem* UiElem::AsParentElem()
{
    return IsElem(parent) ? static_cast<UiElem*>(parent) : nullptr;
}

bool UiElem::IsRoot() const
{
    return AsParentElem() == nullptr;
}

UiElem* UiElem::Root()
{
    UiElem* e = this;
    while (UiElem* p = e->AsParentElem())
        e = p;
    return e;
}

void UiElem::MarkStyleDirty()
{
    UiElem* r = Root();
    r->m_styleDirty = true;
    r->m_layoutDirty = true;
    r->m_paintDirty = true;
}

void UiElem::MarkLayoutDirty()
{
    UiElem* r = Root();
    r->m_layoutDirty = true;
    r->m_paintDirty = true;
}

void UiElem::MarkPaintDirty()
{
    Root()->m_paintDirty = true;
}

void UiElem::AddChild(std::shared_ptr<UiElement> child)
{
    UiSystem::AssertUiThread();
    UiElement::AddChild(child);
    MarkStyleDirty();
    Root()->m_needsScan = true;
}

void UiElem::RemoveChild(std::shared_ptr<UiElement> child)
{
    UiSystem::AssertUiThread();
    UiElement::RemoveChild(child);
    MarkStyleDirty();
}

void UiElem::ClearChildren()
{
    UiSystem::AssertUiThread();
    UiElement::ClearChildren();
    MarkStyleDirty();
}

void UiElem::CollectElems(std::vector<UiElem*>& out)
{
    out.push_back(this);
    for (auto& c : children)
        if (IsElem(c.get()))
            static_cast<UiElem*>(c.get())->CollectElems(out);
}

UiElem* UiElem::FindElem(const std::string& idPath) const
{
    UiSystem::AssertUiThread();
    UiDocumentInstance* scope = OwnedInstance ? OwnedInstance.get() : Instance;
    const UiElem* current = this;
    size_t start = 0;
    UiElem* found = nullptr;
    while (start <= idPath.size())
    {
        size_t slash = idPath.find('/', start);
        if (slash == std::string::npos) slash = idPath.size();
        const std::string part = idPath.substr(start, slash - start);
        if (scope == nullptr)
            break;
        const auto it = scope->Ids.find(part);
        std::shared_ptr<UiElem> element = it == scope->Ids.end() ? nullptr : it->second.lock();
        if (!element)
        {
            found = nullptr;
            break;
        }
        found = element.get();
        current = found;
        scope = found->OwnedInstance ? found->OwnedInstance.get() : found->Instance;
        start = slash + 1;
    }
    (void)current;
    if (found == nullptr)
        UiMarkupLog::Warning(ScopeFile, 0, 0, "Find: no element '" + idPath + "'");
    return found;
}

void UiElem::LogFindTypeMismatch(const std::string& idPath) const
{
    UiMarkupLog::Warning(ScopeFile, 0, 0, "Find: element '" + idPath + "' has a different type");
}

bool UiElem::HasClass(const std::string& cls) const
{
    return std::find(Classes.begin(), Classes.end(), cls) != Classes.end() ||
           std::find(CodeClasses.begin(), CodeClasses.end(), cls) != CodeClasses.end();
}

void UiElem::SetClass(const std::string& cls, bool on)
{
    UiSystem::AssertUiThread();
    auto it = std::find(CodeClasses.begin(), CodeClasses.end(), cls);
    if (on && it == CodeClasses.end())
        CodeClasses.push_back(cls);
    else if (!on && it != CodeClasses.end())
        CodeClasses.erase(it);
    else
        return;
    MarkStyleDirty();
}

UiComputedStyle& UiElem::Style()
{
    UiSystem::AssertUiThread();
    MarkStyleDirty();
    return m_overrides;
}

void UiElem::SetAttribute(const std::string& name, const std::string& value)
{
    UiSystem::AssertUiThread();
    if (m_runtimeAttrs.find(name) == m_runtimeAttrs.end())
    {
        const auto markup = Attributes.find(name);
        m_runtimeAttrs[name] = { markup != Attributes.end(), markup != Attributes.end() ? markup->second : std::string() };
    }
    Attributes[name] = value;
    ApplyAttributeValue(name, value, false);
    MarkStyleDirty();
}

void UiElem::SetVar(const std::string& name, const std::string& value)
{
    UiSystem::AssertUiThread();
    m_runtimeVars.insert(name);
    m_vars[name] = value;
    m_numVars.erase(name);
    std::vector<UiElem*> all;
    CollectElems(all);
    for (UiElem* e : all)
        if (std::find(e->m_varRefs.begin(), e->m_varRefs.end(), name) != e->m_varRefs.end())
            e->ReapplyAttributes();
    MarkStyleDirty();
}

void UiElem::SetVar(const std::string& name, double value)
{
    UiSystem::AssertUiThread();
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%.6g", value);
    m_runtimeVars.insert(name);
    m_vars[name] = std::string(buffer);
    m_numVars[name] = value;   // before the attributes are re-applied, so format specs see the number
    std::vector<UiElem*> all;
    CollectElems(all);
    for (UiElem* e : all)
        if (std::find(e->m_varRefs.begin(), e->m_varRefs.end(), name) != e->m_varRefs.end())
            e->ReapplyAttributes();
    MarkStyleDirty();
}

void UiElem::ReapplyAttributes()
{
    for (const auto& [name, raw] : Attributes)
    {
        if (name == "id" || name == "class" || name == "style")
            continue;
        if (raw.find('{') == std::string::npos && raw.find("${") == std::string::npos)
            continue;
        ApplyAttributeValue(name, raw, false);
    }
    MarkStyleDirty();
}

bool UiElem::ApplyAttributeValue(const std::string& name, const std::string& raw, bool warnUnknown)
{
    (void)warnUnknown;
    if (name == "id")
    {
        Id = raw;
        if (Instance != nullptr && !raw.empty())
        {
            // Exceptions are off in this build: inside a constructor (code-behind) there is no owner yet, Load registers the id later.
            if (auto self = weak_from_this().lock())
                Instance->Ids[raw] = std::static_pointer_cast<UiElem>(self);
        }
        return true;
    }
    if (name == "class")
    {
        Classes = Split(raw);
        MarkStyleDirty();
        return true;
    }
    if (name == "style")
    {
        m_styleAttr = raw;
        MarkStyleDirty();
        return true;
    }
    // on<event>: known events on any element, any on<name> on a component instance (raised with Raise()).
    if (name.size() > 2 && name[0] == 'o' && name[1] == 'n' && (UiActions::IsKnownEvent(name.substr(2)) || !ComponentTag.empty()))
    {
        std::vector<UiActionCall> calls;
        std::string error;
        if (!UiActions::Parse(raw, calls, error))
        {
            UiMarkupLog::Error(ScopeFile, SrcLine, SrcCol, "attribute '" + name + "': " + error);
            m_eventCalls.erase(name.substr(2));
        }
        else
            m_eventCalls[name.substr(2)] = std::move(calls);
        return true;
    }
    if (name == "close-on-back" || name == "shows-below" || name == "pauses-game" || name == "hides-hud" || name == "confirm-changes")
        return true;
    if (name == "nav-up" || name == "nav-down" || name == "nav-left" || name == "nav-right")
    {
        m_navResolved = false;
        return true;
    }

    std::vector<std::string> used;
    const std::string value = UiSubstitute(*this, raw, &used);
    for (const std::string& u : used)
        if (std::find(m_varRefs.begin(), m_varRefs.end(), u) == m_varRefs.end())
            m_varRefs.push_back(u);

    if (name == "visible")
    {
        m_visibleOverrideCollapse = IsFalsey(value);
        MarkStyleDirty();
        return true;
    }
    if (name == "enabled")
    {
        const bool wasDisabled = (States & kStateDisabled) != 0;
        if (IsFalsey(value)) States |= kStateDisabled; else States &= ~kStateDisabled;
        DisableFocus = (States & kStateDisabled) != 0 || (HitCheck && !IsFocusable());
        MarkStyleDirty();
        if (wasDisabled != ((States & kStateDisabled) != 0) && m_loadFired)
            UiSystem::QueueEvent(*this, wasDisabled ? "enabled" : "disabled");
        return true;
    }
    if (name == "selected")
    {
        if (!IsFalsey(value)) States |= kStateSelected; else States &= ~kStateSelected;
        MarkStyleDirty();
        return true;
    }
    if (name == "hittest")
    {
        HitCheck = !IsFalsey(value);
        DisableFocus = (States & kStateDisabled) != 0 || (HitCheck && !IsFocusable());   // a pointer-only surface is not a focus target
        return true;
    }
    if (name == "default-focus" || name == "draggable" || name == "droptarget")
        return true;   // read from Attributes by UiDragDrop
    if (name == "scrollbar")
    {
        if (!IsFalsey(value))
            EnsureScrollbar();
        return true;
    }
    const bool handled = ApplyAttribute(name, value);
    if (handled)
        MarkLayoutDirty();
    return handled;
}

UiRect UiElem::ContentRectLocal() const
{
    const UiComputedStyle& s = m_computed;
    const float inset[4] = { s.EffectiveBorder(0), s.EffectiveBorder(1), s.EffectiveBorder(2), s.EffectiveBorder(3) };
    auto pad = [&](const UiLength& l) { return std::max(0.f, l.Px + l.Percent * 0.01f * BorderRect.W); };
    UiRect r;
    r.X = inset[3] + pad(s.PaddingLeft);
    r.Y = inset[0] + pad(s.PaddingTop);
    r.W = std::max(0.f, BorderRect.W - inset[3] - inset[1] - pad(s.PaddingLeft) - pad(s.PaddingRight));
    r.H = std::max(0.f, BorderRect.H - inset[0] - inset[2] - pad(s.PaddingTop) - pad(s.PaddingBottom));
    return r;
}

UiRect UiElem::ContentRectScreen() const
{
    const UiRect local = ContentRectLocal();
    const glm::vec2 p = TransformPoint(AbsoluteTransform, glm::vec2(local.X, local.Y));
    const glm::vec2 q = TransformPoint(AbsoluteTransform, glm::vec2(local.X + local.W, local.Y + local.H));
    return UiRect{ std::min(p.x, q.x), std::min(p.y, q.y), std::fabs(q.x - p.x), std::fabs(q.y - p.y) };
}

glm::vec2 UiElem::GetSize()
{
    return glm::vec2(BorderRect.W, BorderRect.H);
}

void UiElem::UpdateOffsets()
{
    worldMatrix = AbsoluteTransform;
    size = GetSize();
    topLeft = TransformPoint(AbsoluteTransform, glm::vec2(0.f, 0.f));
    bottomRight = topLeft + size;
    offset = topLeft;
    position = glm::vec2(0.f);
    visible = m_computed.Visibility == UiVisibility::Visible;
    LimitHitTestToBounds = m_computed.Overflow != UiOverflow::Visible;
}

void UiElem::UpdateChildrenOffsets()
{
    const UiRect content = ContentRectScreen();
    for (auto& child : children)
    {
        if (IsElem(child.get()))
            continue;
        child->parentTopLeft = glm::vec2(content.X, content.Y);
        child->parentBottomRight = glm::vec2(content.X + content.W, content.Y + content.H);
        child->parent = this;
        child->UpdateOffsets();
    }
}

UiMenuStack* UiElem::Stack() const
{
    for (const UiElem* p = AsParentElem(); p != nullptr; p = p->AsParentElem())
        if (UiMenuStack* stack = const_cast<UiElem*>(p)->AsMenuStack())
            return stack;
    return nullptr;
}

UiElem* UiElem::PageOf()
{
    for (UiElem* e = this; e != nullptr; e = e->AsParentElem())
    {
        UiElem* p = e->AsParentElem();
        if (p != nullptr && p->AsMenuStack() != nullptr)
            return e;
    }
    return nullptr;
}

UiElem::HostOptions UiElem::ReadHostOptions() const
{
    HostOptions o;
    auto read = [&](const char* name, bool& out) {
        const auto it = Attributes.find(name);
        if (it != Attributes.end())
            out = !IsFalsey(it->second);
    };
    read("close-on-back", o.CloseOnBack);
    read("shows-below", o.ShowsBelow);
    read("pauses-game", o.PausesGame);
    read("hides-hud", o.HidesHud);
    return o;
}

void UiElem::On(const std::string& name, std::function<void()> fn)
{
    On(name, ActionFn([fn = std::move(fn)](UiElem&, const std::vector<std::string>&) { fn(); }));
}

void UiElem::On(const std::string& name, ActionFn fn)
{
    UiSystem::AssertUiThread();
    Handler h;
    h.Fn = std::move(fn);
    m_handlers[name] = std::move(h);
}

void UiElem::On(const std::string& name, std::weak_ptr<void> guard, std::function<void()> fn)
{
    On(name, std::move(guard), ActionFn([fn = std::move(fn)](UiElem&, const std::vector<std::string>&) { fn(); }));
}

void UiElem::On(const std::string& name, std::weak_ptr<void> guard, ActionFn fn)
{
    UiSystem::AssertUiThread();
    Handler h;
    h.Fn = std::move(fn);
    h.Guard = std::move(guard);
    h.Guarded = true;
    m_handlers[name] = std::move(h);
}

const UiElem::Handler* UiElem::FindHandler(const std::string& name) const
{
    const auto it = m_handlers.find(name);
    return it == m_handlers.end() ? nullptr : &it->second;
}

const std::vector<UiActionCall>* UiElem::EventCalls(const std::string& event) const
{
    const auto it = m_eventCalls.find(event);
    return it == m_eventCalls.end() ? nullptr : &it->second;
}

void UiElem::Listen(const std::string& event, EventFn fn)
{
    UiSystem::AssertUiThread();
    Listener l;
    l.Fn = std::move(fn);
    m_listeners[event].push_back(std::move(l));
}

void UiElem::Listen(const std::string& event, std::weak_ptr<void> guard, EventFn fn)
{
    UiSystem::AssertUiThread();
    Listener l;
    l.Fn = std::move(fn);
    l.Guard = std::move(guard);
    l.Guarded = true;
    m_listeners[event].push_back(std::move(l));
}

bool UiElem::HasSubscribers(const std::string& event) const
{
    return m_eventCalls.find(event) != m_eventCalls.end() || m_listeners.find(event) != m_listeners.end();
}

void UiElem::TriggerPulse(const std::string& event)
{
    const std::vector<UiPulseInfo>& pulses = UiPulses();
    for (size_t i = 0; i < pulses.size(); ++i)
    {
        if (event != pulses[i].Event && !(pulses[i].AltEvent != nullptr && event == pulses[i].AltEvent))
            continue;
        const uint64_t expire = SDL_GetTicks64() + static_cast<uint64_t>(std::max(0.f, i < m_pulseSeconds.size() ? m_pulseSeconds[i] : kDefaultPulseSeconds) * 1000.f);
        for (ActivePulse& p : m_pulses)
            if (p.Index == static_cast<int>(i))
            {
                p.Expire = expire;   // already on: it just lasts longer
                return;
            }
        m_pulses.push_back({ static_cast<int>(i), expire, false });
        MarkStyleDirty();
        return;
    }
}

bool UiElem::PulseActive(int index) const
{
    for (const ActivePulse& p : m_pulses)
        if (p.Index == index)
            return true;
    return false;
}

void UiElem::UpdateSoundRules(std::vector<RuleSound>&& now)
{
    if (m_soundsSeeded)
    {
        const bool disabled = (States & kStateDisabled) != 0;
        auto has = [](const std::vector<RuleSound>& list, const std::string& key) {
            return std::any_of(list.begin(), list.end(), [&](const RuleSound& r) { return r.Key == key; });
        };
        auto play = [&](const RuleSound& r, const std::string& path) {
            if (!path.empty() && (!disabled || r.WhileDisabled))
                UiActions::PlaySoundPath(path, r.Volume);
        };
        for (const RuleSound& r : m_soundRules)
            if (!has(now, r.Key))
                play(r, r.Exit);
        for (const RuleSound& r : now)
            if (!has(m_soundRules, r.Key))
                play(r, r.Enter);
    }
    m_soundRules = std::move(now);
    m_soundsSeeded = true;
    for (ActivePulse& p : m_pulses)
        p.Resolved = true;
}

void UiElem::Fire(const std::string& event)
{
    TriggerPulse(event);
    FireSilent(event);
}

void UiElem::FireSilent(const std::string& event)
{
    const auto calls = m_eventCalls.find(event);
    const auto listeners = m_listeners.find(event);
    if (calls == m_eventCalls.end() && listeners == m_listeners.end())
        return;
    // A handler may remove this element (Back, Pop); the copies and the keep-alive make that safe.
    const std::vector<UiActionCall> callsCopy = calls != m_eventCalls.end() ? calls->second : std::vector<UiActionCall>();
    const std::vector<Listener> listenersCopy = listeners != m_listeners.end() ? listeners->second : std::vector<Listener>();
    std::shared_ptr<UiElement> keep;
    try { keep = shared_from_this(); } catch (...) {}
    for (const UiActionCall& call : callsCopy)
        UiActions::Dispatch(*this, nullptr, call);
    for (const Listener& l : listenersCopy)
        if (!l.Guarded || !l.Guard.expired())
            l.Fn(*this);
}

void UiElem::Activate()
{
    ++ActivateCount;
    OnActivate();
    UiSystem::QueueEvent(*this, "activate");
}

void UiElem::OnNavCancel()
{
    if (IsElem(UiNavigation::Focused) && static_cast<UiElem*>(UiNavigation::Focused)->OnFocusedCancel())
        return;
    if (UiDragDrop::NavCancel())
        return;
    UiElem* p = AsParentElem();
    if (p != nullptr && p->AsMenuStack() != nullptr)
        p->AsMenuStack()->Back();
}

void UiElem::ResolveNavOverrides()
{
    m_navResolved = true;
    struct Dir { const char* Attr; std::weak_ptr<UiElement> UiElement::* Field; };
    const Dir dirs[] = { { "nav-up", &UiElement::NavUp }, { "nav-down", &UiElement::NavDown }, { "nav-left", &UiElement::NavLeft }, { "nav-right", &UiElement::NavRight } };
    for (const Dir& d : dirs)
    {
        const auto it = Attributes.find(d.Attr);
        if (it == Attributes.end() || it->second.empty())
            continue;
        std::string target = it->second;
        if (target[0] == '#')
            target.erase(0, 1);
        if (UiElem* found = FindElem(target))
        {
            try { this->*d.Field = found->shared_from_this(); } catch (...) {}
        }
    }
}

void UiElem::ApplyPointerStates()
{
    // a page that is animating out does not react to the pointer any more
    for (const UiElem* p = this; p != nullptr; p = p->AsParentElem())
        if (p->States & kStateLeaving)
        {
            if (States & (kStateHover | kStateActive))
            {
                States &= ~(kStateHover | kStateActive);
                MarkStyleDirty();
            }
            return;
        }
    const uint32_t before = States;
    const bool disabled = (States & kStateDisabled) != 0;
    if (TouchEvents.empty())
    {
        States &= ~kStateHover;
        if (m_pressedTouchId != -1)
        {
            // the pointer left while held: the press is over, and it is no click
            m_pressedTouchId = -1;
            States &= ~kStateActive;
            UiSystem::QueueEvent(*this, "clickend");
        }
    }
    else
    {
        States |= kStateHover;
        for (const TouchEvent& touch : TouchEvents)
        {
            if (touch.pressed)
            {
                m_pressedTouchId = touch.id;
                States |= kStateActive;
                UiSystem::QueueEvent(*this, disabled ? "denied" : "clickstart");
            }
            else if (touch.released && touch.id == m_pressedTouchId)
            {
                m_pressedTouchId = -1;
                States &= ~kStateActive;
                UiSystem::QueueEvent(*this, "clickend");
                if (!SuppressActivate && !disabled)
                {
                    UiSystem::QueueEvent(*this, "click");
                    const uint64_t now = SDL_GetTicks64();
                    if (m_lastClickTicks != 0 && now - m_lastClickTicks <= 350 && glm::length(touch.position - m_lastClickPos) <= 8.f)
                    {
                        m_lastClickTicks = 0;
                        UiSystem::QueueEvent(*this, "doubleclick");
                    }
                    else
                    {
                        m_lastClickTicks = now;
                        m_lastClickPos = touch.position;
                    }
                }
                if (IsFocusable() && !disabled && !SuppressActivate)
                    Activate();
            }
        }
    }
    if ((States ^ before) & kStateHover)
        UiSystem::QueueEvent(*this, (States & kStateHover) ? "mouseover" : "mouseout");
    if (States != before)
        MarkStyleDirty();
}

// ── scrolling

glm::vec2 UiElem::MaxScroll() const
{
    return glm::vec2(std::max(0.f, ScrollExtent.x - ScrollViewport.x), std::max(0.f, ScrollExtent.y - ScrollViewport.y));
}

void UiElem::ScrollTo(glm::vec2 offset, bool smooth)
{
    const glm::vec2 max = MaxScroll();
    offset = glm::clamp(offset, glm::vec2(0.f), max);
    ScrollTarget = offset;
    if (!smooth && ScrollOffset != offset)
    {
        ScrollOffset = offset;
        Root()->m_transformDirty = true;
        MarkPaintDirty();
    }
}

void UiElem::ScrollBy(glm::vec2 delta, bool smooth)
{
    ScrollTo((smooth ? ScrollTarget : ScrollOffset) + delta, smooth);
}

void UiElem::TickScroll(float dt)
{
    if (!IsScrollContainer())
        return;
    const glm::vec2 max = MaxScroll();
    ScrollTarget = glm::clamp(ScrollTarget, glm::vec2(0.f), max);
    glm::vec2 next = ScrollOffset;
    if (m_dragging)
    {
        next = ScrollTarget;   // direct while the pointer holds the content
    }
    else
    {
        if (glm::length(m_dragVelocity) > 1.f)
        {
            // kinetic decay after a drag release: 5/s
            ScrollTarget = glm::clamp(ScrollTarget + m_dragVelocity * dt, glm::vec2(0.f), max);
            m_dragVelocity *= std::exp(-5.f * dt);
            next = ScrollTarget;
        }
        else
        {
            m_dragVelocity = glm::vec2(0.f);
            // exponential approach, rate 15/s
            const float k = 1.f - std::exp(-15.f * dt);
            next = ScrollOffset + (ScrollTarget - ScrollOffset) * k;
            if (glm::length(ScrollTarget - next) < 0.25f)
                next = ScrollTarget;
        }
    }
    if (next != ScrollOffset)
    {
        ScrollOffset = next;
        Root()->m_transformDirty = true;
        MarkPaintDirty();
        UiSystem::QueueEvent(*this, "scroll");
    }
}

UiElem* UiElem::NearestScrollContainer()
{
    for (UiElem* e = this; e != nullptr; e = e->AsParentElem())
        if (e->IsScrollContainer())
            return e;
    return nullptr;
}

void UiElem::ScrollIntoView(const UiElement& target, float margin)
{
    if (!IsScrollContainer())
        return;
    const UiRect view = ContentRectScreen();
    glm::vec2 delta(0.f);
    const float top = target.topLeft.y, bottom = target.topLeft.y + target.size.y;
    const float left = target.topLeft.x, right = target.topLeft.x + target.size.x;
    const float m = std::min(margin, std::max(0.f, view.H * 0.5f - 1.f));
    if (top < view.Y + m) delta.y = top - (view.Y + m);
    else if (bottom > view.Y + view.H - m) delta.y = bottom - (view.Y + view.H - m);
    const float mx = std::min(margin, std::max(0.f, view.W * 0.5f - 1.f));
    if (left < view.X + mx) delta.x = left - (view.X + mx);
    else if (right > view.X + view.W - mx) delta.x = right - (view.X + view.W - mx);
    if (delta != glm::vec2(0.f))
        ScrollTo(ScrollOffset + delta, true);
}

void UiElem::OnScroll(float delta)
{
    UiElem* c = NearestScrollContainer();
    if (c == nullptr)
        return;
    // Wheel up (positive) moves the content down, i.e. the offset decreases. Containers that cannot scroll that way pass it on.
    for (; c != nullptr; c = c->AsParentElem() != nullptr ? c->AsParentElem()->NearestScrollContainer() : nullptr)
    {
        const glm::vec2 max = c->MaxScroll();
        const bool vertical = max.y > 0.f || max.x <= 0.f;
        const glm::vec2 step = vertical ? glm::vec2(0.f, -delta * 60.f) : glm::vec2(-delta * 60.f, 0.f);
        const glm::vec2 before = c->ScrollTarget;
        c->ScrollBy(step, true);
        if (c->ScrollTarget != before)
            return;
    }
}

void UiElem::OnPointerDown(glm::vec2 local)
{
    SuppressActivate = false;
    m_dragTarget = NearestScrollContainer();
    if (m_dragTarget == nullptr)
        return;
    const glm::vec2 screen = TransformPoint(AbsoluteTransform, local);
    m_dragStart = m_dragLast = screen;
    m_dragTarget->m_dragVelocity = glm::vec2(0.f);
    m_dragMoved = false;
    m_barDrag = false;
    float grab = 0.f;
    if (BarHit(*m_dragTarget, screen, grab))
    {
        // a press on the scrollbar moves the thumb, not the content under it
        m_barDrag = true;
        m_barGrab = grab;
        m_dragMoved = true;
        SuppressActivate = true;
        m_dragTarget->m_dragging = true;
        BarDragTo(*m_dragTarget, screen);
    }
}

// Is the point on the visible scrollbar track of `container`? `grab` is where inside the thumb it is held
// (the middle of the thumb when the press is on the track beside it, so the thumb jumps under the pointer).
bool UiElem::BarHit(UiElem& container, glm::vec2 screen, float& grab) const
{
    UiElem* bar = container.ScrollbarElem();
    if (bar == nullptr || container.MaxScroll().y <= 0.5f || bar->BorderRect.W <= 0.f || bar->BorderRect.H <= 0.f)
        return false;
    const glm::vec2 p = TransformPoint(glm::inverse(container.AbsoluteTransform), screen);
    const float slop = 4.f;   // the bar is thin: a few pixels either side still count
    if (p.x < bar->BorderRect.X - slop || p.x > bar->BorderRect.X + bar->BorderRect.W + slop ||
        p.y < bar->BorderRect.Y || p.y > bar->BorderRect.Y + bar->BorderRect.H)
        return false;
    grab = 0.f;
    for (auto& c : bar->children)
        if (IsElem(c.get()))
        {
            const UiRect& t = static_cast<UiElem&>(*c).BorderRect;
            const float y = p.y - bar->BorderRect.Y;
            grab = (y >= t.Y && y <= t.Y + t.H) ? y - t.Y : t.H * 0.5f;
        }
    return true;
}

void UiElem::BarDragTo(UiElem& container, glm::vec2 screen)
{
    UiElem* bar = container.ScrollbarElem();
    if (bar == nullptr)
        return;
    float thumbH = 0.f;
    for (auto& c : bar->children)
        if (IsElem(c.get()))
            thumbH = static_cast<UiElem&>(*c).BorderRect.H;
    const float range = bar->BorderRect.H - thumbH;
    const float maxY = container.MaxScroll().y;
    if (range <= 0.f || maxY <= 0.f)
        return;
    const glm::vec2 p = TransformPoint(glm::inverse(container.AbsoluteTransform), screen);
    const float t = std::clamp((p.y - bar->BorderRect.Y - m_barGrab) / range, 0.f, 1.f);
    container.ScrollTarget.y = t * maxY;
    container.m_dragVelocity = glm::vec2(0.f);
}

void UiElem::OnPointerMove(glm::vec2 local)
{
    UiElem* c = m_dragTarget;
    if (c == nullptr || !IsElem(c))
        return;
    const glm::vec2 screen = TransformPoint(AbsoluteTransform, local);
    if (m_barDrag)
    {
        BarDragTo(*c, screen);
        return;
    }
    if (!m_dragMoved)
    {
        if (glm::length(screen - m_dragStart) < 8.f)
            return;
        m_dragMoved = true;
        SuppressActivate = true;
        c->m_dragging = true;
    }
    const glm::vec2 move = m_dragLast - screen;
    m_dragLast = screen;
    const glm::vec2 max = c->MaxScroll();
    glm::vec2 axisMove = move;
    if (max.y > 0.f && max.x <= 0.f) axisMove.x = 0.f;
    if (max.x > 0.f && max.y <= 0.f) axisMove.y = 0.f;
    c->ScrollTarget = glm::clamp(c->ScrollTarget + axisMove, glm::vec2(0.f), max);
    c->m_dragVelocity = axisMove * 30.f;   // px/s estimate for the kinetic release
}

void UiElem::OnPointerUp(glm::vec2)
{
    UiElem* c = m_dragTarget;
    m_dragTarget = nullptr;
    m_barDrag = false;
    if (c != nullptr && IsElem(c))
        c->m_dragging = false;
    if (!m_dragMoved)
        SuppressActivate = false;
    m_dragMoved = false;
}

void UiElem::EnsureScrollbar()
{
    if (m_scrollbar != nullptr)
        return;
    auto bar = std::make_shared<UiElemPanel>();
    bar->TagName = "Panel";
    bar->InternalOverlay = true;
    bar->Classes = { "scrollbar" };
    bar->ScopeFile = ScopeFile;
    bar->Instance = Instance;
    auto thumb = std::make_shared<UiElemPanel>();
    thumb->TagName = "Panel";
    thumb->InternalOverlay = true;
    thumb->Classes = { "thumb" };
    thumb->ScopeFile = ScopeFile;
    thumb->Instance = Instance;
    bar->AddChild(thumb);
    m_scrollbar = bar.get();
    AddChild(bar);
}

void UiElem::Update()
{
    if (!(UiElement::EditModeActive && EngineMain::MainInstance != nullptr && EngineMain::MainInstance->DebugUiEnabled))
        ApplyPointerStates();
    if (!m_pulses.empty())
    {
        const uint64_t now = SDL_GetTicks64();
        const size_t before = m_pulses.size();
        m_pulses.erase(std::remove_if(m_pulses.begin(), m_pulses.end(), [&](const ActivePulse& p) { return p.Resolved && now >= p.Expire; }), m_pulses.end());
        if (m_pulses.size() != before)
            MarkStyleDirty();
    }
    UiElement::Update();
}

void UiElem::OnNavConfirm()
{
    if (UiDragDrop::NavConfirm(*this))
        return;
    if (!IsFocusable())
        return;
    if (States & kStateDisabled)
        UiSystem::QueueEvent(*this, "denied");
    else
        Activate();
}

void UiElem::Draw()
{
    if (!IsRoot())
        return;
    std::shared_ptr<const UiFrameData> frame = m_frame;   // keeps the snapshot and its legacy children alive for this call
    if (!frame || !UiRenderer::InPass())
        return;

    size_t start = 0;
    for (const LegacyMarker& marker : frame->Markers)
    {
        UiRenderer::CurrentList().AppendResolved(frame->List, start, marker.ItemIndex, UiRenderer::CurrentMaskDepth());
        if (marker.Clipped)
            UiRenderer::PushMask(marker.ClipMatrix, marker.ClipSize);
        marker.Child->Draw();
        if (marker.Clipped)
            UiRenderer::PopMask();
        start = marker.ItemIndex;
    }
    UiRenderer::CurrentList().AppendResolved(frame->List, start, frame->List.Items().size(), UiRenderer::CurrentMaskDepth());
}

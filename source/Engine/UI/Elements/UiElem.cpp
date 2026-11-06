#include "UiElem.h"

#include <EngineMain.h>
#include <Input.h>
#include <Logger.hpp>
#include <UI/Markup/UiDocument.h>
#include <UI/Markup/UiMarkupLog.h>
#include <UI/Markup/UiVariables.h>
#include <UI/Menus/UiMenuStack.h>
#include <UI/UiNavigation.h>
#include <UI/UiSystem.h>

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
    Attributes[name] = value;
    ApplyAttributeValue(name, value, false);
    MarkStyleDirty();
}

void UiElem::SetVar(const std::string& name, const std::string& value)
{
    UiSystem::AssertUiThread();
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
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%.6g", value);
    SetVar(name, std::string(buffer));
    m_numVars[name] = value;
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
    if (name == "close-on-back" || name == "shows-below" || name == "pauses-game" || name == "hides-hud")
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
        if (IsFalsey(value)) States |= kStateDisabled; else States &= ~kStateDisabled;
        DisableFocus = (States & kStateDisabled) != 0 || (HitCheck && !IsFocusable());
        MarkStyleDirty();
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
    if (name == "default-focus")
        return true;
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
    LimitHitTestToBounds = m_computed.Overflow == UiOverflow::Clip;
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

void UiElem::Fire(const std::string& event)
{
    const auto it = m_eventCalls.find(event);
    if (it == m_eventCalls.end())
        return;
    // A handler may remove this element (Back, Pop); the copy and the keep-alive make that safe.
    const std::vector<UiActionCall> calls = it->second;
    std::shared_ptr<UiElement> keep;
    try { keep = shared_from_this(); } catch (...) {}
    for (const UiActionCall& call : calls)
        UiActions::Dispatch(*this, nullptr, call);
}

void UiElem::Activate()
{
    ++ActivateCount;
    OnActivate();
    UiSystem::QueueEvent(*this, "activate");
}

void UiElem::OnNavCancel()
{
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
    const uint32_t before = States;
    if (TouchEvents.empty())
    {
        States &= ~kStateHover;
        if (m_pressedTouchId != -1)
        {
            m_pressedTouchId = -1;
            States &= ~kStateActive;
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
            }
            else if (touch.released && touch.id == m_pressedTouchId)
            {
                m_pressedTouchId = -1;
                States &= ~kStateActive;
                if (IsFocusable() && !(States & kStateDisabled))
                    Activate();
            }
        }
    }
    if ((States ^ before) & kStateHover)
        UiSystem::QueueEvent(*this, (States & kStateHover) ? "mouseover" : "mouseout");
    if (States != before)
        MarkStyleDirty();
}

void UiElem::Update()
{
    if (!(UiElement::EditModeActive && EngineMain::MainInstance != nullptr && EngineMain::MainInstance->DebugUiEnabled))
        ApplyPointerStates();
    UiElement::Update();
}

void UiElem::OnNavConfirm()
{
    if (IsFocusable() && !(States & kStateDisabled))
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

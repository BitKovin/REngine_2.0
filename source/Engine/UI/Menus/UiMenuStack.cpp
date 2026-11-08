#include "UiMenuStack.h"

#include <EngineMain.h>
#include <Input.h>
#include <UI/Kit/UiSettings.h>
#include <UI/Markup/UiDocument.h>
#include <UI/Markup/UiMarkupLog.h>
#include <UI/Style/UiCssParser.h>
#include <UI/UiNavigation.h>
#include <UI/UiSystem.h>

#include <SDL2/SDL.h>
#include <algorithm>

namespace
{
    std::shared_ptr<UiMenuStack> g_stack;
    int g_lastBackFrame = -1000;

    UiElem* FindDefaultFocus(UiElem& page)
    {
        std::vector<UiElem*> all;
        page.CollectElems(all);
        for (UiElem* e : all)
        {
            const auto it = e->Attributes.find("default-focus");
            if (it != e->Attributes.end() && (it->second == "true" || it->second == "1"))
                return e;
        }
        return nullptr;
    }
}

UiMenuStack::UiMenuStack()
{
    TagName = "MenuStack";
    HitCheck = false;
    Style().Width = UiLength{ UiLenKind::Percent, 0.f, 100.f };
    Style().Height = UiLength{ UiLenKind::Percent, 0.f, 100.f };
}

UiElem* UiMenuStack::Top() const
{
    return m_entries.empty() ? nullptr : m_entries.back().Page.get();
}

UiElem* UiMenuStack::PageAt(int index) const
{
    return index >= 0 && index < (int)m_entries.size() ? m_entries[index].Page.get() : nullptr;
}

bool UiMenuStack::Contains(const UiElem* page) const
{
    for (const Entry& e : m_entries)
        if (e.Page.get() == page)
            return true;
    return false;
}

namespace
{
    bool SheetHasLeavingRule(const std::shared_ptr<UiStyleSheet>& sheet, int depth)
    {
        if (!sheet || depth > 8)
            return false;
        for (const UiCssRule& rule : sheet->Rules)
            for (const UiSelector& sel : rule.Selectors)
                for (const UiCompound& part : sel.Parts)
                    if (std::find(part.Pseudos.begin(), part.Pseudos.end(), UiPseudo::Leaving) != part.Pseudos.end())
                        return true;
        for (const std::string& import : sheet->Imports)
            if (SheetHasLeavingRule(UiStyleSheetCache::Get(import), depth + 1))
                return true;
        return false;
    }

    // True when the page's style sheets have a :leaving rule, so popping it has a leave effect worth waiting for. The hover and
    // focus fades of its buttons do not count: they would only keep a closed menu on screen for their duration.
    bool HasLeaveMotion(const UiElem& page)
    {
        if (!page.OwnedInstance || !page.OwnedInstance->Def)
            return false;
        for (const auto& sheet : page.OwnedInstance->Def->Sheets)
            if (SheetHasLeavingRule(sheet, 0))
                return true;
        return false;
    }
}

void UiMenuStack::BringToFront()
{
    // Menus draw above whatever the game added to the viewport since the stack was created.
    UiElement* host = parent;
    if (host == nullptr || host->children.empty() || host->children.back().get() == this)
        return;
    std::shared_ptr<UiElement> self;
    try { self = shared_from_this(); } catch (...) { return; }
    host->RemoveChild(self);
    host->AddChild(self);
}

UiElem* UiMenuStack::Push(const std::string& layoutPath)
{
    std::shared_ptr<UiElem> page = UiElem::Load(UiActions::NormalizeLayoutPath(layoutPath));
    if (!page)
        return nullptr;
    UiSystem::UnregisterRoot(page);   // the stack owns, updates and paints its pages
    return Push(std::move(page));
}

UiElem* UiMenuStack::Push(std::shared_ptr<UiElem> page)
{
    UiSystem::AssertUiThread();
    if (!page)
        return nullptr;
    if (Contains(page.get()))
        return page.get();

    Entry entry;
    if (UiNavigation::Focused != nullptr)
    {
        try { entry.SavedFocus = UiNavigation::Focused->shared_from_this(); } catch (...) {}
    }
    entry.Page = page;
    entry.Options = page->ReadHostOptions();
    entry.PushedGen = m_styleGen;
    {
        const auto confirm = page->Attributes.find("confirm-changes");
        if (confirm != page->Attributes.end() && (confirm->second == "true" || confirm->second == "1"))
            entry.Session = UiSettings::BeginSession();
    }

    if (m_entries.empty())
        BringToFront();
    UiElem* previous = Top();

    AddChild(page);
    page->AddClass("page");
    page->States |= kStateEntering;
    m_entries.push_back(entry);
    Relayer();

    if (m_focusPending)
        m_focusPending = false;
    FocusPage(*page);
    page->OnPushed();
    page->Fire("pushed");
    if (previous != nullptr)
    {
        previous->OnHidden();
        previous->Fire("hidden");
    }
    RecomputeHostState();
    return page.get();
}

void UiMenuStack::PopInternal()
{
    Entry entry = std::move(m_entries.back());
    m_entries.pop_back();
    if (entry.Session >= 0)
        UiSettings::EndSession(entry.Session);
    UiElem& page = *entry.Page;
    page.States |= kStateLeaving;   // the page stays while its leave transition or animation runs

    UiElement* focused = UiNavigation::Focused;
    if (focused != nullptr)
    {
        for (UiElement* p = focused; p != nullptr; p = p->parent)
            if (p == &page)
            {
                UiNavigation::ClearFocus();
                break;
            }
    }
    page.FocusTrap = false;
    if (HasLeaveMotion(page))
    {
        page.DisableFocus = true;   // the legacy navigation skips the whole page
        Leaving leaving;
        leaving.Page = entry.Page;
        leaving.PoppedAt = SDL_GetTicks64();
        m_leaving.push_back(std::move(leaving));
    }
    else
    {
        RemoveChild(entry.Page);
        page.parent = nullptr;
    }
    page.OnPopped();
    page.Fire("popped");

    Relayer();
    if (UiElem* top = Top())
    {
        top->OnShown();
        top->Fire("shown");
        RestoreFocus(entry.SavedFocus, *top);
    }
    RecomputeHostState();
}

void UiMenuStack::Pop()
{
    UiSystem::AssertUiThread();
    if (m_entries.empty())
        return;
    std::shared_ptr<UiElem> keep = m_entries.back().Page;
    PopInternal();
}

void UiMenuStack::PopTo(UiElem* page)
{
    if (!Contains(page))
        return;
    while (!m_entries.empty() && Top() != page)
        Pop();
}

void UiMenuStack::Replace(const std::string& layoutPath)
{
    if (!m_entries.empty())
        Pop();
    Push(layoutPath);
}

void UiMenuStack::Clear()
{
    while (!m_entries.empty())
        Pop();
}

void UiMenuStack::Reset()
{
    UiSystem::AssertUiThread();
    for (Entry& e : m_entries)
    {
        if (e.Session >= 0)
            UiSettings::EndSession(e.Session);
        RemoveChild(e.Page);
        e.Page->parent = nullptr;
    }
    m_entries.clear();
    ClearLeaving();
    m_focusPending = false;
    m_focusPendingRestore = false;
    m_focusTarget.reset();
    m_paused = false;   // no OnPauseChanged: nothing is announced, the level change sets the game state itself
    m_hidesHud = false;
}

void UiMenuStack::Back()
{
    UiSystem::AssertUiThread();
    UiMenus::NoteBack();
    if (m_entries.empty())
    {
        if (OnEmptyBack)
            OnEmptyBack();
        return;
    }
    std::shared_ptr<UiElem> top = m_entries.back().Page;
    top->Fire("cancel");
    if (m_entries.empty() || m_entries.back().Page != top)
        return;   // a handler already closed it
    if (top->OnBack())
        return;
    if (m_entries.back().Options.CloseOnBack)
    {
        if (m_entries.back().Session >= 0 && UiSettings::SessionChanged(m_entries.back().Session))
        {
            Push(ConfirmChangesPage);   // Apply keeps the values, Discard reverts them, Cancel stays
            return;
        }
        Pop();
    }
}

void UiMenuStack::ResolveChanges(bool apply)
{
    if (m_entries.size() < 2)
        return;
    const int session = m_entries[m_entries.size() - 2].Session;
    if (!apply && session >= 0)
        UiSettings::RevertSession(session);
    Pop();   // the confirm page
    if (!m_entries.empty())
        Pop();   // the page that asked
}

void UiMenuStack::Relayer()
{
    // Walk down from the top: a page that does not show below hides everything under it.
    bool visible = true;
    for (int i = (int)m_entries.size() - 1; i >= 0; --i)
    {
        Entry& e = m_entries[i];
        e.Page->Style().Visibility = visible ? UiVisibility::Visible : UiVisibility::Collapse;
        e.Page->FocusTrap = (i == (int)m_entries.size() - 1);
        if (!e.Options.ShowsBelow)
            visible = false;
    }
    MarkStyleDirty();
}

void UiMenuStack::FocusPage(UiElem& page)
{
    UiElem* target = FindDefaultFocus(page);
    if (target != nullptr)
        UiNavigation::SetFocus(target);
    else
        UiNavigation::FocusFirst(&page);
    bool inside = false;
    for (UiElement* p = UiNavigation::Focused; p != nullptr; p = p->parent)
        if (p == &page) { inside = true; break; }
    if (!inside)
        UiNavigation::ClearFocus();   // never leave focus on a hidden lower page
    if (!inside || !UiNavigation::Focused->IsVisible())
    {
        // Layout has not run for the page yet; try again after this frame's layout.
        m_focusPending = true;
        m_focusDelay = 1;
        m_focusPendingRestore = false;
        try { m_focusTarget = page.shared_from_this(); } catch (...) {}
    }
}

void UiMenuStack::RestoreFocus(const std::weak_ptr<UiElement>& saved, UiElem& page)
{
    std::shared_ptr<UiElement> target = saved.lock();
    if (target)
    {
        bool inside = false;
        for (UiElement* p = target.get(); p != nullptr; p = p->parent)
            if (p == &page) { inside = true; break; }
        if (inside)
        {
            UiNavigation::SetFocus(target.get());
            if (UiNavigation::Focused == target.get())
                return;
            // The page was collapsed until now; its elements become visible after the next layout.
            m_focusPending = true;
        m_focusDelay = 1;
            m_focusPendingRestore = true;
            m_focusTarget = target;
            return;
        }
    }
    FocusPage(page);
}

void UiMenuStack::TickLeaving()
{
    for (size_t i = 0; i < m_leaving.size();)
    {
        Leaving& l = m_leaving[i];
        ++l.Frames;
        // the first style pass starts the transition, so the page is judged from the second frame on
        const bool done = SDL_GetTicks64() - l.PoppedAt > 2000 || (l.Frames >= 3 && !UiAnimation::IsAnimating(*l.Page));
        if (!done)
        {
            ++i;
            continue;
        }
        std::shared_ptr<UiElem> page = l.Page;
        m_leaving.erase(m_leaving.begin() + static_cast<long>(i));
        RemoveChild(page);
        page->parent = nullptr;
        MarkStyleDirty();
    }
}

void UiMenuStack::ClearLeaving()
{
    for (Leaving& l : m_leaving)
    {
        RemoveChild(l.Page);
        l.Page->parent = nullptr;
    }
    m_leaving.clear();
}

void UiMenuStack::Tick()
{
    if (!m_leaving.empty())
        TickLeaving();
    if (!m_focusPending)
        return;
    if (m_focusDelay > 0)
    {
        --m_focusDelay;   // layout of this frame has not updated the visibility flags yet
        return;
    }
    m_focusPending = false;
    std::shared_ptr<UiElement> target = m_focusTarget.lock();
    m_focusTarget.reset();
    UiElem* top = Top();
    if (!target || top == nullptr)
        return;
    if (m_focusPendingRestore)
    {
        UiNavigation::SetFocus(target.get());
        if (UiNavigation::Focused == target.get())
            return;
    }
    else if (target.get() != top)
        return;
    UiElem* def = FindDefaultFocus(*top);
    if (def != nullptr)
        UiNavigation::SetFocus(def);
    else
        UiNavigation::FocusFirst(top);
}

void UiMenuStack::ClearEntering()
{
    bool any = false;
    // A page keeps its entering state until a style pass has seen it, so its transitions start from the entering values.
    for (Entry& e : m_entries)
        if ((e.Page->States & kStateEntering) && m_styleGen > e.PushedGen)
        {
            e.Page->States &= ~kStateEntering;
            any = true;
        }
    if (any)
        MarkStyleDirty();
}

void UiMenuStack::SwapPage(UiElem* old, const std::shared_ptr<UiElem>& fresh)
{
    for (Entry& e : m_entries)
        if (e.Page.get() == old)
        {
            const bool wasTop = e.Page.get() == Top();
            const bool hadFocus = wasTop && UiNavigation::Focused != nullptr;
            RemoveChild(e.Page);
            e.Page->parent = nullptr;
            e.Page = fresh;
            e.Options = fresh->ReadHostOptions();
            AddChild(fresh);
            fresh->AddClass("page");
            // AddChild appends; keep the order of the entries.
            std::stable_sort(children.begin(), children.end(), [&](const std::shared_ptr<UiElement>& a, const std::shared_ptr<UiElement>& b) {
                auto index = [&](const UiElement* p) { for (size_t i = 0; i < m_entries.size(); ++i) if (m_entries[i].Page.get() == p) return (int)i; return 1 << 20; };
                return index(a.get()) < index(b.get());
            });
            Relayer();
            if (hadFocus)
                FocusPage(*fresh);
            RecomputeHostState();
            return;
        }
}

void UiMenuStack::RecomputeHostState()
{
    bool paused = false, hides = false;
    for (const Entry& e : m_entries)
    {
        paused = paused || e.Options.PausesGame;
        hides = hides || e.Options.HidesHud;
    }
    m_hidesHud = hides;
    if (paused != m_paused)
    {
        m_paused = paused;
        if (UiMenus::OnPauseChanged)
            UiMenus::OnPauseChanged(paused);
    }
}

// ── UiMenus

void UiMenus::Init()
{
    if (g_stack)
        return;
    g_stack = std::make_shared<UiMenuStack>();
    EngineMain::Viewport.AddChild(g_stack);
    UiSystem::RegisterRoot(g_stack);
}

void UiMenus::Shutdown()
{
    if (g_stack)
    {
        g_stack->Clear();
        if (g_stack->parent != nullptr)
            g_stack->parent->RemoveChild(g_stack);
    }
    g_stack.reset();
}

UiMenuStack& UiMenus::Stack()
{
    if (!g_stack)
        Init();
    return *g_stack;
}

UiElem* UiMenus::Push(const std::string& path) { return Stack().Push(path); }
void UiMenus::Pop() { Stack().Pop(); }
void UiMenus::Clear() { Stack().Clear(); }
bool UiMenus::IsOpen() { return g_stack && !g_stack->IsEmpty(); }
int UiMenus::LastBackFrame() { return g_lastBackFrame; }
void UiMenus::NoteBack() { g_lastBackFrame = UiSystem::FrameIndex(); }

void UiMenus::Update()
{
    if (!g_stack)
        return;
    if (!PauseMenu.empty() && g_stack->IsEmpty() && UiSystem::FrameIndex() - g_lastBackFrame > 1)   // navigation runs just before the frame counter advances
    {
        InputAction* pause = Input::GetAction("pause");
        if (pause->Pressed())
        {
            g_stack->Push(PauseMenu);
            pause->CleanPressed();
        }
    }
}

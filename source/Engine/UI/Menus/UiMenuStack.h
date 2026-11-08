#pragma once
#include <UI/Elements/UiElemPanel.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

// A stack of pages (any UiElem). Only the top page is interactive; lower pages are hidden unless the page above has shows-below.
class UiMenuStack : public UiElemPanel
{
public:
    UiMenuStack();
    UiMenuStack* AsMenuStack() override { return this; }

    UiElem* Push(const std::string& layoutPath);   // UiElem::Load (code-behind aware); nullptr on error
    UiElem* Push(std::shared_ptr<UiElem> page);
    void Pop();
    void PopTo(UiElem* page);                      // pops until `page` is the top (does nothing when it is not on the stack)
    void Replace(const std::string& layoutPath);
    void Clear();
    // Forced and silent (level change): every page, also the ones still leaving, is dropped at once. No popped / hidden / shown
    // events, no leave motion, no back handlers, no confirm-changes page, no pause callback: as if the pages had never been there.
    void Reset();
    void Back();                                   // what ui_cancel does

    UiElem* Top() const;
    UiElem* PageAt(int index) const;               // 0 = bottom
    int LeavingCount() const { return (int)m_leaving.size(); }
    UiElem* LeavingAt(int index) const { return m_leaving[index].Page.get(); }
    bool IsEmpty() const { return m_entries.empty(); }
    int Depth() const { return (int)m_entries.size(); }
    bool Contains(const UiElem* page) const;
    bool PausesGame() const { return m_paused; }
    bool HidesHud() const { return m_hidesHud; }

    std::function<void()> OnEmptyBack;             // host: what back does when the stack is empty

    // Pages with confirm-changes="true" journal the setting changes made through bound widgets. Back with changes pushes
    // `ConfirmChangesPage`; these resolve it from there (the confirm page is on top, the settings page under it).
    inline static std::string ConfirmChangesPage = "GameData/ui/kit/pages/confirm_changes.xml";
    void ResolveChanges(bool apply);

    // UiSystem hooks
    void Tick() override;                          // applies a focus change that had to wait for layout
    void ClearEntering();                          // the entering state lasts for one style pass
    void ClearLeaving();                           // drops the pages that are still animating out (level change)
    void SwapPage(UiElem* old, const std::shared_ptr<UiElem>& fresh);   // hot reload

private:
    struct Entry
    {
        std::shared_ptr<UiElem> Page;
        std::weak_ptr<UiElement> SavedFocus;       // focus before this page was pushed
        HostOptions Options;
        uint32_t PushedGen = 0;                    // style passes done when the page was pushed
        int Session = -1;                          // UiSettings journal of a confirm-changes page
    };
    std::vector<Entry> m_entries;
    // Popped pages whose leave transition or animation is still running; they stay children of the stack, inert, for 2 s at most.
    struct Leaving { std::shared_ptr<UiElem> Page; uint64_t PoppedAt = 0; int Frames = 0; };
    std::vector<Leaving> m_leaving;
    void TickLeaving();
    bool m_paused = false;
    bool m_hidesHud = false;
    bool m_focusPending = false;
    bool m_focusPendingRestore = false;
    int m_focusDelay = 0;
    std::weak_ptr<UiElement> m_focusTarget;

    void Relayer();                                // visibility overrides and focus traps from the current entries
    void FocusPage(UiElem& page);                  // default-focus, else the first focusable element
    void RestoreFocus(const std::weak_ptr<UiElement>& saved, UiElem& page);
    void RecomputeHostState();
    void BringToFront();
    void PopInternal();
};

namespace UiMenus
{
    void Init();                                   // from UiSystem::Init: creates the main stack and adds it to the viewport
    void Shutdown();
    void Update();                                 // from UiSystem::Update: keeps the stack attached, handles the pause action

    UiMenuStack& Stack();
    UiElem* Push(const std::string& path);
    void Pop();
    void Clear();
    bool IsOpen();

    inline std::string PauseMenu;                  // when non-empty: the "pause" action pushes it if the stack is empty
    inline std::function<void(bool paused)> OnPauseChanged;

    // Frame of the last Back(); the pause action does not reopen a menu that was just closed with the same key.
    int LastBackFrame();
    void NoteBack();
}

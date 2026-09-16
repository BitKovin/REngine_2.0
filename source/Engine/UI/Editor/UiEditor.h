#pragma once

#include "UiEditorConfig.h"

#if UI_EDITOR_ENABLED

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

#include "../../glm.h"
#include "../Layout/LayoutBlueprint.h"
#include "../Layout/LoadedLayoutInstance.h"

class UiElement;

// One open (or pending) layout file in the editor. Three states:
//
//  - PENDING (pendingChoice == true): just clicked in the browser. Nothing
//    has been built yet -- the details area shows two buttons ("Find first
//    instance in viewport" / "Create in temp canvas") instead of a tree.
//    `blueprintRoot` is already parsed (so the buttons can act immediately
//    and the peeked root type can be shown) but nothing live exists yet.
//
//  - LIVE (liveInstance set): resolved via picking, or via "Find first
//    instance in viewport". Edits apply straight to the real, already-
//    running element.
//
//  - TEMP CANVAS (previewOwner set): resolved via "Create in temp canvas",
//    or opened fresh via "New Layout...". `previewOwner` is a real element
//    built from `blueprintRoot` the same way LoadLayoutFromFile would, and
//    is shown by reparenting it under UiEditor's shared dev canvas
//    whenever this session is the active tab (see UiEditor::m_devCanvas).
//    Since UiElement::EditModeActive suppresses all touch dispatch while
//    the editor is open, it's inert to normal game input regardless.
struct UiEditorSession
{
    std::string path;
    LayoutBlueprintNodePtr blueprintRoot;
    std::string peekedRootType; // blueprintRoot->type, cached for display before anything is built

    bool pendingChoice = true;

    std::weak_ptr<LoadedLayoutInstance> liveInstance; // set once resolved as LIVE
    std::shared_ptr<UiElement> previewOwner;           // set once resolved as TEMP CANVAS

    LayoutBlueprintNode* selectedNode = nullptr;
    bool dirty = false;

    UiElement* OwnerElement() const; // liveInstance's owner, previewOwner.get(), or nullptr if still pending
};

// A queued mutation to the ACTIVE session's blueprint tree, applied at the
// very start of the next Draw() call rather than immediately from inside a
// tree-node's popup callback. This is what keeps Dear ImGui's tree/ID stack
// balanced: a node's ImGuiTreeNodeFlags_Leaf/NoTreePushOnOpen decision is
// made from `children.empty()` at the top of that node's draw call, and
// mutating the tree (adding/removing/pasting a child) later in that SAME
// call -- e.g. from a "Add Child" menu item -- can invalidate that decision
// after the fact, which is exactly what was producing the "missing
// TreePop()" / "PopID() called too many times" errors. Deferring every
// structural (and, for simplicity/consistency, rename) mutation to a point
// where no tree is currently being drawn removes the hazard entirely,
// rather than patching each individual mismatch as it's found.
struct UiEditorPendingAction
{
    enum class Kind { AddChild, AddChildLayoutRef, Delete, Duplicate, Rename, PasteAsChild, PasteAsSibling, PasteValues, ReplaceType };

    Kind kind;
    LayoutBlueprintNode* targetNode = nullptr; // parent (AddChild/PasteAsChild) or the node itself (others)
    std::string stringArg;                     // type name (AddChild*) or new name (Rename)
    std::string layoutPathArg;                 // AddChildLayoutRef only
};

class UiEditor
{
public:
    static UiEditor& Instance();

    bool IsOpen() const { return m_open; }
    void SetOpen(bool open);
    void Toggle() { SetOpen(!m_open); }

    // Draws every editor ImGui window. Call from the same DebugUi block
    // that already draws Console::Get().Draw() (see INTEGRATION.md). No-op
    // if not open.
    void Draw();

    // Call once per frame, BEFORE the normal per-frame touch dispatch
    // (see INTEGRATION.md), so a click meant for the editor's "pick" tool
    // doesn't also fall through to gameplay. No-op unless picking is armed.
    void UpdatePicking();

    // Console-command entry points (see UI/Editor/UiEditorConsoleCommands.cpp)
    void OpenFromCommand(const std::string& path);
    void OpenFromBrowser(const std::string& path) { OpenFromCommand(path); } // old name, kept as an alias for existing call sites (e.g. ConsoleDefaultCommands.cpp)

private:
    UiEditor() = default;

    bool m_open = false;
    bool m_pickingArmed = false;

    std::vector<UiEditorSession> m_sessions;
    int m_activeSession = -1;
    int m_forceSelectTab = -1; // index to force ImGuiTabItemFlags_SetSelected on, once

    // ── Layout browser ───────────────────────────────────────────────────
    std::vector<std::string> m_discoveredFiles;                         // every .ui under GameData/ui/layout/
    std::unordered_map<std::string, std::string> m_discoveredRootTypes; // path -> peeked root "type"
    std::string m_browserSelectedPath;
    void RefreshFileBrowser();

    void SelectBrowserFile(const std::string& path);          // click in the list -> open/focus a PENDING (or existing) session
    void ResolveAsLive(UiEditorSession& session);              // "Find first instance in viewport"
    void ResolveAsTempCanvas(UiEditorSession& session);        // "Create in temp canvas"
    int  FindSessionForPath(const std::string& path) const;   // -1 if none open
    void OpenPickedLiveInstance(LoadedLayoutInstance* layout); // picking's entry point -- always resolves straight to LIVE
    void CloseSession(UiEditorSession& session);               // tab's [x] -- tears down its temp-canvas preview if it has one
    void Save(UiEditorSession& session);

    // ── New Layout ───────────────────────────────────────────────────────
    bool m_showNewLayoutPopup = false;
    char m_newLayoutPathBuf[256] = "GameData/ui/layout/newLayout.ui";
    int m_newLayoutTypeIndex = 0;
    void DrawNewLayoutPopup();
    void CreateNewLayout(const std::string& path, const std::string& rootType);

    // ── Dev/temp canvas ──────────────────────────────────────────────────
    // Shared across every TEMP CANVAS session -- only the ACTIVE tab's
    // previewOwner is actually parented under it at a time (swapped on tab
    // switch), so several such sessions can stay open without overlapping
    // visually. Created lazily on first use, destroyed when the editor
    // closes. origin/pivot are fixed at (0.5, 0.5) so it's always centered
    // on screen regardless of its (editable) size.
    std::shared_ptr<UiElement> m_devCanvas;
    std::shared_ptr<UiElement> m_devCanvasBackdrop; // permanent child: translucent fill so its bounds are visible
    std::shared_ptr<UiElement> m_devCanvasCurrentChild; // whichever session's previewOwner is shown right now, if any
    glm::vec2 m_devCanvasSize = glm::vec2(800.f, 600.f);
    void EnsureDevCanvas();
    void ShowInDevCanvas(const std::shared_ptr<UiElement>& previewOwner); // pass nullptr to hide/empty it; no-op if already showing it
    void DestroyDevCanvas();

    // ── Clipboard (copy/paste) ───────────────────────────────────────────
    // In-memory only (not the OS clipboard): a deep clone captured at copy
    // time, so later edits to the original don't affect what pastes.
    LayoutBlueprintNodePtr m_clipboard;

    // ── Pending mutation queue (see UiEditorPendingAction) ───────────────
    std::vector<UiEditorPendingAction> m_pendingActions;
    void ApplyPendingActions(); // called at the top of Draw(), before any tree is drawn this frame

    // ── Panels ───────────────────────────────────────────────────────────
    void DrawBrowserPanel();
    void DrawPendingSessionPanel(UiEditorSession& session); // the 2-button panel
    void DrawToolbar(UiEditorSession& session);
    void DrawTreePanel(UiEditorSession& session);
    void DrawTreeNode(UiEditorSession& session, const LayoutBlueprintNodePtr& node);
    void DrawAddChildMenu(UiEditorSession& session, LayoutBlueprintNode& parent); // the two-section submenu
    void DrawReplaceTypeMenu(UiEditorSession& session, LayoutBlueprintNode& node); // same two sections, applied in place
    void DrawTypePickerSections(const std::function<void(const std::string& typeName, const std::string& layoutPath)>& onPick); // shared by both
    void DrawDetailsPanel(UiEditorSession& session);
    void DrawPropertyRow(UiEditorSession& session, LayoutBlueprintNode& node, const struct PropertyInfo& info,
                          class UiElement* liveTarget);

    // ── Structural mutation -- actually applied here, called only from
    // ApplyPendingActions(). Always followed by a Rebuild. ─────────────
    void AddChildNode(UiEditorSession& session, LayoutBlueprintNode& parent, const std::string& typeName, const std::string& layoutPath);
    void DeleteNode(UiEditorSession& session, LayoutBlueprintNode& node);
    void DuplicateNode(UiEditorSession& session, LayoutBlueprintNode& node);
    void RenameNode(UiEditorSession& session, LayoutBlueprintNode& node, const std::string& newName);
    void PasteNode(UiEditorSession& session, LayoutBlueprintNode& target, bool asChild);
    void PasteValuesOntoNode(UiEditorSession& session, LayoutBlueprintNode& target); // copies only property VALUES from the clipboard node, not structure/name
    void ReplaceNodeType(UiEditorSession& session, LayoutBlueprintNode& node, const std::string& typeName, const std::string& layoutPath);
    void RebuildSession(UiEditorSession& session);

    std::string GenerateUniqueName(UiEditorSession& session) const;
    bool NameExistsInSession(UiEditorSession& session, const std::string& candidate) const;
    void ReuniqueNamesRecursive(UiEditorSession& session, LayoutBlueprintNode& node);

    UiEditorSession* ActiveSession(); // nullptr if m_activeSession is out of range
};

#endif // UI_EDITOR_ENABLED

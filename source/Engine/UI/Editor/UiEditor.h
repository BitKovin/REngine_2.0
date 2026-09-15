#pragma once

#include "UiEditorConfig.h"

#if UI_EDITOR_ENABLED

#include <string>
#include <vector>
#include <memory>

#include "../Layout/LayoutBlueprint.h"
#include "../Layout/LoadedLayoutInstance.h"

class UiElement;

// One open layout file in the editor. Either:
//  - a LIVE session: `liveInstance` is set, found via visual picking of an
//    already-running game UI. Edits apply straight to the real element.
//  - an ISOLATED session: opened from the Layout Browser. A `previewOwner`
//    is constructed and attached to the real EngineMain::Viewport so it
//    renders through the normal pipeline for free -- but since
//    UiElement::EditModeActive suppresses all touch dispatch while the
//    editor is open (see INTEGRATION.md), it's inert to normal game input;
//    the only way to interact with it is through the editor itself.
struct UiEditorSession
{
    std::string path;
    LayoutBlueprintNodePtr blueprintRoot;

    std::weak_ptr<LoadedLayoutInstance> liveInstance; // set for a picked, already-running layout
    std::shared_ptr<UiElement> previewOwner;           // set for an isolated (browser-opened) session

    LayoutBlueprintNode* selectedNode = nullptr;
    bool dirty = false;

    UiElement* OwnerElement() const; // liveInstance's owner, or previewOwner.get()
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

    // File I/O
    void OpenFromBrowser(const std::string& path);
    void OpenLiveInstance(LoadedLayoutInstance* layout);
    void Save(UiEditorSession& session);
    void CloseSession(UiEditorSession& session);

    // Console-command entry points (see UI/Editor/UiEditorConsoleCommands.cpp)
    void OpenFromCommand(const std::string& path) { OpenFromBrowser(path); }

private:
    UiEditor() = default;

    bool m_open = false;
    bool m_pickingArmed = false;

    std::vector<UiEditorSession> m_sessions;
    int m_activeSession = -1;

    std::vector<std::string> m_discoveredFiles; // populated from FileSystemEngine::GetFilesInPath
    void RefreshFileBrowser();

    // ── Panels ───────────────────────────────────────────────────────────
    void DrawBrowserPanel();
    void DrawToolbar(UiEditorSession& session);
    void DrawTreePanel(UiEditorSession& session);
    void DrawTreeNode(UiEditorSession& session, const LayoutBlueprintNodePtr& node);
    void DrawDetailsPanel(UiEditorSession& session);
    void DrawPropertyRow(UiEditorSession& session, LayoutBlueprintNode& node, const struct PropertyInfo& info,
                          class UiElement* liveTarget);

    // ── Structural mutation (always followed by a Rebuild) ──────────────
    void AddChildNode(UiEditorSession& session, LayoutBlueprintNode& parent, const std::string& typeName);
    void DeleteNode(UiEditorSession& session, LayoutBlueprintNode& node);
    void DuplicateNode(UiEditorSession& session, LayoutBlueprintNode& node);
    void RenameNode(UiEditorSession& session, LayoutBlueprintNode& node, const std::string& newName);
    void RebuildSession(UiEditorSession& session);

    std::string GenerateUniqueName(UiEditorSession& session) const;
    bool NameExistsInSession(UiEditorSession& session, const std::string& candidate) const;
};

#endif // UI_EDITOR_ENABLED

#include "UiEditor.h"

#if UI_EDITOR_ENABLED

#include "../UiElement.h"
#include "../UiCanvas.hpp"
#include "../UiImage.hpp"
#include "../Layout/UiLayoutSerializer.h"
#include "../Reflection/UiElementRegistry.h"
#include <EngineMain.h>
#include <Input.h>

#include <FileSystem/FileSystem.h>
#include "Logger.hpp"

#include <imgui/imgui.h>
#include <random>
#include <sstream>
#include <algorithm>
#include <functional>

// ─────────────────────────────────────────────────────────────────────────
// NOTE ON IMGUI CALLS: written against vanilla Dear ImGui. ImGui::BeginDisabled/
// EndDisabled (used once, in DrawPendingSessionPanel) need ImGui >= 1.87 --
// if that's not available, just delete that one BeginDisabled/EndDisabled
// pair and leave the button always clickable (ResolveAsLive is already a
// safe no-op when there's nothing to find).
// ─────────────────────────────────────────────────────────────────────────

UiElement* UiEditorSession::OwnerElement() const
{
    if (auto locked = liveInstance.lock())
        return locked->ownerElement;
    return previewOwner.get();
}

UiEditor& UiEditor::Instance()
{
    static UiEditor instance;
    return instance;
}

UiEditorSession* UiEditor::ActiveSession()
{
    if (m_activeSession < 0 || m_activeSession >= (int)m_sessions.size())
        return nullptr;
    return &m_sessions[m_activeSession];
}

void UiEditor::SetOpen(bool open)
{
    m_open = open;
    UiElement::EditModeActive = open;

    if (open)
    {
        RefreshFileBrowser();
    }
    else
    {
        DestroyDevCanvas(); // detaches/destroys whatever temp-canvas preview was showing
        m_sessions.clear();  // drops the last reference to every other temp-canvas preview too
        m_activeSession = -1;
        m_forceSelectTab = -1;
        m_pickingArmed = false;
        m_browserSelectedPath.clear();
        m_pendingActions.clear();
    }
}

void UiEditor::OpenFromCommand(const std::string& path)
{
    SetOpen(true);
    SelectBrowserFile(path);
}

// ── Layout browser ──────────────────────────────────────────────────────

void UiEditor::RefreshFileBrowser()
{
    m_discoveredFiles.clear();
    m_discoveredRootTypes.clear();

    static const std::string layoutDir = "GameData/ui/layout/";

    // GetFilesInPath appears to return bare filenames (no directory) rather
    // than full engine-relative paths -- reconstruct the real path so
    // ReadFile/WriteFile (and everything downstream that stores this as
    // session.path / LoadLayoutFromFile's argument) actually resolves.
    // Handles both conventions, in case that ever changes.
    std::vector<std::string> files = FileSystemEngine::GetFilesInPath(layoutDir);
    for (auto& f : files)
    {
        if (f.size() <= 3 || f.substr(f.size() - 3) != ".ui")
            continue;

        std::string fullPath = f;
        if (fullPath.compare(0, layoutDir.size(), layoutDir) != 0)
            fullPath = layoutDir + f;

        m_discoveredFiles.push_back(fullPath);

        // Peek the root type now (cheap, small files) so the browser and
        // the Add Child "Layout" section can both show/use it without a
        // reparse per frame.
        const std::string jsonText = FileSystemEngine::ReadFile(fullPath);
        if (!jsonText.empty())
        {
            LayoutBlueprintNodePtr root = UiLayoutSerializer::ParseBlueprint(jsonText);
            if (root)
                m_discoveredRootTypes[fullPath] = root->type;
        }
    }

    std::sort(m_discoveredFiles.begin(), m_discoveredFiles.end());
}

int UiEditor::FindSessionForPath(const std::string& path) const
{
    for (int i = 0; i < (int)m_sessions.size(); ++i)
        if (m_sessions[i].path == path)
            return i;
    return -1;
}

void UiEditor::SelectBrowserFile(const std::string& path)
{
    m_browserSelectedPath = path;

    const int existing = FindSessionForPath(path);
    if (existing >= 0)
    {
        m_forceSelectTab = existing;
        return;
    }

    const std::string jsonText = FileSystemEngine::ReadFile(path);
    if (jsonText.empty())
    {
        Logger::Log("UiEditor: could not read '" + path + "'");
        return;
    }

    LayoutBlueprintNodePtr root = UiLayoutSerializer::ParseBlueprint(jsonText);
    if (!root)
        return;

    UiEditorSession session;
    session.path           = path;
    session.blueprintRoot  = root;
    session.peekedRootType = root->type;
    session.pendingChoice  = true;

    m_sessions.push_back(session);
    m_forceSelectTab = (int)m_sessions.size() - 1;
}

void UiEditor::ResolveAsLive(UiEditorSession& session)
{
    std::vector<LoadedLayoutInstancePtr> instances = LoadedLayoutRegistry::Instance().GetByPath(session.path);
    if (instances.empty())
        return; // caller (DrawPendingSessionPanel) already checked / grayed the button

    session.liveInstance  = instances[0];
    session.blueprintRoot = instances[0]->blueprintRoot; // edit the live blueprint directly, no copy
    session.pendingChoice = false;
}

void UiEditor::ResolveAsTempCanvas(UiEditorSession& session)
{
    if (session.blueprintRoot->type.empty())
    {
        Logger::Log("UiEditor: '" + session.path + "' has no root type; cannot preview");
        return;
    }

    std::shared_ptr<UiElement> owner = UiElementRegistry::Instance().Create(session.blueprintRoot->type);
    if (!owner)
        return; // Create() already logged

    // Wire the same fields LoadLayoutFromFile would, then reuse Rebuild()
    // rather than duplicating its logic. Deliberately NOT registered with
    // LoadedLayoutRegistry -- this is an editor-owned, in-memory-authority
    // session; it must not be clobbered by hot-reload-from-disk while open.
    owner->OwnedLayout = std::make_shared<LoadedLayoutInstance>();
    owner->OwnedLayout->path          = session.path;
    owner->OwnedLayout->blueprintRoot = session.blueprintRoot;
    owner->OwnedLayout->ownerElement  = owner.get();
    owner->MemberOfLayout = owner->OwnedLayout.get();

    owner->Rebuild();

    session.previewOwner  = owner;
    session.pendingChoice = false;
}

void UiEditor::OpenPickedLiveInstance(LoadedLayoutInstance* layout)
{
    if (!layout)
        return;

    // At most one session per path: resolve/focus whatever's already open
    // for it rather than opening a second, confusing tab for the same file.
    const int existing = FindSessionForPath(layout->path);
    if (existing >= 0)
    {
        if (m_sessions[existing].pendingChoice)
            ResolveAsLive(m_sessions[existing]);
        m_forceSelectTab = existing;
        return;
    }

    UiEditorSession session;
    session.path           = layout->path;
    session.blueprintRoot  = layout->blueprintRoot;
    session.peekedRootType = layout->blueprintRoot ? layout->blueprintRoot->type : std::string();
    session.liveInstance   = layout->ownerElement->OwnedLayout;
    session.pendingChoice  = false;

    m_sessions.push_back(session);
    m_forceSelectTab = (int)m_sessions.size() - 1;
}

void UiEditor::CloseSession(UiEditorSession& session)
{
    if (session.previewOwner && m_devCanvasCurrentChild == session.previewOwner)
        ShowInDevCanvas(nullptr);

    auto it = std::find_if(m_sessions.begin(), m_sessions.end(),
        [&](const UiEditorSession& s) { return &s == &session; });
    if (it != m_sessions.end())
    {
        const int idx = (int)std::distance(m_sessions.begin(), it);
        m_sessions.erase(it);
        if (m_activeSession >= (int)m_sessions.size())
            m_activeSession = (int)m_sessions.size() - 1;
        else if (m_activeSession > idx)
            --m_activeSession;
    }
}

// ── New Layout ───────────────────────────────────────────────────────────

void UiEditor::CreateNewLayout(const std::string& path, const std::string& rootType)
{
    if (path.empty() || rootType.empty())
        return;

    if (FileSystemEngine::GetFileModificationTime(path) != 0)
    {
        Logger::Log("UiEditor: '" + path + "' already exists -- pick a different path");
        return;
    }

    auto root = std::make_shared<LayoutBlueprintNode>();
    root->type = rootType;
    root->name = "";

    if (!FileSystemEngine::WriteFile(path, UiLayoutSerializer::SerializeBlueprint(root)))
    {
        Logger::Log("UiEditor: failed to create '" + path + "'");
        return;
    }

    RefreshFileBrowser();

    UiEditorSession session;
    session.path           = path;
    session.blueprintRoot  = root;
    session.peekedRootType = rootType;
    session.pendingChoice  = false; // a brand-new file can't have a live instance yet -- skip straight past the pending buttons

    m_sessions.push_back(session);
    ResolveAsTempCanvas(m_sessions.back());
    m_forceSelectTab = (int)m_sessions.size() - 1;
}

// ── Dev / temp canvas ────────────────────────────────────────────────────

void UiEditor::EnsureDevCanvas()
{
    if (m_devCanvas)
        return;

    auto canvas = std::make_shared<UiCanvas>();
    canvas->ScaleToParent = false;
    canvas->size   = m_devCanvasSize;
    canvas->origin = glm::vec2(0.5f);
    canvas->pivot  = glm::vec2(0.5f);
    canvas->visible = false;

    auto backdrop = std::make_shared<UiImage>();
    backdrop->ImagePath = "GameData/textures/generic/white.png"; // matches the fallback texture used elsewhere (see UiButton::Draw)
    backdrop->color = glm::vec4(0.08f, 0.08f, 0.10f, 0.85f);
    backdrop->size  = glm::vec2(1.f, 1.f);
    backdrop->parentRelativeScaling = glm::vec2(1.f, 1.f); // always exactly fills the dev canvas, at whatever size it's set to
    canvas->AddChild(backdrop);

    EngineMain::Viewport.AddChild(canvas);

    m_devCanvas = canvas;
    m_devCanvasBackdrop = backdrop;
}

void UiEditor::ShowInDevCanvas(const std::shared_ptr<UiElement>& previewOwner)
{
    if (m_devCanvasCurrentChild == previewOwner)
    {
        if (m_devCanvas)
            m_devCanvas->visible = (previewOwner != nullptr);
        return; // already showing this (or already empty) -- avoid reparenting every frame
    }

    if (previewOwner)
        EnsureDevCanvas();

    if (m_devCanvas && m_devCanvasCurrentChild)
        m_devCanvas->RemoveChild(m_devCanvasCurrentChild);

    m_devCanvasCurrentChild = previewOwner;

    if (m_devCanvas)
    {
        if (previewOwner)
        {
            m_devCanvas->AddChild(previewOwner);
            // Same reasoning as RebuildSession -- force the newly attached
            // subtree to be laid out AND finalized immediately, rather than
            // relying on the next normal engine frame to get to it.
            m_devCanvas->UpdateChildrenOffsetRecursive();
            m_devCanvas->FinalizeChildren();
        }
        m_devCanvas->visible = (previewOwner != nullptr);
    }
}

void UiEditor::DestroyDevCanvas()
{
    if (m_devCanvas)
        m_devCanvas->RemoveFromParent();

    m_devCanvas.reset();
    m_devCanvasBackdrop.reset();
    m_devCanvasCurrentChild.reset();
}

// ── Save ─────────────────────────────────────────────────────────────────

void UiEditor::Save(UiEditorSession& session)
{
    UiElement* owner = session.OwnerElement();
    if (!owner || !session.blueprintRoot)
        return;

    // Recapture every node's properties from the live tree first, so Save
    // reflects reality even if something outside the editor mutated a
    // property directly (defensive; the normal editor property-edit path
    // already keeps blueprint and live element in sync as it goes).
    std::function<void(LayoutBlueprintNode&, UiElement*)> sync =
        [&](LayoutBlueprintNode& node, UiElement* element)
    {
        if (!element)
            return;

        node.properties = UiLayoutSerializer::CaptureProperties(element, node.type);

        for (auto& childNode : node.children)
        {
            std::shared_ptr<UiElement> childElement;

            if (auto locked = session.liveInstance.lock())
            {
                auto it = locked->namedElements.find(childNode->name);
                if (it != locked->namedElements.end())
                    childElement = it->second.lock();
            }
            else if (owner->OwnedLayout)
            {
                auto it = owner->OwnedLayout->namedElements.find(childNode->name);
                if (it != owner->OwnedLayout->namedElements.end())
                    childElement = it->second.lock();
            }

            sync(*childNode, childElement.get());
        }
    };
    sync(*session.blueprintRoot, owner);

    const std::string text = UiLayoutSerializer::SerializeBlueprint(session.blueprintRoot);
    if (!FileSystemEngine::WriteFile(session.path, text))
    {
        Logger::Log("UiEditor: failed to write '" + session.path + "'");
        return;
    }

    session.dirty = false;
}

// ── Picking ──────────────────────────────────────────────────────────────

void UiEditor::UpdatePicking()
{
    if (!m_open || !m_pickingArmed)
        return;

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return; // click landed on an ImGui window, not the game view

    if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        return;

    const vec2 mousePos = Input::MousePos;

    std::shared_ptr<UiElement> hit = EngineMain::Viewport.GetHitElementUnderPosition(mousePos);
    if (!hit)
        return;

    UiElement* elem = hit.get();
    while (elem && !elem->MemberOfLayout)
        elem = elem->parent;

    m_pickingArmed = false; // one-shot regardless of outcome; re-arm via the toolbar button

    if (!elem)
    {
        Logger::Log("UiEditor: picked element isn't part of any layout file (plain hand-written C++ UI) -- nothing to edit");
        return;
    }

    OpenPickedLiveInstance(elem->MemberOfLayout);

    UiEditorSession* session = ActiveSession();
    if (session && session->blueprintRoot)
    {
        std::vector<LayoutBlueprintNode*> stack = { session->blueprintRoot.get() };
        while (!stack.empty())
        {
            LayoutBlueprintNode* n = stack.back();
            stack.pop_back();
            if (n->name == elem->name)
            {
                session->selectedNode = n;
                break;
            }
            for (auto& c : n->children)
                stack.push_back(c.get());
        }
    }
}

// ── Naming ───────────────────────────────────────────────────────────────

std::string UiEditor::GenerateUniqueName(UiEditorSession& session) const
{
    static std::mt19937 rng(std::random_device{}());
    static const char* hex = "0123456789abcdef";

    std::string candidate;
    do
    {
        std::ostringstream ss;
        ss << '$';
        for (int i = 0; i < 8; ++i)
            ss << hex[rng() % 16];
        candidate = ss.str();
    } while (NameExistsInSession(session, candidate));

    return candidate;
}

bool UiEditor::NameExistsInSession(UiEditorSession& session, const std::string& candidate) const
{
    std::vector<LayoutBlueprintNode*> stack;
    if (session.blueprintRoot)
        stack.push_back(session.blueprintRoot.get());

    while (!stack.empty())
    {
        LayoutBlueprintNode* n = stack.back();
        stack.pop_back();

        if (n->name == candidate)
            return true;

        for (auto& c : n->children)
            stack.push_back(c.get());
    }
    return false;
}

void UiEditor::ReuniqueNamesRecursive(UiEditorSession& session, LayoutBlueprintNode& node)
{
    node.name = GenerateUniqueName(session);
    for (auto& c : node.children)
        ReuniqueNamesRecursive(session, *c);
}

// ── Structural mutation (only ever called from ApplyPendingActions) ──────

void UiEditor::RebuildSession(UiEditorSession& session)
{
    UiElement* owner = session.OwnerElement();
    if (owner)
    {
        owner->Rebuild(); // recomputes offsets internally

        // Rebuild() only recomputes offsets, not the finalized (draw-time)
        // snapshot Draw() actually reads from -- without this, a
        // just-added child (or a just-changed subtree) could stay
        // invisible until some unrelated update happened to trigger a
        // finalize pass first. Forcing it immediately here removes any
        // dependency on frame-ordering between the editor's own mutation
        // and the engine's normal per-frame Update/Finalize/Draw cycle.
        owner->FinalizeChildren();
    }

    session.dirty = true;
    session.selectedNode = nullptr; // old pointer may now dangle after a structural rebuild's tree edits
}

void UiEditor::AddChildNode(UiEditorSession& session, LayoutBlueprintNode& parent, const std::string& typeName, const std::string& layoutPath)
{
    auto node = std::make_shared<LayoutBlueprintNode>();
    node->type       = typeName;
    node->layoutPath = layoutPath;
    node->name       = GenerateUniqueName(session);
    node->parent     = &parent;

    parent.children.push_back(node);
    RebuildSession(session);
    session.selectedNode = node.get();
}

void UiEditor::DeleteNode(UiEditorSession& session, LayoutBlueprintNode& node)
{
    if (!node.parent)
    {
        Logger::Log("UiEditor: can't delete a layout's root node");
        return;
    }

    auto& siblings = node.parent->children;
    siblings.erase(
        std::remove_if(siblings.begin(), siblings.end(),
            [&](const LayoutBlueprintNodePtr& c) { return c.get() == &node; }),
        siblings.end());

    RebuildSession(session);
}

void UiEditor::DuplicateNode(UiEditorSession& session, LayoutBlueprintNode& node)
{
    if (!node.parent)
    {
        Logger::Log("UiEditor: can't duplicate a layout's root node");
        return;
    }

    LayoutBlueprintNodePtr original;
    for (auto& c : node.parent->children)
        if (c.get() == &node) { original = c; break; }
    if (!original)
        return;

    LayoutBlueprintNodePtr clone = CloneBlueprintNode(original);
    clone->parent = node.parent;
    ReuniqueNamesRecursive(session, *clone);

    node.parent->children.push_back(clone);
    RebuildSession(session);
    session.selectedNode = clone.get();
}

void UiEditor::PasteNode(UiEditorSession& session, LayoutBlueprintNode& target, bool asChild)
{
    if (!m_clipboard)
        return;

    LayoutBlueprintNode* parent = asChild ? &target : target.parent;
    if (!parent)
        return; // "paste as sibling" of the root -- root has no parent, nothing to do

    LayoutBlueprintNodePtr clone = CloneBlueprintNode(m_clipboard);
    clone->parent = parent;
    ReuniqueNamesRecursive(session, *clone); // fresh names/ids for every pasted node -- never collides with the original

    parent->children.push_back(clone);
    RebuildSession(session);
    session.selectedNode = clone.get();
}

void UiEditor::PasteValuesOntoNode(UiEditorSession& session, LayoutBlueprintNode& target)
{
    if (!m_clipboard)
        return;

    UiElement* owner = session.OwnerElement();
    UiElement* liveTarget = nullptr;

    if (&target == session.blueprintRoot.get())
    {
        liveTarget = owner;
    }
    else if (owner && owner->OwnedLayout)
    {
        auto it = owner->OwnedLayout->namedElements.find(target.name);
        if (it != owner->OwnedLayout->namedElements.end())
            liveTarget = it->second.lock().get();
    }
    if (!liveTarget)
        return;

    // Only property NAMES that (a) exist in the clipboard node's captured
    // properties AND (b) are actually registered on the target's own type
    // get copied -- e.g. pasting a UiButton's values onto a UiImage still
    // carries over Color/Position/Size (shared, from the UiElement base
    // table) and silently skips HoverColor/OnlyTouch (UiButton-only, not
    // present on UiImage's chain). Deliberately does NOT touch `name` or
    // structure -- see PasteNode for that.
    for (const PropertyTable* table : UiElementRegistry::Instance().GetPropertyChain(target.type))
    {
        for (const PropertyInfo& info : table->properties)
        {
            auto it = m_clipboard->properties.find(info.name);
            if (it == m_clipboard->properties.end())
                continue;

            try
            {
                const PropertyValue value = UiLayoutSerializer::PropertyValueFromJson(it.value(), info.kind);
                info.set(liveTarget, value);
                target.properties[info.name] = it.value();
            }
            catch (const std::exception&)
            {
                // Same property name, incompatible stored JSON shape (rare:
                // would need two different registered kinds sharing a name)
                // -- skip that one property rather than aborting the paste.
            }
        }
    }

    session.dirty = true;

    if (owner)
    {
        owner->UpdateChildrenOffsetRecursive();
        owner->FinalizeChildren();
    }
}

void UiEditor::ReplaceNodeType(UiEditorSession& session, LayoutBlueprintNode& node, const std::string& typeName, const std::string& layoutPath)
{
    if (!node.parent)
    {
        Logger::Log("UiEditor: can't replace a layout's root node's type");
        return;
    }

    node.type       = typeName;
    node.layoutPath = layoutPath;
    // `name` and `properties` are deliberately left as-is: any property
    // name shared between the old and new type (Position, Size, Color, ...)
    // just carries over correctly; anything that no longer applies sits
    // harmlessly unused in the blueprint until the next Save() recaptures
    // only what the new type actually has (see UiLayoutSerializer::
    // CaptureProperties). Existing children are left in place too -- not
    // auto-deleted, since that's destructive and the new type may still
    // want them (e.g. replacing a plain UiCanvas with a UiVerticalBox).

    RebuildSession(session); // reuses the existing rebuild + finalize
}

void UiEditor::RenameNode(UiEditorSession& session, LayoutBlueprintNode& node, const std::string& newName)
{
    if (newName.empty() || NameExistsInSession(session, newName))
        return;

    UiElement* owner = session.OwnerElement();
    UiElement* target = nullptr;

    if (&node == session.blueprintRoot.get())
    {
        target = owner;
    }
    else if (owner && owner->OwnedLayout)
    {
        auto it = owner->OwnedLayout->namedElements.find(node.name);
        if (it != owner->OwnedLayout->namedElements.end())
        {
            auto locked = it->second.lock();
            target = locked.get();
            owner->OwnedLayout->namedElements.erase(it);
            if (locked)
                owner->OwnedLayout->namedElements[newName] = locked;
        }
    }

    node.name = newName;
    if (target)
        target->name = newName;

    session.dirty = true;
}

void UiEditor::ApplyPendingActions()
{
    if (m_pendingActions.empty())
        return;

    std::vector<UiEditorPendingAction> actions;
    actions.swap(m_pendingActions);

    UiEditorSession* session = ActiveSession();
    if (!session)
        return; // the active session went away since these were queued -- drop them

    for (auto& action : actions)
    {
        if (!action.targetNode)
            continue;

        switch (action.kind)
        {
        case UiEditorPendingAction::Kind::AddChild:
            AddChildNode(*session, *action.targetNode, action.stringArg, "");
            break;
        case UiEditorPendingAction::Kind::AddChildLayoutRef:
            AddChildNode(*session, *action.targetNode, action.stringArg, action.layoutPathArg);
            break;
        case UiEditorPendingAction::Kind::Delete:
            DeleteNode(*session, *action.targetNode);
            break;
        case UiEditorPendingAction::Kind::Duplicate:
            DuplicateNode(*session, *action.targetNode);
            break;
        case UiEditorPendingAction::Kind::Rename:
            RenameNode(*session, *action.targetNode, action.stringArg);
            break;
        case UiEditorPendingAction::Kind::PasteAsChild:
            PasteNode(*session, *action.targetNode, true);
            break;
        case UiEditorPendingAction::Kind::PasteAsSibling:
            PasteNode(*session, *action.targetNode, false);
            break;
        case UiEditorPendingAction::Kind::PasteValues:
            PasteValuesOntoNode(*session, *action.targetNode);
            break;
        case UiEditorPendingAction::Kind::ReplaceType:
            ReplaceNodeType(*session, *action.targetNode, action.stringArg, action.layoutPathArg);
            break;
        }
    }
}

// ── Drawing ──────────────────────────────────────────────────────────────

namespace
{
    // Small ImGui InputText helper for std::string (ImGui's C API wants a
    // char buffer). Returns true when the value changed.
    bool InputTextStd(const char* label, std::string& value, size_t bufSize = 256)
    {
        std::vector<char> buf(bufSize, 0);
        std::snprintf(buf.data(), bufSize, "%s", value.c_str());

        if (ImGui::InputText(label, buf.data(), bufSize))
        {
            value = buf.data();
            return true;
        }
        return false;
    }
}

void UiEditor::Draw()
{
    if (!m_open)
        return;

    // Apply whatever the PREVIOUS frame's tree/context-menu interaction
    // queued, before drawing anything this frame -- see
    // UiEditorPendingAction for why this must happen here rather than
    // inline from inside a tree node's popup callback.
    ApplyPendingActions();

    if (m_devCanvas)
        m_devCanvas->size = m_devCanvasSize;

    ImGui::SetNextWindowSize(ImVec2(1150, 720), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("UI Layout Editor", &m_open))
    {
        ImGui::End();
        if (!m_open) SetOpen(false); // closed via the window's [x]
        return;
    }

    DrawBrowserPanel();
    DrawNewLayoutPopup();

    ImGui::SameLine();
    ImGui::BeginChild("##EditorSessionArea", ImVec2(0, 0), false);

    if (!m_sessions.empty())
    {
        if (ImGui::BeginTabBar("##EditorSessionTabs"))
        {
            for (int i = 0; i < (int)m_sessions.size(); ++i)
            {
                UiEditorSession& session = m_sessions[i];

                // Give the tab a STABLE id (the "###..." suffix) independent
                // of its visible text. Without this, the tab's identity was
                // derived from the label alone -- and the label includes the
                // "* " dirty marker, which flips the instant you edit a
                // property. ImGui then saw what looked like a brand new tab
                // (different computed ID) and reset/reassigned which tab was
                // active, which is what was showing up as "editing a value
                // jumps to another tab".
                const std::string displayLabel = (session.pendingChoice ? "" : (session.dirty ? "* " : "")) + session.path;
                const std::string label = displayLabel + "###session_" + session.path;

                ImGuiTabItemFlags tabFlags = 0;
                if (m_forceSelectTab == i)
                {
                    tabFlags |= ImGuiTabItemFlags_SetSelected;
                    m_forceSelectTab = -1;
                }

                bool tabOpen = true;
                if (ImGui::BeginTabItem(label.c_str(), &tabOpen, tabFlags))
                {
                    m_activeSession = i;

                    if (session.pendingChoice)
                    {
                        DrawPendingSessionPanel(session);
                    }
                    else
                    {
                        DrawToolbar(session);
                        ImGui::Separator();

                        ImGui::BeginChild("##TreeColumn", ImVec2(340, 0), true);
                        DrawTreePanel(session);
                        ImGui::EndChild();

                        ImGui::SameLine();

                        ImGui::BeginChild("##DetailsColumn", ImVec2(0, 0), true);
                        DrawDetailsPanel(session);
                        ImGui::EndChild();
                    }

                    ImGui::EndTabItem();
                }

                if (!tabOpen)
                {
                    CloseSession(session);
                    break; // m_sessions was mutated; resume cleanly next frame
                }
            }
            ImGui::EndTabBar();
        }
    }
    else
    {
        ImGui::TextDisabled("Select a layout on the left, or click 'Pick from game' and click an element in the game view.");
    }

    ImGui::EndChild();
    ImGui::End();

    // Keep the dev canvas showing exactly the active session's temp-canvas
    // preview, if it has one, and nothing otherwise -- ShowInDevCanvas is a
    // no-op if that's already the case, so this is cheap to call every frame.
    UiEditorSession* active = ActiveSession();
    ShowInDevCanvas(active ? active->previewOwner : nullptr);
}

void UiEditor::DrawBrowserPanel()
{
    ImGui::BeginChild("##Browser", ImVec2(240, 0), true);

    ImGui::TextUnformatted("Layouts");
    ImGui::Separator();

    if (ImGui::Button("Refresh", ImVec2(-1, 0)))
        RefreshFileBrowser();

    if (ImGui::Button(m_pickingArmed ? "Picking... (click game)" : "Pick from game", ImVec2(-1, 0)))
        m_pickingArmed = !m_pickingArmed;

    if (ImGui::Button("New Layout...", ImVec2(-1, 0)))
        m_showNewLayoutPopup = true;

    ImGui::Separator();

    for (const std::string& path : m_discoveredFiles)
    {
        const bool isOpenOrSelected = (path == m_browserSelectedPath) || (FindSessionForPath(path) >= 0);

        const size_t slash = path.find_last_of('/');
        const std::string displayName = (slash == std::string::npos) ? path : path.substr(slash + 1);

        if (ImGui::Selectable(displayName.c_str(), isOpenOrSelected))
            SelectBrowserFile(path);
    }

    ImGui::EndChild();
}

void UiEditor::DrawNewLayoutPopup()
{
    if (m_showNewLayoutPopup)
    {
        ImGui::OpenPopup("New Layout");
        m_showNewLayoutPopup = false;
    }

    if (ImGui::BeginPopupModal("New Layout", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("File path", m_newLayoutPathBuf, sizeof(m_newLayoutPathBuf));

        const std::vector<std::string> typeNames = UiElementRegistry::Instance().GetAllTypeNames();
        const char* preview = (m_newLayoutTypeIndex >= 0 && m_newLayoutTypeIndex < (int)typeNames.size())
            ? typeNames[m_newLayoutTypeIndex].c_str() : "";

        if (ImGui::BeginCombo("Root type", preview))
        {
            for (int i = 0; i < (int)typeNames.size(); ++i)
                if (ImGui::Selectable(typeNames[i].c_str(), i == m_newLayoutTypeIndex))
                    m_newLayoutTypeIndex = i;
            ImGui::EndCombo();
        }

        const bool canCreate = !typeNames.empty() && m_newLayoutPathBuf[0] != '\0';
        if (!canCreate) ImGui::BeginDisabled();
        if (ImGui::Button("Create"))
        {
            CreateNewLayout(m_newLayoutPathBuf, typeNames[m_newLayoutTypeIndex]);
            ImGui::CloseCurrentPopup();
        }
        if (!canCreate) ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}

void UiEditor::DrawPendingSessionPanel(UiEditorSession& session)
{
    ImGui::TextWrapped("%s", session.path.c_str());
    ImGui::Text("Root type: %s", session.peekedRootType.empty() ? "(unknown)" : session.peekedRootType.c_str());
    ImGui::Separator();
    ImGui::TextWrapped("This file isn't open for editing yet. Choose how to edit it:");
    ImGui::Spacing();

    const bool hasLiveInstance = !LoadedLayoutRegistry::Instance().GetByPath(session.path).empty();

    if (!hasLiveInstance) ImGui::BeginDisabled();
    if (ImGui::Button("Find first instance in viewport", ImVec2(280, 0)))
        ResolveAsLive(session);
    if (!hasLiveInstance) ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::TextDisabled(hasLiveInstance ? "(currently running)" : "(nothing running right now)");

    if (ImGui::Button("Create in temp canvas", ImVec2(280, 0)))
        ResolveAsTempCanvas(session);

    ImGui::SameLine();
    ImGui::TextDisabled("(isolated preview, centered on screen)");
}

void UiEditor::DrawToolbar(UiEditorSession& session)
{
    if (ImGui::Button("Save"))
        Save(session);

    ImGui::SameLine();
    ImGui::TextDisabled(session.dirty ? "(unsaved changes)" : "(saved)");

#if UI_LAYOUT_HOT_RELOAD_ENABLED
    ImGui::SameLine();
    if (ImGui::Button("Reload from disk"))
    {
        UiElement* owner = session.OwnerElement();
        if (owner)
        {
            owner->ReloadFromDisk();
            session.blueprintRoot = owner->OwnedLayout->blueprintRoot;
            session.selectedNode  = nullptr;
            session.dirty = false;
        }
    }
#endif

    ImGui::SameLine();
    if (!session.liveInstance.expired())
    {
        ImGui::TextDisabled("(editing live, running instance)");
    }
    else if (session.previewOwner)
    {
        ImGui::TextDisabled("(temp canvas)");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(90);
        ImGui::DragFloat("##devW", &m_devCanvasSize.x, 1.f, 16.f, 4096.f, "W:%.0f");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(90);
        ImGui::DragFloat("##devH", &m_devCanvasSize.y, 1.f, 16.f, 4096.f, "H:%.0f");
    }
}

void UiEditor::DrawTreePanel(UiEditorSession& session)
{
    if (session.blueprintRoot)
        DrawTreeNode(session, session.blueprintRoot);
}

void UiEditor::DrawAddChildMenu(UiEditorSession& session, LayoutBlueprintNode& parent)
{
    if (!ImGui::BeginMenu("Add Child"))
        return;

    DrawTypePickerSections([&](const std::string& typeName, const std::string& layoutPath)
    {
        if (layoutPath.empty())
            m_pendingActions.push_back({ UiEditorPendingAction::Kind::AddChild, &parent, typeName, "" });
        else
            m_pendingActions.push_back({ UiEditorPendingAction::Kind::AddChildLayoutRef, &parent, typeName, layoutPath });
    });

    ImGui::EndMenu();
}

void UiEditor::DrawReplaceTypeMenu(UiEditorSession& session, LayoutBlueprintNode& node)
{
    if (!ImGui::BeginMenu("Replace Type"))
        return;

    DrawTypePickerSections([&](const std::string& typeName, const std::string& layoutPath)
    {
        m_pendingActions.push_back({ UiEditorPendingAction::Kind::ReplaceType, &node, typeName, layoutPath });
    });

    ImGui::EndMenu();
}

// Shared by "Add Child" and "Replace Type": the same "C++ Type" / "Layout"
// two-section list, differing only in what happens when an entry is
// clicked (`onPick(typeName, layoutPath)` -- layoutPath is "" for a plain
// C++ type). See UiElementRegistry::AssociateLayoutPath for why a .ui file
// already claimed by a registered C++ type is skipped from the "Layout"
// section (it's reachable from "C++ Type" instead, via that type's own
// real constructor rather than this generic wrapper).
void UiEditor::DrawTypePickerSections(const std::function<void(const std::string&, const std::string&)>& onPick)
{
    if (ImGui::BeginMenu("C++ Type"))
    {
        for (const std::string& typeName : UiElementRegistry::Instance().GetAllTypeNames())
            if (ImGui::MenuItem(typeName.c_str()))
                onPick(typeName, "");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Layout"))
    {
        bool anyShown = false;
        for (const std::string& path : m_discoveredFiles)
        {
            if (!UiElementRegistry::Instance().GetTypeNameForLayoutPath(path).empty())
                continue;

            auto it = m_discoveredRootTypes.find(path);
            const std::string rootType = (it != m_discoveredRootTypes.end()) ? it->second : std::string();
            if (rootType.empty())
                continue; // couldn't be parsed, or has no root type -- not placeable

            anyShown = true;
            const std::string label = path + "  (" + rootType + ")";
            if (ImGui::MenuItem(label.c_str()))
                onPick(rootType, path);
        }
        if (!anyShown)
            ImGui::TextDisabled("(none)");
        ImGui::EndMenu();
    }
}

void UiEditor::DrawTreeNode(UiEditorSession& session, const LayoutBlueprintNodePtr& node)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
    if (node.get() == session.selectedNode)
        flags |= ImGuiTreeNodeFlags_Selected;
    if (node->children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    std::string label = node->type;
    if (!node->layoutPath.empty())
        label += " [" + node->layoutPath + "]";
    label += "  (" + (node->name.empty() ? "unnamed" : node->name) + ")";

    // NOTE: `node->children.empty()` above is read exactly once, and nothing
    // below mutates the tree directly (every action is queued into
    // m_pendingActions and applied at the top of next frame's Draw()) -- so
    // this flags decision stays valid for the rest of this call. That's
    // what fixes the "missing TreePop()" / "PopID() called too many times"
    // errors: they came from a menu item mutating `node->children` in the
    // middle of this same call, after the leaf/non-leaf decision above had
    // already been made and pushed onto ImGui's stack.
    const bool opened = ImGui::TreeNodeEx((void*)node.get(), flags, "%s", label.c_str());

    if (ImGui::IsItemClicked())
        session.selectedNode = node.get();

    if (ImGui::BeginPopupContextItem())
    {
        session.selectedNode = node.get();

        DrawAddChildMenu(session, *node);

        if (ImGui::MenuItem("Copy"))
            m_clipboard = CloneBlueprintNode(node);

        if (m_clipboard)
        {
            if (ImGui::MenuItem("Paste as Child"))
                m_pendingActions.push_back({ UiEditorPendingAction::Kind::PasteAsChild, node.get(), "", "" });
            if (node->parent && ImGui::MenuItem("Paste as Sibling"))
                m_pendingActions.push_back({ UiEditorPendingAction::Kind::PasteAsSibling, node.get(), "", "" });

            // Copies only property VALUES from the clipboard node onto this
            // one -- not structure, not name, not type. Useful for e.g.
            // "make this button the same color as that one" without
            // duplicating/replacing anything. Property names not present
            // on this node's own type are silently skipped (see
            // PasteValuesOntoNode).
            if (ImGui::MenuItem("Paste Values"))
                m_pendingActions.push_back({ UiEditorPendingAction::Kind::PasteValues, node.get(), "", "" });
        }

        if (node->parent) // root can't be deleted/duplicated/retyped -- a layout always has exactly one root
        {
            if (ImGui::MenuItem("Duplicate"))
                m_pendingActions.push_back({ UiEditorPendingAction::Kind::Duplicate, node.get(), "", "" });
            if (ImGui::MenuItem("Delete"))
                m_pendingActions.push_back({ UiEditorPendingAction::Kind::Delete, node.get(), "", "" });

            DrawReplaceTypeMenu(session, *node);
        }

        ImGui::EndPopup();
    }

    if (opened && !node->children.empty())
    {
        for (auto& child : node->children)
            DrawTreeNode(session, child);
        ImGui::TreePop();
    }
}

void UiEditor::DrawDetailsPanel(UiEditorSession& session)
{
    LayoutBlueprintNode* node = session.selectedNode;
    if (!node)
    {
        ImGui::TextDisabled("Select an element in the tree.");
        return;
    }

    ImGui::Text("Type: %s", node->type.c_str());
    if (!node->layoutPath.empty())
        ImGui::TextDisabled("References layout: %s", node->layoutPath.c_str());

    std::string nameBuf = node->name;
    ImGui::PushItemWidth(220);
    if (InputTextStd("Name", nameBuf) && ImGui::IsItemDeactivatedAfterEdit())
        m_pendingActions.push_back({ UiEditorPendingAction::Kind::Rename, node, nameBuf, "" });
    ImGui::PopItemWidth();

    if (!node->name.empty() && node->name[0] == '$')
        ImGui::TextDisabled("(auto-generated id -- type a name to make it stable)");

    ImGui::Separator();

    UiElement* owner = session.OwnerElement();
    UiElement* liveTarget = nullptr;

    if (node == session.blueprintRoot.get())
    {
        liveTarget = owner;
    }
    else if (owner && owner->OwnedLayout)
    {
        auto it = owner->OwnedLayout->namedElements.find(node->name);
        if (it != owner->OwnedLayout->namedElements.end())
            liveTarget = it->second.lock().get();
    }

    if (!liveTarget)
    {
        ImGui::TextDisabled("(live element not found -- try Reload)");
        return;
    }

    for (const PropertyTable* table : UiElementRegistry::Instance().GetPropertyChain(node->type))
    {
        if (table->properties.empty())
            continue;

        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "%s", table->typeName.c_str());
        for (const PropertyInfo& info : table->properties)
            DrawPropertyRow(session, *node, info, liveTarget);
    }
}

void UiEditor::DrawPropertyRow(UiEditorSession& session, LayoutBlueprintNode& node, const PropertyInfo& info, UiElement* liveTarget)
{
    ImGui::PushID(info.name.c_str());
    PropertyValue value = info.get(liveTarget);
    bool changed = false;

    switch (info.kind)
    {
    case PropertyKind::Bool:
    {
        bool v = std::get<bool>(value);
        if (ImGui::Checkbox(info.name.c_str(), &v)) { value = v; changed = true; }
        break;
    }
    case PropertyKind::Int:
    {
        int v = std::get<int>(value);
        if (ImGui::DragInt(info.name.c_str(), &v)) { value = v; changed = true; }
        break;
    }
    case PropertyKind::Enum:
    {
        int v = std::get<int>(value);
        if (ImGui::Combo(info.name.c_str(), &v,
                [](void* data, int idx, const char** out) {
                    auto& labels = *static_cast<std::vector<std::string>*>(data);
                    if (idx < 0 || idx >= (int)labels.size()) return false;
                    *out = labels[idx].c_str();
                    return true;
                },
                (void*)&info.enumLabels, (int)info.enumLabels.size()))
        {
            value = v; changed = true;
        }
        break;
    }
    case PropertyKind::Float:
    {
        float v = std::get<float>(value);
        if (ImGui::DragFloat(info.name.c_str(), &v, 0.5f)) { value = v; changed = true; }
        break;
    }
    case PropertyKind::Vec2:
    {
        glm::vec2 v = std::get<glm::vec2>(value);
        if (ImGui::DragFloat2(info.name.c_str(), &v.x, 0.5f)) { value = v; changed = true; }
        break;
    }
    case PropertyKind::Vec4:
    {
        glm::vec4 v = std::get<glm::vec4>(value);
        if (ImGui::DragFloat4(info.name.c_str(), &v.x, 0.01f)) { value = v; changed = true; }
        break;
    }
    case PropertyKind::Color:
    {
        glm::vec4 v = std::get<glm::vec4>(value);
        if (ImGui::ColorEdit4(info.name.c_str(), &v.x)) { value = v; changed = true; }
        break;
    }
    case PropertyKind::String:
    case PropertyKind::LocKey:
    {
        std::string v = std::get<std::string>(value);
        if (InputTextStd(info.name.c_str(), v)) { value = v; changed = true; }
        break;
    }
    case PropertyKind::AssetPath:
    {
        std::string v = std::get<std::string>(value);
        ImGui::PushItemWidth(-60);
        bool edited = InputTextStd("##path", v);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("%s", info.name.c_str());
        // A real "Browse" button here would open a popup listing
        // FileSystemEngine::GetFilesInPath("GameData/textures/") and set
        // `v` on selection -- omitted for brevity, InputText covers v1.
        if (edited) { value = v; changed = true; }
        break;
    }
    }

    if (changed)
    {
        info.set(liveTarget, value);
        node.properties[info.name] = UiLayoutSerializer::PropertyValueToJson(value, info.kind);
        session.dirty = true;

        if (UiElement* owner = session.OwnerElement())
        {
            owner->UpdateChildrenOffsetRecursive();
            owner->FinalizeChildren();
        }
    }

    ImGui::PopID();
}

#endif // UI_EDITOR_ENABLED

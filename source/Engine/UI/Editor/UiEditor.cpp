#include "UiEditor.h"

#if UI_EDITOR_ENABLED

#include "../UiElement.h"
#include "../Layout/UiLayoutSerializer.h"
#include "../Reflection/UiElementRegistry.h"
#include <EngineMain.h>
#include <Input.h>

#include <FileSystem/FileSystem.h>
#include "Logger.hpp"

#include <imgui/imgui.h>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────
// NOTE ON THE IMGUI CALLS BELOW: written against vanilla Dear ImGui, which
// is what ImGuiEngineImpl.h / Console::Get().Draw() imply is already wired
// up elsewhere in the engine. Adjust names/signatures if the engine's
// wrapper differs in any particular.
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
        // Tear down isolated preview sessions (they were only attached to
        // the real Viewport for rendering); leave live sessions' real
        // elements alone, just forget about editing them.
        for (auto& session : m_sessions)
            if (session.previewOwner)
                session.previewOwner->RemoveFromParent();

        m_sessions.clear();
        m_activeSession = -1;
        m_pickingArmed = false;
    }
}

void UiEditor::RefreshFileBrowser()
{
    m_discoveredFiles.clear();

    // Assumes GetFilesInPath returns every file under the directory
    // (including subdirectories) as engine-relative paths; adjust the
    // filter/recursion here if it's shallow-only in your FileSystemEngine.
    std::vector<std::string> files = FileSystemEngine::GetFilesInPath("GameData/ui/layout/");
    for (auto& f : files)
        if (f.size() > 3 && f.substr(f.size() - 3) == ".ui")
            m_discoveredFiles.push_back(f);

    std::sort(m_discoveredFiles.begin(), m_discoveredFiles.end());
}

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

void UiEditor::OpenFromBrowser(const std::string& path)
{
    for (int i = 0; i < (int)m_sessions.size(); ++i)
    {
        if (m_sessions[i].path == path && m_sessions[i].previewOwner)
        {
            m_activeSession = i;
            return;
        }
    }

    const std::string jsonText = FileSystemEngine::ReadFile(path);
    if (jsonText.empty())
    {
        Logger::Log("UiEditor: could not read '" + path + "'");
        return;
    }

    LayoutBlueprintNodePtr root = UiLayoutSerializer::ParseBlueprint(jsonText);
    if (!root || root->type.empty())
    {
        Logger::Log("UiEditor: '" + path + "' has no root type; cannot preview");
        return;
    }

    std::shared_ptr<UiElement> owner = UiElementRegistry::Instance().Create(root->type);
    if (!owner)
        return; // Create() already logged

    UiEditorSession session;
    session.path          = path;
    session.blueprintRoot = root;
    session.previewOwner  = owner;

    // Wire the same fields LoadLayoutFromFile would, then reuse Rebuild()
    // rather than duplicating its logic. Deliberately NOT registered with
    // LoadedLayoutRegistry -- this is an editor-owned, in-memory-authority
    // session; it must not be clobbered by hot-reload-from-disk while open.
    owner->OwnedLayout = std::make_shared<LoadedLayoutInstance>();
    owner->OwnedLayout->path          = path;
    owner->OwnedLayout->blueprintRoot = root;
    owner->OwnedLayout->ownerElement  = owner.get();
    owner->MemberOfLayout = owner->OwnedLayout.get();

    owner->Rebuild();

    // Attach to the real viewport so it renders through the normal
    // pipeline. It's inert to gameplay input: UiElement::EditModeActive is
    // true for the whole time the editor is open, which suppresses the
    // per-frame touch dispatch at its source (see INTEGRATION.md).
    EngineMain::MainInstance->Viewport.AddChild(owner);

    m_sessions.push_back(session);
    m_activeSession = (int)m_sessions.size() - 1;
}

void UiEditor::OpenLiveInstance(LoadedLayoutInstance* layout)
{
    if (!layout)
        return;

    for (int i = 0; i < (int)m_sessions.size(); ++i)
    {
        if (auto locked = m_sessions[i].liveInstance.lock())
        {
            if (locked.get() == layout)
            {
                m_activeSession = i;
                return;
            }
        }
    }

    std::shared_ptr<LoadedLayoutInstance> shared = layout->ownerElement->OwnedLayout;

    UiEditorSession session;
    session.path          = layout->path;
    session.blueprintRoot = layout->blueprintRoot; // edit the live blueprint directly, no copy
    session.liveInstance  = shared;

    m_sessions.push_back(session);
    m_activeSession = (int)m_sessions.size() - 1;
}

void UiEditor::CloseSession(UiEditorSession& session)
{
    if (session.previewOwner)
        session.previewOwner->RemoveFromParent();

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

void UiEditor::RebuildSession(UiEditorSession& session)
{
    UiElement* owner = session.OwnerElement();
    if (owner)
        owner->Rebuild();

    session.dirty = true;
    session.selectedNode = nullptr; // old pointer may now dangle after a structural rebuild's tree edits
}

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

void UiEditor::UpdatePicking()
{
    if (!m_open || !m_pickingArmed)
        return;

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return; // click landed on an ImGui window, not the game view

    if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        return;

    const vec2 mousePos = Input::MousePos; // adjust to the engine's actual mouse-position accessor

    std::shared_ptr<UiElement> hit = EngineMain::MainInstance->Viewport.GetHitElementUnderPosition(mousePos);
    if (!hit)
        return;

    UiElement* elem = hit.get();
    while (elem && !elem->MemberOfLayout)
        elem = elem->parent;

    if (!elem)
    {
        Logger::Info("UiEditor: picked element isn't part of any layout file (plain hand-written C++ UI) -- nothing to edit");
        return;
    }

    OpenLiveInstance(elem->MemberOfLayout);

    if (m_activeSession >= 0)
    {
        // Select the matching node by name in the freshly opened session.
        std::vector<LayoutBlueprintNode*> stack = { m_sessions[m_activeSession].blueprintRoot.get() };
        while (!stack.empty())
        {
            LayoutBlueprintNode* n = stack.back();
            stack.pop_back();
            if (n->name == elem->name)
            {
                m_sessions[m_activeSession].selectedNode = n;
                break;
            }
            for (auto& c : n->children)
                stack.push_back(c.get());
        }
    }

    m_pickingArmed = false; // one-shot; re-arm via the toolbar button
}

// ── Structural mutation ─────────────────────────────────────────────────

void UiEditor::AddChildNode(UiEditorSession& session, LayoutBlueprintNode& parent, const std::string& typeName)
{
    auto node = std::make_shared<LayoutBlueprintNode>();
    node->type   = typeName;
    node->name   = GenerateUniqueName(session);
    node->parent = &parent;

    parent.children.push_back(node);
    RebuildSession(session);
    session.selectedNode = node.get();
}

void UiEditor::DeleteNode(UiEditorSession& session, LayoutBlueprintNode& node)
{
    if (!node.parent)
    {
        Logger::Warning("UiEditor: can't delete a layout's root node");
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
        Logger::Warning("UiEditor: can't duplicate a layout's root node");
        return;
    }

    LayoutBlueprintNodePtr original;
    for (auto& c : node.parent->children)
        if (c.get() == &node) { original = c; break; }
    if (!original)
        return;

    LayoutBlueprintNodePtr clone = CloneBlueprintNode(original);
    clone->parent = node.parent;

    // Re-unique every name in the cloned subtree (CloneBlueprintNode copies
    // names verbatim, which would collide within this layout otherwise).
    std::function<void(LayoutBlueprintNode&)> reuniqueNames = [&](LayoutBlueprintNode& n)
    {
        n.name = GenerateUniqueName(session);
        for (auto& c : n.children)
            reuniqueNames(*c);
    };
    reuniqueNames(*clone);

    node.parent->children.push_back(clone);
    RebuildSession(session);
    session.selectedNode = clone.get();
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

    ImGui::SetNextWindowSize(ImVec2(1100, 700), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("UI Layout Editor", &m_open))
    {
        ImGui::End();
        if (!m_open) SetOpen(false); // closed via the window's [x]
        return;
    }

    DrawBrowserPanel();

    ImGui::SameLine();
    ImGui::BeginChild("##EditorSessionArea", ImVec2(0, 0), false);

    if (!m_sessions.empty())
    {
        if (ImGui::BeginTabBar("##EditorSessionTabs"))
        {
            for (int i = 0; i < (int)m_sessions.size(); ++i)
            {
                UiEditorSession& session = m_sessions[i];
                std::string label = (session.dirty ? "* " : "") + session.path;

                bool tabOpen = true;
                if (ImGui::BeginTabItem(label.c_str(), &tabOpen))
                {
                    m_activeSession = i;

                    DrawToolbar(session);
                    ImGui::Separator();

                    ImGui::BeginChild("##TreeColumn", ImVec2(320, 0), true);
                    DrawTreePanel(session);
                    ImGui::EndChild();

                    ImGui::SameLine();

                    ImGui::BeginChild("##DetailsColumn", ImVec2(0, 0), true);
                    DrawDetailsPanel(session);
                    ImGui::EndChild();

                    ImGui::EndTabItem();
                }

                if (!tabOpen)
                {
                    CloseSession(session);
                    break; // m_sessions was mutated; resume next frame
                }
            }
            ImGui::EndTabBar();
        }
    }
    else
    {
        ImGui::TextDisabled("Open a layout from the browser on the left, or click 'Pick' and click an element in the game view.");
    }

    ImGui::EndChild();
    ImGui::End();
}

void UiEditor::DrawBrowserPanel()
{
    ImGui::BeginChild("##Browser", ImVec2(220, 0), true);

    ImGui::TextUnformatted("Layouts");
    ImGui::Separator();

    if (ImGui::Button("Refresh", ImVec2(-1, 0)))
        RefreshFileBrowser();

    if (ImGui::Button(m_pickingArmed ? "Picking... (click game)" : "Pick from game", ImVec2(-1, 0)))
        m_pickingArmed = !m_pickingArmed;

    ImGui::Separator();

    for (const std::string& path : m_discoveredFiles)
    {
        if (ImGui::Selectable(path.c_str()))
            OpenFromBrowser(path);
    }

    ImGui::EndChild();
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
    if (session.liveInstance.expired() == false)
        ImGui::TextDisabled("(editing live, running instance)");
    else
        ImGui::TextDisabled("(isolated preview)");
}

void UiEditor::DrawTreePanel(UiEditorSession& session)
{
    if (session.blueprintRoot)
        DrawTreeNode(session, session.blueprintRoot);
}

void UiEditor::DrawTreeNode(UiEditorSession& session, const LayoutBlueprintNodePtr& node)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
    if (node.get() == session.selectedNode)
        flags |= ImGuiTreeNodeFlags_Selected;
    if (node->children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    const std::string label = node->type + "  (" + (node->name.empty() ? "unnamed" : node->name) + ")";
    const bool opened = ImGui::TreeNodeEx((void*)node.get(), flags, "%s", label.c_str());

    if (ImGui::IsItemClicked())
        session.selectedNode = node.get();

    if (ImGui::BeginPopupContextItem())
    {
        session.selectedNode = node.get();

        if (ImGui::BeginMenu("Add Child"))
        {
            for (const std::string& typeName : UiElementRegistry::Instance().GetAllTypeNames())
                if (ImGui::MenuItem(typeName.c_str()))
                    AddChildNode(session, *node, typeName);
            ImGui::EndMenu();
        }

        if (node->parent) // root can't be deleted/duplicated -- a layout always has exactly one root
        {
            if (ImGui::MenuItem("Duplicate"))
                DuplicateNode(session, *node);

            if (ImGui::MenuItem("Delete"))
            {
                ImGui::EndPopup();
                if (opened && node->children.empty() == false)
                    ImGui::TreePop();
                // DeleteNode erases `node` from its parent's children vector,
                // which is the storage our `node` reference is bound to --
                // don't touch `node` (or `opened`/anything derived from it)
                // after this call.
                DeleteNode(session, *node);
                return;
            }
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

    std::string nameBuf = node->name;
    ImGui::PushItemWidth(220);
    if (InputTextStd("Name", nameBuf) && ImGui::IsItemDeactivatedAfterEdit())
        RenameNode(session, *node, nameBuf);
    ImGui::PopItemWidth();

    if (node->name.size() > 0 && node->name[0] == '$')
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
    }

    ImGui::PopID();
}

#endif // UI_EDITOR_ENABLED

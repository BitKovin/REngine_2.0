// ─────────────────────────────────────────────────────────────────────────
// Append the contents of this file to the bottom of UiElement.cpp (or
// #include it from there). Kept separate here purely so the new layout/
// editor-system code is easy to review on its own.
// ─────────────────────────────────────────────────────────────────────────

#include "UiElement.h"
#include "Layout/UiLayoutSerializer.h"
#include "Reflection/UiElementRegistry.h"

#include <FileSystem/FileSystem.h> // FileSystemEngine::ReadFile / GetFileModificationTime
#include "Logger.hpp"

void UiElement::LoadLayoutFromFile(const std::string& path)
{
    if (OwnedLayout)
    {
        Logger::Log("UiElement::LoadLayoutFromFile('" + path + "'): this element already owns layout '" +
            OwnedLayout->path + "'. Call Rebuild() or ReloadFromDisk() instead of loading a second file.");
        return;
    }

    const std::string jsonText = FileSystemEngine::ReadFile(path);
    if (jsonText.empty())
    {
        Logger::Log("UiElement::LoadLayoutFromFile: could not read '" + path + "'");
        return;
    }

    LayoutBlueprintNodePtr root = UiLayoutSerializer::ParseBlueprint(jsonText);
    if (!root)
        return; // ParseBlueprint already logged the parse error

    // Sanity check only, non-fatal: the file's declared root type is purely
    // documentation/validation once an owner already exists in C++ (the
    // owner's real type is fixed by whoever wrote `new UiStyledButton()`).
    // A mismatch almost always means the wrong file was pointed at.
    const std::string actualTypeName = UiElementRegistry::Instance().GetTypeNameForInstance(this);
    if (!actualTypeName.empty() && !root->type.empty() && root->type != actualTypeName)
    {
        Logger::Log("UiElement::LoadLayoutFromFile('" + path + "'): file's root type '" + root->type +
            "' doesn't match this element's actual type '" + actualTypeName + "' -- wrong file?");
    }

    auto layout = std::make_shared<LoadedLayoutInstance>();
    layout->path          = path;
    layout->blueprintRoot = root;
    layout->ownerElement  = this;
#if UI_LAYOUT_HOT_RELOAD_ENABLED
    layout->lastLoadedModTime = FileSystemEngine::GetFileModificationTime(path);
#endif

    OwnedLayout     = layout;
    MemberOfLayout  = layout.get(); // an owner's own layout always takes priority -- see header comment

    UiLayoutSerializer::ApplyProperties(this, *root);      // root node's own properties -> onto `this`
    UiLayoutSerializer::BuildChildren(this, *root, layout.get()); // root node's children -> children of `this`

    LoadedLayoutRegistry::Instance().Register(layout);
}

void UiElement::Rebuild()
{
    if (!OwnedLayout)
        return; // not a layout owner; nothing to rebuild

    ClearChildren();
    OwnedLayout->namedElements.clear();

    UiLayoutSerializer::ApplyProperties(this, *OwnedLayout->blueprintRoot);
    UiLayoutSerializer::BuildChildren(this, *OwnedLayout->blueprintRoot, OwnedLayout.get());

    UpdateChildrenOffsetRecursive();

    OnLayoutReloaded();
}

#if UI_LAYOUT_HOT_RELOAD_ENABLED
void UiElement::ReloadFromDisk()
{
    if (!OwnedLayout)
        return;

    const std::string jsonText = FileSystemEngine::ReadFile(OwnedLayout->path);
    if (jsonText.empty())
    {
        Logger::Log("UiElement::ReloadFromDisk: could not read '" + OwnedLayout->path + "'");
        return;
    }

    LayoutBlueprintNodePtr newRoot = UiLayoutSerializer::ParseBlueprint(jsonText);
    if (!newRoot)
        return;

    OwnedLayout->blueprintRoot     = newRoot;
    OwnedLayout->lastLoadedModTime = FileSystemEngine::GetFileModificationTime(OwnedLayout->path);

    Rebuild();
}
#endif

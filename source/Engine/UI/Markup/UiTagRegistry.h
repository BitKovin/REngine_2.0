#pragma once
#include <memory>
#include <string>
#include <vector>

class UiElem;

namespace UiTagRegistry
{
    using Factory = std::shared_ptr<UiElem>(*)();

    struct AttributeInfo
    {
        std::string Name;
        std::string Type;          // "string", "bool", "number", "path", "text"
        std::string Default;
        bool Required = false;
        std::string Description;
    };
    struct TagInfo
    {
        std::string Name;
        std::string Category;      // "layout", "text", "image", "input"
        std::string Description;
        std::vector<AttributeInfo> Attributes;
    };

    void RegisterBuiltin(const std::string& tag, Factory f, const TagInfo& info);   // Panel, Label, Image, Button, TextButton
    Factory FindBuiltin(const std::string& tag);
    const std::vector<TagInfo>& BuiltinInfos();                                     // sorted by name
    // Standard attributes every element accepts (id, class, style, visible, ...).
    const std::vector<AttributeInfo>& StandardAttributes();

    // Code-behind: a layout path -> class. UiElem::Load(path) constructs the class (which calls LoadLayout itself).
    void RegisterCodeBehind(const std::string& layoutPath, Factory f);
    Factory FindCodeBehind(const std::string& layoutPath);
}

#define UI_CONCAT_INNER(a, b) a##b
#define UI_CONCAT(a, b) UI_CONCAT_INNER(a, b)
#define UI_REGISTER_CODE_BEHIND(PATH, CLASS) \
    static const bool UI_CONCAT(_uiCodeBehind_, __LINE__) = (UiTagRegistry::RegisterCodeBehind(PATH, []() -> std::shared_ptr<UiElem> { return std::make_shared<CLASS>(); }), true)

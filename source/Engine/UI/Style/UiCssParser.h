#pragma once
#include <UI/Style/UiCssTokenizer.h>
#include <UI/Style/UiSelector.h>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct UiCssDeclaration
{
    std::string Name;                        // lowercased; custom properties keep their case after "--"
    std::vector<UiCssToken> Value;           // tokens, leading/trailing whitespace trimmed
    int Line = 0, Col = 0;
};

struct UiCssRule
{
    std::vector<UiSelector> Selectors;       // selector list
    std::vector<UiCssDeclaration> Declarations;
    int SourceOrder = 0;                     // increasing across a sheet
};

struct UiFontFaceRule { std::string Family; std::string Src; int Weight = 400; bool Italic = false; int Line = 0; };

// One keyframe of an @keyframes rule: the offset (0..1) and its declarations.
struct UiKeyframeStop
{
    float Offset = 0.f;
    std::vector<UiCssDeclaration> Declarations;
};

struct UiKeyframes
{
    std::string Name;
    std::vector<UiKeyframeStop> Stops;   // sorted by offset
    std::string Path;
    int Line = 0;
};
using UiKeyframeMap = std::map<std::string, UiKeyframes>;

struct UiStyleSheet
{
    std::string Path;                        // file path, or "<inline:file.xml:LINE>" for <style> blocks
    std::vector<UiCssRule> Rules;
    std::vector<std::string> Imports;        // @import targets, resolved before this sheet's own rules
    std::vector<UiFontFaceRule> FontFaces;
    std::vector<UiKeyframes> Keyframes;
    // [P4] media queries
};

// `lineOffset` is added to every token line (used for <style> blocks inside XML).
std::shared_ptr<UiStyleSheet> UiParseStyleSheet(const std::string& source, const std::string& path, int lineOffset);

// Parses "a: b; c: d" (a style="" attribute) into declarations.
std::vector<UiCssDeclaration> UiParseDeclarationList(const std::string& source, const std::string& path, int line, int col);

class UiStyleSheetCache
{
public:
    // Returns the parsed sheet for a .css path (cached), or nullptr after logging when the file is missing.
    static std::shared_ptr<UiStyleSheet> Get(const std::string& path);
    static void Invalidate(const std::string& path);   // hot reload
    static std::vector<std::shared_ptr<UiStyleSheet>> Cached();   // every parsed .css file (the UI database lists their @keyframes)
    static uint32_t ModTime(const std::string& path);
};

#pragma once
#include <glm.h>
#include <UI/Render/UiTextLayout.h>
#include <UI/Style/UiCssParser.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

enum class UiLenKind { Px, Percent, Calc, FitChildren, FillParentFlow, HeightPercentage, WidthPercentage, Auto };

struct UiLength
{
    UiLenKind Kind = UiLenKind::Px;
    float Px = 0.f;        // Px, Calc (pixel part)
    float Percent = 0.f;   // Percent, Calc (percent part), Height/WidthPercentage (0..100)
    float Weight = 1.f;    // FillParentFlow
    float Resolve(float basis) const { return Px + Percent * 0.01f * basis; }   // Px, Percent, Calc only
    bool IsResolvable() const { return Kind == UiLenKind::Px || Kind == UiLenKind::Percent || Kind == UiLenKind::Calc; }
    bool HasPercent() const { return (Kind == UiLenKind::Percent || Kind == UiLenKind::Calc) && Percent != 0.f; }
    bool operator==(const UiLength&) const = default;
};

enum class UiFlow { None, Down, Right, DownWrap, RightWrap };
enum class UiHAlign { Left, Center, Right };
enum class UiVAlign { Top, Center, Bottom };
enum class UiOverflow { Visible, Clip };
enum class UiVisibility { Visible, Hidden, Collapse };
enum class UiObjectFit { Fill, Contain, Cover, None, ScaleDown };

struct UiComputedStyle
{
    // ── layout (changing these needs re-layout)
    UiLength Width  { UiLenKind::FitChildren };
    UiLength Height { UiLenKind::FitChildren };
    UiLength MinWidth { UiLenKind::Px }, MaxWidth { UiLenKind::Auto }, MinHeight { UiLenKind::Px }, MaxHeight { UiLenKind::Auto };
    UiFlow Flow = UiFlow::None;
    UiHAlign HAlign = UiHAlign::Left;
    UiVAlign VAlign = UiVAlign::Top;
    UiLength X, Y;                                      // offsets, % of parent content width / height
    UiLength MarginTop, MarginRight, MarginBottom, MarginLeft;
    UiLength PaddingTop, PaddingRight, PaddingBottom, PaddingLeft;
    UiLength Gap;
    UiOverflow Overflow = UiOverflow::Visible;
    UiVisibility Visibility = UiVisibility::Visible;    // inherited
    int ZIndex = 0;
    // ── box paint
    glm::vec4 BackgroundColor = glm::vec4(0.f);        // straight RGBA
    float BorderWidth[4] = { 3, 3, 3, 3 };              // top right bottom left, px; counts only when BorderSolid (initial `medium`)
    glm::vec4 BorderColor[4] = { glm::vec4(1.f), glm::vec4(1.f), glm::vec4(1.f), glm::vec4(1.f) };
    bool BorderSolid[4] = { false, false, false, false }; // style none => width treated as 0
    UiLength Radius[4];                                  // top-left top-right bottom-right bottom-left (% of the box's smaller side)
    float Opacity = 1.f;
    // ── text (inherited unless noted)
    glm::vec4 Color = glm::vec4(1.f);
    std::string FontFamily = "default";
    float FontSize = 24.f;                               // resolved px
    int FontWeight = 400;
    bool FontItalic = false;
    float LetterSpacing = 0.f;
    float LineHeight = -1.f;                             // -1 normal, else px
    UiTextAlign TextAlign = UiTextAlign::Left;
    UiWhiteSpace WhiteSpace = UiWhiteSpace::Normal;
    UiTextOverflow TextOverflow = UiTextOverflow::Clip;  // NOT inherited
    UiTextTransform TextTransform = UiTextTransform::None;
    // ── image (Image element)
    UiObjectFit ObjectFit = UiObjectFit::Fill;
    // ── custom properties (inherited): name (with "--") -> raw tokens
    std::unordered_map<std::string, std::vector<UiCssToken>> Custom;

    float EffectiveBorder(int side) const { return BorderSolid[side] ? BorderWidth[side] : 0.f; }
};

// Longhand properties produced by shorthand expansion: name -> value tokens.
using UiLonghandList = std::vector<std::pair<std::string, std::vector<UiCssToken>>>;

struct UiStyleParseContext
{
    float ParentFontSize = 24.f;
    float FontSize = 24.f;                 // this element's font size, for em
    float RootFontSize = 24.f;             // rem (P1-D4)
    glm::vec4 CurrentColor = glm::vec4(1.f);
    glm::vec2 Viewport = glm::vec2(1920.f, 1080.f);   // vw / vh
};

// One entry of the property table (also published into the UI database).
struct UiPropertyInfo
{
    const char* Name;
    const char* Grammar;
    bool Inherited;
    const char* Initial;
    const char* Invalidates;   // "layout", "paint", "layout+paint", "style"
    bool Longhand;             // false for shorthands
};
const std::vector<UiPropertyInfo>& UiStyleProperties();
bool UiIsKnownProperty(const std::string& name);

// Replaces var(--x[, fallback]) in `in`. Returns false and sets `error` on a missing variable or a cycle.
bool UiSubstituteVars(const std::vector<UiCssToken>& in, const std::unordered_map<std::string, std::vector<UiCssToken>>& custom,
                      std::vector<UiCssToken>& out, std::string& error);

// Expands a (var-substituted) shorthand to longhands; a longhand returns itself. False + error on bad syntax.
bool UiExpandProperty(const std::string& name, const std::vector<UiCssToken>& value, UiLonghandList& out, std::string& error);

// Parses one longhand value into `style`. False + error leaves `style` untouched for that property.
bool UiApplyLonghand(UiComputedStyle& style, const std::string& name, const std::vector<UiCssToken>& value,
                     const UiStyleParseContext& ctx, std::string& error);

// Splits a value into whitespace-separated components (functions stay in one piece).
std::vector<std::vector<UiCssToken>> UiSplitComponents(const std::vector<UiCssToken>& value);

#pragma once
#include <UI/Render/UiPainter.h>
#include <string>
#include <vector>

// PreWrap keeps spaces and newlines like Pre and wraps at the wrap width like Normal (text fields).
enum class UiWhiteSpace { Normal, NoWrap, Pre, PreWrap };
enum class UiTextAlign { Left, Center, Right };
enum class UiTextOverflow { Clip, Ellipsis, Shrink };
enum class UiTextTransform { None, Uppercase, Lowercase };

struct UiTextParams
{
    std::string Text;             // already localised and variable-substituted
    std::string Family = "default";
    int Weight = 400;
    bool Italic = false;
    float FontSize = 24.f;        // UI units per em
    float LetterSpacing = 0.f;    // UI units
    float LineHeight = -1.f;      // UI units; < 0 = normal
    float WrapWidth = -1.f;       // UI units; < 0 = no wrapping
    float MaxHeight = -1.f;       // for Shrink; < 0 = unlimited
    UiWhiteSpace WhiteSpace = UiWhiteSpace::Normal;
    UiTextAlign Align = UiTextAlign::Left;
    UiTextOverflow Overflow = UiTextOverflow::Clip;
    UiTextTransform Transform = UiTextTransform::None;
    bool Html = false;            // rich text tags are interpreted
    bool WantCarets = false;      // fill UiTextLayoutResult::Carets (needs Pre or PreWrap, no Html, no Transform, and no '\t' or '\r' in the text)
    bool operator==(const UiTextParams&) const = default;
};

// Consecutive quads that share a style (colour, size, weight). HasColor false = the element's colour.
struct UiTextRun { size_t First = 0, Count = 0; bool HasColor = false; glm::vec4 Color = glm::vec4(1.f); float GlyphScale = 0.f; float Dilation = 0.f; };
// An inline image; the label resolves action prompts through UiInputPrompts and paints it with UiPainter::Image.
struct UiTextIcon { float X = 0.f, Y = 0.f, W = 0.f, H = 0.f; bool IsAction = false; std::string Ref; int VAlign = 0; /* 0 baseline, 1 middle, 2 top, 3 bottom (layout input only) */ };

// Where the caret sits between two characters: one stop per boundary 0..N of the N code points of the text. At a soft wrap the
// boundary belongs to the start of the next line; at a newline it stays at the end of the line it ends.
struct UiCaretStop { float X = 0.f, Y = 0.f; int Line = 0; };

// The span of one laid-out line, in caret boundaries: [First, End]. EndX is where the line ends (End itself may be drawn as the start of the next line).
struct UiLineInfo { int First = 0, End = 0; float Y = 0.f, EndX = 0.f; };

struct UiTextLayoutResult
{
    std::vector<UiTextRun> Runs;
    std::vector<UiTextIcon> Icons;
    std::vector<UiGlyphQuad> Quads;   // positions relative to the text box top-left
    float Width = 0.f, Height = 0.f;  // measured box
    int Lines = 0;
    float UsedFontSize = 0.f;         // differs from FontSize after Shrink
    float GlyphScale = 0.f;           // UsedFontSize / kBaseSize
    float Dilation = 0.f;             // synthetic bold, UI units
    std::string DisplayText;          // what is drawn: transformed, wrapped lines joined by '\n', with the ellipsis
    float LineHeight = 0.f;           // height of one line, also when the text is empty
    std::vector<UiCaretStop> Carets;  // WantCarets only; Y is the top of the line
    std::vector<UiLineInfo> LineInfo; // WantCarets only; one per line (none for empty text)
};

// Pure function of params. Callers cache by comparing params.
UiTextLayoutResult UiLayoutText(const UiTextParams& params);

// UTF-8 helpers shared with other UI code.
std::vector<uint32_t> UiDecodeUtf8(const std::string& text);
std::string UiEncodeUtf8(const std::vector<uint32_t>& codepoints);

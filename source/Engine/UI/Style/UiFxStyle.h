#pragma once
#include <UI/Style/UiLength.h>
#include <glm.h>
#include <map>
#include <string>
#include <vector>

// Computed values of the phase 3 painting and motion properties (the members live in UiComputedStyle).

// ── gradients and backgrounds
struct UiGradStop
{
    glm::vec4 Color = glm::vec4(1.f);   // straight RGBA
    bool HasPos = false;
    UiLength Pos;                       // Px or Percent of the gradient line
    bool operator==(const UiGradStop&) const = default;
};

enum class UiGradKind { None, Linear, Radial, Conic };

struct UiGradient
{
    UiGradKind Kind = UiGradKind::None;
    bool Repeating = false;
    float AngleDeg = 180.f;             // linear: direction as in CSS (0 = to top, 90 = to right, 180 = to bottom)
    bool ToCorner = false;              // linear "to top right" etc.: the direction depends on the box
    int CornerX = 0, CornerY = 0;       // -1 left / top, +1 right / bottom
    bool Circle = false;                // radial: circle instead of ellipse
    int SizeKeyword = 0;                // radial: 0 farthest-corner, 1 closest-side, 2 farthest-side, 3 closest-corner, 4 explicit
    UiLength RadiusX, RadiusY;          // radial, explicit sizes (RadiusY unused for a circle)
    UiLength PosX, PosY;                // radial / conic centre
    float FromDeg = 0.f;                // conic start angle
    std::vector<UiGradStop> Stops;
    bool operator==(const UiGradient&) const = default;

    UiGradient()
    {
        PosX.Kind = PosY.Kind = UiLenKind::Percent;
        PosX.Percent = PosY.Percent = 50.f;
    }
};

struct UiBgImage
{
    enum class Type { Url, Gradient } Kind = Type::Gradient;
    std::string Url;
    UiGradient Gradient;
    bool operator==(const UiBgImage&) const = default;
};

struct UiBgSize
{
    enum class Mode { Auto, Cover, Contain, Explicit } M = Mode::Auto;
    UiLength W, H;
    bool WAuto = true, HAuto = true;
    bool operator==(const UiBgSize&) const = default;
};

struct UiBgPos
{
    UiLength X, Y;
    bool operator==(const UiBgPos&) const = default;
    UiBgPos()
    {
        X.Kind = Y.Kind = UiLenKind::Percent;
        X.Percent = Y.Percent = 0.f;
    }
};

struct UiBgRepeat
{
    bool X = true, Y = true;
    bool operator==(const UiBgRepeat&) const = default;
};

struct UiBorderImage
{
    std::string Src;
    float Slice[4] = { 100.f, 100.f, 100.f, 100.f };   // top right bottom left; a number is texels, a percentage is of the image size
    bool SlicePercent[4] = { true, true, true, true };
    bool Fill = false;
    float Width[4] = { 1.f, 1.f, 1.f, 1.f };           // multiples of the border widths
    bool Round = false;                                  // repeat: stretch (false) | round (true)
    bool operator==(const UiBorderImage& o) const
    {
        return Src == o.Src && Fill == o.Fill && Round == o.Round &&
               Slice[0] == o.Slice[0] && Slice[1] == o.Slice[1] && Slice[2] == o.Slice[2] && Slice[3] == o.Slice[3] &&
               SlicePercent[0] == o.SlicePercent[0] && SlicePercent[1] == o.SlicePercent[1] &&
               SlicePercent[2] == o.SlicePercent[2] && SlicePercent[3] == o.SlicePercent[3] &&
               Width[0] == o.Width[0] && Width[1] == o.Width[1] && Width[2] == o.Width[2] && Width[3] == o.Width[3];
    }
};

// ── shadows, outline, text effects
struct UiShadow
{
    float X = 0.f, Y = 0.f, Blur = 0.f, Spread = 0.f;
    glm::vec4 Color = glm::vec4(0.f, 0.f, 0.f, 1.f);
    bool Inset = false;
    bool operator==(const UiShadow&) const = default;
};

// ── filters (colour matrices)
// Blur and DropShadow are not colour matrices: they need an offscreen layer (phase 7).
enum class UiFilterType { Brightness, Contrast, Saturate, Grayscale, Sepia, HueRotate, Invert, Blur, DropShadow };
struct UiFilterFn
{
    UiFilterType Type = UiFilterType::Brightness;
    float Amount = 1.f;                 // factor; degrees for hue-rotate; blur: standard deviation in px
    UiShadow Shadow;                    // drop-shadow: offset, blur radius (sigma = radius / 2) and colour
    bool IsLayerEffect() const { return Type == UiFilterType::Blur || Type == UiFilterType::DropShadow; }
    bool operator==(const UiFilterFn&) const = default;
};

// ── clip-path
struct UiClipPath
{
    enum class Shape { None, Inset, Circle, Ellipse } Kind = Shape::None;
    UiLength A[4];                      // inset: top right bottom left; circle: radius in A[0]; ellipse: radii in A[0], A[1]
    bool HasRadius[2] = { false, false }; // circle / ellipse: radius given (otherwise closest-side style default of 50%)
    UiLength Round[4];                  // inset ... round: corner radii
    UiLength CX, CY;                    // circle / ellipse centre
    bool operator==(const UiClipPath&) const = default;
    UiClipPath()
    {
        CX.Kind = CY.Kind = UiLenKind::Percent;
        CX.Percent = CY.Percent = 50.f;
    }
};

enum class UiBlend { Normal, PlusLighter, Multiply, Screen };
enum class UiImageRendering { Auto, Pixelated };

// ── transforms
enum class UiTransformFnType { Translate, Rotate, Scale, Skew };
struct UiTransformFn
{
    UiTransformFnType Type = UiTransformFnType::Translate;
    UiLength A, B;                      // translate x, y
    float V0 = 0.f, V1 = 0.f;           // rotate: degrees; scale: sx, sy; skew: ax, ay degrees
    bool operator==(const UiTransformFn&) const = default;
};

// ── motion
struct UiEasing
{
    enum class Kind { CubicBezier, Steps } K = Kind::CubicBezier;
    float X1 = 0.25f, Y1 = 0.1f, X2 = 0.25f, Y2 = 1.f;   // "ease"
    int Steps = 1;
    bool StepStart = false;
    bool operator==(const UiEasing&) const = default;
    float Evaluate(float t) const;
    static UiEasing Linear() { UiEasing e; e.X1 = 0.f; e.Y1 = 0.f; e.X2 = 1.f; e.Y2 = 1.f; return e; }
};

struct UiTransitionDef
{
    std::string Property = "all";
    float Duration = 0.f, Delay = 0.f;
    UiEasing Easing;
    bool operator==(const UiTransitionDef&) const = default;
};

struct UiAnimationDef
{
    enum class Direction { Normal, Reverse, Alternate, AlternateReverse } Dir = Direction::Normal;
    enum class Fill { None, Forwards, Backwards, Both } FillMode = Fill::None;
    std::string Name;
    float Duration = 0.f, Delay = 0.f;
    float Iterations = 1.f;             // < 0 = infinite
    UiEasing Easing;
    bool operator==(const UiAnimationDef&) const = default;
};

// The painting and motion properties of one element, as parsed. `Is*` helpers keep the painter free of defaults checks.
struct UiFxStyle
{
    // background layers: the lists are combined per layer (cycling) when painting
    std::vector<UiBgImage> BgImages;
    std::vector<UiBgSize> BgSizes;
    std::vector<UiBgPos> BgPositions;
    std::vector<UiBgRepeat> BgRepeats;
    UiBorderImage BorderImage;
    std::vector<UiShadow> BoxShadow;
    // outline
    float OutlineWidth = 3.f;
    bool OutlineSolid = false;
    glm::vec4 OutlineColor = glm::vec4(1.f);
    float OutlineOffset = 0.f;
    // text (inherited)
    std::vector<UiShadow> TextShadow;
    float TextStrokeWidth = 0.f;
    glm::vec4 TextStrokeColor = glm::vec4(1.f);
    // effects
    std::vector<UiFilterFn> Filter;
    std::vector<UiFilterFn> BackdropFilter;   // backdrop-filter: blur() and the colour functions, applied to the scene behind the element
    bool Isolate = false;                     // isolation: isolate (a group: opacity and blend modes act on the whole subtree)
    bool CacheLayer = false;                  // cache: layer (the group is rendered once and reused until its content changes)
    glm::vec4 WashColor = glm::vec4(0.f);
    UiGradient MaskImage;
    UiClipPath ClipPath;
    UiBlend MixBlend = UiBlend::Normal;
    UiImageRendering ImageRendering = UiImageRendering::Auto;
    UiBgPos ObjectPosition;
    std::vector<UiTransformFn> Transform;
    UiLength OriginX, OriginY;
    // motion
    std::vector<UiTransitionDef> Transitions;
    std::vector<UiAnimationDef> Animations;
    // pulse-duration: how long an event (click, doubleclick, denied, ...) keeps the element in its pulse pseudo-class
    float PulseDuration = 0.2f;

    UiFxStyle()
    {
        OriginX.Kind = OriginY.Kind = UiLenKind::Percent;
        OriginX.Percent = OriginY.Percent = 50.f;
        ObjectPosition.X.Percent = ObjectPosition.Y.Percent = 50.f;
    }
};

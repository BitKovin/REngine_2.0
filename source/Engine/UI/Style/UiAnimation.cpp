#include "UiAnimation.h"

#include <EngineMain.h>
#include <Helpers/CubicBezierEasing.hpp>
#include <UI/Elements/UiElem.h>
#include <UI/Markup/UiMarkupLog.h>
#include <UI/Style/UiStyleResolver.h>

#include <algorithm>
#include <cmath>
#include <set>

float UiEasing::Evaluate(float t) const
{
    if (K == Kind::Steps)
    {
        if (t >= 1.f) return 1.f;
        const float n = static_cast<float>(Steps);
        const float s = StepStart ? std::max(std::ceil(t * n), 1.f) : std::floor(std::max(t, 0.f) * n);
        return std::clamp(s / n, 0.f, 1.f);
    }
    if (X1 == Y1 && X2 == Y2)
        return t;   // linear
    const CubicBezierEasing bezier(std::clamp(X1, 0.f, 1.f), Y1, std::clamp(X2, 0.f, 1.f), Y2);
    return static_cast<float>(bezier(t));
}

namespace
{
    // ── animated values ───────────────────────────────────────────────────────

    enum class VT { Float, Length, Color, Transform, Filter, Shadows };
    enum Inv { kPaint = 0, kLayout = 1, kTransform = 2 };

    struct V
    {
        VT T = VT::Float;
        float F = 0.f;
        UiLength L;
        glm::vec4 C = glm::vec4(0.f);
        std::vector<UiTransformFn> Tf;
        std::vector<UiFilterFn> Fl;
        std::vector<UiShadow> Sh;

        bool operator==(const V& o) const
        {
            if (T != o.T) return false;
            switch (T)
            {
            case VT::Float: return F == o.F;
            case VT::Length: return L == o.L;
            case VT::Color: return C == o.C;
            case VT::Transform: return Tf == o.Tf;
            case VT::Filter: return Fl == o.Fl;
            case VT::Shadows: return Sh == o.Sh;
            }
            return false;
        }
        bool operator!=(const V& o) const { return !(*this == o); }
    };

    V FromFloat(float f) { V v; v.T = VT::Float; v.F = f; return v; }
    V FromLen(const UiLength& l) { V v; v.T = VT::Length; v.L = l; return v; }
    V FromColor(const glm::vec4& c) { V v; v.T = VT::Color; v.C = c; return v; }

    struct AnimProp
    {
        const char* Name;
        VT Type;
        Inv Invalidates;
        V (*Get)(const UiComputedStyle&);
        void (*Set)(UiComputedStyle&, const V&);
    };

#define UI_FLOAT_PROP(NAME, FIELD, INV) { NAME, VT::Float, INV, [](const UiComputedStyle& s) { return FromFloat(s.FIELD); }, [](UiComputedStyle& s, const V& v) { s.FIELD = v.F; } }
#define UI_LEN_PROP(NAME, FIELD, INV) { NAME, VT::Length, INV, [](const UiComputedStyle& s) { return FromLen(s.FIELD); }, [](UiComputedStyle& s, const V& v) { s.FIELD = v.L; } }
#define UI_COLOR_PROP(NAME, FIELD, INV) { NAME, VT::Color, INV, [](const UiComputedStyle& s) { return FromColor(s.FIELD); }, [](UiComputedStyle& s, const V& v) { s.FIELD = v.C; } }

    const std::vector<AnimProp>& Props()
    {
        static const std::vector<AnimProp> props = {
            UI_FLOAT_PROP("opacity", Opacity, kPaint),
            UI_COLOR_PROP("background-color", BackgroundColor, kPaint),
            UI_COLOR_PROP("color", Color, kPaint),
            UI_COLOR_PROP("border-top-color", BorderColor[0], kPaint),
            UI_COLOR_PROP("border-right-color", BorderColor[1], kPaint),
            UI_COLOR_PROP("border-bottom-color", BorderColor[2], kPaint),
            UI_COLOR_PROP("border-left-color", BorderColor[3], kPaint),
            UI_FLOAT_PROP("border-top-width", BorderWidth[0], kLayout),
            UI_FLOAT_PROP("border-right-width", BorderWidth[1], kLayout),
            UI_FLOAT_PROP("border-bottom-width", BorderWidth[2], kLayout),
            UI_FLOAT_PROP("border-left-width", BorderWidth[3], kLayout),
            UI_LEN_PROP("border-top-left-radius", Radius[0], kPaint),
            UI_LEN_PROP("border-top-right-radius", Radius[1], kPaint),
            UI_LEN_PROP("border-bottom-right-radius", Radius[2], kPaint),
            UI_LEN_PROP("border-bottom-left-radius", Radius[3], kPaint),
            UI_LEN_PROP("width", Width, kLayout),
            UI_LEN_PROP("height", Height, kLayout),
            UI_LEN_PROP("min-width", MinWidth, kLayout),
            UI_LEN_PROP("min-height", MinHeight, kLayout),
            UI_LEN_PROP("max-width", MaxWidth, kLayout),
            UI_LEN_PROP("max-height", MaxHeight, kLayout),
            UI_LEN_PROP("x", X, kLayout),
            UI_LEN_PROP("y", Y, kLayout),
            UI_LEN_PROP("margin-top", MarginTop, kLayout),
            UI_LEN_PROP("margin-right", MarginRight, kLayout),
            UI_LEN_PROP("margin-bottom", MarginBottom, kLayout),
            UI_LEN_PROP("margin-left", MarginLeft, kLayout),
            UI_LEN_PROP("padding-top", PaddingTop, kLayout),
            UI_LEN_PROP("padding-right", PaddingRight, kLayout),
            UI_LEN_PROP("padding-bottom", PaddingBottom, kLayout),
            UI_LEN_PROP("padding-left", PaddingLeft, kLayout),
            UI_LEN_PROP("gap", Gap, kLayout),
            UI_COLOR_PROP("outline-color", Fx.OutlineColor, kPaint),
            UI_FLOAT_PROP("outline-width", Fx.OutlineWidth, kPaint),
            UI_FLOAT_PROP("outline-offset", Fx.OutlineOffset, kPaint),
            UI_COLOR_PROP("wash-color", Fx.WashColor, kPaint),
            { "transform", VT::Transform, kTransform,
              [](const UiComputedStyle& s) { V v; v.T = VT::Transform; v.Tf = s.Fx.Transform; return v; },
              [](UiComputedStyle& s, const V& v) { s.Fx.Transform = v.Tf; } },
            { "filter", VT::Filter, kPaint,
              [](const UiComputedStyle& s) { V v; v.T = VT::Filter; v.Fl = s.Fx.Filter; return v; },
              [](UiComputedStyle& s, const V& v) { s.Fx.Filter = v.Fl; } },
            { "box-shadow", VT::Shadows, kPaint,
              [](const UiComputedStyle& s) { V v; v.T = VT::Shadows; v.Sh = s.Fx.BoxShadow; return v; },
              [](UiComputedStyle& s, const V& v) { s.Fx.BoxShadow = v.Sh; } },
            { "text-shadow", VT::Shadows, kPaint,
              [](const UiComputedStyle& s) { V v; v.T = VT::Shadows; v.Sh = s.Fx.TextShadow; return v; },
              [](UiComputedStyle& s, const V& v) { s.Fx.TextShadow = v.Sh; } },
        };
        return props;
    }

    int FindProp(const std::string& name)
    {
        const auto& props = Props();
        for (size_t i = 0; i < props.size(); ++i)
            if (name == props[i].Name)
                return static_cast<int>(i);
        return -1;
    }

    // ── interpolation ─────────────────────────────────────────────────────────

    float Lerp(float a, float b, float t) { return a + (b - a) * t; }

    glm::vec4 LerpColor(const glm::vec4& a, const glm::vec4& b, float t)
    {
        const glm::vec4 pa(glm::vec3(a) * a.a, a.a), pb(glm::vec3(b) * b.a, b.a);
        const glm::vec4 p = pa + (pb - pa) * t;
        return p.a > 0.00001f ? glm::vec4(glm::vec3(p) / p.a, p.a) : glm::vec4(0.f);
    }

    UiLength LerpLength(const UiLength& a, const UiLength& b, float t)
    {
        const bool resolvable = a.IsResolvable() && b.IsResolvable();
        if (resolvable && a.Kind == b.Kind)
        {
            UiLength r = a;
            r.Px = Lerp(a.Px, b.Px, t);
            r.Percent = Lerp(a.Percent, b.Percent, t);
            return r;
        }
        return t < 0.5f ? a : b;   // keywords and px <-> % do not interpolate
    }

    glm::mat3 Translation(float x, float y)
    {
        glm::mat3 m(1.f);
        m[2][0] = x;
        m[2][1] = y;
        return m;
    }

    glm::mat3 FnMatrix(const UiTransformFn& f, float bw, float bh)
    {
        constexpr float kRad = 0.01745329252f;
        glm::mat3 m(1.f);
        switch (f.Type)
        {
        case UiTransformFnType::Translate:
            return Translation(f.A.Resolve(bw), f.B.Resolve(bh));
        case UiTransformFnType::Rotate:
        {
            const float c = std::cos(f.V0 * kRad), s = std::sin(f.V0 * kRad);
            m[0][0] = c; m[0][1] = s; m[1][0] = -s; m[1][1] = c;
            return m;
        }
        case UiTransformFnType::Scale:
            m[0][0] = f.V0;
            m[1][1] = f.V1;
            return m;
        case UiTransformFnType::Skew:
            m[1][0] = std::tan(f.V0 * kRad);
            m[0][1] = std::tan(f.V1 * kRad);
            return m;
        }
        return m;
    }

    glm::mat3 ListMatrix(const std::vector<UiTransformFn>& list, float bw, float bh)
    {
        glm::mat3 m(1.f);
        for (const UiTransformFn& f : list)
            m = m * FnMatrix(f, bw, bh);
        return m;
    }

    UiTransformFn Identity(UiTransformFnType type)
    {
        UiTransformFn f;
        f.Type = type;
        if (type == UiTransformFnType::Scale)
            f.V0 = f.V1 = 1.f;
        return f;
    }

    // 2D decomposition into translate, rotate, skewX and scale, interpolated and recomposed as a function list.
    std::vector<UiTransformFn> InterpolateMatrices(const glm::mat3& a, const glm::mat3& b, float t)
    {
        struct Parts { float tx, ty, angle, skew, sx, sy; };
        auto decompose = [](const glm::mat3& m) {
            float ca = m[0][0], cb = m[0][1], cc = m[1][0], cd = m[1][1];
            Parts p{ m[2][0], m[2][1], 0.f, 0.f, 1.f, 1.f };
            p.sx = std::sqrt(ca * ca + cb * cb);
            if (p.sx > 0.00001f) { ca /= p.sx; cb /= p.sx; }
            float skew = ca * cc + cb * cd;
            cc -= ca * skew;
            cd -= cb * skew;
            p.sy = std::sqrt(cc * cc + cd * cd);
            if (p.sy > 0.00001f) { cc /= p.sy; cd /= p.sy; skew /= p.sy; }
            if (ca * cd - cb * cc < 0.f) { ca = -ca; cb = -cb; skew = -skew; p.sx = -p.sx; }
            p.angle = std::atan2(cb, ca) * 57.2957795f;
            p.skew = std::atan(skew) * 57.2957795f;
            return p;
        };
        const Parts pa = decompose(a), pb = decompose(b);
        float angleB = pb.angle;
        // rotate the short way round
        while (angleB - pa.angle > 180.f) angleB -= 360.f;
        while (angleB - pa.angle < -180.f) angleB += 360.f;
        std::vector<UiTransformFn> out(4);
        out[0].Type = UiTransformFnType::Translate;
        out[0].A.Px = Lerp(pa.tx, pb.tx, t);
        out[0].B.Px = Lerp(pa.ty, pb.ty, t);
        out[1].Type = UiTransformFnType::Rotate;
        out[1].V0 = Lerp(pa.angle, angleB, t);
        out[2].Type = UiTransformFnType::Skew;
        out[2].V0 = Lerp(pa.skew, pb.skew, t);
        out[3].Type = UiTransformFnType::Scale;
        out[3].V0 = Lerp(pa.sx, pb.sx, t);
        out[3].V1 = Lerp(pa.sy, pb.sy, t);
        return out;
    }

    std::vector<UiTransformFn> InterpolateTransforms(std::vector<UiTransformFn> a, std::vector<UiTransformFn> b, float t, float bw, float bh)
    {
        // "none" pads with identities of the other list's functions
        if (a.empty())
            for (const UiTransformFn& f : b) a.push_back(Identity(f.Type));
        if (b.empty())
            for (const UiTransformFn& f : a) b.push_back(Identity(f.Type));
        bool matching = a.size() == b.size();
        for (size_t i = 0; matching && i < a.size(); ++i)
            matching = a[i].Type == b[i].Type;
        if (!matching)
            return InterpolateMatrices(ListMatrix(a, bw, bh), ListMatrix(b, bw, bh), t);
        std::vector<UiTransformFn> out = a;
        for (size_t i = 0; i < a.size(); ++i)
        {
            if (a[i].Type == UiTransformFnType::Translate)
            {
                auto length = [&](const UiLength& x, const UiLength& y, float basis) {
                    if (x.Kind == y.Kind) return LerpLength(x, y, t);
                    UiLength r;
                    r.Px = Lerp(x.Resolve(basis), y.Resolve(basis), t);   // px <-> %: go through pixels
                    return r;
                };
                out[i].A = length(a[i].A, b[i].A, bw);
                out[i].B = length(a[i].B, b[i].B, bh);
            }
            else
            {
                out[i].V0 = Lerp(a[i].V0, b[i].V0, t);
                out[i].V1 = Lerp(a[i].V1, b[i].V1, t);
            }
        }
        return out;
    }

    V Interpolate(const AnimProp& prop, const V& a, const V& b, float t, float bw, float bh)
    {
        V r = a;
        switch (prop.Type)
        {
        case VT::Float: r.F = Lerp(a.F, b.F, t); break;
        case VT::Length: r.L = LerpLength(a.L, b.L, t); break;
        case VT::Color: r.C = LerpColor(a.C, b.C, t); break;
        case VT::Transform: r.Tf = InterpolateTransforms(a.Tf, b.Tf, t, bw, bh); break;
        case VT::Filter:
        {
            std::vector<UiFilterFn> fa = a.Fl, fb = b.Fl;
            auto identity = [](UiFilterType type) {
                UiFilterFn f;
                f.Type = type;
                f.Amount = type == UiFilterType::HueRotate || type == UiFilterType::Grayscale || type == UiFilterType::Sepia || type == UiFilterType::Invert ? 0.f : 1.f;
                return f;
            };
            for (size_t i = fa.size(); i < fb.size(); ++i) fa.push_back(identity(fb[i].Type));
            for (size_t i = fb.size(); i < fa.size(); ++i) fb.push_back(identity(fa[i].Type));
            bool matching = true;
            for (size_t i = 0; i < fa.size(); ++i)
                matching = matching && fa[i].Type == fb[i].Type;
            if (!matching)
                r.Fl = t < 0.5f ? a.Fl : b.Fl;
            else
            {
                r.Fl = fa;
                for (size_t i = 0; i < fa.size(); ++i)
                    r.Fl[i].Amount = Lerp(fa[i].Amount, fb[i].Amount, t);
            }
            break;
        }
        case VT::Shadows:
        {
            bool matching = a.Sh.size() == b.Sh.size();
            for (size_t i = 0; matching && i < a.Sh.size(); ++i)
                matching = a.Sh[i].Inset == b.Sh[i].Inset;
            if (!matching)
                r.Sh = t < 0.5f ? a.Sh : b.Sh;
            else
                for (size_t i = 0; i < a.Sh.size(); ++i)
                {
                    r.Sh[i].X = Lerp(a.Sh[i].X, b.Sh[i].X, t);
                    r.Sh[i].Y = Lerp(a.Sh[i].Y, b.Sh[i].Y, t);
                    r.Sh[i].Blur = Lerp(a.Sh[i].Blur, b.Sh[i].Blur, t);
                    r.Sh[i].Spread = Lerp(a.Sh[i].Spread, b.Sh[i].Spread, t);
                    r.Sh[i].Color = LerpColor(a.Sh[i].Color, b.Sh[i].Color, t);
                }
            break;
        }
        }
        return r;
    }

    // ── per element state ─────────────────────────────────────────────────────

    struct Transition
    {
        int Prop = 0;
        V From, To;
        float Elapsed = 0.f, Duration = 0.f, Delay = 0.f;
        UiEasing Easing;
    };

    struct Track
    {
        int Prop = 0;
        std::vector<std::pair<float, V>> Keys;   // sorted by offset, always has 0 and 1
    };

    struct Running
    {
        UiAnimationDef Def;
        float Time = 0.f;
        bool Finished = false;
        std::vector<Track> Tracks;
        const UiKeyframes* Frames = nullptr;
    };

    std::vector<UiAnimState*>& Active()
    {
        static std::vector<UiAnimState*> active;
        return active;
    }

    bool g_reduced = false;
    bool g_paused = false;
    std::set<std::string>& WarnedNames()
    {
        static std::set<std::string> warned;
        return warned;
    }
}

struct UiAnimState
{
    UiElem* Owner = nullptr;
    UiComputedStyle Target;
    bool HasTarget = false;
    std::vector<Transition> Transitions;
    std::vector<Running> Anims;
    bool Registered = false;

    explicit UiAnimState(UiElem* owner) : Owner(owner) {}
    ~UiAnimState() { SetRegistered(false); }

    void SetRegistered(bool on)
    {
        if (on == Registered)
            return;
        Registered = on;
        auto& list = Active();
        if (on)
            list.push_back(this);
        else
            list.erase(std::remove(list.begin(), list.end(), this), list.end());
    }
    bool HasMotion() const
    {
        if (!Transitions.empty())
            return true;
        for (const Running& a : Anims)
            if (!a.Finished)
                return true;
        return false;
    }
};

void UiAnimStateDeleter::operator()(UiAnimState* state) const { delete state; }

namespace
{
    // A transition property names a longhand, a shorthand (border-color covers the four sides) or "all".
    bool Covers(const std::string& listed, const std::string& property)
    {
        if (listed == "all" || listed == property)
            return true;
        auto starts = [&](const char* prefix) { return property.rfind(prefix, 0) == 0; };
        auto ends = [&](const char* suffix)
        {
            const std::string s(suffix);
            return property.size() >= s.size() && property.compare(property.size() - s.size(), s.size(), s) == 0;
        };
        if (listed == "border-color") return starts("border-") && ends("-color");
        if (listed == "border-width") return starts("border-") && ends("-width");
        if (listed == "border-radius") return ends("-radius");
        if (listed == "margin") return starts("margin-");
        if (listed == "padding") return starts("padding-");
        if (listed == "outline") return starts("outline-");
        return false;
    }

    const UiTransitionDef* FindTransition(const std::vector<UiTransitionDef>& defs, const char* property)
    {
        const UiTransitionDef* found = nullptr;
        for (const UiTransitionDef& d : defs)
            if (Covers(d.Property, property))
                found = &d;   // the last matching entry wins
        return found;
    }

    // Longhand names a declaration touches that can be animated.
    std::vector<int> DeclaredProps(const UiCssDeclaration& decl, const UiComputedStyle& style)
    {
        std::vector<int> out;
        if (decl.Name == "outline")
        {
            out.push_back(FindProp("outline-color"));
            out.push_back(FindProp("outline-width"));
            return out;
        }
        std::vector<UiCssToken> tokens;
        std::string error;
        if (!UiSubstituteVars(decl.Value, style.Custom, tokens, error))
            return out;
        UiLonghandList longhands;
        if (!UiExpandProperty(decl.Name, tokens, longhands, error))
            return out;
        for (const auto& [name, value] : longhands)
        {
            const int index = FindProp(name);
            if (index >= 0)
                out.push_back(index);
        }
        return out;
    }

    void BuildTracks(UiElem& e, Running& run, const UiComputedStyle& target)
    {
        run.Tracks.clear();
        if (run.Frames == nullptr)
            return;
        UiStyleParseContext ctx;
        ctx.FontSize = target.FontSize;
        ctx.ParentFontSize = target.FontSize;
        ctx.CurrentColor = target.Color;
        ctx.Viewport = EngineMain::Viewport.GetSize();

        const auto& props = Props();
        std::vector<Track> tracks(props.size());
        std::vector<bool> used(props.size(), false);
        for (const UiKeyframeStop& stop : run.Frames->Stops)
        {
            UiComputedStyle scratch = target;
            std::vector<int> declared;
            for (const UiCssDeclaration& decl : stop.Declarations)
            {
                std::string error;
                if (!UiStyleResolver::ApplyDeclaration(scratch, decl, ctx, error))
                {
                    const std::string key = run.Frames->Path + ":" + run.Frames->Name + ":" + decl.Name;
                    if (WarnedNames().insert(key).second)
                        UiMarkupLog::Warning(run.Frames->Path, decl.Line, decl.Col, "@keyframes " + run.Frames->Name + ": invalid value for '" + decl.Name + "': " + error);
                    continue;
                }
                for (int index : DeclaredProps(decl, target))
                    declared.push_back(index);
            }
            for (int index : declared)
            {
                used[index] = true;
                tracks[index].Prop = index;
                auto& keys = tracks[index].Keys;
                // a later declaration of the same offset replaces the earlier one
                keys.erase(std::remove_if(keys.begin(), keys.end(), [&](const auto& k) { return k.first == stop.Offset; }), keys.end());
                keys.push_back({ stop.Offset, props[index].Get(scratch) });
            }
        }
        for (size_t i = 0; i < props.size(); ++i)
        {
            if (!used[i])
                continue;
            Track& track = tracks[i];
            std::stable_sort(track.Keys.begin(), track.Keys.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
            if (track.Keys.front().first > 0.f)
                track.Keys.insert(track.Keys.begin(), { 0.f, props[i].Get(target) });
            if (track.Keys.back().first < 1.f)
                track.Keys.push_back({ 1.f, props[i].Get(target) });
            run.Tracks.push_back(std::move(track));
        }
        (void)e;
    }

    // Progress (0..1) of the animation's current iteration, and whether the animation currently has an effect.
    bool AnimationProgress(const Running& run, float& progress)
    {
        const UiAnimationDef& d = run.Def;
        const float x = run.Time - d.Delay;
        const bool forwards = d.FillMode == UiAnimationDef::Fill::Forwards || d.FillMode == UiAnimationDef::Fill::Both;
        const bool backwards = d.FillMode == UiAnimationDef::Fill::Backwards || d.FillMode == UiAnimationDef::Fill::Both;
        auto directed = [&](float f, int iteration) {
            switch (d.Dir)
            {
            case UiAnimationDef::Direction::Normal: return f;
            case UiAnimationDef::Direction::Reverse: return 1.f - f;
            case UiAnimationDef::Direction::Alternate: return (iteration % 2 == 1) ? 1.f - f : f;
            case UiAnimationDef::Direction::AlternateReverse: return (iteration % 2 == 1) ? f : 1.f - f;
            }
            return f;
        };
        if (x < 0.f)
        {
            if (!backwards)
                return false;
            progress = directed(0.f, 0);
            return true;
        }
        if (d.Duration <= 0.f)
        {
            if (!forwards)
                return false;
            progress = directed(1.f, 0);
            return true;
        }
        const float iterations = d.Iterations;
        if (iterations >= 0.f && x >= d.Duration * iterations)
        {
            if (!forwards)
                return false;
            const int last = std::max(0, static_cast<int>(std::ceil(iterations)) - 1);
            float f = iterations - std::floor(iterations);
            if (f <= 0.f)
                f = 1.f;
            progress = directed(f, last);
            return true;
        }
        const float total = x / d.Duration;
        const int iteration = static_cast<int>(std::floor(total));
        progress = directed(total - std::floor(total), iteration);
        return true;
    }

    bool AnimationDone(const Running& run)
    {
        const UiAnimationDef& d = run.Def;
        if (d.Iterations < 0.f)
            return false;
        return run.Time - d.Delay >= d.Duration * d.Iterations;
    }

    V EvaluateTrack(const Track& track, const AnimProp& prop, const UiEasing& easing, float progress, float bw, float bh)
    {
        const auto& keys = track.Keys;
        size_t i = 0;
        while (i + 2 < keys.size() && keys[i + 1].first <= progress)
            ++i;
        const float span = keys[i + 1].first - keys[i].first;
        const float local = span > 0.00001f ? std::clamp((progress - keys[i].first) / span, 0.f, 1.f) : 1.f;
        return Interpolate(prop, keys[i].second, keys[i + 1].second, easing.Evaluate(local), bw, bh);
    }

    // Writes the current value of every running transition and animation into `style`.
    void WriteOverlay(UiElem& e, UiAnimState& st, UiComputedStyle& style, unsigned& changedMask)
    {
        const auto& props = Props();
        const float bw = e.BorderRect.W, bh = e.BorderRect.H;
        auto write = [&](int index, const V& value) {
            const AnimProp& p = props[index];
            if (p.Get(style) != value)
            {
                p.Set(style, value);
                changedMask |= 1u << p.Invalidates;
            }
        };
        for (const Transition& t : st.Transitions)
        {
            const float x = t.Elapsed - t.Delay;
            if (x <= 0.f)
                write(t.Prop, t.From);
            else
                write(t.Prop, Interpolate(props[t.Prop], t.From, t.To, t.Easing.Evaluate(std::clamp(x / t.Duration, 0.f, 1.f)), bw, bh));
        }
        for (const Running& run : st.Anims)
        {
            float progress = 0.f;
            const bool applies = !run.Tracks.empty() && AnimationProgress(run, progress) && !(g_reduced && run.Def.Iterations < 0.f);
            for (const Track& track : run.Tracks)
            {
                if (applies)
                    write(track.Prop, EvaluateTrack(track, props[track.Prop], run.Def.Easing, progress, bw, bh));
                else if (st.HasTarget &&
                         std::none_of(st.Transitions.begin(), st.Transitions.end(), [&](const Transition& t) { return t.Prop == track.Prop; }))
                    write(track.Prop, props[track.Prop].Get(st.Target));   // the animation no longer has an effect: back to the CSS value
            }
        }
    }

    void SyncRegistration(UiAnimState& st) { st.SetRegistered(st.HasMotion()); }
}

glm::mat3 UiTransformListMatrix(const std::vector<UiTransformFn>& list, float bw, float bh)
{
    return ListMatrix(list, bw, bh);
}

namespace UiAnimation
{
    void OnStyleResolved(UiElem& e, UiComputedStyle& target)
    {
        UiAnimState* st = e.m_anim.get();
        const bool wants = !target.Fx.Transitions.empty() || !target.Fx.Animations.empty();
        if (st == nullptr && !wants)
            return;
        if (st == nullptr)
        {
            e.m_anim.reset(new UiAnimState(&e));
            st = e.m_anim.get();
        }
        const auto& props = Props();

        // 1. transitions: a changed target starts one from the value displayed now
        if (st->HasTarget)
        {
            for (size_t i = 0; i < props.size(); ++i)
            {
                const V oldTarget = props[i].Get(st->Target), newTarget = props[i].Get(target);
                if (oldTarget == newTarget)
                    continue;
                const auto running = std::find_if(st->Transitions.begin(), st->Transitions.end(), [&](const Transition& t) { return t.Prop == static_cast<int>(i); });
                const UiTransitionDef* def = FindTransition(target.Fx.Transitions, props[i].Name);
                const bool instant = def == nullptr || g_reduced || (def->Duration <= 0.f && def->Delay <= 0.f);
                const V displayed = props[i].Get(e.m_computed);
                if (instant || displayed == newTarget)
                {
                    if (running != st->Transitions.end())
                        st->Transitions.erase(running);
                    continue;
                }
                Transition t;
                t.Prop = static_cast<int>(i);
                t.From = displayed;
                t.To = newTarget;
                t.Duration = std::max(def->Duration, 0.0001f);
                t.Delay = def->Delay;
                t.Easing = def->Easing;
                if (running != st->Transitions.end())
                    *running = t;
                else
                    st->Transitions.push_back(t);
            }
        }
        st->Target = target;
        st->HasTarget = true;

        // 2. animations: keep the running ones that are still named, start new ones
        std::vector<Running> next;
        const UiKeyframeMap* frames = e.Root() != nullptr && e.Root()->m_keyframes ? e.Root()->m_keyframes.get() : nullptr;
        for (const UiAnimationDef& def : target.Fx.Animations)
        {
            if (def.Name.empty())
                continue;
            const auto found = frames != nullptr ? frames->find(def.Name) : UiKeyframeMap::const_iterator();
            if (frames == nullptr || found == frames->end())
            {
                const std::string key = e.ScopeFile + ":" + def.Name;
                if (WarnedNames().insert(key).second)
                    UiMarkupLog::Warning(e.ScopeFile, e.SrcLine, e.SrcCol, "animation '" + def.Name + "' has no @keyframes");
                continue;
            }
            Running run;
            const auto existing = std::find_if(st->Anims.begin(), st->Anims.end(), [&](const Running& r) { return r.Def.Name == def.Name; });
            if (existing != st->Anims.end())
                run = std::move(*existing);
            run.Def = def;
            run.Frames = &found->second;
            BuildTracks(e, run, target);
            next.push_back(std::move(run));
        }
        st->Anims = std::move(next);

        // 3. overlay the displayed values
        unsigned mask = 0;
        WriteOverlay(e, *st, target, mask);
        SyncRegistration(*st);
        if (!st->HasMotion() && !wants && st->Anims.empty())
            e.m_anim.reset();
    }

    void Tick(float seconds)
    {
        if (Active().empty() || g_paused)
            return;
        const std::vector<UiAnimState*> states = Active();   // the list can change while ticking
        const auto& props = Props();
        for (UiAnimState* st : states)
        {
            if (std::find(Active().begin(), Active().end(), st) == Active().end())
                continue;
            UiElem& e = *st->Owner;
            for (Transition& t : st->Transitions)
                t.Elapsed += seconds;
            for (Running& r : st->Anims)
            {
                if (r.Finished)
                    continue;
                r.Time += seconds;
                if (AnimationDone(r))
                    r.Finished = true;
            }
            unsigned mask = 0;
            // finished transitions end on their target value
            for (size_t i = 0; i < st->Transitions.size();)
            {
                const Transition& t = st->Transitions[i];
                if (t.Elapsed - t.Delay >= t.Duration)
                {
                    const AnimProp& p = props[t.Prop];
                    if (p.Get(e.m_computed) != t.To)
                    {
                        p.Set(e.m_computed, t.To);
                        mask |= 1u << p.Invalidates;
                    }
                    st->Transitions.erase(st->Transitions.begin() + static_cast<long>(i));
                }
                else
                    ++i;
            }
            WriteOverlay(e, *st, e.m_computed, mask);
            SyncRegistration(*st);
            if (mask & (1u << kLayout))
                e.MarkLayoutDirty();
            else if (mask & (1u << kTransform))
            {
                if (UiElem* root = e.Root())
                    root->m_transformDirty = true;
                e.MarkPaintDirty();
            }
            else if (mask & (1u << kPaint))
                e.MarkPaintDirty();
        }
    }

    bool IsAnimating(const UiElem& e)
    {
        if (e.m_anim != nullptr && e.m_anim->HasMotion())
            return true;
        for (const auto& c : e.children)
            if (UiElem::IsElem(c.get()) && IsAnimating(static_cast<const UiElem&>(*c)))
                return true;
        return false;
    }

    void SetPaused(bool paused) { g_paused = paused; }
    bool Paused() { return g_paused; }

    void Seek(UiElem& root, float seconds)
    {
        std::vector<UiElem*> all;
        root.CollectElems(all);
        for (UiElem* e : all)
        {
            if (e->m_anim == nullptr)
                continue;
            UiAnimState& st = *e->m_anim;
            for (Transition& t : st.Transitions)
                t.Elapsed = seconds;
            for (Running& r : st.Anims)
            {
                r.Time = seconds;
                r.Finished = AnimationDone(r);
            }
            unsigned mask = 0;
            WriteOverlay(*e, st, e->m_computed, mask);
            if (mask & (1u << kLayout))
                e->MarkLayoutDirty();
            else if (mask & (1u << kTransform))
            {
                if (UiElem* r = e->Root())
                    r->m_transformDirty = true;
                e->MarkPaintDirty();
            }
            else if (mask & (1u << kPaint))
                e->MarkPaintDirty();
        }
    }

    bool ReducedMotion() { return g_reduced; }
    void SetReducedMotion(bool reduced) { g_reduced = reduced; }

    std::vector<const char*> AnimatableProperties()
    {
        std::vector<const char*> names;
        for (const AnimProp& p : Props())
            names.push_back(p.Name);
        return names;
    }

    std::vector<const char*> EasingFunctions()
    {
        return { "linear", "ease", "ease-in", "ease-out", "ease-in-out", "step-start", "step-end", "cubic-bezier()", "steps()" };
    }

    int RunningCount() { return static_cast<int>(Active().size()); }
}

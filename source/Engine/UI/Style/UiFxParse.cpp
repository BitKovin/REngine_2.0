#include "UiFxParse.h"

#include <algorithm>
#include <cmath>

namespace
{
    using Tokens = std::vector<UiCssToken>;

    bool IsWs(const UiCssToken& t) { return t.Type == UiCssTok::Whitespace; }

    std::string Lower(std::string s)
    {
        for (char& c : s)
            if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
        return s;
    }

    Tokens Trim(Tokens v)
    {
        while (!v.empty() && IsWs(v.back())) v.pop_back();
        size_t a = 0;
        while (a < v.size() && IsWs(v[a])) ++a;
        v.erase(v.begin(), v.begin() + a);
        return v;
    }

    std::vector<Tokens> SplitCommas(const Tokens& v)
    {
        std::vector<Tokens> out;
        Tokens cur;
        int depth = 0;
        for (const UiCssToken& t : v)
        {
            if (t.Type == UiCssTok::Function || t.Type == UiCssTok::LParen) ++depth;
            else if (t.Type == UiCssTok::RParen) depth = std::max(0, depth - 1);
            if (depth == 0 && t.Type == UiCssTok::Comma)
            {
                out.push_back(Trim(std::move(cur)));
                cur.clear();
                continue;
            }
            cur.push_back(t);
        }
        out.push_back(Trim(std::move(cur)));
        return out;
    }

    bool IsIdent(const UiCssToken& t, const char* s) { return t.Type == UiCssTok::Ident && Lower(t.Text) == s; }
    bool IsNone(const Tokens& v) { return v.size() == 1 && IsIdent(v[0], "none"); }
    bool IsFunction(const Tokens& comp, const char* name) { return !comp.empty() && comp[0].Type == UiCssTok::Function && Lower(comp[0].Text) == name; }

    // The tokens between "name(" and the closing ")".
    Tokens FunctionArgs(const Tokens& comp)
    {
        Tokens inner(comp.begin() + 1, comp.end());
        if (!inner.empty() && inner.back().Type == UiCssTok::RParen)
            inner.pop_back();
        return inner;
    }

    bool ParseNumberComp(const Tokens& c, float& out)
    {
        if (c.size() == 1 && c[0].Type == UiCssTok::Number) { out = static_cast<float>(c[0].Number); return true; }
        return false;
    }

    // A number or a percentage as a factor (50% = 0.5).
    bool ParseFactor(const Tokens& c, float& out)
    {
        if (c.size() == 1 && c[0].Type == UiCssTok::Number) { out = static_cast<float>(c[0].Number); return true; }
        if (c.size() == 1 && c[0].Type == UiCssTok::Percentage) { out = static_cast<float>(c[0].Number) * 0.01f; return true; }
        return false;
    }

    bool ParseAngle(const Tokens& c, float& deg)
    {
        if (c.size() == 1 && c[0].Type == UiCssTok::Number && c[0].Number == 0.0) { deg = 0.f; return true; }
        if (c.size() == 1 && c[0].Type == UiCssTok::Dimension)
        {
            const std::string u = Lower(c[0].Text);
            const float v = static_cast<float>(c[0].Number);
            if (u == "deg") { deg = v; return true; }
            if (u == "rad") { deg = v * 57.2957795f; return true; }
            if (u == "turn") { deg = v * 360.f; return true; }
            if (u == "grad") { deg = v * 0.9f; return true; }
        }
        return false;
    }

    bool ParseTime(const Tokens& c, float& seconds)
    {
        if (c.size() == 1 && c[0].Type == UiCssTok::Dimension)
        {
            const std::string u = Lower(c[0].Text);
            if (u == "s") { seconds = static_cast<float>(c[0].Number); return true; }
            if (u == "ms") { seconds = static_cast<float>(c[0].Number) * 0.001f; return true; }
        }
        if (c.size() == 1 && c[0].Type == UiCssTok::Number && c[0].Number == 0.0) { seconds = 0.f; return true; }
        return false;
    }

    bool ParseUrl(const Tokens& c, std::string& out)
    {
        if (c.size() == 1 && c[0].Type == UiCssTok::Url) { out = c[0].Text; return true; }
        if (c.size() >= 2 && IsFunction(c, "url"))
            for (size_t i = 1; i < c.size(); ++i)
                if (c[i].Type == UiCssTok::String) { out = c[i].Text; return true; }
        return false;
    }

    // ── positions ─────────────────────────────────────────────────────────────

    // One component of a position: a keyword or a length-percentage. Returns 0 = x axis keyword, 1 = y axis keyword, 2 = centre, -1 = value.
    int PositionKeyword(const UiCssToken& t, UiLength& out)
    {
        if (t.Type != UiCssTok::Ident) return -1;
        const std::string n = Lower(t.Text);
        out = UiLength(); out.Kind = UiLenKind::Percent;
        if (n == "left") { out.Percent = 0.f; return 0; }
        if (n == "right") { out.Percent = 100.f; return 0; }
        if (n == "top") { out.Percent = 0.f; return 1; }
        if (n == "bottom") { out.Percent = 100.f; return 1; }
        if (n == "center") { out.Percent = 50.f; return 2; }
        return -1;
    }

    bool ParsePosition(const std::vector<Tokens>& comps, const UiStyleParseContext& ctx, UiLength& x, UiLength& y, std::string& err)
    {
        if (comps.empty() || comps.size() > 2)
        {
            err = "expected one or two position values";
            return false;
        }
        UiLength v[2];
        int kind[2] = { -1, -1 };
        for (size_t i = 0; i < comps.size(); ++i)
        {
            if (comps[i].size() == 1)
                kind[i] = PositionKeyword(comps[i][0], v[i]);
            if (kind[i] < 0 && !UiParseLengthComponent(comps[i], ctx, v[i], err, true))
                return false;
        }
        UiLength centre; centre.Kind = UiLenKind::Percent; centre.Percent = 50.f;
        if (comps.size() == 1)
        {
            if (kind[0] == 1) { x = centre; y = v[0]; }
            else { x = v[0]; y = centre; }
            return true;
        }
        // two values: a vertical keyword first, or a horizontal one second, swaps the order
        if (kind[0] == 1 || kind[1] == 0)
        {
            if (kind[0] == 0 || kind[1] == 1)
            {
                err = "invalid position";
                return false;
            }
            x = v[1]; y = v[0];
        }
        else
        {
            x = v[0]; y = v[1];
        }
        return true;
    }

    // ── colours and stops ─────────────────────────────────────────────────────

    bool ParseStopPosition(const Tokens& comp, const UiStyleParseContext& ctx, bool conic, UiLength& out, std::string& err)
    {
        float deg;
        if (conic && ParseAngle(comp, deg))
        {
            out = UiLength(); out.Kind = UiLenKind::Percent; out.Percent = deg / 3.6f;
            return true;
        }
        return UiParseLengthComponent(comp, ctx, out, err, true);
    }

    bool ParseStops(const std::vector<Tokens>& args, size_t first, const UiStyleParseContext& ctx, bool conic, UiGradient& g, std::string& err)
    {
        for (size_t i = first; i < args.size(); ++i)
        {
            const auto comps = UiSplitComponents(args[i]);
            if (comps.empty() || comps.size() > 3)
            {
                err = "invalid colour stop";
                return false;
            }
            UiGradStop stop;
            std::string colourErr;
            if (!UiParseColorComponent(comps[0], ctx, stop.Color, colourErr))
            {
                err = comps.size() == 1 ? "colour hints are not supported" : colourErr;
                return false;
            }
            if (comps.size() == 1)
            {
                g.Stops.push_back(stop);
                continue;
            }
            stop.HasPos = true;
            if (!ParseStopPosition(comps[1], ctx, conic, stop.Pos, err))
                return false;
            g.Stops.push_back(stop);
            if (comps.size() == 3)
            {
                UiGradStop second = stop;
                if (!ParseStopPosition(comps[2], ctx, conic, second.Pos, err))
                    return false;
                g.Stops.push_back(second);
            }
        }
        if (g.Stops.size() < 2)
        {
            err = "a gradient needs at least two colour stops";
            return false;
        }
        return true;
    }

    bool ParseGradient(const Tokens& comp, const UiStyleParseContext& ctx, UiGradient& out, std::string& err)
    {
        if (comp.empty() || comp[0].Type != UiCssTok::Function)
        {
            err = "expected a gradient";
            return false;
        }
        std::string name = Lower(comp[0].Text);
        UiGradient g;
        if (name.rfind("repeating-", 0) == 0)
        {
            g.Repeating = true;
            name = name.substr(10);
        }
        if (name == "linear-gradient") g.Kind = UiGradKind::Linear;
        else if (name == "radial-gradient") g.Kind = UiGradKind::Radial;
        else if (name == "conic-gradient") g.Kind = UiGradKind::Conic;
        else
        {
            err = "unknown function '" + comp[0].Text + "'";
            return false;
        }
        const std::vector<Tokens> args = SplitCommas(FunctionArgs(comp));
        if (args.empty() || args[0].empty())
        {
            err = "empty gradient";
            return false;
        }

        // The first argument is the configuration unless it starts with a colour.
        size_t first = 0;
        const auto firstComps = UiSplitComponents(args[0]);
        glm::vec4 probe;
        std::string probeErr;
        const bool startsWithColour = !firstComps.empty() && UiParseColorComponent(firstComps[0], ctx, probe, probeErr);
        if (!startsWithColour)
        {
            first = 1;
            if (g.Kind == UiGradKind::Linear)
            {
                if (firstComps.size() >= 2 && IsIdent(firstComps[0][0], "to"))
                {
                    g.ToCorner = true;
                    for (size_t i = 1; i < firstComps.size(); ++i)
                    {
                        if (firstComps[i].size() != 1 || firstComps[i][0].Type != UiCssTok::Ident)
                        {
                            err = "invalid gradient direction";
                            return false;
                        }
                        const std::string n = Lower(firstComps[i][0].Text);
                        if (n == "left") g.CornerX = -1;
                        else if (n == "right") g.CornerX = 1;
                        else if (n == "top") g.CornerY = -1;
                        else if (n == "bottom") g.CornerY = 1;
                        else { err = "invalid gradient direction '" + n + "'"; return false; }
                    }
                    if (g.CornerX == 0 && g.CornerY == 0)
                    {
                        err = "invalid gradient direction";
                        return false;
                    }
                    // a single side is an angle
                    if (g.CornerX == 0 || g.CornerY == 0)
                    {
                        g.ToCorner = false;
                        g.AngleDeg = g.CornerY < 0 ? 0.f : g.CornerY > 0 ? 180.f : g.CornerX > 0 ? 90.f : 270.f;
                    }
                }
                else if (firstComps.size() == 1 && ParseAngle(firstComps[0], g.AngleDeg))
                {
                }
                else
                {
                    err = "invalid gradient direction";
                    return false;
                }
            }
            else if (g.Kind == UiGradKind::Radial)
            {
                std::vector<UiLength> sizes;
                size_t i = 0;
                for (; i < firstComps.size(); ++i)
                {
                    const Tokens& c = firstComps[i];
                    if (c.size() == 1 && c[0].Type == UiCssTok::Ident)
                    {
                        const std::string n = Lower(c[0].Text);
                        if (n == "circle") { g.Circle = true; continue; }
                        if (n == "ellipse") { g.Circle = false; continue; }
                        if (n == "closest-side") { g.SizeKeyword = 1; continue; }
                        if (n == "farthest-side") { g.SizeKeyword = 2; continue; }
                        if (n == "closest-corner") { g.SizeKeyword = 3; continue; }
                        if (n == "farthest-corner") { g.SizeKeyword = 0; continue; }
                        if (n == "at") { ++i; break; }
                        err = "invalid radial gradient '" + n + "'";
                        return false;
                    }
                    UiLength len;
                    if (!UiParseLengthComponent(c, ctx, len, err, true))
                        return false;
                    sizes.push_back(len);
                }
                if (!sizes.empty())
                {
                    if (sizes.size() > 2 || (g.Circle && sizes.size() != 1))
                    {
                        err = "invalid radial gradient size";
                        return false;
                    }
                    g.SizeKeyword = 4;
                    g.RadiusX = sizes[0];
                    g.RadiusY = sizes.size() > 1 ? sizes[1] : sizes[0];
                    if (sizes.size() == 1)
                        g.Circle = true;
                }
                if (i < firstComps.size())
                {
                    const std::vector<Tokens> at(firstComps.begin() + i, firstComps.end());
                    if (!ParsePosition(at, ctx, g.PosX, g.PosY, err))
                        return false;
                }
            }
            else
            {
                size_t i = 0;
                while (i < firstComps.size())
                {
                    if (IsIdent(firstComps[i][0], "from") && i + 1 < firstComps.size())
                    {
                        if (!ParseAngle(firstComps[i + 1], g.FromDeg))
                        {
                            err = "invalid conic start angle";
                            return false;
                        }
                        i += 2;
                    }
                    else if (IsIdent(firstComps[i][0], "at"))
                    {
                        const std::vector<Tokens> at(firstComps.begin() + i + 1, firstComps.end());
                        if (!ParsePosition(at, ctx, g.PosX, g.PosY, err))
                            return false;
                        break;
                    }
                    else
                    {
                        err = "invalid conic gradient";
                        return false;
                    }
                }
            }
        }
        if (!ParseStops(args, first, ctx, g.Kind == UiGradKind::Conic, g, err))
            return false;
        out = g;
        return true;
    }

    // ── lists ─────────────────────────────────────────────────────────────────

    bool ParseShadowList(const Tokens& value, const UiStyleParseContext& ctx, bool allowInset, bool allowSpread, std::vector<UiShadow>& out, std::string& err)
    {
        std::vector<UiShadow> result;
        if (IsNone(value))
        {
            out = result;
            return true;
        }
        for (const Tokens& item : SplitCommas(value))
        {
            UiShadow sh;
            sh.Color = ctx.CurrentColor;
            std::vector<float> lengths;
            bool haveColour = false;
            for (const Tokens& comp : UiSplitComponents(item))
            {
                if (comp.size() == 1 && IsIdent(comp[0], "inset"))
                {
                    if (!allowInset) { err = "inset is not allowed here"; return false; }
                    sh.Inset = true;
                    continue;
                }
                UiLength len;
                std::string lenErr;
                if (UiParseLengthComponent(comp, ctx, len, lenErr, false))
                {
                    lengths.push_back(len.Px);
                    continue;
                }
                if (haveColour || !UiParseColorComponent(comp, ctx, sh.Color, err))
                {
                    if (haveColour) err = "more than one colour in a shadow";
                    return false;
                }
                haveColour = true;
            }
            const size_t maxLengths = allowSpread ? 4 : 3;
            if (lengths.size() < 2 || lengths.size() > maxLengths)
            {
                err = allowSpread ? "a shadow needs 2 to 4 lengths" : "a text shadow needs 2 or 3 lengths";
                return false;
            }
            sh.X = lengths[0];
            sh.Y = lengths[1];
            if (lengths.size() > 2) sh.Blur = std::max(0.f, lengths[2]);
            if (lengths.size() > 3) sh.Spread = lengths[3];
            result.push_back(sh);
        }
        out = result;
        return true;
    }

    // ── easing ────────────────────────────────────────────────────────────────

    bool ParseEasing(const Tokens& c, UiEasing& e, std::string& err)
    {
        if (c.size() == 1 && c[0].Type == UiCssTok::Ident)
        {
            const std::string n = Lower(c[0].Text);
            e = UiEasing();
            if (n == "linear") { e = UiEasing::Linear(); return true; }
            if (n == "ease") { e.X1 = 0.25f; e.Y1 = 0.1f; e.X2 = 0.25f; e.Y2 = 1.f; return true; }
            if (n == "ease-in") { e.X1 = 0.42f; e.Y1 = 0.f; e.X2 = 1.f; e.Y2 = 1.f; return true; }
            if (n == "ease-out") { e.X1 = 0.f; e.Y1 = 0.f; e.X2 = 0.58f; e.Y2 = 1.f; return true; }
            if (n == "ease-in-out") { e.X1 = 0.42f; e.Y1 = 0.f; e.X2 = 0.58f; e.Y2 = 1.f; return true; }
            if (n == "step-start") { e.K = UiEasing::Kind::Steps; e.Steps = 1; e.StepStart = true; return true; }
            if (n == "step-end") { e.K = UiEasing::Kind::Steps; e.Steps = 1; e.StepStart = false; return true; }
        }
        if (IsFunction(c, "cubic-bezier"))
        {
            const std::vector<Tokens> args = SplitCommas(FunctionArgs(c));
            float v[4];
            if (args.size() != 4)
            {
                err = "cubic-bezier() needs four numbers";
                return false;
            }
            for (int i = 0; i < 4; ++i)
                if (!ParseNumberComp(args[i], v[i]))
                {
                    err = "cubic-bezier() needs four numbers";
                    return false;
                }
            if (v[0] < 0.f || v[0] > 1.f || v[2] < 0.f || v[2] > 1.f)
            {
                err = "cubic-bezier() x values must be between 0 and 1";
                return false;
            }
            e = UiEasing();
            e.X1 = v[0]; e.Y1 = v[1]; e.X2 = v[2]; e.Y2 = v[3];
            return true;
        }
        if (IsFunction(c, "steps"))
        {
            const std::vector<Tokens> args = SplitCommas(FunctionArgs(c));
            float n;
            if (args.empty() || args.size() > 2 || !ParseNumberComp(args[0], n) || n < 1.f)
            {
                err = "steps() needs a positive integer";
                return false;
            }
            e = UiEasing();
            e.K = UiEasing::Kind::Steps;
            e.Steps = static_cast<int>(n);
            if (args.size() == 2)
            {
                if (args[1].size() == 1 && (IsIdent(args[1][0], "start") || IsIdent(args[1][0], "jump-start"))) e.StepStart = true;
                else if (args[1].size() == 1 && (IsIdent(args[1][0], "end") || IsIdent(args[1][0], "jump-end"))) e.StepStart = false;
                else
                {
                    err = "invalid steps() direction";
                    return false;
                }
            }
            return true;
        }
        if (err.empty())
            err = "expected a timing function";
        return false;
    }

    bool LooksLikeEasing(const Tokens& c)
    {
        if (c.size() == 1 && c[0].Type == UiCssTok::Ident)
        {
            const std::string n = Lower(c[0].Text);
            return n == "linear" || n == "ease" || n == "ease-in" || n == "ease-out" || n == "ease-in-out" || n == "step-start" || n == "step-end";
        }
        return IsFunction(c, "cubic-bezier") || IsFunction(c, "steps");
    }

    // ── per-property parsers ──────────────────────────────────────────────────

    bool ParseFilter(const Tokens& value, const UiStyleParseContext& ctx, bool allowShadow, std::vector<UiFilterFn>& out, std::string& err)
    {
        out.clear();
        if (IsNone(value))
            return true;
        for (const Tokens& comp : UiSplitComponents(value))
        {
            if (comp.empty() || comp[0].Type != UiCssTok::Function)
            {
                err = "expected a filter function";
                return false;
            }
            const std::string fn = Lower(comp[0].Text);
            const Tokens args = Trim(FunctionArgs(comp));
            UiFilterFn f;
            if (fn == "hue-rotate")
            {
                f.Type = UiFilterType::HueRotate;
                f.Amount = 0.f;
                if (!args.empty() && !ParseAngle(args, f.Amount))
                {
                    err = "hue-rotate() needs an angle";
                    return false;
                }
                out.push_back(f);
                continue;
            }
            if (fn == "blur")
            {
                f.Type = UiFilterType::Blur;
                f.Amount = 0.f;
                UiLength len;
                if (!args.empty() && (!UiParseLengthComponent(args, ctx, len, err, false) || len.Kind != UiLenKind::Px || len.Px < 0.f))
                {
                    err = "blur() needs a non-negative length in px";
                    return false;
                }
                if (!args.empty())
                    f.Amount = len.Px;
                out.push_back(f);
                continue;
            }
            if (fn == "drop-shadow")
            {
                if (!allowShadow)
                {
                    err = "drop-shadow() is not supported in backdrop-filter";
                    return false;
                }
                std::vector<UiShadow> shadows;
                if (!ParseShadowList(args, ctx, false, false, shadows, err))
                    return false;
                if (shadows.size() != 1)
                {
                    err = "drop-shadow() takes one shadow: <offset-x> <offset-y> <blur-radius>? <color>?";
                    return false;
                }
                f.Type = UiFilterType::DropShadow;
                f.Shadow = shadows[0];
                out.push_back(f);
                continue;
            }
            if (fn == "brightness") f.Type = UiFilterType::Brightness;
            else if (fn == "contrast") f.Type = UiFilterType::Contrast;
            else if (fn == "saturate") f.Type = UiFilterType::Saturate;
            else if (fn == "grayscale") f.Type = UiFilterType::Grayscale;
            else if (fn == "sepia") f.Type = UiFilterType::Sepia;
            else if (fn == "invert") f.Type = UiFilterType::Invert;
            else
            {
                err = "unknown filter function '" + comp[0].Text + "'";
                return false;
            }
            f.Amount = 1.f;
            if (!args.empty() && (!ParseFactor(args, f.Amount) || f.Amount < 0.f))
            {
                err = fn + "() needs a non-negative number or percentage";
                return false;
            }
            if (f.Type == UiFilterType::Grayscale || f.Type == UiFilterType::Sepia || f.Type == UiFilterType::Invert)
                f.Amount = std::min(f.Amount, 1.f);
            out.push_back(f);
        }
        return true;
    }

    bool ParseClipPath(const Tokens& value, const UiStyleParseContext& ctx, UiClipPath& out, std::string& err)
    {
        UiClipPath c;
        if (IsNone(value))
        {
            out = c;
            return true;
        }
        const auto comps = UiSplitComponents(value);
        if (comps.size() != 1 || comps[0].empty() || comps[0][0].Type != UiCssTok::Function)
        {
            err = "expected inset(), circle() or ellipse()";
            return false;
        }
        const std::string fn = Lower(comps[0][0].Text);
        const auto args = UiSplitComponents(FunctionArgs(comps[0]));
        // split off "at <position>"
        size_t atIndex = args.size();
        for (size_t i = 0; i < args.size(); ++i)
            if (args[i].size() == 1 && IsIdent(args[i][0], "at"))
                atIndex = i;
        if (fn == "inset")
        {
            c.Kind = UiClipPath::Shape::Inset;
            size_t roundIndex = args.size();
            for (size_t i = 0; i < args.size(); ++i)
                if (args[i].size() == 1 && IsIdent(args[i][0], "round"))
                    roundIndex = i;
            std::vector<UiLength> v;
            for (size_t i = 0; i < roundIndex; ++i)
            {
                UiLength len;
                if (!UiParseLengthComponent(args[i], ctx, len, err, true))
                    return false;
                v.push_back(len);
            }
            if (v.empty() || v.size() > 4)
            {
                err = "inset() needs 1 to 4 lengths";
                return false;
            }
            c.A[0] = v[0];
            c.A[1] = v.size() > 1 ? v[1] : v[0];
            c.A[2] = v.size() > 2 ? v[2] : v[0];
            c.A[3] = v.size() > 3 ? v[3] : c.A[1];
            if (roundIndex < args.size())
            {
                std::vector<UiLength> r;
                for (size_t i = roundIndex + 1; i < args.size(); ++i)
                {
                    UiLength len;
                    if (!UiParseLengthComponent(args[i], ctx, len, err, true))
                        return false;
                    r.push_back(len);
                }
                if (r.empty() || r.size() > 4)
                {
                    err = "inset() round needs 1 to 4 lengths";
                    return false;
                }
                c.Round[0] = r[0];
                c.Round[1] = r.size() > 1 ? r[1] : r[0];
                c.Round[2] = r.size() > 2 ? r[2] : r[0];
                c.Round[3] = r.size() > 3 ? r[3] : c.Round[1];
            }
            out = c;
            return true;
        }
        if (fn != "circle" && fn != "ellipse")
        {
            err = "unsupported shape '" + comps[0][0].Text + "' (inset, circle and ellipse are supported)";
            return false;
        }
        const bool circle = fn == "circle";
        c.Kind = circle ? UiClipPath::Shape::Circle : UiClipPath::Shape::Ellipse;
        const size_t radii = circle ? 1 : 2;
        if (atIndex != 0 && atIndex != radii && !(atIndex == args.size() && (args.size() == radii || args.empty())))
        {
            err = std::string(fn) + "() needs " + (circle ? "one radius" : "two radii");
            return false;
        }
        for (size_t i = 0; i < atIndex && i < radii; ++i)
        {
            if (!UiParseLengthComponent(args[i], ctx, c.A[i], err, true))
                return false;
            c.HasRadius[i] = true;
        }
        if (atIndex < args.size())
        {
            const std::vector<Tokens> at(args.begin() + atIndex + 1, args.end());
            if (!ParsePosition(at, ctx, c.CX, c.CY, err))
                return false;
        }
        out = c;
        return true;
    }

    bool ParseTransform(const Tokens& value, const UiStyleParseContext& ctx, std::vector<UiTransformFn>& out, std::string& err)
    {
        out.clear();
        if (IsNone(value))
            return true;
        for (const Tokens& comp : UiSplitComponents(value))
        {
            if (comp.empty() || comp[0].Type != UiCssTok::Function)
            {
                err = "expected a transform function";
                return false;
            }
            const std::string fn = Lower(comp[0].Text);
            const std::vector<Tokens> args = SplitCommas(FunctionArgs(comp));
            UiTransformFn t;
            auto length = [&](const Tokens& a, UiLength& dst) { return UiParseLengthComponent(a, ctx, dst, err, true); };
            if (fn == "translate" || fn == "translatex" || fn == "translatey")
            {
                t.Type = UiTransformFnType::Translate;
                if (args.empty() || args.size() > (fn == "translate" ? 2u : 1u) || !length(args[0], fn == "translatey" ? t.B : t.A))
                {
                    if (err.empty()) err = fn + "() has the wrong arguments";
                    return false;
                }
                if (args.size() == 2 && !length(args[1], t.B))
                    return false;
            }
            else if (fn == "rotate")
            {
                t.Type = UiTransformFnType::Rotate;
                if (args.size() != 1 || !ParseAngle(args[0], t.V0))
                {
                    err = "rotate() needs an angle";
                    return false;
                }
            }
            else if (fn == "scale" || fn == "scalex" || fn == "scaley")
            {
                t.Type = UiTransformFnType::Scale;
                t.V0 = t.V1 = 1.f;
                float a = 1.f, b = 1.f;
                if (args.empty() || args.size() > (fn == "scale" ? 2u : 1u) || !ParseFactor(args[0], a) ||
                    (args.size() == 2 && !ParseFactor(args[1], b)))
                {
                    err = fn + "() needs numbers";
                    return false;
                }
                if (fn == "scale") { t.V0 = a; t.V1 = args.size() == 2 ? b : a; }
                else if (fn == "scalex") t.V0 = a;
                else t.V1 = a;
            }
            else if (fn == "skew" || fn == "skewx" || fn == "skewy")
            {
                t.Type = UiTransformFnType::Skew;
                float a = 0.f, b = 0.f;
                if (args.empty() || args.size() > (fn == "skew" ? 2u : 1u) || !ParseAngle(args[0], a) ||
                    (args.size() == 2 && !ParseAngle(args[1], b)))
                {
                    err = fn + "() needs angles";
                    return false;
                }
                if (fn == "skew") { t.V0 = a; t.V1 = b; }
                else if (fn == "skewx") t.V0 = a;
                else t.V1 = a;
            }
            else
            {
                err = "unknown transform function '" + comp[0].Text + "'";
                return false;
            }
            out.push_back(t);
        }
        return true;
    }

    bool ParseBorderWidth(const Tokens& comp, const UiStyleParseContext& ctx, float& out, std::string& err)
    {
        if (comp.size() == 1 && comp[0].Type == UiCssTok::Ident)
        {
            const std::string n = Lower(comp[0].Text);
            if (n == "thin") { out = 1.f; return true; }
            if (n == "medium") { out = 3.f; return true; }
            if (n == "thick") { out = 5.f; return true; }
        }
        UiLength len;
        if (!UiParseLengthComponent(comp, ctx, len, err, false))
            return false;
        out = std::max(0.f, len.Px);
        return true;
    }

    bool IsLineStyle(const UiCssToken& t)
    {
        static const char* kStyles[] = { "none", "hidden", "solid", "dashed", "dotted", "double", "groove", "ridge", "inset", "outset" };
        if (t.Type != UiCssTok::Ident) return false;
        const std::string n = Lower(t.Text);
        for (const char* s : kStyles) if (n == s) return true;
        return false;
    }

    template <class T>
    T& Slot(std::vector<T>& v, size_t i)
    {
        if (v.size() <= i)
            v.resize(i + 1);
        return v[i];
    }

    bool ParseBorderImageSlice(const Tokens& value, UiBorderImage& bi, std::string& err)
    {
        float vals[4];
        bool pct[4];
        size_t n = 0;
        bool fill = false;
        for (const Tokens& comp : UiSplitComponents(value))
        {
            if (comp.size() == 1 && IsIdent(comp[0], "fill")) { fill = true; continue; }
            if (n >= 4 || comp.size() != 1 || (comp[0].Type != UiCssTok::Number && comp[0].Type != UiCssTok::Percentage))
            {
                err = "border-image-slice needs 1 to 4 numbers or percentages";
                return false;
            }
            vals[n] = static_cast<float>(comp[0].Number);
            pct[n] = comp[0].Type == UiCssTok::Percentage;
            ++n;
        }
        if (n == 0)
        {
            err = "border-image-slice needs 1 to 4 numbers or percentages";
            return false;
        }
        const size_t idx[4][4] = { { 0, 0, 0, 0 }, { 0, 1, 0, 1 }, { 0, 1, 2, 1 }, { 0, 1, 2, 3 } };
        for (int i = 0; i < 4; ++i)
        {
            bi.Slice[i] = vals[idx[n - 1][i]];
            bi.SlicePercent[i] = pct[idx[n - 1][i]];
        }
        bi.Fill = fill;
        return true;
    }

    bool ParseBorderImageWidth(const Tokens& value, UiBorderImage& bi, std::string& err)
    {
        float vals[4];
        size_t n = 0;
        for (const Tokens& comp : UiSplitComponents(value))
        {
            if (n >= 4 || comp.size() != 1 || comp[0].Type != UiCssTok::Number)
            {
                err = "border-image-width needs 1 to 4 numbers (multiples of the border width)";
                return false;
            }
            vals[n++] = static_cast<float>(comp[0].Number);
        }
        if (n == 0)
        {
            err = "border-image-width needs 1 to 4 numbers";
            return false;
        }
        const size_t idx[4][4] = { { 0, 0, 0, 0 }, { 0, 1, 0, 1 }, { 0, 1, 2, 1 }, { 0, 1, 2, 3 } };
        for (int i = 0; i < 4; ++i)
            bi.Width[i] = vals[idx[n - 1][i]];
        return true;
    }

    bool ParseNamedList(const Tokens& value, std::vector<std::string>& names, std::string& err)
    {
        names.clear();
        for (const Tokens& item : SplitCommas(value))
        {
            if (item.size() != 1 || item[0].Type != UiCssTok::Ident)
            {
                err = "expected a name";
                return false;
            }
            names.push_back(Lower(item[0].Text));
        }
        return true;
    }

    bool ParseTransitionItem(const Tokens& item, UiTransitionDef& t, std::string& err)
    {
        int times = 0;
        t = UiTransitionDef();
        for (const Tokens& comp : UiSplitComponents(item))
        {
            float seconds;
            if (ParseTime(comp, seconds))
            {
                if (times == 0) t.Duration = std::max(0.f, seconds);
                else if (times == 1) t.Delay = seconds;
                else { err = "too many times in transition"; return false; }
                ++times;
            }
            else if (LooksLikeEasing(comp))
            {
                if (!ParseEasing(comp, t.Easing, err))
                    return false;
            }
            else if (comp.size() == 1 && comp[0].Type == UiCssTok::Ident)
                t.Property = Lower(comp[0].Text);
            else
            {
                err = "invalid transition";
                return false;
            }
        }
        return true;
    }

    bool ParseAnimationItem(const Tokens& item, UiAnimationDef& a, std::string& err)
    {
        int times = 0;
        a = UiAnimationDef();
        for (const Tokens& comp : UiSplitComponents(item))
        {
            float v;
            if (ParseTime(comp, v) && comp[0].Type == UiCssTok::Dimension)
            {
                if (times == 0) a.Duration = std::max(0.f, v);
                else if (times == 1) a.Delay = v;
                else { err = "too many times in animation"; return false; }
                ++times;
                continue;
            }
            if (LooksLikeEasing(comp))
            {
                if (!ParseEasing(comp, a.Easing, err))
                    return false;
                continue;
            }
            if (comp.size() == 1 && comp[0].Type == UiCssTok::Number)
            {
                a.Iterations = std::max(0.f, static_cast<float>(comp[0].Number));
                continue;
            }
            if (comp.size() == 1 && comp[0].Type == UiCssTok::Ident)
            {
                const std::string n = Lower(comp[0].Text);
                if (n == "infinite") a.Iterations = -1.f;
                else if (n == "normal") a.Dir = UiAnimationDef::Direction::Normal;
                else if (n == "reverse") a.Dir = UiAnimationDef::Direction::Reverse;
                else if (n == "alternate") a.Dir = UiAnimationDef::Direction::Alternate;
                else if (n == "alternate-reverse") a.Dir = UiAnimationDef::Direction::AlternateReverse;
                else if (n == "forwards") a.FillMode = UiAnimationDef::Fill::Forwards;
                else if (n == "backwards") a.FillMode = UiAnimationDef::Fill::Backwards;
                else if (n == "both") a.FillMode = UiAnimationDef::Fill::Both;
                else if (n == "running" || n == "paused") {}
                else a.Name = comp[0].Text;
                continue;
            }
            err = "invalid animation";
            return false;
        }
        return true;
    }

    bool ParseColour(const Tokens& value, const UiStyleParseContext& ctx, glm::vec4& out, std::string& err)
    {
        const auto comps = UiSplitComponents(value);
        if (comps.size() != 1)
        {
            err = "expected a colour";
            return false;
        }
        return UiParseColorComponent(comps[0], ctx, out, err);
    }
}

const std::vector<UiPropertyInfo>& UiFxProperties()
{
    static const std::vector<UiPropertyInfo> props = {
        { "background-image", "none | [url() | linear-gradient() | radial-gradient() | conic-gradient() | repeating-*()]#", false, "none", "paint", true },
        { "background-size", "[auto | cover | contain | <length-percentage>{1,2}]#", false, "auto", "paint", true },
        { "background-position", "[<position>]#", false, "0% 0%", "paint", true },
        { "background-repeat", "[repeat | no-repeat | repeat-x | repeat-y]#", false, "repeat", "paint", true },
        { "border-image", "<source> <slice> [/ <width>] <repeat>", false, "none", "paint", false },
        { "border-image-source", "none | url()", false, "none", "paint", true },
        { "border-image-slice", "[<number> | <percentage>]{1,4} && fill?", false, "100%", "paint", true },
        { "border-image-width", "<number>{1,4}", false, "1", "paint", true },
        { "border-image-repeat", "stretch | round", false, "stretch", "paint", true },
        { "box-shadow", "none | [inset? && <length>{2,4} && <color>?]#", false, "none", "paint", true },
        { "outline", "<width> || <style> || <color>", false, "medium none currentcolor", "paint", false },
        { "outline-width", "<length> | thin | medium | thick", false, "medium", "paint", true },
        { "outline-style", "none | solid", false, "none", "paint", true },
        { "outline-color", "<color>", false, "currentcolor", "paint", true },
        { "outline-offset", "<length>", false, "0", "paint", true },
        { "text-shadow", "none | [<length>{2,3} && <color>?]#", true, "none", "paint", true },
        { "text-stroke", "<width> <color>", true, "0", "paint", false },
        { "-webkit-text-stroke", "<width> <color>", true, "0", "paint", false },
        { "text-stroke-width", "<length>", true, "0", "paint", true },
        { "text-stroke-color", "<color>", true, "currentcolor", "paint", true },
        { "filter", "none | [brightness() | contrast() | saturate() | grayscale() | sepia() | hue-rotate() | invert() | blur() | drop-shadow()]+", false, "none", "paint", true },
        { "backdrop-filter", "none | [blur() | brightness() | contrast() | saturate() | grayscale() | sepia() | hue-rotate() | invert()]+", false, "none", "paint", true },
        { "isolation", "auto | isolate", false, "auto", "paint", true },
        { "cache", "none | layer", false, "none", "paint", true },
        { "mask-image", "none | linear-gradient() | radial-gradient()", false, "none", "paint", true },
        { "clip-path", "none | inset() | circle() | ellipse()", false, "none", "paint", true },
        { "mix-blend-mode", "normal | plus-lighter | multiply | screen", false, "normal", "paint", true },
        { "transform", "none | [translate() | translateX() | translateY() | rotate() | scale() | scaleX() | scaleY() | skew() | skewX() | skewY()]+", false, "none", "transform", true },
        { "transform-origin", "<position>", false, "50% 50%", "transform", true },
        { "wash-color", "<color>", false, "transparent", "paint", true },
        { "image-rendering", "auto | pixelated", true, "auto", "paint", true },
        { "object-position", "<position>", false, "50% 50%", "paint", true },
        { "transition", "none | [<property> || <time> || <timing-function> || <time>]#", false, "none", "none", false },
        { "transition-property", "none | [all | <property>]#", false, "all", "none", true },
        { "transition-duration", "<time>#", false, "0s", "none", true },
        { "transition-timing-function", "<timing-function>#", false, "ease", "none", true },
        { "transition-delay", "<time>#", false, "0s", "none", true },
        { "animation", "none | [<name> || <time> || <timing-function> || <time> || <iteration-count> || <direction> || <fill-mode>]#", false, "none", "none", false },
        { "animation-name", "none | <name>#", false, "none", "none", true },
        { "animation-duration", "<time>#", false, "0s", "none", true },
        { "animation-timing-function", "<timing-function>#", false, "ease", "none", true },
        { "animation-delay", "<time>#", false, "0s", "none", true },
        { "animation-iteration-count", "[<number> | infinite]#", false, "1", "none", true },
        { "animation-direction", "[normal | reverse | alternate | alternate-reverse]#", false, "normal", "none", true },
        { "animation-fill-mode", "[none | forwards | backwards | both]#", false, "none", "none", true },
        { "sound-enter", "none | <string> | url()", false, "none", "none", true },
        { "sound-exit", "none | <string> | url()", false, "none", "none", true },
        { "sound-volume", "<number>", false, "1", "none", true },
        { "pulse-duration", "<time>", false, "0.2s", "none", true },
    };
    return props;
}

bool UiFxApplyProperty(UiComputedStyle& s, const std::string& name, const std::vector<UiCssToken>& valueIn, const UiStyleParseContext& ctx,
                       std::string& error, bool& handled)
{
    handled = true;
    const Tokens value = Trim(valueIn);
    UiFxStyle& fx = s.Fx;
    const bool none = IsNone(value);

    // ── backgrounds
    if (name == "background-image")
    {
        std::vector<UiBgImage> list;
        if (!none)
            for (const Tokens& item : SplitCommas(value))
            {
                const auto comps = UiSplitComponents(item);
                if (comps.size() != 1)
                {
                    error = "expected one image per layer";
                    return false;
                }
                UiBgImage image;
                if (IsNone(comps[0]))
                    continue;
                if (ParseUrl(comps[0], image.Url))
                    image.Kind = UiBgImage::Type::Url;
                else
                {
                    image.Kind = UiBgImage::Type::Gradient;
                    if (!ParseGradient(comps[0], ctx, image.Gradient, error))
                        return false;
                }
                list.push_back(std::move(image));
            }
        fx.BgImages = std::move(list);
        return true;
    }
    if (name == "background-size")
    {
        std::vector<UiBgSize> list;
        for (const Tokens& item : SplitCommas(value))
        {
            const auto comps = UiSplitComponents(item);
            UiBgSize size;
            if (comps.size() == 1 && IsIdent(comps[0][0], "cover")) size.M = UiBgSize::Mode::Cover;
            else if (comps.size() == 1 && IsIdent(comps[0][0], "contain")) size.M = UiBgSize::Mode::Contain;
            else if (comps.size() == 1 && IsIdent(comps[0][0], "auto")) size.M = UiBgSize::Mode::Auto;
            else if (comps.size() == 1 || comps.size() == 2)
            {
                size.M = UiBgSize::Mode::Explicit;
                if (IsIdent(comps[0][0], "auto")) size.WAuto = true;
                else if (UiParseLengthComponent(comps[0], ctx, size.W, error, true)) size.WAuto = false;
                else return false;
                if (comps.size() == 2)
                {
                    if (IsIdent(comps[1][0], "auto")) size.HAuto = true;
                    else if (UiParseLengthComponent(comps[1], ctx, size.H, error, true)) size.HAuto = false;
                    else return false;
                }
            }
            else
            {
                error = "invalid background-size";
                return false;
            }
            list.push_back(size);
        }
        fx.BgSizes = std::move(list);
        return true;
    }
    if (name == "background-position")
    {
        std::vector<UiBgPos> list;
        for (const Tokens& item : SplitCommas(value))
        {
            UiBgPos p;
            if (!ParsePosition(UiSplitComponents(item), ctx, p.X, p.Y, error))
                return false;
            list.push_back(p);
        }
        fx.BgPositions = std::move(list);
        return true;
    }
    if (name == "background-repeat")
    {
        std::vector<UiBgRepeat> list;
        for (const Tokens& item : SplitCommas(value))
        {
            const auto comps = UiSplitComponents(item);
            UiBgRepeat r;
            auto keyword = [&](const UiCssToken& t, bool& rx, bool& ry) {
                const std::string n = Lower(t.Text);
                if (n == "repeat") { rx = ry = true; return true; }
                if (n == "no-repeat") { rx = ry = false; return true; }
                if (n == "repeat-x") { rx = true; ry = false; return true; }
                if (n == "repeat-y") { rx = false; ry = true; return true; }
                return false;
            };
            if (comps.size() == 1 && comps[0].size() == 1 && comps[0][0].Type == UiCssTok::Ident && keyword(comps[0][0], r.X, r.Y))
            {
            }
            else if (comps.size() == 2 && comps[0].size() == 1 && comps[1].size() == 1 && comps[0][0].Type == UiCssTok::Ident &&
                     comps[1][0].Type == UiCssTok::Ident)
            {
                const std::string a = Lower(comps[0][0].Text), b = Lower(comps[1][0].Text);
                if ((a != "repeat" && a != "no-repeat") || (b != "repeat" && b != "no-repeat"))
                {
                    error = "invalid background-repeat";
                    return false;
                }
                r.X = a == "repeat";
                r.Y = b == "repeat";
            }
            else
            {
                error = "invalid background-repeat (repeat, no-repeat, repeat-x and repeat-y are supported)";
                return false;
            }
            list.push_back(r);
        }
        fx.BgRepeats = std::move(list);
        return true;
    }

    // ── border-image
    if (name == "border-image-source")
    {
        std::string url;
        if (none) { fx.BorderImage.Src.clear(); return true; }
        if (!ParseUrl(value, url)) { error = "expected url()"; return false; }
        fx.BorderImage.Src = url;
        return true;
    }
    if (name == "border-image-slice") return ParseBorderImageSlice(value, fx.BorderImage, error);
    if (name == "border-image-width") return ParseBorderImageWidth(value, fx.BorderImage, error);
    if (name == "border-image-repeat")
    {
        if (value.size() == 1 && IsIdent(value[0], "stretch")) { fx.BorderImage.Round = false; return true; }
        if (value.size() == 1 && IsIdent(value[0], "round")) { fx.BorderImage.Round = true; return true; }
        error = "border-image-repeat supports stretch and round";
        return false;
    }
    if (name == "border-image")
    {
        UiBorderImage bi = fx.BorderImage;
        bi.Src.clear();
        if (none) { fx.BorderImage = UiBorderImage(); return true; }
        Tokens slice, width;
        bool sawSlash = false;
        for (const Tokens& comp : UiSplitComponents(value))
        {
            std::string url;
            if (ParseUrl(comp, url)) { bi.Src = url; continue; }
            if (comp.size() == 1 && comp[0].Type == UiCssTok::Delim && comp[0].Text == "/") { sawSlash = true; continue; }
            if (comp.size() == 1 && IsIdent(comp[0], "stretch")) { bi.Round = false; continue; }
            if (comp.size() == 1 && IsIdent(comp[0], "round")) { bi.Round = true; continue; }
            Tokens& target = sawSlash ? width : slice;
            if (!target.empty()) { UiCssToken ws; ws.Type = UiCssTok::Whitespace; target.push_back(ws); }
            target.insert(target.end(), comp.begin(), comp.end());
        }
        if (bi.Src.empty()) { error = "border-image needs a url()"; return false; }
        if (!slice.empty() && !ParseBorderImageSlice(slice, bi, error)) return false;
        if (!width.empty() && !ParseBorderImageWidth(width, bi, error)) return false;
        fx.BorderImage = bi;
        return true;
    }

    // ── shadows, outline, text effects
    if (name == "box-shadow") return ParseShadowList(value, ctx, true, true, fx.BoxShadow, error);
    if (name == "text-shadow") return ParseShadowList(value, ctx, false, false, fx.TextShadow, error);
    if (name == "outline-width") return ParseBorderWidth(value, ctx, fx.OutlineWidth, error);
    if (name == "outline-style")
    {
        if (value.size() == 1 && IsLineStyle(value[0]))
        {
            const std::string n = Lower(value[0].Text);
            fx.OutlineSolid = n != "none" && n != "hidden";
            return true;
        }
        error = "invalid outline style";
        return false;
    }
    if (name == "outline-color") return ParseColour(value, ctx, fx.OutlineColor, error);
    if (name == "outline-offset")
    {
        UiLength len;
        if (!UiParseLengthComponent(value, ctx, len, error, false)) return false;
        fx.OutlineOffset = len.Px;
        return true;
    }
    if (name == "outline")
    {
        float width = 3.f;
        bool solid = false;
        glm::vec4 colour = ctx.CurrentColor;
        for (const Tokens& comp : UiSplitComponents(value))
        {
            if (comp.size() == 1 && IsLineStyle(comp[0]))
            {
                const std::string n = Lower(comp[0].Text);
                solid = n != "none" && n != "hidden";
            }
            else if (comp.size() == 1 && (comp[0].Type == UiCssTok::Dimension || comp[0].Type == UiCssTok::Number ||
                                          IsIdent(comp[0], "thin") || IsIdent(comp[0], "medium") || IsIdent(comp[0], "thick")))
            {
                if (!ParseBorderWidth(comp, ctx, width, error)) return false;
            }
            else if (!UiParseColorComponent(comp, ctx, colour, error))
                return false;
        }
        fx.OutlineWidth = width;
        fx.OutlineSolid = solid;
        fx.OutlineColor = colour;
        return true;
    }
    if (name == "text-stroke-width" || name == "-webkit-text-stroke-width")
    {
        UiLength len;
        if (!UiParseLengthComponent(value, ctx, len, error, false)) return false;
        fx.TextStrokeWidth = std::max(0.f, len.Px);
        return true;
    }
    if (name == "text-stroke-color" || name == "-webkit-text-stroke-color") return ParseColour(value, ctx, fx.TextStrokeColor, error);
    if (name == "text-stroke" || name == "-webkit-text-stroke")
    {
        float width = 0.f;
        glm::vec4 colour = ctx.CurrentColor;
        for (const Tokens& comp : UiSplitComponents(value))
        {
            if (comp.size() == 1 && (comp[0].Type == UiCssTok::Dimension || comp[0].Type == UiCssTok::Number ||
                                     IsIdent(comp[0], "thin") || IsIdent(comp[0], "medium") || IsIdent(comp[0], "thick")))
            {
                if (!ParseBorderWidth(comp, ctx, width, error)) return false;
            }
            else if (!UiParseColorComponent(comp, ctx, colour, error))
                return false;
        }
        fx.TextStrokeWidth = width;
        fx.TextStrokeColor = colour;
        return true;
    }

    // ── effects
    if (name == "filter") return ParseFilter(value, ctx, true, fx.Filter, error);
    if (name == "backdrop-filter")
    {
        if (!ParseFilter(value, ctx, false, fx.BackdropFilter, error))
            return false;
        return true;
    }
    if (name == "isolation")
    {
        if (value.size() == 1 && IsIdent(value[0], "auto")) { fx.Isolate = false; return true; }
        if (value.size() == 1 && IsIdent(value[0], "isolate")) { fx.Isolate = true; return true; }
        error = "isolation supports auto and isolate";
        return false;
    }
    if (name == "cache")
    {
        if (none || (value.size() == 1 && IsIdent(value[0], "auto"))) { fx.CacheLayer = false; return true; }
        if (value.size() == 1 && IsIdent(value[0], "layer")) { fx.CacheLayer = true; return true; }
        error = "cache supports none and layer";
        return false;
    }
    if (name == "wash-color") return ParseColour(value, ctx, fx.WashColor, error);
    if (name == "mask-image")
    {
        if (none) { fx.MaskImage = UiGradient(); return true; }
        const auto comps = UiSplitComponents(value);
        if (comps.size() != 1) { error = "expected one gradient"; return false; }
        UiGradient g;
        if (!ParseGradient(comps[0], ctx, g, error)) return false;
        if (g.Kind == UiGradKind::Conic) { error = "mask-image supports linear-gradient() and radial-gradient()"; return false; }
        fx.MaskImage = g;
        return true;
    }
    if (name == "clip-path") return ParseClipPath(value, ctx, fx.ClipPath, error);
    if (name == "mix-blend-mode")
    {
        if (value.size() == 1 && value[0].Type == UiCssTok::Ident)
        {
            const std::string n = Lower(value[0].Text);
            if (n == "normal") { fx.MixBlend = UiBlend::Normal; return true; }
            if (n == "plus-lighter") { fx.MixBlend = UiBlend::PlusLighter; return true; }
            if (n == "multiply") { fx.MixBlend = UiBlend::Multiply; return true; }
            if (n == "screen") { fx.MixBlend = UiBlend::Screen; return true; }
        }
        error = "mix-blend-mode supports normal, plus-lighter, multiply and screen";
        return false;
    }
    if (name == "image-rendering")
    {
        if (value.size() == 1 && IsIdent(value[0], "auto")) { fx.ImageRendering = UiImageRendering::Auto; return true; }
        if (value.size() == 1 && IsIdent(value[0], "pixelated")) { fx.ImageRendering = UiImageRendering::Pixelated; return true; }
        error = "image-rendering supports auto and pixelated";
        return false;
    }
    if (name == "object-position")
    {
        UiBgPos p;
        if (!ParsePosition(UiSplitComponents(value), ctx, p.X, p.Y, error)) return false;
        fx.ObjectPosition = p;
        return true;
    }
    if (name == "transform") return ParseTransform(value, ctx, fx.Transform, error);
    if (name == "transform-origin")
    {
        UiLength x, y;
        if (!ParsePosition(UiSplitComponents(value), ctx, x, y, error)) return false;
        fx.OriginX = x;
        fx.OriginY = y;
        return true;
    }

    // ── transitions
    if (name == "transition")
    {
        std::vector<UiTransitionDef> list;
        if (!none)
            for (const Tokens& item : SplitCommas(value))
            {
                UiTransitionDef t;
                if (!ParseTransitionItem(item, t, error)) return false;
                list.push_back(t);
            }
        fx.Transitions = std::move(list);
        return true;
    }
    if (name == "transition-property")
    {
        std::vector<std::string> names;
        if (none) { fx.Transitions.clear(); return true; }
        if (!ParseNamedList(value, names, error)) return false;
        for (size_t i = 0; i < names.size(); ++i)
            Slot(fx.Transitions, i).Property = names[i];
        fx.Transitions.resize(names.size());
        return true;
    }
    if (name == "transition-duration" || name == "transition-delay")
    {
        std::vector<float> times;
        for (const Tokens& item : SplitCommas(value))
        {
            float t;
            if (!ParseTime(item, t)) { error = "expected a time"; return false; }
            times.push_back(t);
        }
        for (size_t i = 0; i < times.size(); ++i)
            (name == "transition-duration" ? Slot(fx.Transitions, i).Duration : Slot(fx.Transitions, i).Delay) =
                name == "transition-duration" ? std::max(0.f, times[i]) : times[i];
        return true;
    }
    if (name == "transition-timing-function")
    {
        std::vector<UiEasing> list;
        for (const Tokens& item : SplitCommas(value))
        {
            UiEasing e;
            if (!ParseEasing(item, e, error)) return false;
            list.push_back(e);
        }
        for (size_t i = 0; i < list.size(); ++i)
            Slot(fx.Transitions, i).Easing = list[i];
        return true;
    }

    // ── animations
    if (name == "animation")
    {
        std::vector<UiAnimationDef> list;
        if (!none)
            for (const Tokens& item : SplitCommas(value))
            {
                UiAnimationDef a;
                if (!ParseAnimationItem(item, a, error)) return false;
                list.push_back(a);
            }
        fx.Animations = std::move(list);
        return true;
    }
    if (name == "animation-name")
    {
        std::vector<std::string> names;
        if (none) { fx.Animations.clear(); return true; }
        std::vector<std::string> raw;
        for (const Tokens& item : SplitCommas(value))
        {
            if (item.size() != 1 || item[0].Type != UiCssTok::Ident) { error = "expected an animation name"; return false; }
            raw.push_back(item[0].Text);
        }
        for (size_t i = 0; i < raw.size(); ++i)
            Slot(fx.Animations, i).Name = raw[i];
        fx.Animations.resize(raw.size());
        return true;
    }
    if (name == "animation-duration" || name == "animation-delay")
    {
        std::vector<float> times;
        for (const Tokens& item : SplitCommas(value))
        {
            float t;
            if (!ParseTime(item, t)) { error = "expected a time"; return false; }
            times.push_back(t);
        }
        for (size_t i = 0; i < times.size(); ++i)
        {
            if (name == "animation-duration") Slot(fx.Animations, i).Duration = std::max(0.f, times[i]);
            else Slot(fx.Animations, i).Delay = times[i];
        }
        return true;
    }
    if (name == "animation-timing-function")
    {
        std::vector<UiEasing> list;
        for (const Tokens& item : SplitCommas(value))
        {
            UiEasing e;
            if (!ParseEasing(item, e, error)) return false;
            list.push_back(e);
        }
        for (size_t i = 0; i < list.size(); ++i)
            Slot(fx.Animations, i).Easing = list[i];
        return true;
    }
    if (name == "animation-iteration-count")
    {
        std::vector<float> list;
        for (const Tokens& item : SplitCommas(value))
        {
            float n;
            if (item.size() == 1 && IsIdent(item[0], "infinite")) list.push_back(-1.f);
            else if (ParseNumberComp(item, n) && n >= 0.f) list.push_back(n);
            else { error = "expected a number or infinite"; return false; }
        }
        for (size_t i = 0; i < list.size(); ++i)
            Slot(fx.Animations, i).Iterations = list[i];
        return true;
    }
    if (name == "animation-direction")
    {
        std::vector<UiAnimationDef::Direction> list;
        for (const Tokens& item : SplitCommas(value))
        {
            if (item.size() != 1 || item[0].Type != UiCssTok::Ident) { error = "invalid animation-direction"; return false; }
            const std::string n = Lower(item[0].Text);
            if (n == "normal") list.push_back(UiAnimationDef::Direction::Normal);
            else if (n == "reverse") list.push_back(UiAnimationDef::Direction::Reverse);
            else if (n == "alternate") list.push_back(UiAnimationDef::Direction::Alternate);
            else if (n == "alternate-reverse") list.push_back(UiAnimationDef::Direction::AlternateReverse);
            else { error = "invalid animation-direction '" + n + "'"; return false; }
        }
        for (size_t i = 0; i < list.size(); ++i)
            Slot(fx.Animations, i).Dir = list[i];
        return true;
    }
    if (name == "animation-fill-mode")
    {
        std::vector<UiAnimationDef::Fill> list;
        for (const Tokens& item : SplitCommas(value))
        {
            if (item.size() != 1 || item[0].Type != UiCssTok::Ident) { error = "invalid animation-fill-mode"; return false; }
            const std::string n = Lower(item[0].Text);
            if (n == "none") list.push_back(UiAnimationDef::Fill::None);
            else if (n == "forwards") list.push_back(UiAnimationDef::Fill::Forwards);
            else if (n == "backwards") list.push_back(UiAnimationDef::Fill::Backwards);
            else if (n == "both") list.push_back(UiAnimationDef::Fill::Both);
            else { error = "invalid animation-fill-mode '" + n + "'"; return false; }
        }
        for (size_t i = 0; i < list.size(); ++i)
            Slot(fx.Animations, i).FillMode = list[i];
        return true;
    }

    // ── sounds and pulses
    if (UiFxIsRuleSoundProperty(name))
    {
        UiRuleSounds ignored;   // the resolver reads these from the rules, see UiElem::UpdateSoundRules; here the value is only checked
        return UiFxParseRuleSound(name, value, ignored, error);
    }
    if (name == "pulse-duration")
    {
        float t;
        if (!ParseTime(value, t) || t < 0.f) { error = "expected a time >= 0"; return false; }
        fx.PulseDuration = t;
        return true;
    }

    handled = false;
    return false;
}

bool UiFxIsRuleSoundProperty(const std::string& name)
{
    return name == "sound-enter" || name == "sound-exit" || name == "sound-volume";
}

bool UiFxParseRuleSound(const std::string& property, const std::vector<UiCssToken>& valueIn, UiRuleSounds& into, std::string& error)
{
    const Tokens value = Trim(valueIn);
    if (property == "sound-volume")
    {
        float v = 1.f;
        if (value.size() != 1 || !ParseNumberComp(value, v) || v < 0.f) { error = "expected a number >= 0"; return false; }
        into.Volume = v;
        return true;
    }
    std::string path;
    if (IsNone(value)) path.clear();
    else if (value.size() == 1 && value[0].Type == UiCssTok::String) path = value[0].Text;
    else if (!ParseUrl(value, path)) { error = "expected a sound path as a string or url()"; return false; }
    (property == "sound-enter" ? into.Enter : into.Exit) = path;
    return true;
}

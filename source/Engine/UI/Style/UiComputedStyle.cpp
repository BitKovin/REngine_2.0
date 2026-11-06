#include "UiComputedStyle.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
    const char* kSides[4] = { "top", "right", "bottom", "left" };
    const char* kCorners[4] = { "top-left", "top-right", "bottom-right", "bottom-left" };

    bool IsWs(const UiCssToken& t) { return t.Type == UiCssTok::Whitespace; }
    bool IsIdent(const UiCssToken& t, const char* s) { return t.Type == UiCssTok::Ident && t.Text == s; }

    std::string Lower(std::string s)
    {
        for (char& c : s)
            if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
        return s;
    }

    // ── lengths ───────────────────────────────────────────────────────────────

    struct Calc { float Px = 0.f, Pct = 0.f, Num = 0.f; bool IsNum = false; bool Ok = true; };

    struct CalcParser
    {
        const std::vector<UiCssToken>& T;
        size_t I = 0;
        const UiStyleParseContext& Ctx;
        std::string Err;

        void Ws() { while (I < T.size() && IsWs(T[I])) ++I; }

        bool Unit(const UiCssToken& t, float& px) const
        {
            const float v = (float)t.Number;
            if (t.Text == "px") px = v;
            else if (t.Text == "em") px = v * Ctx.FontSize;
            else if (t.Text == "rem") px = v * Ctx.RootFontSize;
            else if (t.Text == "vw") px = v * 0.01f * Ctx.Viewport.x;
            else if (t.Text == "vh") px = v * 0.01f * Ctx.Viewport.y;
            else return false;
            return true;
        }

        Calc Primary()
        {
            Calc c;
            Ws();
            if (I >= T.size()) { c.Ok = false; Err = "unexpected end of calc()"; return c; }
            const UiCssToken& t = T[I];
            if (t.Type == UiCssTok::Number) { c.IsNum = true; c.Num = (float)t.Number; ++I; return c; }
            if (t.Type == UiCssTok::Percentage) { c.Pct = (float)t.Number; ++I; return c; }
            if (t.Type == UiCssTok::Dimension)
            {
                if (!Unit(t, c.Px)) { c.Ok = false; Err = "unknown unit '" + t.Text + "'"; }
                ++I;
                return c;
            }
            if (t.Type == UiCssTok::LParen || (t.Type == UiCssTok::Function && t.Text == "calc"))
            {
                ++I;
                c = Sum();
                Ws();
                if (I < T.size() && T[I].Type == UiCssTok::RParen) ++I;
                else { c.Ok = false; Err = "missing ')' in calc()"; }
                return c;
            }
            c.Ok = false;
            Err = "unexpected token in calc()";
            return c;
        }

        Calc Product()
        {
            Calc a = Primary();
            while (a.Ok)
            {
                Ws();
                if (I >= T.size() || T[I].Type != UiCssTok::Delim || (T[I].Text != "*" && T[I].Text != "/")) break;
                const bool mul = T[I].Text == "*";
                ++I;
                Calc b = Primary();
                if (!b.Ok) return b;
                if (mul)
                {
                    if (a.IsNum && b.IsNum) a.Num *= b.Num;
                    else if (a.IsNum) { a.Px = b.Px * a.Num; a.Pct = b.Pct * a.Num; a.IsNum = false; }
                    else if (b.IsNum) { a.Px *= b.Num; a.Pct *= b.Num; }
                    else { a.Ok = false; Err = "calc(): cannot multiply two lengths"; }
                }
                else
                {
                    if (!b.IsNum || b.Num == 0.f) { a.Ok = false; Err = "calc(): can only divide by a non-zero number"; }
                    else if (a.IsNum) a.Num /= b.Num;
                    else { a.Px /= b.Num; a.Pct /= b.Num; }
                }
            }
            return a;
        }

        Calc Sum()
        {
            Calc a = Product();
            while (a.Ok)
            {
                // '+'/'-' need surrounding whitespace in CSS, the tokenizer yields them as Delim.
                size_t save = I;
                Ws();
                if (I >= T.size() || T[I].Type != UiCssTok::Delim || (T[I].Text != "+" && T[I].Text != "-")) { I = save; break; }
                const bool plus = T[I].Text == "+";
                ++I;
                Calc b = Product();
                if (!b.Ok) return b;
                if (a.IsNum || b.IsNum) { a.Ok = false; Err = "calc(): cannot add a number to a length"; break; }
                const float s = plus ? 1.f : -1.f;
                a.Px += s * b.Px;
                a.Pct += s * b.Pct;
            }
            return a;
        }
    };

    // Parses a single component as a length-percentage (px, em, rem, vw, vh, %, 0, calc()).
    bool ParseLengthPct(const std::vector<UiCssToken>& comp, const UiStyleParseContext& ctx, UiLength& out, std::string& err, bool allowPercent = true)
    {
        if (comp.size() == 1)
        {
            const UiCssToken& t = comp[0];
            if (t.Type == UiCssTok::Number && t.Number == 0.0) { out = UiLength(); return true; }
            if (t.Type == UiCssTok::Percentage)
            {
                if (!allowPercent) { err = "percentages are not allowed here"; return false; }
                out = UiLength(); out.Kind = UiLenKind::Percent; out.Percent = (float)t.Number; return true;
            }
            if (t.Type == UiCssTok::Dimension)
            {
                CalcParser cp{ comp, 0, ctx };
                float px = 0.f;
                if (!cp.Unit(t, px)) { err = "unknown unit '" + t.Text + "'"; return false; }
                out = UiLength(); out.Px = px; return true;
            }
            err = "expected a length";
            return false;
        }
        if (!comp.empty() && comp[0].Type == UiCssTok::Function && comp[0].Text == "calc")
        {
            std::vector<UiCssToken> inner(comp.begin() + 1, comp.end());
            if (!inner.empty() && inner.back().Type == UiCssTok::RParen) inner.pop_back();
            CalcParser cp{ inner, 0, ctx };
            Calc c = cp.Sum();
            cp.Ws();
            if (!c.Ok) { err = cp.Err; return false; }
            if (cp.I != inner.size()) { err = "unexpected token in calc()"; return false; }
            if (c.IsNum) { err = "calc() must produce a length"; return false; }
            if (c.Pct != 0.f && !allowPercent) { err = "percentages are not allowed here"; return false; }
            out = UiLength(); out.Kind = UiLenKind::Calc; out.Px = c.Px; out.Percent = c.Pct;
            return true;
        }
        err = "expected a length";
        return false;
    }

    bool ParseColor(const std::vector<UiCssToken>& comp, const UiStyleParseContext& ctx, glm::vec4& out, std::string& err)
    {
        if (comp.size() == 1 && comp[0].Type == UiCssTok::Hash)
        {
            const std::string& h = comp[0].Text;
            auto hex = [&](char c) -> int {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                return -1;
            };
            for (char c : h) if (hex(c) < 0) { err = "invalid hex colour"; return false; }
            float v[4] = { 0, 0, 0, 1 };
            if (h.size() == 3 || h.size() == 4)
                for (size_t i = 0; i < h.size(); ++i) v[i] = (hex(h[i]) * 17) / 255.f;
            else if (h.size() == 6 || h.size() == 8)
                for (size_t i = 0; i < h.size() / 2; ++i) v[i] = (hex(h[i * 2]) * 16 + hex(h[i * 2 + 1])) / 255.f;
            else { err = "invalid hex colour"; return false; }
            out = glm::vec4(v[0], v[1], v[2], v[3]);
            return true;
        }
        if (comp.size() == 1 && comp[0].Type == UiCssTok::Ident)
        {
            const std::string n = Lower(comp[0].Text);
            if (n == "transparent") { out = glm::vec4(0.f); return true; }
            if (n == "currentcolor") { out = ctx.CurrentColor; return true; }
            static const struct { const char* N; unsigned V; } kNames[] = {
                { "black", 0x000000 }, { "white", 0xffffff }, { "red", 0xff0000 }, { "green", 0x008000 }, { "blue", 0x0000ff },
                { "yellow", 0xffff00 }, { "cyan", 0x00ffff }, { "magenta", 0xff00ff }, { "gray", 0x808080 }, { "grey", 0x808080 },
                { "orange", 0xffa500 }, { "purple", 0x800080 }, { "silver", 0xc0c0c0 }, { "maroon", 0x800000 }, { "olive", 0x808000 },
                { "lime", 0x00ff00 }, { "teal", 0x008080 }, { "navy", 0x000080 }, { "aqua", 0x00ffff }, { "fuchsia", 0xff00ff } };
            for (const auto& e : kNames)
                if (n == e.N)
                {
                    out = glm::vec4(((e.V >> 16) & 255) / 255.f, ((e.V >> 8) & 255) / 255.f, (e.V & 255) / 255.f, 1.f);
                    return true;
                }
            err = "unknown colour '" + comp[0].Text + "'";
            return false;
        }
        if (!comp.empty() && comp[0].Type == UiCssTok::Function)
        {
            const std::string fn = Lower(comp[0].Text);
            const bool rgb = fn == "rgb" || fn == "rgba";
            const bool hsl = fn == "hsl" || fn == "hsla";
            if (rgb || hsl)
            {
                std::vector<UiCssToken> args;
                for (size_t i = 1; i < comp.size(); ++i)
                {
                    const UiCssToken& t = comp[i];
                    if (t.Type == UiCssTok::Comma || t.Type == UiCssTok::Whitespace || (t.Type == UiCssTok::Delim && t.Text == "/")) continue;
                    if (t.Type == UiCssTok::RParen) break;
                    args.push_back(t);
                }
                if (args.size() < 3 || args.size() > 4) { err = "expected 3 or 4 colour components"; return false; }
                float c[4] = { 0, 0, 0, 1 };
                for (size_t i = 0; i < args.size(); ++i)
                {
                    const UiCssToken& a = args[i];
                    if (a.Type != UiCssTok::Number && a.Type != UiCssTok::Percentage && !(a.Type == UiCssTok::Dimension && a.Text == "deg"))
                    { err = "invalid colour component"; return false; }
                    float v = (float)a.Number;
                    if (i == 3) c[3] = a.Type == UiCssTok::Percentage ? v / 100.f : v;
                    else if (rgb) c[i] = a.Type == UiCssTok::Percentage ? v / 100.f : v / 255.f;
                    else c[i] = v;   // hsl handled below
                }
                if (rgb)
                {
                    out = glm::vec4(std::clamp(c[0], 0.f, 1.f), std::clamp(c[1], 0.f, 1.f), std::clamp(c[2], 0.f, 1.f), std::clamp(c[3], 0.f, 1.f));
                    return true;
                }
                float h = std::fmod(c[0], 360.f);
                if (h < 0.f) h += 360.f;
                const float s = std::clamp(c[1] / 100.f, 0.f, 1.f), l = std::clamp(c[2] / 100.f, 0.f, 1.f);
                auto f = [&](float n) {
                    const float k = std::fmod(n + h / 30.f, 12.f);
                    const float a = s * std::min(l, 1.f - l);
                    return l - a * std::max(-1.f, std::min({ k - 3.f, 9.f - k, 1.f }));
                };
                out = glm::vec4(f(0), f(8), f(4), std::clamp(c[3], 0.f, 1.f));
                return true;
            }
        }
        err = "expected a colour";
        return false;
    }

    template <size_t N>
    bool ParseKeyword(const std::vector<UiCssToken>& comp, const char* const (&names)[N], int& index, std::string& err)
    {
        if (comp.size() == 1 && comp[0].Type == UiCssTok::Ident)
        {
            const std::string n = Lower(comp[0].Text);
            for (size_t i = 0; i < N; ++i)
                if (n == names[i]) { index = (int)i; return true; }
        }
        err = "invalid keyword";
        return false;
    }

    bool ParseNumber(const std::vector<UiCssToken>& comp, float& out)
    {
        if (comp.size() == 1 && comp[0].Type == UiCssTok::Number) { out = (float)comp[0].Number; return true; }
        return false;
    }

    // width / height with the Panorama-style keywords.
    bool ParseSize(const std::vector<UiCssToken>& comp, const UiStyleParseContext& ctx, bool isWidth, UiLength& out, std::string& err)
    {
        if (comp.size() == 1 && comp[0].Type == UiCssTok::Ident)
        {
            const std::string n = Lower(comp[0].Text);
            if (n == "fit-children") { out = UiLength(); out.Kind = UiLenKind::FitChildren; return true; }
            if (n == "auto") { out = UiLength(); out.Kind = UiLenKind::FitChildren; return true; }
            err = "invalid size '" + comp[0].Text + "'";
            return false;
        }
        if (!comp.empty() && comp[0].Type == UiCssTok::Function)
        {
            const std::string fn = Lower(comp[0].Text);
            float v = 0.f;
            bool percentArg = false;
            for (size_t i = 1; i < comp.size(); ++i)
            {
                if (comp[i].Type == UiCssTok::Number) { v = (float)comp[i].Number; break; }
                if (comp[i].Type == UiCssTok::Percentage) { v = (float)comp[i].Number; percentArg = true; break; }
            }
            if (fn == "fill-parent-flow")
            {
                out = UiLength(); out.Kind = UiLenKind::FillParentFlow; out.Weight = percentArg ? v / 100.f : v;
                if (out.Weight <= 0.f) { err = "fill-parent-flow() needs a positive weight"; return false; }
                return true;
            }
            if (fn == "height-percentage" && isWidth) { out = UiLength(); out.Kind = UiLenKind::HeightPercentage; out.Percent = v; return true; }
            if (fn == "width-percentage" && !isWidth) { out = UiLength(); out.Kind = UiLenKind::WidthPercentage; out.Percent = v; return true; }
        }
        return ParseLengthPct(comp, ctx, out, err);
    }

    bool ParseBorderWidthComp(const std::vector<UiCssToken>& comp, const UiStyleParseContext& ctx, float& out, std::string& err)
    {
        if (comp.size() == 1 && comp[0].Type == UiCssTok::Ident)
        {
            const std::string n = Lower(comp[0].Text);
            if (n == "thin") { out = 1.f; return true; }
            if (n == "medium") { out = 3.f; return true; }
            if (n == "thick") { out = 5.f; return true; }
        }
        UiLength len;
        if (!ParseLengthPct(comp, ctx, len, err, false)) return false;
        out = std::max(0.f, len.Px);
        return true;
    }

    bool IsStyleKeyword(const UiCssToken& t)
    {
        static const char* kStyles[] = { "none", "hidden", "solid", "dashed", "dotted", "double", "groove", "ridge", "inset", "outset" };
        if (t.Type != UiCssTok::Ident) return false;
        const std::string n = Lower(t.Text);
        for (const char* s : kStyles) if (n == s) return true;
        return false;
    }

    int SideIndex(const std::string& s)
    {
        for (int i = 0; i < 4; ++i) if (s == kSides[i]) return i;
        return -1;
    }
}

std::vector<std::vector<UiCssToken>> UiSplitComponents(const std::vector<UiCssToken>& value)
{
    std::vector<std::vector<UiCssToken>> out;
    std::vector<UiCssToken> cur;
    int depth = 0;
    for (const UiCssToken& t : value)
    {
        if (t.Type == UiCssTok::Function || t.Type == UiCssTok::LParen) ++depth;
        else if (t.Type == UiCssTok::RParen) depth = std::max(0, depth - 1);
        if (depth == 0 && t.Type == UiCssTok::Whitespace)
        {
            if (!cur.empty()) { out.push_back(std::move(cur)); cur.clear(); }
            continue;
        }
        cur.push_back(t);
    }
    if (!cur.empty()) out.push_back(std::move(cur));
    return out;
}

const std::vector<UiPropertyInfo>& UiStyleProperties()
{
    static const std::vector<UiPropertyInfo> props = {
        { "width", "<length-percentage> | fit-children | fill-parent-flow(<number>) | height-percentage(<percentage>) | calc()", false, "fit-children", "layout", true },
        { "height", "<length-percentage> | fit-children | fill-parent-flow(<number>) | width-percentage(<percentage>) | calc()", false, "fit-children", "layout", true },
        { "min-width", "<length-percentage>", false, "0", "layout", true },
        { "min-height", "<length-percentage>", false, "0", "layout", true },
        { "max-width", "<length-percentage> | none", false, "none", "layout", true },
        { "max-height", "<length-percentage> | none", false, "none", "layout", true },
        { "flow-children", "none | down | right | down-wrap | right-wrap", false, "none", "layout", true },
        { "horizontal-align", "left | center | right", false, "left", "layout", true },
        { "vertical-align", "top | center | bottom", false, "top", "layout", true },
        { "x", "<length-percentage>", false, "0", "layout", true },
        { "y", "<length-percentage>", false, "0", "layout", true },
        { "margin", "<length-percentage>{1,4}", false, "0", "layout", false },
        { "margin-top", "<length-percentage>", false, "0", "layout", true },
        { "margin-right", "<length-percentage>", false, "0", "layout", true },
        { "margin-bottom", "<length-percentage>", false, "0", "layout", true },
        { "margin-left", "<length-percentage>", false, "0", "layout", true },
        { "padding", "<length-percentage>{1,4}", false, "0", "layout", false },
        { "padding-top", "<length-percentage>", false, "0", "layout", true },
        { "padding-right", "<length-percentage>", false, "0", "layout", true },
        { "padding-bottom", "<length-percentage>", false, "0", "layout", true },
        { "padding-left", "<length-percentage>", false, "0", "layout", true },
        { "gap", "<length>", false, "0", "layout", true },
        { "overflow", "visible | clip", false, "visible", "layout+paint", true },
        { "visibility", "visible | hidden | collapse", true, "visible", "layout", true },
        { "z-index", "<integer>", false, "0", "paint", true },
        { "background-color", "<color>", false, "transparent", "paint", true },
        { "border", "<width> || <style> || <color>", false, "0 none black", "layout+paint", false },
        { "border-width", "<length> | thin | medium | thick, 1-4 values", false, "medium", "layout+paint", false },
        { "border-style", "none | solid, 1-4 values", false, "none", "layout+paint", false },
        { "border-color", "<color>, 1-4 values", false, "currentcolor", "paint", false },
        { "border-top", "<width> || <style> || <color>", false, "", "layout+paint", false },
        { "border-right", "<width> || <style> || <color>", false, "", "layout+paint", false },
        { "border-bottom", "<width> || <style> || <color>", false, "", "layout+paint", false },
        { "border-left", "<width> || <style> || <color>", false, "", "layout+paint", false },
        { "border-top-width", "<length> | thin | medium | thick", false, "medium", "layout+paint", true },
        { "border-right-width", "<length> | thin | medium | thick", false, "medium", "layout+paint", true },
        { "border-bottom-width", "<length> | thin | medium | thick", false, "medium", "layout+paint", true },
        { "border-left-width", "<length> | thin | medium | thick", false, "medium", "layout+paint", true },
        { "border-top-style", "none | solid", false, "none", "layout+paint", true },
        { "border-right-style", "none | solid", false, "none", "layout+paint", true },
        { "border-bottom-style", "none | solid", false, "none", "layout+paint", true },
        { "border-left-style", "none | solid", false, "none", "layout+paint", true },
        { "border-top-color", "<color>", false, "currentcolor", "paint", true },
        { "border-right-color", "<color>", false, "currentcolor", "paint", true },
        { "border-bottom-color", "<color>", false, "currentcolor", "paint", true },
        { "border-left-color", "<color>", false, "currentcolor", "paint", true },
        { "border-radius", "<length-percentage>{1,4}", false, "0", "paint", false },
        { "border-top-left-radius", "<length-percentage>", false, "0", "paint", true },
        { "border-top-right-radius", "<length-percentage>", false, "0", "paint", true },
        { "border-bottom-right-radius", "<length-percentage>", false, "0", "paint", true },
        { "border-bottom-left-radius", "<length-percentage>", false, "0", "paint", true },
        { "opacity", "<number> | <percentage>", false, "1", "paint", true },
        { "color", "<color>", true, "white", "paint", true },
        { "font-family", "<ident | string>#", true, "default", "layout", true },
        { "font-size", "<length>", true, "24px", "layout", true },
        { "font-weight", "normal | bold | 100..900", true, "400", "layout", true },
        { "font-style", "normal | italic", true, "normal", "layout", true },
        { "letter-spacing", "normal | <length>", true, "0", "layout", true },
        { "line-height", "normal | <number> | <length>", true, "normal", "layout", true },
        { "text-align", "left | center | right", true, "left", "paint", true },
        { "white-space", "normal | nowrap | pre", true, "normal", "layout", true },
        { "text-overflow", "clip | ellipsis | shrink", false, "clip", "layout", true },
        { "text-transform", "none | uppercase | lowercase", true, "none", "layout", true },
        { "object-fit", "fill | contain | cover | none | scale-down", false, "fill", "paint", true },
    };
    return props;
}

bool UiIsKnownProperty(const std::string& name)
{
    if (name.rfind("--", 0) == 0)
        return true;
    for (const UiPropertyInfo& p : UiStyleProperties())
        if (name == p.Name)
            return true;
    return false;
}

bool UiSubstituteVars(const std::vector<UiCssToken>& in, const std::unordered_map<std::string, std::vector<UiCssToken>>& custom,
                      std::vector<UiCssToken>& out, std::string& error)
{
    std::vector<UiCssToken> cur = in;
    for (int pass = 0; pass <= 8; ++pass)
    {
        bool found = false;
        std::vector<UiCssToken> next;
        for (size_t i = 0; i < cur.size(); ++i)
        {
            if (!(cur[i].Type == UiCssTok::Function && cur[i].Text == "var"))
            {
                next.push_back(cur[i]);
                continue;
            }
            found = true;
            // collect the arguments up to the matching ')'
            size_t j = i + 1;
            int depth = 1;
            std::vector<UiCssToken> args;
            for (; j < cur.size(); ++j)
            {
                if (cur[j].Type == UiCssTok::Function || cur[j].Type == UiCssTok::LParen) ++depth;
                else if (cur[j].Type == UiCssTok::RParen && --depth == 0) break;
                args.push_back(cur[j]);
            }
            size_t a = 0;
            while (a < args.size() && IsWs(args[a])) ++a;
            if (a >= args.size() || args[a].Type != UiCssTok::Ident || args[a].Text.rfind("--", 0) != 0)
            {
                error = "var() needs a custom property name";
                return false;
            }
            const std::string name = args[a].Text;
            size_t b = a + 1;
            while (b < args.size() && IsWs(args[b])) ++b;
            std::vector<UiCssToken> fallback;
            const bool hasFallback = b < args.size() && args[b].Type == UiCssTok::Comma;
            if (hasFallback)
            {
                fallback.assign(args.begin() + b + 1, args.end());
                while (!fallback.empty() && IsWs(fallback.front())) fallback.erase(fallback.begin());
            }
            const auto it = custom.find(name);
            if (it != custom.end())
                next.insert(next.end(), it->second.begin(), it->second.end());
            else if (hasFallback)
                next.insert(next.end(), fallback.begin(), fallback.end());
            else
            {
                error = "undefined variable " + name;
                return false;
            }
            i = j;
        }
        cur = std::move(next);
        if (!found)
        {
            out = std::move(cur);
            return true;
        }
    }
    error = "var() cycle or nesting deeper than 8";
    return false;
}

bool UiExpandProperty(const std::string& name, const std::vector<UiCssToken>& value, UiLonghandList& out, std::string& error)
{
    auto sides4 = [&](const std::string& prefix, const std::string& suffix) {
        const auto comps = UiSplitComponents(value);
        if (comps.empty() || comps.size() > 4) { error = "expected 1 to 4 values"; return false; }
        const std::vector<UiCssToken>* v[4];
        v[0] = &comps[0];
        v[1] = comps.size() > 1 ? &comps[1] : v[0];
        v[2] = comps.size() > 2 ? &comps[2] : v[0];
        v[3] = comps.size() > 3 ? &comps[3] : v[1];
        for (int i = 0; i < 4; ++i)
            out.push_back({ prefix + kSides[i] + suffix, *v[i] });
        return true;
    };

    if (name == "margin") return sides4("margin-", "");
    if (name == "padding") return sides4("padding-", "");
    if (name == "border-width") return sides4("border-", "-width");
    if (name == "border-style") return sides4("border-", "-style");
    if (name == "border-color") return sides4("border-", "-color");
    if (name == "border-radius")
    {
        const auto comps = UiSplitComponents(value);
        if (comps.empty() || comps.size() > 4) { error = "expected 1 to 4 values"; return false; }
        const std::vector<UiCssToken>* v[4];
        v[0] = &comps[0];
        v[1] = comps.size() > 1 ? &comps[1] : v[0];   // TR
        v[2] = comps.size() > 2 ? &comps[2] : v[0];   // BR
        v[3] = comps.size() > 3 ? &comps[3] : v[1];   // BL
        for (int i = 0; i < 4; ++i)
            out.push_back({ std::string("border-") + kCorners[i] + "-radius", *v[i] });
        return true;
    }
    const bool allSides = name == "border";
    const int single = name.rfind("border-", 0) == 0 ? SideIndex(name.substr(7)) : -1;
    if (allSides || single >= 0)
    {
        std::vector<UiCssToken> width = { UiCssToken{ UiCssTok::Ident, "medium" } };
        std::vector<UiCssToken> style = { UiCssToken{ UiCssTok::Ident, "none" } };
        std::vector<UiCssToken> color = { UiCssToken{ UiCssTok::Ident, "currentcolor" } };
        for (const auto& comp : UiSplitComponents(value))
        {
            if (comp.size() == 1 && IsStyleKeyword(comp[0])) style = comp;
            else if ((comp.size() == 1 && (comp[0].Type == UiCssTok::Dimension || comp[0].Type == UiCssTok::Number ||
                                           IsIdent(comp[0], "thin") || IsIdent(comp[0], "medium") || IsIdent(comp[0], "thick"))) ||
                     (comp[0].Type == UiCssTok::Function && comp[0].Text == "calc")) width = comp;
            else color = comp;
        }
        for (int i = 0; i < 4; ++i)
        {
            if (!allSides && i != single) continue;
            out.push_back({ std::string("border-") + kSides[i] + "-width", width });
            out.push_back({ std::string("border-") + kSides[i] + "-style", style });
            out.push_back({ std::string("border-") + kSides[i] + "-color", color });
        }
        return true;
    }
    out.push_back({ name, value });
    return true;
}

bool UiApplyLonghand(UiComputedStyle& s, const std::string& name, const std::vector<UiCssToken>& valueTokens,
                     const UiStyleParseContext& ctx, std::string& error)
{
    std::vector<UiCssToken> value = valueTokens;
    while (!value.empty() && IsWs(value.back())) value.pop_back();
    const auto comps = UiSplitComponents(value);

    if (name.rfind("--", 0) == 0)
    {
        s.Custom[name] = value;
        return true;
    }
    if (comps.size() != 1 && name != "font-family")
    {
        error = "expected a single value";
        return false;
    }
    const std::vector<UiCssToken>& c = comps.empty() ? value : comps[0];

    auto setLen = [&](UiLength& dst, bool allowPercent = true) {
        UiLength tmp;
        if (!ParseLengthPct(c, ctx, tmp, error, allowPercent)) return false;
        dst = tmp;
        return true;
    };

    if (name == "width" || name == "height")
    {
        UiLength tmp;
        if (!ParseSize(c, ctx, name == "width", tmp, error)) return false;
        (name == "width" ? s.Width : s.Height) = tmp;
        return true;
    }
    if (name == "min-width") return setLen(s.MinWidth);
    if (name == "min-height") return setLen(s.MinHeight);
    if (name == "max-width" || name == "max-height")
    {
        UiLength tmp;
        if (c.size() == 1 && IsIdent(c[0], "none")) { tmp.Kind = UiLenKind::Auto; }
        else if (!ParseLengthPct(c, ctx, tmp, error)) return false;
        (name == "max-width" ? s.MaxWidth : s.MaxHeight) = tmp;
        return true;
    }
    if (name == "flow-children")
    {
        static const char* const k[] = { "none", "down", "right", "down-wrap", "right-wrap" };
        int i; if (!ParseKeyword(c, k, i, error)) return false;
        s.Flow = (UiFlow)i; return true;
    }
    if (name == "horizontal-align")
    {
        static const char* const k[] = { "left", "center", "right" };
        int i; if (!ParseKeyword(c, k, i, error)) return false;
        s.HAlign = (UiHAlign)i; return true;
    }
    if (name == "vertical-align")
    {
        static const char* const k[] = { "top", "center", "bottom" };
        int i; if (!ParseKeyword(c, k, i, error)) return false;
        s.VAlign = (UiVAlign)i; return true;
    }
    if (name == "x") return setLen(s.X);
    if (name == "y") return setLen(s.Y);
    if (name == "margin-top") return setLen(s.MarginTop);
    if (name == "margin-right") return setLen(s.MarginRight);
    if (name == "margin-bottom") return setLen(s.MarginBottom);
    if (name == "margin-left") return setLen(s.MarginLeft);
    if (name == "padding-top") return setLen(s.PaddingTop);
    if (name == "padding-right") return setLen(s.PaddingRight);
    if (name == "padding-bottom") return setLen(s.PaddingBottom);
    if (name == "padding-left") return setLen(s.PaddingLeft);
    if (name == "gap") return setLen(s.Gap, false);
    if (name == "overflow")
    {
        static const char* const k[] = { "visible", "clip" };
        int i; if (!ParseKeyword(c, k, i, error)) return false;
        s.Overflow = (UiOverflow)i; return true;
    }
    if (name == "visibility")
    {
        static const char* const k[] = { "visible", "hidden", "collapse" };
        int i; if (!ParseKeyword(c, k, i, error)) return false;
        s.Visibility = (UiVisibility)i; return true;
    }
    if (name == "z-index")
    {
        if (c.size() == 1 && c[0].Type == UiCssTok::Number && c[0].IsInteger) { s.ZIndex = (int)c[0].Number; return true; }
        error = "expected an integer"; return false;
    }
    if (name == "background-color") return ParseColor(c, ctx, s.BackgroundColor, error);
    if (name == "color") return ParseColor(c, ctx, s.Color, error);

    for (int i = 0; i < 4; ++i)
    {
        const std::string side = kSides[i];
        if (name == "border-" + side + "-width") return ParseBorderWidthComp(c, ctx, s.BorderWidth[i], error);
        if (name == "border-" + side + "-style")
        {
            if (c.size() == 1 && IsStyleKeyword(c[0]))
            {
                const std::string n = Lower(c[0].Text);
                s.BorderSolid[i] = n != "none" && n != "hidden";
                return true;
            }
            error = "invalid border style"; return false;
        }
        if (name == "border-" + side + "-color") return ParseColor(c, ctx, s.BorderColor[i], error);
        if (name == std::string("border-") + kCorners[i] + "-radius") return setLen(s.Radius[i]);
    }

    if (name == "opacity")
    {
        if (c.size() == 1 && c[0].Type == UiCssTok::Number) { s.Opacity = std::clamp((float)c[0].Number, 0.f, 1.f); return true; }
        if (c.size() == 1 && c[0].Type == UiCssTok::Percentage) { s.Opacity = std::clamp((float)c[0].Number / 100.f, 0.f, 1.f); return true; }
        error = "expected a number"; return false;
    }
    if (name == "font-family")
    {
        // comma list, first known family wins; names are resolved lazily by UiFontLibrary (unknown -> default)
        std::string first;
        for (size_t i = 0; i < value.size(); ++i)
        {
            if (IsWs(value[i])) continue;
            if (value[i].Type == UiCssTok::Ident || value[i].Type == UiCssTok::String) { first = value[i].Text; break; }
        }
        if (first.empty()) { error = "expected a font family"; return false; }
        s.FontFamily = first;
        return true;
    }
    if (name == "font-size")
    {
        UiLength tmp;
        if (c.size() == 1 && c[0].Type == UiCssTok::Percentage) { s.FontSize = ctx.ParentFontSize * (float)c[0].Number * 0.01f; return true; }
        UiStyleParseContext fctx = ctx;
        fctx.FontSize = ctx.ParentFontSize;   // em on font-size itself is relative to the parent
        if (!ParseLengthPct(c, fctx, tmp, error, false)) return false;
        s.FontSize = std::max(0.f, tmp.Px);
        return true;
    }
    if (name == "font-weight")
    {
        if (c.size() == 1 && c[0].Type == UiCssTok::Ident)
        {
            const std::string n = Lower(c[0].Text);
            if (n == "normal") { s.FontWeight = 400; return true; }
            if (n == "bold") { s.FontWeight = 700; return true; }
        }
        if (c.size() == 1 && c[0].Type == UiCssTok::Number && c[0].Number >= 100 && c[0].Number <= 900) { s.FontWeight = (int)c[0].Number; return true; }
        error = "expected normal, bold or 100..900"; return false;
    }
    if (name == "font-style")
    {
        static const char* const k[] = { "normal", "italic" };
        int i; if (!ParseKeyword(c, k, i, error)) return false;
        s.FontItalic = i == 1; return true;
    }
    if (name == "letter-spacing")
    {
        if (c.size() == 1 && IsIdent(c[0], "normal")) { s.LetterSpacing = 0.f; return true; }
        UiLength tmp;
        if (!ParseLengthPct(c, ctx, tmp, error, false)) return false;
        s.LetterSpacing = tmp.Px; return true;
    }
    if (name == "line-height")
    {
        float n;
        if (c.size() == 1 && IsIdent(c[0], "normal")) { s.LineHeight = -1.f; return true; }
        if (ParseNumber(c, n)) { s.LineHeight = n * ctx.FontSize; return true; }
        UiLength tmp;
        if (!ParseLengthPct(c, ctx, tmp, error, false)) return false;
        s.LineHeight = tmp.Px; return true;
    }
    if (name == "text-align")
    {
        static const char* const k[] = { "left", "center", "right" };
        int i; if (!ParseKeyword(c, k, i, error)) return false;
        s.TextAlign = (UiTextAlign)i; return true;
    }
    if (name == "white-space")
    {
        static const char* const k[] = { "normal", "nowrap", "pre" };
        int i; if (!ParseKeyword(c, k, i, error)) return false;
        s.WhiteSpace = (UiWhiteSpace)i; return true;
    }
    if (name == "text-overflow")
    {
        static const char* const k[] = { "clip", "ellipsis", "shrink" };
        int i; if (!ParseKeyword(c, k, i, error)) return false;
        s.TextOverflow = (UiTextOverflow)i; return true;
    }
    if (name == "text-transform")
    {
        static const char* const k[] = { "none", "uppercase", "lowercase" };
        int i; if (!ParseKeyword(c, k, i, error)) return false;
        s.TextTransform = (UiTextTransform)i; return true;
    }
    if (name == "object-fit")
    {
        static const char* const k[] = { "fill", "contain", "cover", "none", "scale-down" };
        int i; if (!ParseKeyword(c, k, i, error)) return false;
        s.ObjectFit = (UiObjectFit)i; return true;
    }
    error = "unknown property '" + name + "'";
    return false;
}

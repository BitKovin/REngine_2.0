#include "UiTextLayout.h"

#include <UI/Render/UiFontLibrary.h>

#include <Logger.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <set>

namespace
{
    bool IsCjk(uint32_t c)
    {
        return (c >= 0x3040 && c <= 0x30FF) || (c >= 0x3400 && c <= 0x4DBF) || (c >= 0x4E00 && c <= 0x9FFF) ||
               (c >= 0xF900 && c <= 0xFAFF) || (c >= 0xFF00 && c <= 0xFFEF);
    }

    uint32_t ToUpper(uint32_t c)
    {
        if (c >= 'a' && c <= 'z') return c - 0x20;
        if (c >= 0xE0 && c <= 0xFE && c != 0xF7) return c - 0x20;
        if (c >= 0x430 && c <= 0x44F) return c - 0x20;
        if (c >= 0x451 && c <= 0x45F) return c - 0x50;
        return c;
    }

    uint32_t ToLower(uint32_t c)
    {
        if (c >= 'A' && c <= 'Z') return c + 0x20;
        if (c >= 0xC0 && c <= 0xDE && c != 0xD7) return c + 0x20;
        if (c >= 0x410 && c <= 0x42F) return c + 0x20;
        if (c >= 0x401 && c <= 0x40F) return c + 0x50;
        return c;
    }

    struct Item
    {
        uint32_t Cp = 0;
        const UiGlyph* Glyph = nullptr;   // null for icons
        float Advance = 0.f;   // pen advance (glyph advance * scale + letter spacing)
        float Pre = 0.f;       // kerning against the previous item (UI units)
        int Style = 0;
        int Icon = -1;         // index into the icon list, or -1
    };

    struct LayoutContext
    {
        int Face = 0;
        float Scale = 1.f;
        float Size = 0.f;
        float LetterSpacing = 0.f;
        bool SyntheticBold = false, SyntheticItalic = false;
        float LineHeight = 0.f, Baseline = 0.f;
    };

    Item MakeItem(const LayoutContext& ctx, int style, uint32_t cp, const Item* previous)
    {
        Item item;
        item.Cp = cp;
        item.Style = style;
        item.Glyph = &UiFontLibrary::Glyph(ctx.Face, cp);
        item.Advance = item.Glyph->Advance * ctx.Scale + ctx.LetterSpacing;
        if (previous != nullptr && previous->Glyph != nullptr && previous->Style == style && previous->Glyph->FaceIndex == item.Glyph->FaceIndex)
            item.Pre = UiFontLibrary::Kerning(item.Glyph->FaceIndex, previous->Cp, cp) * ctx.Scale;
        return item;
    }

    float RangeWidth(const std::vector<Item>& items, size_t a, size_t b)
    {
        float w = 0.f;
        for (size_t i = a; i < b; ++i)
            w += items[i].Advance + (i > a ? items[i].Pre : 0.f);
        return w;
    }

    struct Line { std::vector<Item> Items; float Width = 0.f; };

    // Rich text (html="true"): <b> <i> <color=#rrggbb> <size=NN> <icon=action:NAME> <icon=img:PATH>
    struct StyleDef
    {
        bool Bold = false, Italic = false, HasColor = false;
        glm::vec4 Color = glm::vec4(1.f);
        float Size = 0.f;   // 0 = the label's size
        bool operator==(const StyleDef& o) const { return Bold == o.Bold && Italic == o.Italic && HasColor == o.HasColor && Color == o.Color && Size == o.Size; }
    };
    struct Atom { uint32_t Cp = 0; int Style = 0; int Icon = -1; };

    bool ParseColor(const std::string& text, glm::vec4& out)
    {
        if (text.size() != 7 && text.size() != 9)
            return false;
        if (text[0] != '#')
            return false;
        float v[4] = { 0, 0, 0, 255 };
        for (size_t i = 0; i < (text.size() - 1) / 2; ++i)
        {
            const std::string hex = text.substr(1 + i * 2, 2);
            char* endp = nullptr;
            const long n = std::strtol(hex.c_str(), &endp, 16);
            if (endp == hex.c_str() || *endp != 0)
                return false;
            v[i] = (float)n;
        }
        out = glm::vec4(v[0], v[1], v[2], v[3]) / 255.f;
        return true;
    }

    void WarnTag(const std::string& tag)
    {
        static std::set<std::string> seen;
        if (seen.insert(tag).second)
            Logger::Warning("[UI] rich text: unknown tag <%s> is drawn literally", tag.c_str());
    }

    void ParseRich(const std::vector<uint32_t>& cps, std::vector<Atom>& atoms, std::vector<StyleDef>& styles, std::vector<UiTextIcon>& icons)
    {
        int bold = 0, italic = 0;
        std::vector<glm::vec4> colors;
        std::vector<float> sizes;
        auto currentStyle = [&]() {
            StyleDef d;
            d.Bold = bold > 0;
            d.Italic = italic > 0;
            if (!colors.empty()) { d.HasColor = true; d.Color = colors.back(); }
            if (!sizes.empty()) d.Size = sizes.back();
            for (size_t i = 0; i < styles.size(); ++i)
                if (styles[i] == d)
                    return (int)i;
            styles.push_back(d);
            return (int)styles.size() - 1;
        };
        for (size_t i = 0; i < cps.size(); ++i)
        {
            if (cps[i] == '<')
            {
                size_t close = i + 1;
                while (close < cps.size() && cps[close] != '>' && cps[close] != '<')
                    ++close;
                if (close < cps.size() && cps[close] == '>')
                {
                    std::vector<uint32_t> inner(cps.begin() + i + 1, cps.begin() + close);
                    const std::string tag = UiEncodeUtf8(inner);
                    bool ok = true;
                    if (tag == "b") ++bold;
                    else if (tag == "/b") bold = std::max(0, bold - 1);
                    else if (tag == "i") ++italic;
                    else if (tag == "/i") italic = std::max(0, italic - 1);
                    else if (tag.rfind("color=", 0) == 0)
                    {
                        glm::vec4 c;
                        if (ParseColor(tag.substr(6), c)) colors.push_back(c); else ok = false;
                    }
                    else if (tag == "/color") { if (!colors.empty()) colors.pop_back(); }
                    else if (tag.rfind("size=", 0) == 0)
                    {
                        const float v = (float)std::atof(tag.c_str() + 5);
                        if (v > 0.f) sizes.push_back(v); else ok = false;
                    }
                    else if (tag == "/size") { if (!sizes.empty()) sizes.pop_back(); }
                    else if (tag.rfind("icon=action:", 0) == 0 || tag.rfind("icon=img:", 0) == 0)
                    {
                        const bool isAction = tag[5] == 'a';
                        UiTextIcon icon;
                        icon.IsAction = isAction;
                        icon.Ref = tag.substr(isAction ? 12 : 9);
                        // Options after the reference: <icon=action:jump,valign=middle>
                        const size_t comma = icon.Ref.find(',');
                        if (comma != std::string::npos)
                        {
                            const std::string opt = icon.Ref.substr(comma + 1);
                            icon.Ref.resize(comma);
                            if (opt == "valign=middle") icon.VAlign = 1;
                            else if (opt == "valign=top") icon.VAlign = 2;
                            else if (opt == "valign=bottom") icon.VAlign = 3;
                            else if (opt != "valign=baseline") ok = false;
                        }
                        if (icon.Ref.empty())
                            ok = false;
                        else
                        {
                            icons.push_back(icon);
                            Atom a;
                            a.Cp = 0xFFFC;
                            a.Style = currentStyle();
                            a.Icon = (int)icons.size() - 1;
                            atoms.push_back(a);
                        }
                    }
                    else ok = false;
                    if (ok)
                    {
                        i = close;
                        continue;
                    }
                    WarnTag(tag);
                }
            }
            Atom a;
            a.Cp = cps[i];
            a.Style = currentStyle();
            atoms.push_back(a);
        }
    }

    UiTextLayoutResult LayoutOnce(const UiTextParams& params, float fontSize)
    {
        UiTextLayoutResult result;
        result.UsedFontSize = fontSize;

        // 1-3. decode, rich tags, transform, white space
        std::vector<uint32_t> cps = UiDecodeUtf8(params.Text);
        std::vector<StyleDef> styles(1);
        std::vector<Atom> atoms;
        if (params.Html)
            ParseRich(cps, atoms, styles, result.Icons);
        else
            for (uint32_t c : cps)
                atoms.push_back(Atom{ c, 0, -1 });
        for (Atom& a : atoms)
        {
            if (a.Icon >= 0)
                continue;
            if (params.Transform == UiTextTransform::Uppercase) a.Cp = ToUpper(a.Cp);
            else if (params.Transform == UiTextTransform::Lowercase) a.Cp = ToLower(a.Cp);
        }
        std::vector<Atom> text;
        const bool pre = params.WhiteSpace == UiWhiteSpace::Pre;
        for (Atom a : atoms)
        {
            if (a.Cp == '\r' && a.Icon < 0)
                continue;
            if (a.Cp == '\t' && a.Icon < 0)
            {
                if (pre)
                {
                    Atom sp = a;
                    sp.Cp = ' ';
                    for (int i = 0; i < 4; ++i)
                        text.push_back(sp);
                    continue;
                }
                a.Cp = ' ';
            }
            if (!pre && a.Cp == ' ' && a.Icon < 0 && !text.empty() && text.back().Cp == ' ' && text.back().Icon < 0)
                continue;
            text.push_back(a);
        }

        // 4-5. faces, line height
        const float ratio = params.FontSize > 0.f ? fontSize / params.FontSize : 1.f;
        std::vector<LayoutContext> ctxs(styles.size());
        float lineHeight = 0.f, baseline = 0.f;
        for (size_t si = 0; si < styles.size(); ++si)
        {
            LayoutContext& c = ctxs[si];
            const int weight = styles[si].Bold ? std::max(700, params.Weight) : params.Weight;
            c.Face = UiFontLibrary::ResolveFace(params.Family, weight, params.Italic || styles[si].Italic, c.SyntheticBold, c.SyntheticItalic);
            c.Size = styles[si].Size > 0.f ? styles[si].Size * ratio : fontSize;
            c.Scale = c.Size / UiFontLibrary::kBaseSize;
            c.LetterSpacing = params.LetterSpacing;
            const UiFaceMetrics& metrics = UiFontLibrary::Metrics(c.Face);
            const float naturalHeight = (metrics.Ascent - metrics.Descent) * c.Scale;
            c.LineHeight = params.LineHeight >= 0.f ? params.LineHeight : (metrics.Ascent - metrics.Descent + metrics.LineGap) * c.Scale;
            c.Baseline = (c.LineHeight - naturalHeight) * 0.5f + metrics.Ascent * c.Scale;
            lineHeight = std::max(lineHeight, c.LineHeight);
            baseline = std::max(baseline, c.Baseline);
        }
        result.GlyphScale = ctxs[0].Scale;
        result.Dilation = ctxs[0].SyntheticBold ? 0.025f * ctxs[0].Size : 0.f;
        const LayoutContext& ctx = ctxs[0];

        auto makeItem = [&](const Atom& a, const Item* previous) {
            if (a.Icon >= 0)
            {
                // The icon is a square of 1.1 em; the advance adds 0.1 em.
                Item item;
                item.Cp = a.Cp;
                item.Style = a.Style;
                item.Icon = a.Icon;
                item.Advance = ctxs[a.Style].Size * 1.2f;
                return item;
            }
            return MakeItem(ctxs[a.Style], a.Style, a.Cp, previous);
        };

        // 6-7. advances and wrapping
        const bool wrap = params.WrapWidth >= 0.f && params.WhiteSpace == UiWhiteSpace::Normal;
        std::vector<Line> lines;
        size_t pos = 0;
        bool anyText = !text.empty();
        while (anyText && pos <= text.size())
        {
            size_t end = pos;
            while (end < text.size() && !(text[end].Cp == '\n' && text[end].Icon < 0))
                ++end;
            std::vector<Item> items;
            items.reserve(end - pos);
            for (size_t i = pos; i < end; ++i)
                items.push_back(makeItem(text[i], items.empty() ? nullptr : &items.back()));

            size_t start = 0;
            if (items.empty())
                lines.push_back(Line{});
            while (start < items.size())
            {
                size_t i = start, lastBreak = 0;
                bool haveBreak = false;
                float w = 0.f;
                bool overflow = false;
                for (; i < items.size(); ++i)
                {
                    const bool space = items[i].Cp == ' ' && items[i].Icon < 0;
                    const float add = items[i].Advance + (i > start ? items[i].Pre : 0.f);
                    if (space)
                    {
                        w += add;
                        lastBreak = i + 1;
                        haveBreak = true;
                        continue;
                    }
                    if (wrap && i > start && w + add > params.WrapWidth + 0.001f)
                    {
                        overflow = true;
                        break;
                    }
                    w += add;
                    // An icon is an unbreakable word: only the text around it offers break points.
                    if (items[i].Icon < 0 && (items[i].Cp == '-' || (i + 1 < items.size() && (IsCjk(items[i].Cp) || IsCjk(items[i + 1].Cp)))))
                    {
                        lastBreak = i + 1;
                        haveBreak = true;
                    }
                }
                size_t lineEnd, next;
                if (!overflow)
                {
                    lineEnd = items.size();
                    next = items.size();
                }
                else if (haveBreak && lastBreak > start)
                {
                    lineEnd = lastBreak;
                    next = lastBreak;
                }
                else
                {
                    lineEnd = i;
                    next = i;
                }
                size_t a = start, b = lineEnd;
                if (!pre)
                {
                    while (a < b && items[a].Cp == ' ' && items[a].Icon < 0) ++a;
                    while (b > a && items[b - 1].Cp == ' ' && items[b - 1].Icon < 0) --b;
                }
                Line line;
                line.Items.assign(items.begin() + a, items.begin() + b);
                line.Width = RangeWidth(line.Items, 0, line.Items.size());
                lines.push_back(std::move(line));
                // skip leading spaces of the next line
                start = next;
                if (!pre)
                    while (start < items.size() && items[start].Cp == ' ' && items[start].Icon < 0 && overflow)
                        ++start;
            }
            pos = end + 1;
            if (end >= text.size())
                break;
        }

        // 8. ellipsis on the last line
        if (params.Overflow == UiTextOverflow::Ellipsis && params.WrapWidth >= 0.f && !lines.empty())
        {
            Line& last = lines.back();
            if (last.Width > params.WrapWidth + 0.001f)
            {
                std::vector<Item> ellipsis;
                if (UiFontLibrary::HasCodepoint(ctx.Face, 0x2026))
                    ellipsis.push_back(MakeItem(ctx, 0, 0x2026, nullptr));
                else
                    for (int i = 0; i < 3; ++i)
                        ellipsis.push_back(MakeItem(ctx, 0, '.', ellipsis.empty() ? nullptr : &ellipsis.back()));
                const float ellW = RangeWidth(ellipsis, 0, ellipsis.size());
                while (!last.Items.empty() && RangeWidth(last.Items, 0, last.Items.size()) + ellW > params.WrapWidth)
                    last.Items.pop_back();
                for (const Item& e : ellipsis)
                    last.Items.push_back(e);
                last.Width = RangeWidth(last.Items, 0, last.Items.size());
            }
        }

        // 9-10. emit
        std::vector<UiTextIcon> sourceIcons;
        sourceIcons.swap(result.Icons);
        float widest = 0.f;
        for (const Line& line : lines)
            widest = std::max(widest, line.Width);
        const float boxWidth = params.WrapWidth >= 0.f ? params.WrapWidth : widest;
        bool wrapped = false;
        int runStyle = -1;
        for (size_t li = 0; li < lines.size(); ++li)
        {
            const Line& line = lines[li];
            float offset = 0.f;
            if (params.Align == UiTextAlign::Center) offset = (boxWidth - line.Width) * 0.5f;
            else if (params.Align == UiTextAlign::Right) offset = boxWidth - line.Width;
            const float top = (float)li * lineHeight;
            float pen = 0.f;
            std::vector<uint32_t> shown;
            for (size_t i = 0; i < line.Items.size(); ++i)
            {
                const Item& item = line.Items[i];
                if (i > 0)
                    pen += item.Pre;
                if (item.Icon >= 0)
                {
                    UiTextIcon icon = sourceIcons[item.Icon];
                    icon.H = icon.W = ctxs[item.Style].Size * 1.1f;
                    icon.X = offset + pen;
                    icon.Y = top + baseline - icon.H;
                    // valign is measured against the label's own size: a capital is ~0.7 em tall, a descender ~0.2 em
                    const float em = ctxs[0].Size;
                    if (icon.VAlign == 1) icon.Y = top + baseline - em * 0.35f - icon.H * 0.5f;
                    else if (icon.VAlign == 2) icon.Y = top + baseline - em * 0.7f;
                    else if (icon.VAlign == 3) icon.Y = top + baseline + em * 0.2f - icon.H;
                    result.Icons.push_back(icon);
                    pen += item.Advance;
                    continue;
                }
                const LayoutContext& ic = ctxs[item.Style];
                shown.push_back(item.Cp);
                if (item.Glyph->Valid)
                {
                    UiGlyphQuad q;
                    q.X = offset + pen + item.Glyph->OffsetX * ic.Scale;
                    q.Y = top + baseline + item.Glyph->OffsetY * ic.Scale;
                    q.W = item.Glyph->Width * ic.Scale;
                    q.H = item.Glyph->Height * ic.Scale;
                    q.U0 = item.Glyph->U0; q.V0 = item.Glyph->V0; q.U1 = item.Glyph->U1; q.V1 = item.Glyph->V1;
                    q.Page = item.Glyph->Page;
                    q.Italic = ic.SyntheticItalic;
                    if (runStyle != item.Style)
                    {
                        UiTextRun run;
                        run.First = result.Quads.size();
                        run.HasColor = styles[item.Style].HasColor;
                        run.Color = styles[item.Style].Color;
                        run.GlyphScale = ic.Scale;
                        run.Dilation = ic.SyntheticBold ? 0.025f * ic.Size : 0.f;
                        result.Runs.push_back(run);
                        runStyle = item.Style;
                    }
                    ++result.Runs.back().Count;
                    result.Quads.push_back(q);
                }
                pen += item.Advance;
            }
            if (li > 0)
                result.DisplayText += '\n';
            result.DisplayText += UiEncodeUtf8(shown);
        }
        wrapped = wrap && lines.size() > 1;
        result.Lines = (int)lines.size();
        result.Width = (params.WrapWidth >= 0.f && wrapped) ? std::max(params.WrapWidth, widest) : widest;
        result.Height = (float)lines.size() * lineHeight;
        return result;
    }
}

std::vector<uint32_t> UiDecodeUtf8(const std::string& s)
{
    std::vector<uint32_t> out;
    const size_t n = s.size();
    for (size_t i = 0; i < n;)
    {
        const unsigned char c = (unsigned char)s[i];
        int len = 0;
        uint32_t cp = 0;
        if (c < 0x80) { cp = c; len = 1; }
        else if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; len = 2; }
        else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; len = 3; }
        else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; len = 4; }
        bool valid = len > 0 && i + len <= n;
        for (int k = 1; valid && k < len; ++k)
        {
            const unsigned char cc = (unsigned char)s[i + k];
            if ((cc & 0xC0) != 0x80) { valid = false; break; }
            cp = (cp << 6) | (cc & 0x3F);
        }
        if (!valid)
        {
            out.push_back(0xFFFD);
            ++i;
        }
        else
        {
            out.push_back(cp);
            i += len;
        }
    }
    return out;
}

std::string UiEncodeUtf8(const std::vector<uint32_t>& cps)
{
    std::string out;
    for (uint32_t cp : cps)
    {
        if (cp < 0x80) out += (char)cp;
        else if (cp < 0x800) { out += (char)(0xC0 | (cp >> 6)); out += (char)(0x80 | (cp & 0x3F)); }
        else if (cp < 0x10000) { out += (char)(0xE0 | (cp >> 12)); out += (char)(0x80 | ((cp >> 6) & 0x3F)); out += (char)(0x80 | (cp & 0x3F)); }
        else { out += (char)(0xF0 | (cp >> 18)); out += (char)(0x80 | ((cp >> 12) & 0x3F)); out += (char)(0x80 | ((cp >> 6) & 0x3F)); out += (char)(0x80 | (cp & 0x3F)); }
    }
    return out;
}

UiTextLayoutResult UiLayoutText(const UiTextParams& params)
{
    UiTextLayoutResult result = LayoutOnce(params, params.FontSize);
    if (params.Overflow != UiTextOverflow::Shrink)
        return result;

    const bool wraps = params.WhiteSpace == UiWhiteSpace::Normal && params.WrapWidth >= 0.f;
    auto fits = [&](const UiTextLayoutResult& r) {
        if (params.WrapWidth >= 0.f && !wraps && r.Width > params.WrapWidth + 0.001f)
            return false;
        if (params.MaxHeight >= 0.f && r.Height > params.MaxHeight + 0.001f)
            return false;
        return true;
    };
    if (fits(result))
        return result;
    for (int n = 1; n <= 14; ++n)
    {
        result = LayoutOnce(params, params.FontSize * std::pow(0.95f, (float)n));
        if (fits(result))
            break;
    }
    return result;
}

#include "UiCssParser.h"

#include <FileSystem/FileSystem.h>
#include <UI/Markup/UiMarkupLog.h>
#include <UI/Render/UiFontLibrary.h>

#include <algorithm>
#include <map>

namespace
{
    struct Parser
    {
        std::vector<UiCssToken> Tokens;
        size_t Pos = 0;
        std::string Path;
        int LineOffset = 0;

        const UiCssToken& Cur() const { return Tokens[std::min(Pos, Tokens.size() - 1)]; }
        bool AtEnd() const { return Cur().Type == UiCssTok::Eof; }
        int Line(const UiCssToken& t) const { return t.Line + LineOffset; }

        void SkipWs()
        {
            while (Cur().Type == UiCssTok::Whitespace)
                ++Pos;
        }

        // Skips a balanced {...} block; Pos is on '{'. Runs to the end when unclosed.
        void SkipBlock()
        {
            int depth = 0;
            while (!AtEnd())
            {
                if (Cur().Type == UiCssTok::LBrace) ++depth;
                else if (Cur().Type == UiCssTok::RBrace && --depth == 0) { ++Pos; return; }
                ++Pos;
            }
        }
    };

    std::vector<UiCssToken> Trim(std::vector<UiCssToken> v)
    {
        while (!v.empty() && v.back().Type == UiCssTok::Whitespace) v.pop_back();
        size_t a = 0;
        while (a < v.size() && v[a].Type == UiCssTok::Whitespace) ++a;
        v.erase(v.begin(), v.begin() + a);
        return v;
    }

    // Reads declarations up to '}' (consumed) or Eof/terminator. `stopAtBrace` false = a bare style="" list.
    std::vector<UiCssDeclaration> ParseBlock(Parser& p, bool stopAtBrace)
    {
        std::vector<UiCssDeclaration> out;
        while (!p.AtEnd())
        {
            p.SkipWs();
            if (p.Cur().Type == UiCssTok::RBrace && stopAtBrace) { ++p.Pos; return out; }
            if (p.Cur().Type == UiCssTok::Semicolon) { ++p.Pos; continue; }
            if (p.AtEnd()) break;

            // one declaration: name ':' value up to ';' or '}' at depth 0
            const UiCssToken start = p.Cur();
            std::vector<UiCssToken> raw;
            int depth = 0;
            while (!p.AtEnd())
            {
                const UiCssTok type = p.Cur().Type;
                if (type == UiCssTok::LParen || type == UiCssTok::LBracket || type == UiCssTok::Function) ++depth;
                else if (type == UiCssTok::RParen || type == UiCssTok::RBracket) depth = std::max(0, depth - 1);
                if (depth == 0 && (type == UiCssTok::Semicolon || (type == UiCssTok::RBrace && stopAtBrace)))
                    break;
                raw.push_back(p.Cur());
                ++p.Pos;
            }
            raw = Trim(std::move(raw));
            if (raw.empty())
                continue;
            if (raw[0].Type != UiCssTok::Ident)
            {
                UiMarkupLog::Error(p.Path, p.Line(start), start.Col, "expected a property name");
                continue;
            }
            size_t i = 1;
            while (i < raw.size() && raw[i].Type == UiCssTok::Whitespace) ++i;
            if (i >= raw.size() || raw[i].Type != UiCssTok::Colon)
            {
                UiMarkupLog::Error(p.Path, p.Line(start), start.Col, "expected ':' after property name '" + raw[0].Text + "'");
                continue;
            }
            UiCssDeclaration decl;
            decl.Name = raw[0].Text;
            if (decl.Name.rfind("--", 0) != 0)
                for (char& c : decl.Name)
                    if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
            decl.Line = p.Line(raw[0]);
            decl.Col = raw[0].Col;
            decl.Value.assign(raw.begin() + i + 1, raw.end());
            decl.Value = Trim(std::move(decl.Value));
            // !important
            for (size_t k = 0; k + 1 < decl.Value.size(); ++k)
            {
                if (decl.Value[k].Type == UiCssTok::Delim && decl.Value[k].Text == "!" &&
                    decl.Value[k + 1].Type == UiCssTok::Ident && decl.Value[k + 1].Text == "important")
                {
                    UiMarkupLog::Warning(p.Path, decl.Line, decl.Col, "!important is not supported");
                    decl.Value.erase(decl.Value.begin() + k, decl.Value.end());
                    decl.Value = Trim(std::move(decl.Value));
                    break;
                }
            }
            if (decl.Value.empty())
                continue;
            out.push_back(std::move(decl));
        }
        return out;
    }

    int WeightFromValue(const std::vector<UiCssToken>& v)
    {
        if (v.empty()) return 400;
        if (v[0].Type == UiCssTok::Ident)
            return v[0].Text == "bold" ? 700 : 400;
        if (v[0].Type == UiCssTok::Number)
            return (int)v[0].Number;
        return 400;
    }

    std::map<std::string, std::shared_ptr<UiStyleSheet>>& Cache()
    {
        static std::map<std::string, std::shared_ptr<UiStyleSheet>> cache;
        return cache;
    }
    std::map<std::string, uint32_t>& ModTimes()
    {
        static std::map<std::string, uint32_t> times;
        return times;
    }
}

std::shared_ptr<UiStyleSheet> UiParseStyleSheet(const std::string& source, const std::string& path, int lineOffset)
{
    auto sheet = std::make_shared<UiStyleSheet>();
    sheet->Path = path;
    Parser p;
    p.Tokens = UiCssTokenize(source);
    p.Path = path;
    p.LineOffset = lineOffset;
    int order = 0;
    bool seenRule = false;

    while (!p.AtEnd())
    {
        p.SkipWs();
        if (p.AtEnd()) break;
        const UiCssToken head = p.Cur();

        if (head.Type == UiCssTok::AtKeyword)
        {
            ++p.Pos;
            if (head.Text == "import")
            {
                p.SkipWs();
                std::string target;
                if (p.Cur().Type == UiCssTok::Url || p.Cur().Type == UiCssTok::String)
                    target = p.Cur().Text;
                while (!p.AtEnd() && p.Cur().Type != UiCssTok::Semicolon) ++p.Pos;
                if (!p.AtEnd()) ++p.Pos;
                if (target.empty())
                    UiMarkupLog::Error(path, p.Line(head), head.Col, "@import needs a url");
                else if (seenRule)
                    UiMarkupLog::Warning(path, p.Line(head), head.Col, "@import must come before any rule; ignored");
                else
                    sheet->Imports.push_back(target);
            }
            else if (head.Text == "keyframes")
            {
                p.SkipWs();
                UiKeyframes frames;
                frames.Path = path;
                frames.Line = p.Line(head);
                if (p.Cur().Type == UiCssTok::Ident || p.Cur().Type == UiCssTok::String)
                {
                    frames.Name = p.Cur().Text;
                    ++p.Pos;
                }
                while (!p.AtEnd() && p.Cur().Type != UiCssTok::LBrace) ++p.Pos;
                if (p.AtEnd()) break;
                ++p.Pos;   // '{'
                while (!p.AtEnd())
                {
                    p.SkipWs();
                    if (p.Cur().Type == UiCssTok::RBrace) { ++p.Pos; break; }
                    std::vector<UiCssToken> prelude;
                    while (!p.AtEnd() && p.Cur().Type != UiCssTok::LBrace && p.Cur().Type != UiCssTok::RBrace)
                    {
                        prelude.push_back(p.Cur());
                        ++p.Pos;
                    }
                    if (p.Cur().Type != UiCssTok::LBrace)
                        continue;
                    const int selectorLine = prelude.empty() ? p.Line(head) : p.Line(prelude.front());
                    const int selectorCol = prelude.empty() ? head.Col : prelude.front().Col;
                    ++p.Pos;
                    std::vector<UiCssDeclaration> decls = ParseBlock(p, true);
                    // "from", "to" and percentages, comma separated
                    std::vector<float> offsets;
                    bool ok = true;
                    for (const UiCssToken& t : prelude)
                    {
                        if (t.Type == UiCssTok::Whitespace || t.Type == UiCssTok::Comma) continue;
                        if (t.Type == UiCssTok::Ident && t.Text == "from") offsets.push_back(0.f);
                        else if (t.Type == UiCssTok::Ident && t.Text == "to") offsets.push_back(1.f);
                        else if (t.Type == UiCssTok::Percentage && t.Number >= 0.0 && t.Number <= 100.0) offsets.push_back(static_cast<float>(t.Number) * 0.01f);
                        else ok = false;
                    }
                    if (!ok || offsets.empty())
                    {
                        UiMarkupLog::Error(path, selectorLine, selectorCol, "invalid keyframe selector (use from, to or a percentage)");
                        continue;
                    }
                    for (float offset : offsets)
                        frames.Stops.push_back({ offset, decls });
                }
                std::stable_sort(frames.Stops.begin(), frames.Stops.end(), [](const UiKeyframeStop& a, const UiKeyframeStop& b) { return a.Offset < b.Offset; });
                if (frames.Name.empty())
                    UiMarkupLog::Error(path, frames.Line, head.Col, "@keyframes needs a name");
                else if (frames.Stops.empty())
                    UiMarkupLog::Warning(path, frames.Line, head.Col, "@keyframes " + frames.Name + " has no keyframes");
                else
                    sheet->Keyframes.push_back(std::move(frames));
                seenRule = true;
            }
            else if (head.Text == "font-face")
            {
                while (!p.AtEnd() && p.Cur().Type != UiCssTok::LBrace) ++p.Pos;
                if (p.AtEnd()) break;
                ++p.Pos;
                UiFontFaceRule face;
                face.Line = p.Line(head);
                for (const UiCssDeclaration& d : ParseBlock(p, true))
                {
                    if (d.Name == "font-family" && !d.Value.empty()) face.Family = d.Value[0].Text;
                    else if (d.Name == "src" && !d.Value.empty() && d.Value[0].Type == UiCssTok::Url) face.Src = d.Value[0].Text;
                    else if (d.Name == "font-weight") face.Weight = WeightFromValue(d.Value);
                    else if (d.Name == "font-style") face.Italic = !d.Value.empty() && d.Value[0].Text == "italic";
                }
                if (face.Family.empty() || face.Src.empty())
                    UiMarkupLog::Error(path, face.Line, head.Col, "@font-face needs font-family and src: url(...)");
                else
                    sheet->FontFaces.push_back(face);
                seenRule = true;
            }
            else
            {
                UiMarkupLog::Warning(path, p.Line(head), head.Col, "unsupported at-rule '@" + head.Text + "'");
                while (!p.AtEnd() && p.Cur().Type != UiCssTok::Semicolon && p.Cur().Type != UiCssTok::LBrace) ++p.Pos;
                if (p.Cur().Type == UiCssTok::LBrace) p.SkipBlock();
                else if (!p.AtEnd()) ++p.Pos;
            }
            continue;
        }

        // qualified rule: prelude up to '{'
        std::vector<UiCssToken> prelude;
        while (!p.AtEnd() && p.Cur().Type != UiCssTok::LBrace)
        {
            prelude.push_back(p.Cur());
            ++p.Pos;
        }
        if (p.AtEnd())
        {
            UiMarkupLog::Error(path, p.Line(head), head.Col, "expected '{' after selector");
            break;
        }
        ++p.Pos;   // '{'
        // line numbers inside the prelude need the offset for messages
        for (UiCssToken& t : prelude) t.Line += lineOffset;

        UiCssRule rule;
        const bool selectorsOk = UiParseSelectorList(prelude, path, rule.Selectors);
        std::vector<UiCssDeclaration> decls = ParseBlock(p, true);
        seenRule = true;
        if (!selectorsOk)
            continue;
        if (rule.Selectors.empty())
            continue;
        rule.Declarations = std::move(decls);
        for (const UiCssDeclaration& d : rule.Declarations)
            if (d.Name == "pulse-duration")
                rule.SetsPulseDuration = true;
        rule.SourceOrder = order++;
        sheet->Rules.push_back(std::move(rule));
    }

    for (const UiFontFaceRule& face : sheet->FontFaces)
        UiFontLibrary::RegisterFace(face.Family, face.Weight, face.Italic, face.Src);
    return sheet;
}

std::vector<UiCssDeclaration> UiParseDeclarationList(const std::string& source, const std::string& path, int line, int col)
{
    Parser p;
    p.Tokens = UiCssTokenize(source);
    p.Path = path;
    p.LineOffset = line - 1;
    (void)col;
    return ParseBlock(p, false);
}

std::shared_ptr<UiStyleSheet> UiStyleSheetCache::Get(const std::string& path)
{
    auto& cache = Cache();
    const auto found = cache.find(path);
    if (found != cache.end())
        return found->second;
    const std::string text = FileSystemEngine::ReadFile(path);
    if (text.empty())
    {
        UiMarkupLog::Error(path, 0, 0, "cannot read style sheet");
        return nullptr;
    }
    auto sheet = UiParseStyleSheet(text, path, 0);
    cache[path] = sheet;
    ModTimes()[path] = FileSystemEngine::GetFileModificationTime(path);
    return sheet;
}

std::vector<std::shared_ptr<UiStyleSheet>> UiStyleSheetCache::Cached()
{
    std::vector<std::shared_ptr<UiStyleSheet>> out;
    for (const auto& [path, sheet] : Cache())
        out.push_back(sheet);
    return out;
}

void UiStyleSheetCache::Invalidate(const std::string& path)
{
    Cache().erase(path);
    ModTimes().erase(path);
}

uint32_t UiStyleSheetCache::ModTime(const std::string& path)
{
    return FileSystemEngine::GetFileModificationTime(path);
}

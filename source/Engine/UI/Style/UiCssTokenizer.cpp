#include "UiCssTokenizer.h"

#include <cstdio>
#include <cstdlib>

namespace
{
    bool IsDigit(char c) { return c >= '0' && c <= '9'; }
    bool IsAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
    bool IsNonAscii(char c) { return (unsigned char)c >= 0x80; }
    bool IsIdentStart(char c) { return IsAlpha(c) || c == '_' || IsNonAscii(c); }
    bool IsNameChar(char c) { return IsIdentStart(c) || IsDigit(c) || c == '-'; }
    bool IsSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f'; }

    std::string Lower(std::string s)
    {
        for (char& c : s)
            if (c >= 'A' && c <= 'Z')
                c = (char)(c - 'A' + 'a');
        return s;
    }

    struct Lexer
    {
        const std::string& s;
        size_t i = 0;
        int line = 1, col = 1;

        char Peek(size_t o = 0) const { return i + o < s.size() ? s[i + o] : '\0'; }
        bool End() const { return i >= s.size(); }
        char Next()
        {
            const char c = s[i++];
            if (c == '\n') { ++line; col = 1; }
            else ++col;
            return c;
        }

        bool IdentAhead(size_t o = 0) const
        {
            const char c = Peek(o);
            if (IsIdentStart(c)) return true;
            if (c == '-')
            {
                const char d = Peek(o + 1);
                return IsIdentStart(d) || d == '-';
            }
            return false;
        }

        std::string ReadName()
        {
            std::string out;
            while (!End() && IsNameChar(Peek()))
                out += Next();
            return out;
        }
    };
}

std::vector<UiCssToken> UiCssTokenize(const std::string& source)
{
    std::vector<UiCssToken> out;
    Lexer lx{ source };
    while (!lx.End())
    {
        UiCssToken tok;
        tok.Line = lx.line;
        tok.Col = lx.col;
        const char c = lx.Peek();

        if (c == '/' && lx.Peek(1) == '*')
        {
            lx.Next(); lx.Next();
            while (!lx.End() && !(lx.Peek() == '*' && lx.Peek(1) == '/'))
                lx.Next();
            if (!lx.End()) { lx.Next(); lx.Next(); }
            continue;
        }
        if (IsSpace(c))
        {
            while (!lx.End() && IsSpace(lx.Peek()))
                lx.Next();
            tok.Type = UiCssTok::Whitespace;
            out.push_back(tok);
            continue;
        }
        if (c == '"' || c == '\'')
        {
            const char quote = lx.Next();
            tok.Type = UiCssTok::String;
            while (true)
            {
                if (lx.End()) break;
                const char d = lx.Peek();
                if (d == quote) { lx.Next(); break; }
                if (d == '\n') { tok.Type = UiCssTok::BadString; break; }
                if (d == '\\')
                {
                    lx.Next();
                    if (!lx.End()) tok.Text += lx.Next();
                    continue;
                }
                tok.Text += lx.Next();
            }
            out.push_back(tok);
            continue;
        }
        const bool numberStart = IsDigit(c) || (c == '.' && IsDigit(lx.Peek(1))) ||
                                 ((c == '+' || c == '-') && (IsDigit(lx.Peek(1)) || (lx.Peek(1) == '.' && IsDigit(lx.Peek(2)))));
        if (numberStart)
        {
            std::string num;
            bool integer = true;
            if (c == '+' || c == '-') num += lx.Next();
            while (IsDigit(lx.Peek())) num += lx.Next();
            if (lx.Peek() == '.' && IsDigit(lx.Peek(1)))
            {
                integer = false;
                num += lx.Next();
                while (IsDigit(lx.Peek())) num += lx.Next();
            }
            if ((lx.Peek() == 'e' || lx.Peek() == 'E') &&
                (IsDigit(lx.Peek(1)) || ((lx.Peek(1) == '+' || lx.Peek(1) == '-') && IsDigit(lx.Peek(2)))))
            {
                integer = false;
                num += lx.Next();
                if (lx.Peek() == '+' || lx.Peek() == '-') num += lx.Next();
                while (IsDigit(lx.Peek())) num += lx.Next();
            }
            tok.Number = std::strtod(num.c_str(), nullptr);
            tok.IsInteger = integer;
            if (lx.Peek() == '%')
            {
                lx.Next();
                tok.Type = UiCssTok::Percentage;
            }
            else if (lx.IdentAhead())
            {
                tok.Type = UiCssTok::Dimension;
                tok.Text = Lower(lx.ReadName());
            }
            else
                tok.Type = UiCssTok::Number;
            out.push_back(tok);
            continue;
        }
        if (lx.IdentAhead())
        {
            std::string name = lx.ReadName();
            if (lx.Peek() == '(')
            {
                lx.Next();
                if (Lower(name) == "url")
                {
                    tok.Type = UiCssTok::Url;
                    while (!lx.End() && IsSpace(lx.Peek())) lx.Next();
                    std::string body;
                    while (!lx.End() && lx.Peek() != ')')
                        body += lx.Next();
                    if (!lx.End()) lx.Next();
                    // trim and strip quotes
                    size_t a = body.find_first_not_of(" \t\r\n\f");
                    size_t b = body.find_last_not_of(" \t\r\n\f");
                    body = a == std::string::npos ? "" : body.substr(a, b - a + 1);
                    if (body.size() >= 2 && (body.front() == '"' || body.front() == '\'') && body.back() == body.front())
                        body = body.substr(1, body.size() - 2);
                    tok.Text = body;
                }
                else
                {
                    tok.Type = UiCssTok::Function;
                    tok.Text = name;
                }
            }
            else
            {
                tok.Type = UiCssTok::Ident;
                tok.Text = name;
            }
            out.push_back(tok);
            continue;
        }
        if (c == '#' && IsNameChar(lx.Peek(1)))
        {
            lx.Next();
            tok.Type = UiCssTok::Hash;
            tok.Text = lx.ReadName();
            out.push_back(tok);
            continue;
        }
        if (c == '@' && IsIdentStart(lx.Peek(1)))
        {
            lx.Next();
            tok.Type = UiCssTok::AtKeyword;
            tok.Text = lx.ReadName();
            out.push_back(tok);
            continue;
        }
        lx.Next();
        switch (c)
        {
        case ':': tok.Type = UiCssTok::Colon; break;
        case ';': tok.Type = UiCssTok::Semicolon; break;
        case ',': tok.Type = UiCssTok::Comma; break;
        case '{': tok.Type = UiCssTok::LBrace; break;
        case '}': tok.Type = UiCssTok::RBrace; break;
        case '(': tok.Type = UiCssTok::LParen; break;
        case ')': tok.Type = UiCssTok::RParen; break;
        case '[': tok.Type = UiCssTok::LBracket; break;
        case ']': tok.Type = UiCssTok::RBracket; break;
        default: tok.Type = UiCssTok::Delim; tok.Text = std::string(1, c); break;
        }
        out.push_back(tok);
    }
    UiCssToken eof;
    eof.Type = UiCssTok::Eof;
    eof.Line = lx.line;
    eof.Col = lx.col;
    out.push_back(eof);
    return out;
}

std::string UiCssTokensToString(const std::vector<UiCssToken>& tokens)
{
    std::string out;
    char buf[64];
    for (const UiCssToken& t : tokens)
    {
        switch (t.Type)
        {
        case UiCssTok::Ident: out += t.Text; break;
        case UiCssTok::Function: out += t.Text + "("; break;
        case UiCssTok::AtKeyword: out += "@" + t.Text; break;
        case UiCssTok::Hash: out += "#" + t.Text; break;
        case UiCssTok::String: out += "\"" + t.Text + "\""; break;
        case UiCssTok::BadString: out += "\"" + t.Text; break;
        case UiCssTok::Number: std::snprintf(buf, sizeof(buf), "%g", t.Number); out += buf; break;
        case UiCssTok::Percentage: std::snprintf(buf, sizeof(buf), "%g%%", t.Number); out += buf; break;
        case UiCssTok::Dimension: std::snprintf(buf, sizeof(buf), "%g", t.Number); out += buf + t.Text; break;
        case UiCssTok::Whitespace: out += ' '; break;
        case UiCssTok::Colon: out += ':'; break;
        case UiCssTok::Semicolon: out += ';'; break;
        case UiCssTok::Comma: out += ','; break;
        case UiCssTok::LBrace: out += '{'; break;
        case UiCssTok::RBrace: out += '}'; break;
        case UiCssTok::LParen: out += '('; break;
        case UiCssTok::RParen: out += ')'; break;
        case UiCssTok::LBracket: out += '['; break;
        case UiCssTok::RBracket: out += ']'; break;
        case UiCssTok::Delim: out += t.Text; break;
        case UiCssTok::Url: out += "url(" + t.Text + ")"; break;
        case UiCssTok::Eof: break;
        }
    }
    return out;
}

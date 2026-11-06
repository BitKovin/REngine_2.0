#pragma once
#include <string>
#include <vector>

enum class UiCssTok { Ident, Function, AtKeyword, Hash, String, Number, Percentage, Dimension,
                      Whitespace, Colon, Semicolon, Comma, LBrace, RBrace, LParen, RParen, LBracket, RBracket,
                      Delim, Url, BadString, Eof };

struct UiCssToken
{
    UiCssTok Type = UiCssTok::Eof;
    std::string Text;     // ident/function name (without '('), at-keyword (without '@'), hash (without '#'), string contents, url contents, unit for Dimension, delim char
    double Number = 0;    // Number/Percentage/Dimension
    bool IsInteger = false;
    int Line = 1, Col = 1;
};

// Subset of CSS Syntax Level 3. Always ends with an Eof token.
std::vector<UiCssToken> UiCssTokenize(const std::string& source);

// Source text of tokens joined back together (for error messages and var() substitution).
std::string UiCssTokensToString(const std::vector<UiCssToken>& tokens);

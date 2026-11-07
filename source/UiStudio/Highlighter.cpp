#include "Highlighter.h"

namespace
{
    QTextCharFormat Fmt(const char* color, bool bold = false, bool italic = false)
    {
        QTextCharFormat f;
        f.setForeground(QColor(color));
        if (bold) f.setFontWeight(QFont::DemiBold);
        f.setFontItalic(italic);
        return f;
    }
}

Highlighter::Highlighter(QTextDocument* doc, Language language)
    : QSyntaxHighlighter(doc), m_language(language)
{
    m_comment = Fmt("#6a7388", false, true);
    if (language == Language::Xml)
    {
        m_rules.append({ QRegularExpression(R"(</?[A-Za-z_][\w\-.:]*)"), Fmt("#e0788a", true) });
        m_rules.append({ QRegularExpression(R"(/?>)"), Fmt("#e0788a") });
        m_rules.append({ QRegularExpression(R"(\b[A-Za-z_][\w\-.:]*(?==))"), Fmt("#e6c07b") });
        m_rules.append({ QRegularExpression(R"("[^"]*"|'[^']*')"), Fmt("#98c379") });
        m_rules.append({ QRegularExpression(R"(\$\{[^}]*\}|\{[^}]*\})"), Fmt("#56b6c2", true) });
    }
    else
    {
        m_rules.append({ QRegularExpression(R"([.#]?[A-Za-z_][\w\-]*(?=[^{}]*\{))"), Fmt("#e0788a") });
        m_rules.append({ QRegularExpression(R"(:[A-Za-z\-]+)"), Fmt("#c678dd") });
        m_rules.append({ QRegularExpression(R"(--[\w\-]+|\b[a-z\-]+(?=\s*:))"), Fmt("#e6c07b") });
        m_rules.append({ QRegularExpression(R"(#[0-9a-fA-F]{3,8}\b)"), Fmt("#d19a66") });
        m_rules.append({ QRegularExpression(R"(\b-?\d+(\.\d+)?(px|%|s|ms|deg|em)?\b)"), Fmt("#d19a66") });
        m_rules.append({ QRegularExpression(R"("[^"]*"|'[^']*')"), Fmt("#98c379") });
        m_rules.append({ QRegularExpression(R"(\b(var|rgb|rgba|linear-gradient|radial-gradient|url|calc)\()"), Fmt("#61afef") });
    }
}

void Highlighter::highlightBlock(const QString& text)
{
    for (const Rule& rule : m_rules)
    {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext())
        {
            const auto m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), rule.format);
        }
    }
    if (m_language == Language::Xml)
        HighlightXml(text);
    else
        HighlightCss(text);
}

// Comments span lines, so they use block state: 1 = inside a comment at the end of the previous block.
void Highlighter::HighlightXml(const QString& text)
{
    setCurrentBlockState(0);
    int start = previousBlockState() == 1 ? 0 : text.indexOf("<!--");
    while (start >= 0)
    {
        const int end = text.indexOf("-->", start);
        int length;
        if (end < 0) { setCurrentBlockState(1); length = text.length() - start; }
        else length = end - start + 3;
        setFormat(start, length, m_comment);
        start = text.indexOf("<!--", start + length);
    }
}

void Highlighter::HighlightCss(const QString& text)
{
    setCurrentBlockState(0);
    int start = previousBlockState() == 1 ? 0 : text.indexOf("/*");
    while (start >= 0)
    {
        const int end = text.indexOf("*/", start);
        int length;
        if (end < 0) { setCurrentBlockState(1); length = text.length() - start; }
        else length = end - start + 2;
        setFormat(start, length, m_comment);
        start = text.indexOf("/*", start + length);
    }
}

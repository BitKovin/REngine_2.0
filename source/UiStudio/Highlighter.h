#pragma once

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

// Highlights markup (.xml) or style (.css) text. The language is picked from the file name.
class Highlighter : public QSyntaxHighlighter
{
public:
    enum class Language { Xml, Css };

    Highlighter(QTextDocument* doc, Language language);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct Rule { QRegularExpression pattern; QTextCharFormat format; };

    void HighlightXml(const QString& text);
    void HighlightCss(const QString& text);

    Language m_language;
    QVector<Rule> m_rules;
    QTextCharFormat m_comment;
};

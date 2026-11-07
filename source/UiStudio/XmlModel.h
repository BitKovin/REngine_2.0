#pragma once

#include <QString>
#include <QVector>
#include <memory>
#include <vector>

// A read-only view of a markup file that remembers where every element and attribute sits in the text.
// Visual edits are returned as TextEdit splices, so everything the user did not touch (comments, spacing,
// attribute order, other lines) stays exactly as written.
struct TextEdit
{
    int pos = 0;
    int removeLen = 0;
    QString insert;
};

struct XmlAttr
{
    QString name;
    QString value;
    int start = 0;        // first character of the name
    int end = 0;          // one past the closing quote
    int valueStart = 0;   // first character inside the quotes
    int valueEnd = 0;     // one past the last character inside the quotes
};

struct XmlNode
{
    QString tag;
    int start = 0;          // the '<'
    int nameEnd = 0;        // one past the tag name
    int openEnd = 0;        // one past the '>' of the opening tag
    int closeStart = -1;    // the '<' of the closing tag, -1 when self closing
    int end = 0;            // one past the whole element
    int selfCloseAt = -1;   // position of the '/' in "/>", -1 otherwise
    int line = 1;           // 1-based
    int column = 1;         // 1-based
    QVector<XmlAttr> attrs;
    XmlNode* parent = nullptr;
    std::vector<std::unique_ptr<XmlNode>> children;

    const XmlAttr* Attr(const QString& name) const;
    bool SelfClosing() const { return selfCloseAt >= 0; }
    int IndexInParent() const;
    bool IsDescendantOf(const XmlNode* other) const;
};

class XmlModel
{
public:
    // Fails (with a message) when the text is not well formed; the caller then falls back to text editing.
    bool Parse(const QString& text, QString* error);
    const QString& Text() const { return m_text; }
    XmlNode* Root() { return m_root.get(); }

    // The element whose opening '<' is on this 1-based line, nearest to the column when several share a line.
    XmlNode* FindByPosition(int line, int column);

    // --- edits (each returns splices against Text(); apply them from the highest position down) ---
    QVector<TextEdit> SetAttribute(const XmlNode* node, const QString& name, const QString& value) const;
    QVector<TextEdit> RemoveAttribute(const XmlNode* node, const QString& name) const;
    QVector<TextEdit> DeleteNode(const XmlNode* node) const;
    // Puts `snippet` (one or more lines, indented relative to its own first line) into `parent` before `before`,
    // or at the end when `before` is null.
    QVector<TextEdit> InsertChild(const XmlNode* parent, const XmlNode* before, const QString& snippet) const;
    QVector<TextEdit> Duplicate(const XmlNode* node) const;
    QVector<TextEdit> Move(const XmlNode* node, const XmlNode* newParent, const XmlNode* before) const;
    QVector<TextEdit> MoveUpDown(const XmlNode* node, bool up) const;

    QString NodeText(const XmlNode* node) const { return m_text.mid(node->start, node->end - node->start); }
    QString IndentOf(const XmlNode* node) const;

    static QString Escape(const QString& value);
    static QString Unescape(const QString& value);
    static QString DefaultSnippet(const QString& tag, bool hasText);

private:
    int LineStart(int pos) const;
    int LineEnd(int pos) const;                  // position of the '\n' (or text end)
    bool OnlyWhitespaceBefore(int pos) const;    // on its own line up to this position
    bool OnlyWhitespaceAfter(int pos) const;     // up to the end of the line
    QString ChildIndent(const XmlNode* parent) const;
    static QString Reindent(const QString& fragment, const QString& from, const QString& to);
    QVector<int> m_lineStarts;

    QString m_text;
    std::unique_ptr<XmlNode> m_root;
    QVector<XmlNode*> m_all;
};

#include "XmlModel.h"

#include <QRegularExpression>
#include <algorithm>

const XmlAttr* XmlNode::Attr(const QString& name) const
{
    for (const XmlAttr& a : attrs)
        if (a.name == name)
            return &a;
    return nullptr;
}

int XmlNode::IndexInParent() const
{
    if (!parent)
        return -1;
    for (int i = 0; i < int(parent->children.size()); ++i)
        if (parent->children[i].get() == this)
            return i;
    return -1;
}

bool XmlNode::IsDescendantOf(const XmlNode* other) const
{
    for (const XmlNode* n = parent; n; n = n->parent)
        if (n == other)
            return true;
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
bool XmlModel::Parse(const QString& text, QString* error)
{
    m_text = text;
    m_all.clear();
    m_lineStarts.clear();
    m_lineStarts.push_back(0);
    for (int i = 0; i < text.size(); ++i)
        if (text[i] == '\n')
            m_lineStarts.push_back(i + 1);

    m_root = std::make_unique<XmlNode>();
    m_root->end = text.size();
    XmlNode* current = m_root.get();
    const int n = text.size();
    auto fail = [&](const QString& why, int pos) {
        if (error)
        {
            const int line = int(std::upper_bound(m_lineStarts.begin(), m_lineStarts.end(), pos) - m_lineStarts.begin());
            *error = QString("line %1: %2").arg(line).arg(why);
        }
        return false;
    };
    auto isNameChar = [](QChar c) { return c.isLetterOrNumber() || c == '_' || c == '-' || c == '.' || c == ':'; };

    int i = 0;
    while (i < n)
    {
        if (text[i] != '<')
        {
            ++i;
            continue;
        }
        if (text.mid(i, 4) == "<!--")
        {
            const int e = text.indexOf("-->", i + 4);
            if (e < 0) return fail("comment is not closed", i);
            i = e + 3;
            continue;
        }
        if (text.mid(i, 9) == "<![CDATA[")
        {
            const int e = text.indexOf("]]>", i);
            if (e < 0) return fail("CDATA is not closed", i);
            i = e + 3;
            continue;
        }
        if (i + 1 < n && (text[i + 1] == '?' || text[i + 1] == '!'))
        {
            const int e = text.indexOf('>', i);
            if (e < 0) return fail("unfinished declaration", i);
            i = e + 1;
            continue;
        }
        if (i + 1 < n && text[i + 1] == '/')
        {
            const int e = text.indexOf('>', i);
            if (e < 0) return fail("closing tag is not finished", i);
            const QString name = text.mid(i + 2, e - i - 2).trimmed();
            if (current == m_root.get() || current->tag != name)
                return fail(QString("unexpected </%1>").arg(name), i);
            current->closeStart = i;
            current->end = e + 1;
            current = current->parent;
            i = e + 1;
            continue;
        }

        // opening tag
        auto node = std::make_unique<XmlNode>();
        XmlNode* ptr = node.get();
        ptr->start = i;
        ptr->parent = current;
        int p = i + 1;
        while (p < n && isNameChar(text[p])) ++p;
        if (p == i + 1) return fail("a tag name is missing after '<'", i);
        ptr->tag = text.mid(i + 1, p - i - 1);
        ptr->nameEnd = p;
        const int lineIdx = int(std::upper_bound(m_lineStarts.begin(), m_lineStarts.end(), i) - m_lineStarts.begin());
        ptr->line = lineIdx;
        ptr->column = i - m_lineStarts[lineIdx - 1] + 1;

        bool closed = false;
        while (p < n)
        {
            while (p < n && text[p].isSpace()) ++p;
            if (p >= n) break;
            if (text[p] == '>')
            {
                ptr->openEnd = p + 1;
                closed = true;
                break;
            }
            if (text[p] == '/' && p + 1 < n && text[p + 1] == '>')
            {
                ptr->selfCloseAt = p;
                ptr->openEnd = ptr->end = p + 2;
                closed = true;
                break;
            }
            XmlAttr a;
            a.start = p;
            while (p < n && isNameChar(text[p])) ++p;
            if (p == a.start) return fail("unexpected character in a tag", p);
            a.name = text.mid(a.start, p - a.start);
            while (p < n && text[p].isSpace()) ++p;
            if (p < n && text[p] == '=')
            {
                ++p;
                while (p < n && text[p].isSpace()) ++p;
                if (p >= n) break;
                if (text[p] == '"' || text[p] == '\'')
                {
                    const QChar q = text[p];
                    const int e = text.indexOf(q, p + 1);
                    if (e < 0) return fail("attribute value is not closed", p);
                    a.valueStart = p + 1;
                    a.valueEnd = e;
                    a.value = Unescape(text.mid(a.valueStart, e - a.valueStart));
                    a.end = e + 1;
                    p = e + 1;
                }
                else
                {
                    a.valueStart = p;
                    while (p < n && !text[p].isSpace() && text[p] != '>' && !(text[p] == '/' && p + 1 < n && text[p + 1] == '>')) ++p;
                    a.valueEnd = a.end = p;
                    a.value = Unescape(text.mid(a.valueStart, p - a.valueStart));
                }
            }
            else
            {
                a.valueStart = a.valueEnd = a.end = p;     // attribute without a value
            }
            ptr->attrs.push_back(a);
        }
        if (!closed) return fail("the tag is not finished", i);

        current->children.push_back(std::move(node));
        m_all.push_back(ptr);
        if (!ptr->SelfClosing())
            current = ptr;
        i = ptr->openEnd;
    }
    if (current != m_root.get())
        return fail(QString("<%1> is never closed").arg(current->tag), current->start);
    return true;
}

XmlNode* XmlModel::FindByPosition(int line, int column)
{
    XmlNode* best = nullptr;
    int bestDiff = 1 << 30;
    for (XmlNode* n : m_all)
    {
        if (n->line != line)
            continue;
        const int d = std::abs(n->column - column);
        if (d < bestDiff)
        {
            bestDiff = d;
            best = n;
        }
    }
    return best;
}

// ---------------------------------------------------------------------------------------------------------------------
int XmlModel::LineStart(int pos) const
{
    const int idx = int(std::upper_bound(m_lineStarts.begin(), m_lineStarts.end(), pos) - m_lineStarts.begin());
    return m_lineStarts[qMax(0, idx - 1)];
}

int XmlModel::LineEnd(int pos) const
{
    const int e = m_text.indexOf('\n', pos);
    return e < 0 ? m_text.size() : e;
}

bool XmlModel::OnlyWhitespaceBefore(int pos) const
{
    for (int i = LineStart(pos); i < pos; ++i)
        if (!m_text[i].isSpace())
            return false;
    return true;
}

bool XmlModel::OnlyWhitespaceAfter(int pos) const
{
    for (int i = pos; i < LineEnd(pos); ++i)
        if (!m_text[i].isSpace())
            return false;
    return true;
}

QString XmlModel::IndentOf(const XmlNode* node) const
{
    const int ls = LineStart(node->start);
    int e = ls;
    while (e < m_text.size() && (m_text[e] == ' ' || m_text[e] == '\t'))
        ++e;
    return m_text.mid(ls, e - ls);
}

QString XmlModel::ChildIndent(const XmlNode* parent) const
{
    for (const auto& c : parent->children)
        if (OnlyWhitespaceBefore(c->start))
            return IndentOf(c.get());
    const QString pi = parent->parent ? IndentOf(parent) : QString();
    return pi + (pi.contains('\t') ? "\t" : "    ");
}

QString XmlModel::Reindent(const QString& fragment, const QString& from, const QString& to)
{
    QStringList lines = fragment.split('\n');
    for (int i = 1; i < lines.size(); ++i)
    {
        QString l = lines[i];
        if (l.endsWith('\r')) l.chop(1);
        if (l.startsWith(from)) l = l.mid(from.size());
        lines[i] = l.isEmpty() ? l : to + l;
    }
    return lines.join('\n');
}

// ---------------------------------------------------------------------------------------------------------------------
QString XmlModel::Escape(const QString& v)
{
    QString s = v;
    s.replace("&", "&amp;").replace("<", "&lt;").replace("\"", "&quot;");
    return s;
}

QString XmlModel::Unescape(const QString& v)
{
    QString s = v;
    s.replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'").replace("&amp;", "&");
    return s;
}

QString XmlModel::DefaultSnippet(const QString& tag, bool hasText)
{
    if (hasText)
        return QString("<%1 text=\"New %1\"/>").arg(tag);
    return QString("<%1/>").arg(tag);
}

// ---------------------------------------------------------------------------------------------------------------------
QVector<TextEdit> XmlModel::SetAttribute(const XmlNode* node, const QString& name, const QString& value) const
{
    if (const XmlAttr* a = node->Attr(name))
    {
        if (a->valueStart == a->end)       // unquoted or valueless: rewrite the whole attribute
            return { { a->start, a->end - a->start, QString("%1=\"%2\"").arg(name, Escape(value)) } };
        return { { a->valueStart, a->valueEnd - a->valueStart, Escape(value) } };
    }
    return { { node->nameEnd, 0, QString(" %1=\"%2\"").arg(name, Escape(value)) } };
}

QVector<TextEdit> XmlModel::RemoveAttribute(const XmlNode* node, const QString& name) const
{
    const XmlAttr* a = node->Attr(name);
    if (!a)
        return {};
    int s = a->start;
    while (s > node->nameEnd && m_text[s - 1].isSpace())
        --s;
    return { { s, a->end - s, QString() } };
}

QVector<TextEdit> XmlModel::DeleteNode(const XmlNode* node) const
{
    if (OnlyWhitespaceBefore(node->start) && OnlyWhitespaceAfter(node->end))
    {
        const int s = LineStart(node->start);
        int e = LineEnd(node->end);
        if (e < m_text.size())
            ++e;
        return { { s, e - s, QString() } };
    }
    return { { node->start, node->end - node->start, QString() } };
}

QVector<TextEdit> XmlModel::InsertChild(const XmlNode* parent, const XmlNode* before, const QString& snippet) const
{
    auto withIndent = [&](const QString& ind) { return ind + Reindent(snippet, QString(), ind); };
    if (before)
    {
        if (OnlyWhitespaceBefore(before->start))
        {
            const QString ind = IndentOf(before);
            return { { LineStart(before->start), 0, withIndent(ind) + "\n" } };
        }
        return { { before->start, 0, snippet } };
    }

    const QString ci = ChildIndent(parent);
    const QString pi = parent->parent ? IndentOf(parent) : QString();
    if (parent->SelfClosing())
    {
        int k = parent->selfCloseAt;
        while (k > parent->start && m_text[k - 1] == ' ')
            --k;
        return { { k, parent->end - k, ">\n" + withIndent(ci) + "\n" + pi + "</" + parent->tag + ">" } };
    }
    if (OnlyWhitespaceBefore(parent->closeStart))
        return { { LineStart(parent->closeStart), 0, withIndent(ci) + "\n" } };
    return { { parent->closeStart, 0, "\n" + withIndent(ci) + "\n" + pi } };
}

QVector<TextEdit> XmlModel::Duplicate(const XmlNode* node) const
{
    QString snippet = Reindent(NodeText(node), IndentOf(node), QString());
    // Ids must stay unique inside a page: rename the copy's id.
    static const QRegularExpression idRe(R"re(\bid\s*=\s*"([^"]*)")re");
    const int firstTagEnd = snippet.indexOf('>');
    const QRegularExpressionMatch m = idRe.match(snippet.left(firstTagEnd < 0 ? snippet.size() : firstTagEnd));
    if (m.hasMatch() && !m.captured(1).isEmpty())
        snippet.replace(m.capturedStart(1), m.capturedLength(1), m.captured(1) + "_copy");
    const XmlNode* parent = node->parent;
    const int idx = node->IndexInParent();
    const XmlNode* next = idx + 1 < parent->children.size() ? parent->children[idx + 1].get() : nullptr;
    return InsertChild(parent, next, snippet);
}

QVector<TextEdit> XmlModel::Move(const XmlNode* node, const XmlNode* newParent, const XmlNode* before) const
{
    if (!node->parent || newParent == node || newParent->IsDescendantOf(node) || before == node)
        return {};
    if (newParent == node->parent)
    {
        const int idx = node->IndexInParent();
        const XmlNode* next = idx + 1 < node->parent->children.size() ? node->parent->children[idx + 1].get() : nullptr;
        if (before == next)
            return {};                       // already there
    }
    const QString snippet = Reindent(NodeText(node), IndentOf(node), QString());
    QVector<TextEdit> edits = DeleteNode(node);
    edits += InsertChild(newParent, before, snippet);
    return edits;
}

QVector<TextEdit> XmlModel::MoveUpDown(const XmlNode* node, bool up) const
{
    const XmlNode* parent = node->parent;
    if (!parent)
        return {};
    const int i = node->IndexInParent();
    if (up)
        return i > 0 ? Move(node, parent, parent->children[i - 1].get()) : QVector<TextEdit>();
    if (i + 1 >= parent->children.size())
        return {};
    return Move(node, parent, i + 2 < parent->children.size() ? parent->children[i + 2].get() : nullptr);
}

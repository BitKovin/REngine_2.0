#include "CodeEditor.h"

#include "Highlighter.h"

#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QPainter>
#include <QTextBlock>

class CodeEditor::Gutter : public QWidget
{
public:
    explicit Gutter(CodeEditor* editor) : QWidget(editor), m_editor(editor) {}
    QSize sizeHint() const override { return QSize(m_editor->GutterWidth(), 0); }

protected:
    void paintEvent(QPaintEvent* e) override { m_editor->PaintGutter(e); }

private:
    CodeEditor* m_editor;
};

CodeEditor::CodeEditor(const QString& filePath, QWidget* parent)
    : QPlainTextEdit(parent), m_gutter(new Gutter(this)), m_filePath(filePath)
{
    QFont font("Cascadia Mono");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(10);
    setFont(font);
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);
    setUndoRedoEnabled(true);

    const bool css = filePath.endsWith(".css", Qt::CaseInsensitive);
    const bool script = filePath.endsWith(".uitest", Qt::CaseInsensitive);
    m_highlighter = new Highlighter(document(), css ? Highlighter::Language::Css : script ? Highlighter::Language::Script : Highlighter::Language::Xml);

    connect(this, &QPlainTextEdit::blockCountChanged, this, [this] { UpdateGutterWidth(); });
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::UpdateGutterScroll);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::HighlightCurrentLine);
    UpdateGutterWidth();
    HighlightCurrentLine();
}

bool CodeEditor::Load()
{
    QFile f(m_filePath);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    // Keep the bytes as they are: no newline or encoding conversion happens beyond what QString needs.
    setPlainText(QString::fromUtf8(f.readAll()));
    document()->setModified(false);
    m_diskStamp = QFileInfo(m_filePath).lastModified().toMSecsSinceEpoch();
    return true;
}

bool CodeEditor::Save(QString* error)
{
    QFile f(m_filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        if (error) *error = f.errorString();
        return false;
    }
    f.write(toPlainText().toUtf8());
    f.close();
    document()->setModified(false);
    m_diskStamp = QFileInfo(m_filePath).lastModified().toMSecsSinceEpoch();
    return true;
}

bool CodeEditor::DiskChanged() const
{
    const QFileInfo info(m_filePath);
    return info.exists() && info.lastModified().toMSecsSinceEpoch() != m_diskStamp;
}

void CodeEditor::GoToLine(int line, int column)
{
    QTextCursor c(document()->findBlockByNumber(qMax(0, line - 1)));
    c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, qMax(0, column - 1));
    setTextCursor(c);
    centerCursor();
    setFocus();
}

void CodeEditor::SetProblems(const QMap<int, QPair<bool, QString>>& problems)
{
    m_problems = problems;
    HighlightCurrentLine();
    m_gutter->update();
}

int CodeEditor::GutterWidth() const
{
    int digits = 1;
    for (int max = qMax(1, blockCount()); max >= 10; max /= 10)
        ++digits;
    return 22 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void CodeEditor::UpdateGutterWidth()
{
    setViewportMargins(GutterWidth(), 0, 0, 0);
}

void CodeEditor::UpdateGutterScroll(const QRect& rect, int dy)
{
    if (dy)
        m_gutter->scroll(0, dy);
    else
        m_gutter->update(0, rect.y(), m_gutter->width(), rect.height());
    if (rect.contains(viewport()->rect()))
        UpdateGutterWidth();
}

void CodeEditor::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);
    const QRect cr = contentsRect();
    m_gutter->setGeometry(QRect(cr.left(), cr.top(), GutterWidth(), cr.height()));
}

void CodeEditor::HighlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> selections;
    QTextEdit::ExtraSelection line;
    line.format.setBackground(QColor(40, 44, 54));
    line.format.setProperty(QTextFormat::FullWidthSelection, true);
    line.cursor = textCursor();
    line.cursor.clearSelection();
    selections.append(line);

    for (auto it = m_problems.begin(); it != m_problems.end(); ++it)
    {
        QTextEdit::ExtraSelection s;
        s.cursor = QTextCursor(document()->findBlockByNumber(it.key() - 1));
        s.cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        s.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        s.format.setUnderlineColor(it.value().first ? QColor(240, 90, 90) : QColor(230, 190, 80));
        selections.append(s);
    }
    setExtraSelections(selections);
}

void CodeEditor::PaintGutter(QPaintEvent* event)
{
    QPainter p(m_gutter);
    p.fillRect(event->rect(), QColor(30, 32, 38));
    QTextBlock block = firstVisibleBlock();
    int number = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    const int current = textCursor().blockNumber();
    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            const auto problem = m_problems.constFind(number + 1);
            if (problem != m_problems.constEnd())
            {
                p.setPen(Qt::NoPen);
                p.setBrush(problem.value().first ? QColor(240, 90, 90) : QColor(230, 190, 80));
                p.drawEllipse(QPoint(8, top + fontMetrics().height() / 2), 3, 3);
            }
            p.setPen(number == current ? QColor(220, 224, 232) : QColor(110, 118, 135));
            p.drawText(0, top, m_gutter->width() - 8, fontMetrics().height(), Qt::AlignRight, QString::number(number + 1));
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++number;
    }
}

void CodeEditor::keyPressEvent(QKeyEvent* event)
{
    // Enter keeps the indentation of the line above, which is what people expect from markup.
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        const QString line = textCursor().block().text();
        QString indent;
        for (const QChar c : line)
        {
            if (c == ' ' || c == '\t') indent += c;
            else break;
        }
        QPlainTextEdit::keyPressEvent(event);
        insertPlainText(indent);
        return;
    }
    if (event->key() == Qt::Key_Tab && !(event->modifiers() & Qt::ShiftModifier))
    {
        insertPlainText("    ");
        return;
    }
    QPlainTextEdit::keyPressEvent(event);
}

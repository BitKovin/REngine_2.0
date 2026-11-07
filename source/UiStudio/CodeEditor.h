#pragma once

#include <QMap>
#include <QPlainTextEdit>

class Highlighter;

// A source editor: line numbers, current-line highlight, problem markers, and a stable save that never
// reformats the text (the file on disk is exactly what the user typed).
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(const QString& filePath, QWidget* parent = nullptr);

    const QString& FilePath() const { return m_filePath; }
    bool Load();
    bool Save(QString* error);
    bool DiskChanged() const;           // the file changed on disk after we loaded or saved it
    void GoToLine(int line, int column = 1);

    // problems: line -> severity (true = error). Shown as a coloured mark in the gutter and a wavy underline.
    void SetProblems(const QMap<int, QPair<bool, QString>>& problems);

    int GutterWidth() const;
    void PaintGutter(QPaintEvent* event);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void UpdateGutterWidth();
    void UpdateGutterScroll(const QRect& rect, int dy);
    void HighlightCurrentLine();

    class Gutter;
    Gutter* m_gutter;
    Highlighter* m_highlighter = nullptr;
    QString m_filePath;
    qint64 m_diskStamp = 0;
    QMap<int, QPair<bool, QString>> m_problems;
};

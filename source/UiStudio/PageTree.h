#pragma once

#include <QListWidget>
#include <QTreeWidget>

// MIME type used when a widget is dragged out of the palette.
inline const char* const kTagMime = "application/x-uistudio-tag";

// The page structure. Items can be dragged to reorder or re-parent them, and palette entries can be dropped on it.
// The tree never moves items itself: it only reports what was dropped where, and the page source is edited instead.
class PageTree : public QTreeWidget
{
    Q_OBJECT
public:
    enum class Where { Onto, Above, Below };

    explicit PageTree(QWidget* parent = nullptr);

signals:
    void ElementDropped(QTreeWidgetItem* source, QTreeWidgetItem* target, PageTree::Where where);
    void TagDropped(const QString& tag, QTreeWidgetItem* target, PageTree::Where where);

protected:
    void dragEnterEvent(QDragEnterEvent*) override;
    void dragMoveEvent(QDragMoveEvent*) override;
    void dragLeaveEvent(QDragLeaveEvent*) override;
    void dropEvent(QDropEvent*) override;
    void paintEvent(QPaintEvent*) override;

private:
    bool Accepts(const QMimeData*, QObject* source) const;
    void UpdateTarget(const QPoint& pos);

    QTreeWidgetItem* m_dropItem = nullptr;
    Where m_where = Where::Onto;
};

// The list of widgets that can be dragged into a page.
class PaletteList : public QListWidget
{
    Q_OBJECT
public:
    explicit PaletteList(QWidget* parent = nullptr);

protected:
    QMimeData* mimeData(const QList<QListWidgetItem*>& items) const override;
};

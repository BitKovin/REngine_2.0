#include "PageTree.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QPainter>

PageTree::PageTree(QWidget* parent) : QTreeWidget(parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::CopyAction);
    setDropIndicatorShown(false);
}

bool PageTree::Accepts(const QMimeData* mime, QObject* source) const
{
    return mime->hasFormat(kTagMime) || source == this || source == viewport();
}

void PageTree::UpdateTarget(const QPoint& pos)
{
    m_dropItem = itemAt(pos);
    if (!m_dropItem)
        return;
    const QRect r = visualItemRect(m_dropItem);
    const int y = pos.y() - r.top();
    m_where = y < r.height() / 4 ? Where::Above : y > r.height() * 3 / 4 ? Where::Below : Where::Onto;
    viewport()->update();
}

void PageTree::dragEnterEvent(QDragEnterEvent* e)
{
    if (Accepts(e->mimeData(), e->source()))
    {
        e->setDropAction(Qt::CopyAction);
        e->accept();
    }
    else
        e->ignore();
}

void PageTree::dragMoveEvent(QDragMoveEvent* e)
{
    if (!Accepts(e->mimeData(), e->source()))
    {
        e->ignore();
        return;
    }
    UpdateTarget(e->position().toPoint());
    if (m_dropItem)
    {
        e->setDropAction(Qt::CopyAction);
        e->accept();
    }
    else
        e->ignore();
}

void PageTree::dragLeaveEvent(QDragLeaveEvent* e)
{
    m_dropItem = nullptr;
    viewport()->update();
    QTreeWidget::dragLeaveEvent(e);
}

void PageTree::dropEvent(QDropEvent* e)
{
    UpdateTarget(e->position().toPoint());
    QTreeWidgetItem* target = m_dropItem;
    const Where where = m_where;
    m_dropItem = nullptr;
    viewport()->update();
    if (!target || !Accepts(e->mimeData(), e->source()))
    {
        e->ignore();
        return;
    }
    e->setDropAction(Qt::CopyAction);
    e->accept();                    // accepted as a copy so the view does not remove the dragged item itself
    if (e->mimeData()->hasFormat(kTagMime))
        emit TagDropped(QString::fromUtf8(e->mimeData()->data(kTagMime)), target, where);
    else if (QTreeWidgetItem* source = currentItem())
        emit ElementDropped(source, target, where);
}

void PageTree::paintEvent(QPaintEvent* e)
{
    QTreeWidget::paintEvent(e);
    if (!m_dropItem)
        return;
    QPainter p(viewport());
    const QRect r = visualItemRect(m_dropItem);
    p.setPen(QPen(QColor("#5aa0ff"), 2));
    if (m_where == Where::Onto)
    {
        p.setBrush(QColor(90, 160, 255, 50));
        p.drawRect(r.adjusted(1, 1, -1, -1));
    }
    else
    {
        const int y = m_where == Where::Above ? r.top() : r.bottom();
        p.drawLine(r.left(), y, r.right(), y);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
PaletteList::PaletteList(QWidget* parent) : QListWidget(parent)
{
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
    setDefaultDropAction(Qt::CopyAction);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setUniformItemSizes(false);
}

QMimeData* PaletteList::mimeData(const QList<QListWidgetItem*>& items) const
{
    auto* mime = new QMimeData;
    if (!items.isEmpty())
        mime->setData(kTagMime, items.first()->data(Qt::UserRole).toString().toUtf8());
    return mime;
}

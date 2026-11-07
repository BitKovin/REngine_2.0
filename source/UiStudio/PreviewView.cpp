#include "PreviewView.h"

#include "PageTree.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

PreviewView::PreviewView(QWidget* parent) : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumSize(320, 180);
    setCursor(Qt::CrossCursor);
    setAcceptDrops(true);
}

void PreviewView::SetImage(const QImage& image)
{
    m_image = image;
    if (!image.isNull() && !m_viewportKnown)
        m_viewport = QSizeF(image.width(), image.height());
    update();
}

QRectF PreviewView::ImageRect() const
{
    if (m_image.isNull())
        return rect();
    const QSizeF avail = size() - QSize(16, 16);
    const double scale = qMin(avail.width() / m_image.width(), avail.height() / m_image.height());
    const QSizeF s(m_image.width() * scale, m_image.height() * scale);
    return QRectF((width() - s.width()) / 2, (height() - s.height()) / 2, s.width(), s.height());
}

QPointF PreviewView::ToGame(const QPoint& p, bool* inside) const
{
    const QRectF r = ImageRect();
    if (inside) *inside = r.contains(p);
    return QPointF((p.x() - r.left()) / r.width() * m_viewport.width(), (p.y() - r.top()) / r.height() * m_viewport.height());
}

void PreviewView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(20, 22, 27));
    if (m_image.isNull())
    {
        p.setPen(QColor(143, 151, 170));
        p.drawText(rect().adjusted(24, 24, -24, -24), Qt::AlignCenter | Qt::TextWordWrap,
                   m_message.isEmpty() ? tr("Nothing to show yet.") : m_message);
        return;
    }
    const QRectF r = ImageRect();
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    // A checkerboard shows transparent areas of the page.
    p.fillRect(r, QColor(46, 49, 58));
    p.drawImage(r, m_image);
    p.setPen(QPen(QColor(58, 63, 75), 1));
    p.drawRect(r);

    if (m_outlines && !m_play && !m_selection.isEmpty())
    {
        const double sx = r.width() / m_viewport.width(), sy = r.height() / m_viewport.height();
        const QRectF s(r.left() + m_selection.left() * sx, r.top() + m_selection.top() * sy, m_selection.width() * sx, m_selection.height() * sy);
        p.setBrush(QColor(74, 138, 244, 45));
        p.setPen(QPen(QColor(74, 138, 244), 2));
        p.drawRect(s);
    }
    if (!m_message.isEmpty())
    {
        p.setPen(QColor(220, 224, 232));
        p.fillRect(QRectF(r.left(), r.bottom() - 26, r.width(), 26), QColor(0, 0, 0, 150));
        p.drawText(QRectF(r.left() + 8, r.bottom() - 26, r.width() - 16, 26), Qt::AlignVCenter | Qt::AlignLeft, m_message);
    }
}

void PreviewView::SetPlayMode(bool on)
{
    m_play = on;
    setCursor(on ? Qt::ArrowCursor : Qt::CrossCursor);
    setFocusPolicy(on ? Qt::StrongFocus : Qt::NoFocus);
    if (on)
        setFocus();
    update();
}

void PreviewView::mouseReleaseEvent(QMouseEvent* e)
{
    if (m_play && e->button() == Qt::LeftButton)
        emit PointerReleased();
}

void PreviewView::leaveEvent(QEvent*)
{
}

void PreviewView::keyPressEvent(QKeyEvent* e)
{
    if (!m_play)
        return QWidget::keyPressEvent(e);
    QString action;
    switch (e->key())
    {
    case Qt::Key_Up: action = "ui_up"; break;
    case Qt::Key_Down: action = "ui_down"; break;
    case Qt::Key_Left: action = "ui_left"; break;
    case Qt::Key_Right: action = "ui_right"; break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space: action = "ui_confirm"; break;
    case Qt::Key_Escape:
    case Qt::Key_Backspace: action = "ui_cancel"; break;
    default: return QWidget::keyPressEvent(e);
    }
    emit ActionKey(action);
}

void PreviewView::mousePressEvent(QMouseEvent* e)
{
    bool inside = false;
    const QPointF g = ToGame(e->pos(), &inside);
    if (m_play)
    {
        setFocus();
        if (inside && e->button() == Qt::LeftButton)
            emit PointerPressed(g.x(), g.y());
        return;
    }
    if (inside && e->button() == Qt::LeftButton)
        emit Picked(g.x(), g.y());
}

void PreviewView::mouseMoveEvent(QMouseEvent* e)
{
    bool inside = false;
    const QPointF g = ToGame(e->pos(), &inside);
    if (m_play)
    {
        if (inside)
            emit PointerMoved(g.x(), g.y());
        return;
    }
    if (inside)
        emit Hovered(g.x(), g.y());
}

void PreviewView::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasFormat(kTagMime) && !m_image.isNull())
        e->acceptProposedAction();
}

void PreviewView::dragMoveEvent(QDragMoveEvent* e)
{
    if (e->mimeData()->hasFormat(kTagMime))
        e->acceptProposedAction();
}

void PreviewView::dropEvent(QDropEvent* e)
{
    bool inside = false;
    const QPointF g = ToGame(e->position().toPoint(), &inside);
    if (!inside || !e->mimeData()->hasFormat(kTagMime))
        return;
    e->acceptProposedAction();
    emit TagDropped(QString::fromUtf8(e->mimeData()->data(kTagMime)), g.x(), g.y());
}

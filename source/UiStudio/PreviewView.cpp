#include "PreviewView.h"

#include <QMouseEvent>
#include <QPainter>

PreviewView::PreviewView(QWidget* parent) : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumSize(320, 180);
    setCursor(Qt::CrossCursor);
}

void PreviewView::SetImage(const QImage& image)
{
    m_image = image;
    if (!image.isNull())
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

    if (m_outlines && !m_selection.isEmpty())
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

void PreviewView::mousePressEvent(QMouseEvent* e)
{
    bool inside = false;
    const QPointF g = ToGame(e->pos(), &inside);
    if (inside && e->button() == Qt::LeftButton)
        emit Picked(g.x(), g.y());
}

void PreviewView::mouseMoveEvent(QMouseEvent* e)
{
    bool inside = false;
    const QPointF g = ToGame(e->pos(), &inside);
    if (inside)
        emit Hovered(g.x(), g.y());
}

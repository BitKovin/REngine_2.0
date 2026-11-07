#pragma once

#include <QImage>
#include <QRectF>
#include <QWidget>

// Shows the latest game screenshot scaled to fit. The mouse hovers and picks elements in game coordinates;
// the selected element is outlined, and the hovered one is shaded.
class PreviewView : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewView(QWidget* parent = nullptr);

    void SetImage(const QImage& image);
    void SetViewportSize(const QSizeF& size) { m_viewport = size; update(); }
    void SetSelection(const QRectF& rect) { m_selection = rect; update(); }
    void SetMessage(const QString& message) { m_message = message; update(); }
    bool HasImage() const { return !m_image.isNull(); }
    void SetShowOutlines(bool on) { m_outlines = on; update(); }

signals:
    void Picked(double x, double y);     // game coordinates
    void Hovered(double x, double y);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;

private:
    QRectF ImageRect() const;
    QPointF ToGame(const QPoint& p, bool* inside) const;

    QImage m_image;
    QSizeF m_viewport{ 1920, 1080 };
    QRectF m_selection;
    QString m_message;
    bool m_outlines = true;
};

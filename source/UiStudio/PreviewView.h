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
    void SetViewportSize(const QSizeF& size) { m_viewport = size; m_viewportKnown = true; update(); }
    void SetSelection(const QRectF& rect) { m_selection = rect; update(); }
    void SetMessage(const QString& message) { m_message = message; update(); }
    bool HasImage() const { return !m_image.isNull(); }
    void SetShowOutlines(bool on) { m_outlines = on; update(); }
    // Play mode: the mouse and keys go to the game instead of picking elements.
    void SetPlayMode(bool on);
    bool PlayMode() const { return m_play; }

signals:
    void TagDropped(const QString& tag, double x, double y);   // a palette widget was dropped here (game coordinates)
    void Picked(double x, double y);     // game coordinates
    void Hovered(double x, double y);
    void PointerMoved(double x, double y);              // play mode, game coordinates
    void PointerPressed(double x, double y);
    void PointerReleased();
    void ActionKey(const QString& action);              // play mode, for example "ui_confirm"

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void leaveEvent(QEvent*) override;
    void dragEnterEvent(QDragEnterEvent*) override;
    void dragMoveEvent(QDragMoveEvent*) override;
    void dropEvent(QDropEvent*) override;

private:
    QRectF ImageRect() const;
    QPointF ToGame(const QPoint& p, bool* inside) const;

    QImage m_image;
    QSizeF m_viewport{ 1920, 1080 };
    QRectF m_selection;
    QString m_message;
    bool m_outlines = true;
    bool m_play = false;
    bool m_viewportKnown = false;     // the game told us its UI size; the picture may be smaller than that
};

#pragma once

#include <QWidget>

// Play mode: shows the game's own window inside Studio. The game runs hidden (-headless) for editing; for playing, its
// window is re-parented into this widget, so the game draws itself and gets the real mouse and keyboard with no copying.
// Windows only. Elsewhere Attach() returns false and Studio falls back to the shared-frame preview.
class EmbeddedGameView : public QWidget
{
    Q_OBJECT
public:
    explicit EmbeddedGameView(QWidget* parent = nullptr);
    ~EmbeddedGameView() override;

    // Finds the game process's window and moves it into this widget (still hidden). True when it worked.
    bool Attach(qint64 processId);
    bool IsAttached() const { return m_child != nullptr; }
    void Detach();                       // the game ended or Studio is closing: forget the window

    // Shows (play) or hides (edit) the game window. While hidden it keeps a full HD size, so editor screenshots stay sharp.
    void SetActive(bool active);

protected:
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;
    void paintEvent(QPaintEvent*) override;
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;

private:
    void Layout();

    void* m_child = nullptr;             // HWND of the game window
    unsigned long m_childThread = 0;     // its thread, whose input queue is joined with Studio's
    bool m_active = false;
};

#include "EmbeddedGameView.h"

#include <QPainter>
#include <QResizeEvent>
#include <QTimer>

#ifdef Q_OS_WIN
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

EmbeddedGameView::EmbeddedGameView(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow);            // the game window needs a real window to live in
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMinimumSize(320, 180);
    setFocusPolicy(Qt::StrongFocus);
    // If the game moves or resizes its own window anyway, put it back.
    auto* keeper = new QTimer(this);
    keeper->setInterval(400);
    connect(keeper, &QTimer::timeout, this, [this] { Layout(); });
    keeper->start();
}

EmbeddedGameView::~EmbeddedGameView()
{
    Detach();
}

#ifdef Q_OS_WIN
namespace
{
    struct Find
    {
        DWORD pid;
        HWND result;
    };

    BOOL CALLBACK Match(HWND hwnd, LPARAM param)
    {
        auto* f = reinterpret_cast<Find*>(param);
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        if (pid != f->pid || GetWindow(hwnd, GW_OWNER) != nullptr)
            return TRUE;
        char cls[64] = {};
        GetClassNameA(hwnd, cls, sizeof(cls));
        if (strcmp(cls, "SDL_app") != 0)              // the engine's SDL window; other windows of the process are skipped
            return TRUE;
        f->result = hwnd;
        return FALSE;
    }
}
#endif

bool EmbeddedGameView::Attach(qint64 processId)
{
#ifdef Q_OS_WIN
    if (m_child)
        return true;
    Find f{ static_cast<DWORD>(processId), nullptr };
    EnumWindows(Match, reinterpret_cast<LPARAM>(&f));
    if (!f.result)
        return false;
    HWND child = f.result;
    LONG_PTR style = GetWindowLongPtr(child, GWL_STYLE);
    style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_BORDER | WS_DLGFRAME);
    style |= WS_CHILD | WS_CLIPSIBLINGS;
    SetWindowLongPtr(child, GWL_STYLE, style);
    LONG_PTR ex = GetWindowLongPtr(child, GWL_EXSTYLE);
    ex &= ~(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME);
    SetWindowLongPtr(child, GWL_EXSTYLE, ex);
    SetParent(child, reinterpret_cast<HWND>(winId()));
    SetWindowPos(child, nullptr, 0, 0, 1920, 1080, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    ShowWindow(child, SW_HIDE);
    m_child = child;
    m_active = false;
    // Join the game window's input queue with Studio's: this is what lets keyboard focus move into and out of the game reliably.
    m_childThread = GetWindowThreadProcessId(child, nullptr);
    if (m_childThread)
        AttachThreadInput(GetCurrentThreadId(), m_childThread, TRUE);
    return true;
#else
    Q_UNUSED(processId);
    return false;
#endif
}

void EmbeddedGameView::Detach()
{
#ifdef Q_OS_WIN
    if (m_child && m_childThread)
        AttachThreadInput(GetCurrentThreadId(), m_childThread, FALSE);
#endif
    m_child = nullptr;
    m_childThread = 0;
    m_active = false;
}

// A click inside the game window tells its parent (this widget) about it. Use that to hand the keyboard to the game,
// so keys always go where the mouse was last used.
bool EmbeddedGameView::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
#ifdef Q_OS_WIN
    if (m_child && m_active && eventType == "windows_generic_MSG")
    {
        const MSG* msg = static_cast<const MSG*>(message);
        if (msg->message == WM_PARENTNOTIFY)
        {
            const unsigned event = LOWORD(msg->wParam);
            if (event == WM_LBUTTONDOWN || event == WM_RBUTTONDOWN || event == WM_MBUTTONDOWN)
                SetFocus(static_cast<HWND>(m_child));
        }
    }
#endif
    return QWidget::nativeEvent(eventType, message, result);
}

void EmbeddedGameView::SetActive(bool active)
{
#ifdef Q_OS_WIN
    if (!m_child)
        return;
    HWND child = static_cast<HWND>(m_child);
    m_active = active;
    if (active)
    {
        Layout();
        ShowWindow(child, SW_SHOWNA);
        SetFocus(child);
    }
    else
    {
        ShowWindow(child, SW_HIDE);
        SetWindowPos(child, nullptr, 0, 0, 1920, 1080, SWP_NOZORDER | SWP_NOACTIVATE);
        window()->activateWindow();
    }
#else
    Q_UNUSED(active);
#endif
}

void EmbeddedGameView::Layout()
{
#ifdef Q_OS_WIN
    if (!m_child || !m_active)
        return;
    // The game's picture is 16:9: fit the largest such rectangle, centred.
    const qreal ratio = devicePixelRatioF();
    const int w = qRound(width() * ratio), h = qRound(height() * ratio);
    int cw = w, ch = w * 9 / 16;
    if (ch > h)
    {
        ch = h;
        cw = h * 16 / 9;
    }
    const int x = (w - cw) / 2, y = (h - ch) / 2;
    cw = qMax(cw, 16);
    ch = qMax(ch, 9);
    RECT now{};
    HWND child = static_cast<HWND>(m_child);
    GetWindowRect(child, &now);
    POINT topLeft{ now.left, now.top };
    ScreenToClient(reinterpret_cast<HWND>(winId()), &topLeft);
    if (topLeft.x == x && topLeft.y == y && now.right - now.left == cw && now.bottom - now.top == ch)
        return;
    SetWindowPos(child, nullptr, x, y, cw, ch, SWP_NOZORDER | SWP_NOACTIVATE);
#endif
}

void EmbeddedGameView::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    Layout();
}

void EmbeddedGameView::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);
    Layout();
}

void EmbeddedGameView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(8, 9, 12));
}

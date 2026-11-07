#include "Theme.h"

#include <QApplication>
#include <QPalette>
#include <QStyleFactory>

void Theme::Apply(QApplication& app)
{
    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette p;
    const QColor bg(30, 32, 38), panel(37, 40, 48), base(24, 26, 31), text(220, 224, 232), accent(86, 156, 255);
    p.setColor(QPalette::Window, panel);
    p.setColor(QPalette::WindowText, text);
    p.setColor(QPalette::Base, base);
    p.setColor(QPalette::AlternateBase, bg);
    p.setColor(QPalette::ToolTipBase, bg);
    p.setColor(QPalette::ToolTipText, text);
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Button, QColor(48, 52, 62));
    p.setColor(QPalette::ButtonText, text);
    p.setColor(QPalette::BrightText, Qt::white);
    p.setColor(QPalette::Highlight, accent);
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::PlaceholderText, QColor(130, 136, 150));
    p.setColor(QPalette::Disabled, QPalette::Text, QColor(110, 115, 128));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(110, 115, 128));
    app.setPalette(p);

    app.setStyleSheet(R"(
        QToolTip { background:#1e2026; color:#dce0e8; border:1px solid #3a3f4b; padding:4px; }
        QMainWindow::separator { background:#1b1d22; width:3px; height:3px; }
        QDockWidget { titlebar-close-icon:none; font-weight:600; }
        QDockWidget::title { background:#2a2d36; padding:6px 10px; border-bottom:1px solid #1b1d22; }
        QToolBar { background:#2a2d36; border:none; spacing:4px; padding:4px; }
        QToolButton { padding:5px 9px; border-radius:5px; }
        QToolButton:hover { background:#3a3f4b; }
        QToolButton:checked { background:#3a5a8f; }
        QPushButton { background:#3a3f4b; border:none; border-radius:5px; padding:7px 14px; }
        QPushButton:hover { background:#464c5b; }
        QPushButton#primary { background:#4a8af4; color:white; font-weight:600; }
        QPushButton#primary:hover { background:#5b97ff; }
        QLineEdit, QComboBox { background:#181a1f; border:1px solid #3a3f4b; border-radius:5px; padding:6px 8px; }
        QLineEdit:focus { border-color:#4a8af4; }
        QTreeView, QTreeWidget, QTableWidget, QListWidget, QPlainTextEdit, QTextEdit {
            background:#181a1f; border:none; selection-background-color:#31527f; alternate-background-color:#1c1e24; }
        QHeaderView::section { background:#2a2d36; border:none; padding:4px 8px; color:#a8afc0; }
        QTabWidget::pane { border:none; }
        QTabBar::tab { background:#25282f; padding:7px 14px; border-right:1px solid #1b1d22; }
        QTabBar::tab:selected { background:#181a1f; border-bottom:2px solid #4a8af4; }
        QTabBar::tab:hover:!selected { background:#2f333c; }
        QStatusBar { background:#2a2d36; }
        QMenuBar { background:#2a2d36; }
        QMenuBar::item:selected, QMenu::item:selected { background:#3a5a8f; }
        QMenu { background:#25282f; border:1px solid #3a3f4b; }
        QLabel#hint { color:#8f97aa; }
        QLabel#title { font-size:26px; font-weight:700; }
        QFrame#card { background:#25282f; border-radius:10px; }
    )");
}

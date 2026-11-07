#include "MainWindow.h"

#include "CodeEditor.h"
#include "GameSession.h"
#include "PreviewView.h"
#include "Templates.h"
#include "EmbeddedGameView.h"
#include "PageTree.h"
#include "PropertiesPanel.h"
#include "TestPanel.h"
#include "TreeRoles.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QFormLayout>
#include <QFrame>
#include <QMenu>
#include <QRegularExpression>
#include <functional>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTabBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QToolBar>
#include <QTreeView>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace
{
    QDockWidget* MakeDock(QMainWindow* w, const QString& title, QWidget* content, Qt::DockWidgetArea area, const QString& name)
    {
        auto* dock = new QDockWidget(title, w);
        dock->setObjectName(name);
        dock->setWidget(content);
        dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        w->addDockWidget(area, dock);
        return dock;
    }

    QRectF RectOf(const QJsonValue& v)
    {
        const QJsonObject o = v.toObject();
        return QRectF(o["x"].toDouble(), o["y"].toDouble(), o["w"].toDouble(), o["h"].toDouble());
    }

    // Short, readable text for a computed value: lengths as "12px", colours unchanged, lists joined.
    QString Pretty(const QJsonValue& v)
    {
        if (v.isDouble())
            return QString::number(v.toDouble(), 'g', 6);
        if (v.isBool())
            return v.toBool() ? "true" : "false";
        if (v.isString())
            return v.toString();
        if (v.isArray())
        {
            QStringList parts;
            for (const QJsonValue& e : v.toArray())
                parts << Pretty(e);
            return parts.join("  ");
        }
        if (v.isObject())
        {
            const QJsonObject o = v.toObject();
            const QString kind = o["kind"].toString();
            if (kind == "px") return QString::number(o["px"].toDouble(), 'g', 6) + "px";
            if (kind == "percent") return QString::number(o["percent"].toDouble(), 'g', 6) + "%";
            if (kind == "auto") return "auto";
            if (o.contains("color") && o.contains("width"))
                return QString::number(o["width"].toDouble(), 'g', 6) + "px " + o["color"].toString();
            QStringList parts;
            for (auto it = o.begin(); it != o.end(); ++it)
                parts << it.key() + "=" + Pretty(it.value());
            return parts.join(", ");
        }
        return QString();
    }

    QColor ParseColor(const QString& s)
    {
        // The engine writes #rrggbbaa; Qt reads #aarrggbb.
        if (s.size() == 9 && s[0] == '#')
            return QColor("#" + s.mid(7, 2) + s.mid(1, 6));
        if (s.size() == 7 && s[0] == '#')
            return QColor(s);
        return QColor();
    }

    QIcon Swatch(const QColor& c)
    {
        QPixmap pm(14, 14);
        pm.fill(c);
        return QIcon(pm);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
MainWindow::MainWindow()
{
    setWindowTitle("UI Studio");
    resize(1500, 900);

    m_session = new GameSession(this);
    connect(m_session, &GameSession::LogLine, this, &MainWindow::AppendLog);
    connect(m_session, &GameSession::Started, this, [this] { SetGameState(tr("Game is starting…"), "#e6c07b"); });
    connect(m_session, &GameSession::Ready, this, [this] {
        SetGameState(tr("Game is running"), "#98c379");
        if (m_pendingShow)
        {
            m_pendingShow = false;
            ShowInPreview();
        }
    });
    connect(m_session, &GameSession::Stopped, this, [this](int code, bool crashed) {
        SetGameState(crashed ? tr("Game crashed (code %1)").arg(code) : tr("Game is not running"), crashed ? "#e0788a" : "#8f97aa");
        m_actRun->setText(tr("▶  Start game"));
        m_actPreview->setEnabled(false);
        m_embed->Detach();
        if (m_embedded)
        {
            m_embedded = false;
            m_previewStack->setCurrentWidget(m_preview);
        }
        if (m_play)
            SetPlayMode(false);
        if (crashed)
            m_preview->SetMessage(tr("The game closed unexpectedly. Check the Log tab for the reason, then press Start game."));
    });
    connect(m_session, &GameSession::ScreenshotReady, this, [this](const QImage& image) {
        if (m_play)
            return;                                                   // Play mode shows the shared live frames instead
        m_preview->SetImage(image);
        m_preview->SetMessage(QString());
    });
    connect(m_session, &GameSession::FrameReady, this, [this](const QImage& image) {
        if (!m_play)
            return;
        m_preview->SetImage(image);
        m_preview->SetMessage(QString());
    });
    connect(m_session, &GameSession::JsonReady, this, &MainWindow::OnJson);
    connect(m_session, &GameSession::RequestFailed, this, [this](const QString& what) {

        statusBar()->showMessage(tr("The game did not answer (%1). It may still be loading.").arg(what), 5000);
    });

    m_refreshTimer.setSingleShot(true);
    connect(&m_refreshTimer, &QTimer::timeout, this, [this] {
        if (!m_session->IsReady())
            return;
        m_session->RequestScreenshot();
        m_session->RequestJson("ui_dump_json 64", "SaveData/ui_inspect/tree.json", "tree");
    });
    m_lintTimer.setSingleShot(true);
    connect(&m_lintTimer, &QTimer::timeout, this, &MainWindow::RunLint);
    connect(&m_pickTimer, &QTimer::timeout, this, &MainWindow::PollPickFile);
    m_pickTimer.start(400);

    m_stack = new QStackedWidget;
    m_welcome = BuildWelcome();
    m_stack->addWidget(m_welcome);
    setCentralWidget(m_stack);

    BuildActions();
    BuildWorkspace();
    BuildMenus();

    m_stateLabel = new QLabel;
    m_hintLabel = new QLabel;
    m_hintLabel->setObjectName("hint");
    statusBar()->addWidget(m_stateLabel);
    statusBar()->addPermanentWidget(m_hintLabel);
    SetGameState(tr("Game is not running"), "#8f97aa");

    installEventFilter(this);
    RebuildRecent();
}

MainWindow::~MainWindow() = default;

void MainWindow::OpenFromArguments(const QStringList& args)
{
    if (args.size() < 2)
        return;
    const QString dir = args[1];
    if (!OpenProject(dir, args.size() > 2 && !args[2].isEmpty() ? args[2] : GuessExecutable(dir)))
        return;
    // Optional third argument: a page (path relative to GameData) to open and show in the preview.
    if (args.size() > 3 && OpenFile(FileOfGamePath(args[3])))
        ShowInPreview();
}

// ---------------------------------------------------------------------------------------------------------------------
QWidget* MainWindow::BuildWelcome()
{
    auto* page = new QWidget;
    auto* outer = new QVBoxLayout(page);
    outer->addStretch(1);

    auto* card = new QFrame;
    card->setObjectName("card");
    card->setMaximumWidth(700);
    auto* lay = new QVBoxLayout(card);
    lay->setContentsMargins(36, 32, 36, 32);
    lay->setSpacing(10);

    auto* title = new QLabel(tr("UI Studio"));
    title->setObjectName("title");
    auto* sub = new QLabel(tr("Design and test menus and HUD for your game. Edit the page on the left, see the result in a live preview on the right."));
    sub->setObjectName("hint");
    sub->setWordWrap(true);
    lay->addWidget(title);
    lay->addWidget(sub);
    lay->addSpacing(14);

    auto addPicker = [&](const QString& label, const QString& placeholder, QLineEdit*& edit, bool folder) {
        auto* l = new QLabel(label);
        l->setStyleSheet("font-weight:600;");
        lay->addWidget(l);
        auto* row = new QHBoxLayout;
        edit = new QLineEdit;
        edit->setPlaceholderText(placeholder);
        auto* browse = new QPushButton(tr("Browse…"));
        row->addWidget(edit, 1);
        row->addWidget(browse);
        lay->addLayout(row);
        connect(browse, &QPushButton::clicked, this, [this, edit, folder] {
            const QString start = edit->text().isEmpty() ? QDir::homePath() : edit->text();
            const QString chosen = folder ? QFileDialog::getExistingDirectory(this, tr("Choose the GameData folder"), start)
                                          : QFileDialog::getOpenFileName(this, tr("Choose the game program"), start, tr("Programs (*.exe);;All files (*)"));
            if (!chosen.isEmpty())
                edit->setText(QDir::toNativeSeparators(chosen));
        });
    };
    addPicker(tr("1.  Your game's GameData folder"), tr("For example …/source/Games/MyGame/GameData"), m_projectEdit, true);
    addPicker(tr("2.  The game program used for the live preview"), tr("Found automatically when possible"), m_exeEdit, false);

    connect(m_projectEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        if (m_exeEdit->text().isEmpty() || m_exeEdit->property("auto").toBool())
        {
            const QString guess = GuessExecutable(text);
            m_exeEdit->setText(QDir::toNativeSeparators(guess));
            m_exeEdit->setProperty("auto", true);
        }
    });
    connect(m_exeEdit, &QLineEdit::textEdited, this, [this] { m_exeEdit->setProperty("auto", false); });

    lay->addSpacing(10);
    auto* open = new QPushButton(tr("Open project"));
    open->setObjectName("primary");
    open->setMinimumHeight(38);
    connect(open, &QPushButton::clicked, this, &MainWindow::OnOpenProject);
    lay->addWidget(open);

    lay->addSpacing(14);
    auto* recentTitle = new QLabel(tr("Recent projects"));
    recentTitle->setStyleSheet("font-weight:600;");
    lay->addWidget(recentTitle);
    m_recent = new QListWidget;
    m_recent->setMaximumHeight(110);
    connect(m_recent, &QListWidget::itemActivated, this, [this](QListWidgetItem* item) {
        OpenProject(item->data(Qt::UserRole).toString(), item->data(Qt::UserRole + 1).toString());
    });
    lay->addWidget(m_recent);

    auto* row = new QHBoxLayout;
    row->addStretch(1);
    row->addWidget(card, 1);
    row->addStretch(1);
    outer->addLayout(row);
    outer->addStretch(2);
    return page;
}

void MainWindow::RebuildRecent()
{
    m_recent->clear();
    QSettings s("UiStudio", "UiStudio");
    for (const QString& entry : s.value("recent").toStringList())
    {
        const QStringList parts = entry.split('|');
        if (parts.size() < 2 || !QDir(parts[0]).exists())
            continue;
        auto* item = new QListWidgetItem(QDir::toNativeSeparators(parts[0]), m_recent);
        item->setData(Qt::UserRole, parts[0]);
        item->setData(Qt::UserRole + 1, parts[1]);
        item->setToolTip(tr("Game program: %1").arg(parts[1]));
    }
    if (m_recent->count() == 0)
        m_recent->addItem(tr("Nothing yet. Projects you open appear here."));
    if (m_recent->count() > 0 && m_projectEdit->text().isEmpty() && m_recent->item(0)->data(Qt::UserRole).isValid())
    {
        m_projectEdit->setText(QDir::toNativeSeparators(m_recent->item(0)->data(Qt::UserRole).toString()));
        m_exeEdit->setText(QDir::toNativeSeparators(m_recent->item(0)->data(Qt::UserRole + 1).toString()));
        m_exeEdit->setProperty("auto", false);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MainWindow::BuildActions()
{
    auto make = [this](const QString& text, const QKeySequence& key, auto slot) {
        auto* a = new QAction(text, this);
        if (!key.isEmpty())
            a->setShortcut(key);
        connect(a, &QAction::triggered, this, slot);
        addAction(a);
        m_paletteActions << a;
        return a;
    };
    m_actOpen = make(tr("Open project…"), QKeySequence::Open, [this] {
        while (m_tabs->count() > 0)
        {
            const int before = m_tabs->count();
            CloseTab(0);
            if (m_tabs->count() == before)
                return;                     // the user cancelled
        }
        StopGame();
        m_preview->SetImage(QImage());
        m_stack->setCurrentWidget(m_welcome);
        for (QDockWidget* d : findChildren<QDockWidget*>())
            d->hide();
        for (QToolBar* t : findChildren<QToolBar*>())
            t->hide();
        RebuildRecent();
    });
    m_actSave = make(tr("Save"), QKeySequence::Save, [this] { if (CurrentEditor()) SaveEditor(CurrentEditor()); });
    m_actSaveAll = make(tr("Save all"), QKeySequence("Ctrl+Shift+S"), [this] { SaveAll(); });
    m_actNew = make(tr("New file…"), QKeySequence::New, [this] { NewFile(); });
    m_actCloseTab = make(tr("Close tab"), QKeySequence("Ctrl+W"), [this] { if (m_tabs->count()) CloseTab(m_tabs->currentIndex()); });
    m_actRun = make(tr("▶  Start game"), QKeySequence("F5"), [this] { m_session->IsRunning() ? StopGame() : StartGame(); });
    m_actPreview = make(tr("Show this page in preview"), QKeySequence("F6"), [this] { ShowInPreview(); });
    m_actEditMode = make(tr("✎  Edit mode"), QKeySequence("F7"), [this] { SetPlayMode(false); });
    m_actPlayMode = make(tr("▶  Play mode"), QKeySequence("F8"), [this] { SetPlayMode(true); });
    for (QAction* a : { m_actEditMode, m_actPlayMode })
        a->setCheckable(true);
    m_actEditMode->setChecked(true);
    m_actEditMode->setToolTip(tr("Edit mode: click elements in the preview to select and change them (F7)"));
    m_actPlayMode->setToolTip(tr("Play mode: use the page like a player would. Mouse, arrow keys, Enter and Esc go to the game (F8)"));
    {
        auto* group = new QActionGroup(this);
        group->addAction(m_actEditMode);
        group->addAction(m_actPlayMode);
    }
    m_actLint = make(tr("Check for problems"), QKeySequence("F7"), [this] { RunLint(); });
    m_actPalette = make(tr("Command palette…"), QKeySequence("Ctrl+K"), [this] { ShowPalette(); });
    m_actHelp = make(tr("Getting started"), QKeySequence::HelpContents, [this] {
        QMessageBox::information(this, tr("Getting started"),
            tr("<b>1.</b> Pick a page in the Project list and open it. It appears as a tab.<br>"
               "<b>2.</b> Press <b>Start game</b> (F5), then <b>Show this page in preview</b> (F6).<br>"
               "<b>3.</b> Click anything in the preview to see which line of the file made it.<br>"
               "<b>4.</b> Edit and press <b>Ctrl+S</b>. The preview refreshes by itself.<br>"
               "<b>5.</b> <b>Check for problems</b> (F7) lists mistakes with the line they are on.<br><br>"
               "Press <b>Ctrl+K</b> any time to search every command and file."));
    });
    new QShortcut(QKeySequence("Ctrl+P"), this, [this] { ShowPalette(); });
}

void MainWindow::BuildMenus()
{
    QMenu* file = menuBar()->addMenu(tr("&File"));
    file->addAction(m_actNew);
    file->addAction(m_actOpen);
    file->addSeparator();
    file->addAction(m_actSave);
    file->addAction(m_actSaveAll);
    file->addAction(m_actCloseTab);
    file->addSeparator();
    auto* quit = file->addAction(tr("Quit"));
    quit->setShortcut(QKeySequence("Ctrl+Q"));
    connect(quit, &QAction::triggered, this, &QWidget::close);

    QMenu* game = menuBar()->addMenu(tr("&Game"));
    game->addAction(m_actRun);
    game->addAction(m_actPreview);
    game->addAction(m_actLint);

    QMenu* view = menuBar()->addMenu(tr("&View"));
    for (QDockWidget* d : findChildren<QDockWidget*>())
        view->addAction(d->toggleViewAction());
    view->addSeparator();
    view->addAction(m_actPalette);

    QMenu* help = menuBar()->addMenu(tr("&Help"));
    help->addAction(m_actHelp);
}

void MainWindow::BuildWorkspace()
{
    // Centre: editor tabs.
    m_tabs = new QTabWidget;
    m_tabs->setTabsClosable(true);
    m_tabs->setMovable(true);
    m_tabs->setDocumentMode(true);
    connect(m_tabs, &QTabWidget::tabCloseRequested, this, &MainWindow::CloseTab);
    connect(m_tabs, &QTabWidget::currentChanged, this, [this] {
        CodeEditor* e = CurrentEditor();
        const bool xml = e && e->FilePath().endsWith(".xml", Qt::CaseInsensitive);
        m_actPreview->setEnabled(xml);
        m_hintLabel->setText(e ? QDir::toNativeSeparators(e->FilePath()) : QString());
    });
    // With no file open, the centre explains what to do instead of showing an empty grey area.
    auto* empty = new QLabel(tr("Open a page from the Project list to start editing.\nPress Ctrl+K to search all files and commands."));
    empty->setObjectName("hint");
    empty->setAlignment(Qt::AlignCenter);
    m_tabs->setCornerWidget(nullptr);
    m_centerStack = new QStackedWidget;
    m_centerStack->addWidget(empty);
    m_centerStack->addWidget(m_tabs);
    m_stack->addWidget(m_centerStack);
    connect(m_tabs, &QTabWidget::currentChanged, this, [this] { m_centerStack->setCurrentIndex(m_tabs->count() ? 1 : 0); });

    // Toolbar.
    auto* tb = addToolBar(tr("Main"));
    tb->setObjectName("main");
    tb->setMovable(false);
    tb->setToolButtonStyle(Qt::ToolButtonTextOnly);
    tb->addAction(m_actNew);
    tb->addAction(m_actSave);
    tb->addSeparator();
    tb->addAction(m_actRun);
    tb->addAction(m_actPreview);
    tb->addAction(m_actLint);
    tb->addSeparator();
    tb->addAction(m_actEditMode);
    tb->addAction(m_actPlayMode);
    auto* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(spacer);
    tb->addAction(m_actPalette);
    tb->hide();
    m_actPalette->setText(tr("Search… (Ctrl+K)"));
    m_actPreview->setText(tr("Show page in preview"));
    m_actLint->setText(tr("✓  Check for problems"));
    m_actSave->setText(tr("Save"));
    m_actNew->setText(tr("＋  New file"));
    m_actPreview->setEnabled(false);

    // The side docks run the full height; the Output dock sits under the editor only.
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    // Left: project tree + hierarchy.
    m_model = new QFileSystemModel(this);
    m_model->setNameFilters({ "*.xml", "*.css" });
    m_model->setNameFilterDisables(false);
    m_tree = new QTreeView;
    m_tree->setModel(m_model);
    m_tree->setHeaderHidden(true);
    for (int c = 1; c < 4; ++c)
        m_tree->hideColumn(c);
    m_tree->setAnimated(true);
    connect(m_tree, &QTreeView::activated, this, [this](const QModelIndex& i) {
        if (!m_model->isDir(i)) OpenFile(m_model->filePath(i));
    });
    connect(m_tree, &QTreeView::doubleClicked, this, [this](const QModelIndex& i) {
        if (!m_model->isDir(i)) OpenFile(m_model->filePath(i));
    });
    QDockWidget* projectDock = MakeDock(this, tr("Project"), m_tree, Qt::LeftDockWidgetArea, "project");

    m_hierarchy = new PageTree;
    m_hierarchy->setHeaderHidden(true);
    m_hierarchy->setAnimated(true);
    connect(m_hierarchy, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* item, QTreeWidgetItem*) {
        if (item) SelectItem(item, true);
    });
    QDockWidget* hierDock = MakeDock(this, tr("Page structure"), m_hierarchy, Qt::LeftDockWidgetArea, "hierarchy");
    splitDockWidget(projectDock, hierDock, Qt::Vertical);

    // Right: preview + inspector.
    m_preview = new PreviewView;
    m_preview->SetMessage(tr("Open a page, press Start game, then Show page in preview.\nClick anything here to find its code."));
    connect(m_preview, &PreviewView::Picked, this, [this](double x, double y) {
        if (!m_session->IsReady()) return;
        m_session->RequestJson(QString("ui_pick_at %1 %2").arg(qRound(x)).arg(qRound(y)), "SaveData/ui_inspect/pick.json", "pick");
    });
    // Edit mode shows a still picture of the game (so elements can be picked); Play mode shows the game window itself.
    m_embed = new EmbeddedGameView;
    m_previewStack = new QStackedWidget;
    m_previewStack->addWidget(m_preview);
    m_previewStack->addWidget(m_embed);
    QDockWidget* previewDock = MakeDock(this, tr("Live preview"), m_previewStack, Qt::RightDockWidgetArea, "preview");

    m_inspector = new QTableWidget(0, 2);
    m_inspector->setHorizontalHeaderLabels({ tr("Property"), tr("Value") });
    m_inspector->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_inspector->horizontalHeader()->setStretchLastSection(true);
    m_inspector->verticalHeader()->hide();
    m_inspector->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_inspector->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_inspector->setShowGrid(false);
    QDockWidget* inspectDock = MakeDock(this, tr("Inspector"), m_inspector, Qt::RightDockWidgetArea, "inspector");
    splitDockWidget(previewDock, inspectDock, Qt::Vertical);
    BuildVisualEditing(projectDock, inspectDock);

    // Bottom: problems, log, console.
    m_bottom = new QTabWidget;
    m_bottom->setDocumentMode(true);
    m_problems = new QTableWidget(0, 4);
    m_problems->setHorizontalHeaderLabels({ "", tr("File"), tr("Line"), tr("What is wrong") });
    m_problems->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_problems->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_problems->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_problems->horizontalHeader()->setStretchLastSection(true);
    m_problems->verticalHeader()->hide();
    m_problems->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_problems->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_problems->setShowGrid(false);
    connect(m_problems, &QTableWidget::cellActivated, this, [this](int row, int) {
        const QTableWidgetItem* it = m_problems->item(row, 1);
        if (!it) return;
        const QString file = FileOfGamePath(it->data(RoleFile).toString());
        if (QFileInfo::exists(file))
            OpenFile(file, it->data(RoleLine).toInt(), it->data(RoleCol).toInt());
    });
    connect(m_problems, &QTableWidget::cellDoubleClicked, m_problems, &QTableWidget::cellActivated);
    m_bottom->addTab(m_problems, tr("Problems"));

    m_log = new QPlainTextEdit;
    m_log->setReadOnly(true);
    m_log->setMaximumBlockCount(4000);
    QFont mono("Cascadia Mono");
    mono.setStyleHint(QFont::Monospace);
    mono.setPointSize(9);
    m_log->setFont(mono);
    auto* consolePage = new QWidget;
    auto* cl = new QVBoxLayout(consolePage);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->setSpacing(0);
    cl->addWidget(m_log, 1);
    m_console = new QLineEdit;
    m_console->setPlaceholderText(tr("Type a game console command, for example ui_stack, then press Enter"));
    m_console->installEventFilter(this);
    connect(m_console, &QLineEdit::returnPressed, this, &MainWindow::RunConsoleCommand);
    cl->addWidget(m_console);
    m_bottom->addTab(consolePage, tr("Game log and console"));

    m_tests = new TestPanel;
    m_bottom->addTab(m_tests, tr("Tests"));
    connect(m_tests, &TestPanel::OpenScript, this, [this](const QString& file, int line) { OpenFile(file, line); });
    connect(m_tests, &TestPanel::RunStarting, this, [this] {
        // A test uses the same game program and save folder as the preview game, so the preview steps aside.
        m_gameWasRunning = m_session->IsRunning();
        if (m_gameWasRunning)
            StopGame();
        m_preview->SetMessage(tr("A test is running. The preview comes back when it is done."));
    });
    connect(m_tests, &TestPanel::RunFinished, this, [this] {
        if (m_gameWasRunning)
            StartGame();
        m_gameWasRunning = false;
    });
    MakeDock(this, tr("Output"), m_bottom, Qt::BottomDockWidgetArea, "output");

    for (QDockWidget* d : findChildren<QDockWidget*>())
        d->hide();
    resizeDocks({ projectDock, previewDock }, { 300, 620 }, Qt::Horizontal);
    resizeDocks({ findChild<QDockWidget*>("output") }, { 220 }, Qt::Vertical);
}

// ---------------------------------------------------------------------------------------------------------------------
QString MainWindow::GuessExecutable(const QString& gameDataDir) const
{
    // .../source/Games/<Game>/GameData  ->  <repo>/Build/<Game>/windows/bin/<Game>.exe
    QDir dir(gameDataDir);
    if (!dir.exists())
        return QString();
    const QString game = QFileInfo(dir.absolutePath()).dir().dirName();
    QDir up = dir;
    for (int i = 0; i < 6 && up.cdUp(); ++i)
    {
        const QString candidate = up.filePath(QString("Build/%1/windows/bin/%1.exe").arg(game));
        if (QFileInfo::exists(candidate))
            return QDir::cleanPath(candidate);
    }
    return QString();
}

QString MainWindow::GamePathOf(const QString& file) const
{
    return "GameData/" + QDir(m_projectDir).relativeFilePath(file);
}

QString MainWindow::FileOfGamePath(const QString& gamePath) const
{
    QString rel = gamePath;
    if (rel.startsWith("GameData/"))
        rel = rel.mid(9);
    return QDir(m_projectDir).filePath(rel);
}

void MainWindow::OnOpenProject()
{
    OpenProject(m_projectEdit->text().trimmed(), m_exeEdit->text().trimmed());
}

bool MainWindow::OpenProject(const QString& gameDataDir, const QString& exe)
{
    const QDir dir(gameDataDir);
    if (gameDataDir.isEmpty() || !dir.exists())
    {
        QMessageBox::warning(this, tr("Open project"), tr("Please choose your game's GameData folder first."));
        return false;
    }
    if (!dir.exists("ui") && dir.dirName() != "GameData")
    {
        QMessageBox::warning(this, tr("Open project"),
                             tr("This folder has no \"ui\" folder inside. Choose the game's GameData folder, for example …/Games/MyGame/GameData."));
        return false;
    }
    m_projectDir = QDir::cleanPath(dir.absolutePath());
    m_exe = exe;
    m_previewPage.clear();

    const QString root = dir.exists("ui") ? dir.filePath("ui") : m_projectDir;
    m_model->setRootPath(root);
    m_tree->setRootIndex(m_model->index(root));

    {
        QSettings s("UiStudio", "UiStudio");
        QStringList recent = s.value("recent").toStringList();
        const QString entry = m_projectDir + "|" + exe;
        recent.removeAll(entry);
        recent.prepend(entry);
        while (recent.size() > 8)
            recent.removeLast();
        s.setValue("recent", recent);
    }

    m_stack->setCurrentWidget(m_centerStack);
    for (QToolBar* t : findChildren<QToolBar*>())
        t->show();
    for (QDockWidget* d : findChildren<QDockWidget*>())
        d->show();
    setWindowTitle(QString("UI Studio - %1").arg(QFileInfo(QFileInfo(m_projectDir).dir().path()).fileName() + "/" + QFileInfo(m_projectDir).dir().dirName()));
    if (exe.isEmpty() || !QFileInfo::exists(exe))
        m_preview->SetMessage(tr("No game program was found, so the live preview is off.\nYou can still edit files. Use File > Open project to choose the program."));
    m_actRun->setEnabled(!exe.isEmpty() && QFileInfo::exists(exe));
    m_tests->SetProject(exe);
    {
        QStringList where;
        if (!exe.isEmpty())
        {
            QDir bin = QFileInfo(exe).dir();
            where << bin.absolutePath();
            if (bin.cdUp())
                where << bin.absolutePath();
        }
        where << QFileInfo(m_projectDir).dir().absolutePath();
        m_db.Load(where);
        FillPalette(QString());
        m_properties->ShowNothing(tr("Click an element in the preview or in the page structure to edit it here."));
        m_propertiesDock->raise();
    }
    statusBar()->showMessage(tr("Project opened. Double-click a page to edit it."), 6000);
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
CodeEditor* MainWindow::CurrentEditor() const
{
    return qobject_cast<CodeEditor*>(m_tabs->currentWidget());
}

CodeEditor* MainWindow::OpenFile(const QString& path, int line, int column)
{
    const QString canonical = QFileInfo(path).absoluteFilePath();
    for (int i = 0; i < m_tabs->count(); ++i)
    {
        auto* e = qobject_cast<CodeEditor*>(m_tabs->widget(i));
        if (e && QFileInfo(e->FilePath()).absoluteFilePath() == canonical)
        {
            m_tabs->setCurrentIndex(i);
            if (line > 0) e->GoToLine(line, column);
            return e;
        }
    }
    auto* editor = new CodeEditor(canonical);
    if (!editor->Load())
    {
        delete editor;
        QMessageBox::warning(this, tr("Open file"), tr("The file could not be read:\n%1").arg(canonical));
        return nullptr;
    }
    const int index = m_tabs->addTab(editor, QFileInfo(canonical).fileName());
    m_centerStack->setCurrentIndex(1);
    m_tabs->setTabToolTip(index, QDir::toNativeSeparators(canonical));
    m_tabs->setCurrentIndex(index);
    connect(editor->document(), &QTextDocument::modificationChanged, this, [this, editor] { UpdateTabTitle(editor); });
    editor->SetProblems(m_problemsByFile.value(GamePathOf(canonical)));
    if (line > 0)
        editor->GoToLine(line, column);
    return editor;
}

void MainWindow::UpdateTabTitle(CodeEditor* editor)
{
    const int i = m_tabs->indexOf(editor);
    if (i >= 0)
        m_tabs->setTabText(i, QFileInfo(editor->FilePath()).fileName() + (editor->document()->isModified() ? " ●" : ""));
}

void MainWindow::CopyToGame(const QString& file)
{
    if (m_exe.isEmpty() || !file.startsWith(m_projectDir + "/"))
        return;
    // The game reads its own GameData folder (next to the program's "bin"), not the source folder we edit.
    const QDir work = QFileInfo(m_exe).dir();
    QDir root = work;
    root.cdUp();
    const QString gameData = root.exists("GameData") ? root.filePath("GameData") : work.filePath("GameData");
    if (QDir::cleanPath(gameData) == m_projectDir || !QDir(gameData).exists())
        return;
    const QString target = QDir(gameData).filePath(QDir(m_projectDir).relativeFilePath(file));
    QDir().mkpath(QFileInfo(target).absolutePath());
    QFile::remove(target);
    if (!QFile::copy(file, target))
        statusBar()->showMessage(tr("Saved, but the file could not be copied to the game folder: %1").arg(target), 8000);
}

bool MainWindow::SaveEditor(CodeEditor* editor)
{
    QString error;
    if (!editor->Save(&error))
    {
        QMessageBox::warning(this, tr("Save"), tr("The file could not be saved:\n%1").arg(error));
        return false;
    }
    CopyToGame(editor->FilePath());
    statusBar()->showMessage(tr("Saved %1").arg(QFileInfo(editor->FilePath()).fileName()), 3000);
    const bool pageFile = editor->FilePath().endsWith(".xml", Qt::CaseInsensitive) || editor->FilePath().endsWith(".css", Qt::CaseInsensitive);
    if (pageFile && m_session->IsReady() && !m_previewPage.isEmpty())
    {
        RefreshPreview(1200);          // the game reloads changed files by itself, then we look again
        m_lintTimer.start(1400);
    }
    return true;
}

bool MainWindow::SaveAll()
{
    for (int i = 0; i < m_tabs->count(); ++i)
        if (auto* e = qobject_cast<CodeEditor*>(m_tabs->widget(i)))
            if (e->document()->isModified() && !SaveEditor(e))
                return false;
    return true;
}

bool MainWindow::ConfirmDiscard(CodeEditor* editor)
{
    if (!editor->document()->isModified())
        return true;
    const auto answer = QMessageBox::question(this, tr("Unsaved changes"),
        tr("\"%1\" has changes that are not saved.").arg(QFileInfo(editor->FilePath()).fileName()),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (answer == QMessageBox::Cancel)
        return false;
    if (answer == QMessageBox::Save)
        return SaveEditor(editor);
    return true;
}

void MainWindow::CloseTab(int index)
{
    auto* e = qobject_cast<CodeEditor*>(m_tabs->widget(index));
    if (!e || !ConfirmDiscard(e))
        return;
    m_tabs->removeTab(index);
    delete e;
    m_centerStack->setCurrentIndex(m_tabs->count() ? 1 : 0);
}

void MainWindow::NewFile()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("New file"));
    dlg.setMinimumWidth(460);
    auto* lay = new QVBoxLayout(&dlg);
    lay->addWidget(new QLabel(tr("What do you want to make?")));
    auto* list = new QListWidget;
    for (const FileTemplate& t : Templates::All())
    {
        auto* item = new QListWidgetItem(t.name + "  -  " + t.description, list);
        item->setData(Qt::UserRole, list->count() - 1);
    }
    list->setCurrentRow(0);
    lay->addWidget(list);
    auto* name = new QLineEdit;
    name->setPlaceholderText(tr("File name, for example my_menu"));
    lay->addWidget(new QLabel(tr("Name")));
    lay->addWidget(name);

    QString folder = m_model->rootPath();
    const QModelIndex cur = m_tree->currentIndex();
    if (cur.isValid())
        folder = m_model->isDir(cur) ? m_model->filePath(cur) : QFileInfo(m_model->filePath(cur)).absolutePath();
    auto* where = new QLabel(tr("It will be created in: %1").arg(QDir(m_projectDir).relativeFilePath(folder)));
    where->setObjectName("hint");
    lay->addWidget(where);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Create"));
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    lay->addWidget(buttons);
    if (dlg.exec() != QDialog::Accepted)
        return;

    QString base = name->text().trimmed();
    base.remove(QRegularExpression("\\.(xml|css)$", QRegularExpression::CaseInsensitiveOption));
    base.replace(QRegularExpression("[^A-Za-z0-9_\\-]"), "_");
    if (base.isEmpty())
    {
        QMessageBox::information(this, tr("New file"), tr("Please type a name."));
        return;
    }
    const FileTemplate& t = Templates::All().at(list->currentRow());
    const QString path = QDir(folder).filePath(base + t.extension);
    if (QFileInfo::exists(path))
    {
        QMessageBox::warning(this, tr("New file"), tr("A file with this name already exists."));
        return;
    }
    // Templates link their style sheet by game path, so give them the folder this file lives in.
    const QString gameFolder = "GameData/" + QDir(m_projectDir).relativeFilePath(folder);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, tr("New file"), tr("The file could not be created."));
        return;
    }
    f.write(Templates::Render(t, base, gameFolder).toUtf8());
    f.close();
    CopyToGame(path);
    OpenFile(path);
}

void MainWindow::CheckExternalChanges()
{
    for (int i = 0; i < m_tabs->count(); ++i)
    {
        auto* e = qobject_cast<CodeEditor*>(m_tabs->widget(i));
        if (!e || !e->DiskChanged())
            continue;
        if (!e->document()->isModified())
        {
            const int line = e->textCursor().blockNumber() + 1;
            e->Load();
            e->GoToLine(line);
            statusBar()->showMessage(tr("%1 changed outside Studio and was reloaded.").arg(QFileInfo(e->FilePath()).fileName()), 5000);
            continue;
        }
        const auto answer = QMessageBox::question(this, tr("File changed outside Studio"),
            tr("\"%1\" was changed by another program, and you also have unsaved edits.\n\nLoad the version from disk and lose your edits?")
                .arg(QFileInfo(e->FilePath()).fileName()),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (answer == QMessageBox::Yes)
            e->Load();
        else
        {
            e->document()->setModified(true);
            e->Save(nullptr);               // keep ours; this also refreshes the remembered timestamp
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MainWindow::SetGameState(const QString& text, const char* color)
{
    if (m_stateLabel)
        m_stateLabel->setText(QString("<span style='color:%1'>●</span>  %2").arg(color, text));
}

void MainWindow::StartGame()
{
    if (m_exe.isEmpty() || !QFileInfo::exists(m_exe))
    {
        QMessageBox::information(this, tr("Start game"), tr("No game program is set for this project. Use File > Open project and choose it."));
        return;
    }
    const QDir bin = QFileInfo(m_exe).dir();
    QDir work = bin;
    if (!work.exists("GameData") && work.cdUp() && !work.exists("GameData"))
        work = bin;
    QString error;
    if (!m_session->Start(m_exe, work.absolutePath(), &error))
    {
        QMessageBox::warning(this, tr("Start game"), error);
        return;
    }
    m_actRun->setText(tr("■  Stop game"));
    m_preview->SetMessage(tr("Starting the game. The first start takes a few seconds…"));
}

void MainWindow::StopGame()
{
    m_pendingShow = false;
    m_session->Stop();
}

void MainWindow::ShowInPreview()
{
    CodeEditor* e = CurrentEditor();
    if (!e || !e->FilePath().endsWith(".xml", Qt::CaseInsensitive))
    {
        statusBar()->showMessage(tr("Open an XML page first."), 4000);
        return;
    }
    if (e->document()->isModified())
        SaveEditor(e);
    else
        CopyToGame(e->FilePath());
    m_previewPage = GamePathOf(e->FilePath());
    if (!m_session->IsRunning())
    {
        m_pendingShow = true;
        StartGame();
        return;
    }
    if (!m_session->IsReady())
    {
        m_pendingShow = true;
        return;
    }
    // Pushing on the menu stack (like the game does) runs the page's own code, so tabs, lists and titles get filled.
    m_session->Send("ui_clear_stack");
    m_session->Send("ui_push " + m_previewPage);
    m_selectedPath.clear();
    m_preview->SetSelection(QRectF());
    RefreshPreview(700);
    m_lintTimer.start(900);
}

void MainWindow::RefreshPreview(int delayMs)
{
    m_refreshTimer.start(delayMs);
}

void MainWindow::RunLint()
{
    if (!m_session->IsReady())
    {
        statusBar()->showMessage(tr("Start the game first. Checking uses the game's own loader."), 5000);
        return;
    }
    SaveAll();
    m_session->RequestJson("ui_lint", "SaveData/ui_lint.json", "lint");
}

void MainWindow::OnJson(const QString& tag, const QJsonDocument& doc)
{
    if (tag == "tree")
        FillHierarchy(doc);
    else if (tag == "lint")
        FillProblems(doc);
    else if (tag == "pick")
    {
        const QJsonObject element = doc.object()["element"].toObject();
        if (!doc.object()["hit"].toBool() || element["path"].toString().isEmpty())
        {
            statusBar()->showMessage(tr("Nothing there."), 2000);
            return;
        }
        if (!m_dropTag.isEmpty())
        {
            const QString dropped = m_dropTag;
            m_dropTag.clear();
            if (QTreeWidgetItem* target = ItemByPath(element["path"].toString()))
                InsertTag(dropped, target, 0);
            else
                statusBar()->showMessage(tr("That element is not part of an editable page."), 4000);
            return;
        }
        SelectByPath(element["path"].toString());
    }
    else if (tag == "computed")
    {
        const QJsonObject o = doc.object();
        if (!o["found"].toBool())
            return;
        FillInspector(o);
        m_preview->SetSelection(RectOf(o["borderRect"]));
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MainWindow::FillHierarchy(const QJsonDocument& doc)
{
    const QJsonObject root = doc.object();
    const QJsonObject vp = root["viewport"].toObject();
    if (vp["w"].toDouble() > 0)
        m_preview->SetViewportSize(QSizeF(vp["w"].toDouble(), vp["h"].toDouble()));

    QSignalBlocker block(m_hierarchy);
    m_hierarchy->clear();
    // Only pages that come from a file are interesting: skip empty infrastructure roots.
    std::function<void(const QJsonObject&, QTreeWidgetItem*)> add = [&](const QJsonObject& n, QTreeWidgetItem* parent) {
        auto* item = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(m_hierarchy);
        QString text = n["tag"].toString();
        if (!n["id"].toString().isEmpty())
            text += "  #" + n["id"].toString();
        const QJsonArray classes = n["classes"].toArray();
        QStringList cls;
        for (const QJsonValue& c : classes)
            if (!c.toString().startsWith("input-"))
                cls << "." + c.toString();
        if (!cls.isEmpty())
            text += "   " + cls.join(" ");
        item->setText(0, text);
        item->setData(0, RolePath, n["path"].toString());
        item->setData(0, RoleRect, RectOf(n["rect"]));
        item->setData(0, RoleFile, n["file"].toString());
        item->setData(0, RoleLine, n["line"].toInt());
        item->setData(0, RoleCol, n["col"].toInt());
        const QString file = n["file"].toString();
        item->setToolTip(0, file.isEmpty() ? n["path"].toString() : QString("%1:%2").arg(file).arg(n["line"].toInt()));
        if (!n["flags"].toObject()["visible"].toBool())
            item->setForeground(0, QColor(110, 118, 135));
        for (const QJsonValue& c : n["children"].toArray())
            add(c.toObject(), item);
    };
    for (const QJsonValue& r : root["roots"].toArray())
    {
        const QJsonObject n = r.toObject();
        if (n["children"].toArray().isEmpty() && n["file"].toString().isEmpty())
            continue;
        add(n, nullptr);
    }
    m_hierarchy->expandToDepth(2);
    if (!m_selectedPath.isEmpty())
        SelectByPath(m_selectedPath);
}

// The lightweight link: a running game's `ui_pick` writes SaveData/ui_pick.json. A newer file than the last one seen selects its node.
// This is the fallback; the runtime bridge (ui_pick_at over the console) is what the visual editor uses.
void MainWindow::PollPickFile()
{
    if (m_session == nullptr || m_session->WorkDir().isEmpty())
        return;
    const QFileInfo info(QDir(m_session->WorkDir()).filePath("SaveData/ui_pick.json"));
    if (!info.exists())
        return;
    const qint64 stamp = info.lastModified().toMSecsSinceEpoch();
    if (m_pickStamp == 0)
    {
        m_pickStamp = stamp;   // what was there before Studio looked is not a request
        return;
    }
    if (stamp == m_pickStamp)
        return;
    QFile file(info.filePath());
    if (!file.open(QIODevice::ReadOnly))
        return;   // still being written: the next tick tries again
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return;
    m_pickStamp = stamp;
    const QString path = doc.object()["path"].toString();
    if (!path.isEmpty())
        SelectByPath(path);
}

void MainWindow::SelectByPath(const QString& path)
{
    QTreeWidgetItemIterator it(m_hierarchy);
    while (*it)
    {
        if ((*it)->data(0, RolePath).toString() == path)
        {
            QSignalBlocker block(m_hierarchy);
            m_hierarchy->setCurrentItem(*it);
            for (QTreeWidgetItem* p = (*it)->parent(); p; p = p->parent())
                p->setExpanded(true);
            m_hierarchy->scrollToItem(*it);
            SelectItem(*it, true);
            return;
        }
        ++it;
    }
    m_selectedPath = path;
    m_session->RequestJson("ui_computed " + path, "SaveData/ui_inspect/computed.json", "computed");
}

void MainWindow::SelectItem(QTreeWidgetItem* item, bool jumpToSource)
{
    m_selectedPath = item->data(0, RolePath).toString();
    m_preview->SetSelection(item->data(0, RoleRect).toRectF());
    if (m_session->IsReady())
        m_session->RequestJson("ui_computed " + m_selectedPath, "SaveData/ui_inspect/computed.json", "computed");
    const QString file = item->data(0, RoleFile).toString();
    const int line = item->data(0, RoleLine).toInt();
    if (jumpToSource && !file.isEmpty() && line > 0)
    {
        const QString local = FileOfGamePath(file);
        if (QFileInfo::exists(local))
            OpenFile(local, line, item->data(0, RoleCol).toInt());
    }
    UpdateProperties();
}

void MainWindow::FillInspector(const QJsonObject& o)
{
    m_inspector->setRowCount(0);
    auto section = [this](const QString& title) {
        const int r = m_inspector->rowCount();
        m_inspector->insertRow(r);
        auto* item = new QTableWidgetItem(title);
        QFont f = item->font();
        f.setBold(true);
        item->setFont(f);
        item->setBackground(QColor(42, 45, 54));
        item->setForeground(QColor(143, 177, 245));
        m_inspector->setItem(r, 0, item);
        auto* filler = new QTableWidgetItem;
        filler->setBackground(QColor(42, 45, 54));
        m_inspector->setItem(r, 1, filler);
    };
    auto row = [this](const QString& key, const QString& value) {
        const int r = m_inspector->rowCount();
        m_inspector->insertRow(r);
        m_inspector->setItem(r, 0, new QTableWidgetItem(key));
        auto* v = new QTableWidgetItem(value);
        const QColor c = ParseColor(value);
        if (c.isValid())
            v->setIcon(Swatch(c));
        m_inspector->setItem(r, 1, v);
    };

    const QJsonObject el = o["element"].toObject();
    section(tr("Element"));
    row(tr("Tag"), el["tag"].toString());
    if (!el["id"].toString().isEmpty()) row(tr("Id"), el["id"].toString());
    QStringList classes;
    for (const QJsonValue& c : o["classes"].toArray()) classes << c.toString();
    if (!classes.isEmpty()) row(tr("Classes"), classes.join("  "));
    if (!el["file"].toString().isEmpty()) row(tr("Defined in"), QString("%1:%2").arg(el["file"].toString()).arg(el["line"].toInt()));

    section(tr("Size and position (game pixels)"));
    const QRectF b = RectOf(o["borderRect"]);
    row(tr("Position"), QString("%1, %2").arg(b.x(), 0, 'g', 6).arg(b.y(), 0, 'g', 6));
    row(tr("Size"), QString("%1 × %2").arg(b.width(), 0, 'g', 6).arg(b.height(), 0, 'g', 6));

    QStringList flags;
    const QJsonObject f = o["flags"].toObject();
    for (auto it = f.begin(); it != f.end(); ++it)
        if (it.value().toBool()) flags << it.key();
    section(tr("State"));
    row(tr("Active flags"), flags.isEmpty() ? tr("none") : flags.join(", "));

    const QJsonObject attrs = o["attributes"].toObject();
    if (!attrs.isEmpty())
    {
        section(tr("Attributes written in the file"));
        for (auto it = attrs.begin(); it != attrs.end(); ++it)
            row(it.key(), it.value().toString());
    }

    section(tr("Style the element ends up with"));
    const QJsonObject computed = o["computed"].toObject();
    for (auto it = computed.begin(); it != computed.end(); ++it)
        row(it.key(), Pretty(it.value()));
}

void MainWindow::FillProblems(const QJsonDocument& doc)
{
    const QJsonArray list = doc.object()["diagnostics"].toArray();
    m_problems->setRowCount(0);
    m_problemsByFile.clear();
    int errors = 0, warnings = 0;
    for (const QJsonValue& v : list)
    {
        const QJsonObject d = v.toObject();
        const bool isError = d["severity"].toString() == "error";
        (isError ? errors : warnings)++;
        const int r = m_problems->rowCount();
        m_problems->insertRow(r);
        auto* sev = new QTableWidgetItem(isError ? "●" : "▲");
        sev->setForeground(isError ? QColor(240, 90, 90) : QColor(230, 190, 80));
        sev->setToolTip(isError ? tr("Error") : tr("Warning"));
        m_problems->setItem(r, 0, sev);
        QString path = d["path"].toString();
        auto* file = new QTableWidgetItem(path.section('/', -1));
        file->setToolTip(path);
        file->setData(RoleFile, path);
        file->setData(RoleLine, d["line"].toInt());
        file->setData(RoleCol, d["column"].toInt());
        m_problems->setItem(r, 1, file);
        m_problems->setItem(r, 2, new QTableWidgetItem(QString::number(d["line"].toInt())));
        QString text = d["message"].toString();
        if (!d["help"].toString().isEmpty())
            text += "   -   " + d["help"].toString();
        auto* msg = new QTableWidgetItem(text);
        msg->setToolTip(d["code"].toString());
        m_problems->setItem(r, 3, msg);
        if (d["line"].toInt() > 0)
            m_problemsByFile[path][d["line"].toInt()] = qMakePair(isError, d["message"].toString());
    }
    m_bottom->setTabText(0, list.isEmpty() ? tr("Problems") : tr("Problems (%1)").arg(list.size()));
    ApplyProblemsToEditors();
    statusBar()->showMessage(list.isEmpty() ? tr("No problems found.") : tr("%1 errors, %2 warnings. Open the Problems tab.").arg(errors).arg(warnings), 6000);
}

void MainWindow::ApplyProblemsToEditors()
{
    for (int i = 0; i < m_tabs->count(); ++i)
        if (auto* e = qobject_cast<CodeEditor*>(m_tabs->widget(i)))
            e->SetProblems(m_problemsByFile.value(GamePathOf(e->FilePath())));
}

void MainWindow::AppendLog(const QString& line)
{
    m_log->appendPlainText(line);
}

void MainWindow::RunConsoleCommand()
{
    const QString cmd = m_console->text().trimmed();
    if (cmd.isEmpty())
        return;
    if (!m_session->IsRunning())
    {
        statusBar()->showMessage(tr("Start the game first."), 4000);
        return;
    }
    m_history.removeAll(cmd);
    m_history << cmd;
    m_historyPos = m_history.size();
    m_log->appendPlainText("> " + cmd);
    m_session->Send(cmd);
    m_console->clear();
}

// ---------------------------------------------------------------------------------------------------------------------
void MainWindow::ShowPalette()
{
    QDialog dlg(this, Qt::Popup);
    dlg.setMinimumWidth(560);
    auto* lay = new QVBoxLayout(&dlg);
    auto* edit = new QLineEdit;
    edit->setPlaceholderText(tr("Type a command or a file name…"));
    auto* list = new QListWidget;
    list->setMinimumHeight(300);
    lay->addWidget(edit);
    lay->addWidget(list);

    struct Entry { QString label; QAction* action; QString file; };
    QList<Entry> entries;
    for (QAction* a : m_paletteActions)
    {
        if (!a->isEnabled() || a == m_actPalette)
            continue;
        QString text = a->text();
        text.remove('&');
        const QString key = a->shortcut().toString(QKeySequence::NativeText);
        entries.append({ key.isEmpty() ? text : text + "    " + key, a, QString() });
    }
    if (!m_projectDir.isEmpty())
    {
        const QString root = QDir(m_projectDir).exists("ui") ? QDir(m_projectDir).filePath("ui") : m_projectDir;
        QDirIterator it(root, { "*.xml", "*.css" }, QDir::Files, QDirIterator::Subdirectories);
        QStringList files;
        while (it.hasNext())
            files << it.next();
        files.sort();
        for (const QString& f : files)
            entries.append({ tr("Open  ") + QDir(root).relativeFilePath(f), nullptr, f });
    }

    auto refill = [&](const QString& filter) {
        list->clear();
        const QStringList words = filter.toLower().split(' ', Qt::SkipEmptyParts);
        int shown = 0;
        for (int i = 0; i < entries.size() && shown < 200; ++i)
        {
            const QString lower = entries[i].label.toLower();
            bool ok = true;
            for (const QString& w : words)
                ok = ok && lower.contains(w);
            if (!ok) continue;
            auto* item = new QListWidgetItem(entries[i].label, list);
            item->setData(Qt::UserRole, i);
            ++shown;
        }
        if (list->count()) list->setCurrentRow(0);
    };
    refill(QString());
    connect(edit, &QLineEdit::textChanged, &dlg, refill);

    int chosen = -1;
    auto accept = [&] {
        if (list->currentItem())
        {
            chosen = list->currentItem()->data(Qt::UserRole).toInt();
            dlg.accept();
        }
    };
    connect(edit, &QLineEdit::returnPressed, &dlg, accept);
    connect(list, &QListWidget::itemActivated, &dlg, accept);
    edit->installEventFilter(&dlg);
    // Up/Down in the text box move the list selection.
    struct Nav : QObject
    {
        QListWidget* list;
        bool eventFilter(QObject*, QEvent* e) override
        {
            if (e->type() != QEvent::KeyPress) return false;
            const int k = static_cast<QKeyEvent*>(e)->key();
            if (k == Qt::Key_Down) { list->setCurrentRow(qMin(list->count() - 1, list->currentRow() + 1)); return true; }
            if (k == Qt::Key_Up) { list->setCurrentRow(qMax(0, list->currentRow() - 1)); return true; }
            return false;
        }
    } nav;
    nav.list = list;
    edit->installEventFilter(&nav);

    dlg.move(geometry().center().x() - 280, geometry().top() + 90);
    edit->setFocus();
    if (dlg.exec() == QDialog::Accepted && chosen >= 0)
    {
        if (entries[chosen].action)
            entries[chosen].action->trigger();
        else
            OpenFile(entries[chosen].file);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool MainWindow::eventFilter(QObject* obj, QEvent* e)
{
    if (obj == this && e->type() == QEvent::WindowActivate && m_tabs)
        QTimer::singleShot(0, this, [this] { CheckExternalChanges(); });
    if (obj == m_console && e->type() == QEvent::KeyPress && !m_history.isEmpty())
    {
        const int k = static_cast<QKeyEvent*>(e)->key();
        if (k == Qt::Key_Up || k == Qt::Key_Down)
        {
            m_historyPos = qBound(0, m_historyPos + (k == Qt::Key_Up ? -1 : 1), m_history.size());
            m_console->setText(m_historyPos < m_history.size() ? m_history[m_historyPos] : QString());
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, e);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    for (int i = 0; i < m_tabs->count(); ++i)
    {
        auto* e = qobject_cast<CodeEditor*>(m_tabs->widget(i));
        if (e && !ConfirmDiscard(e))
        {
            event->ignore();
            return;
        }
    }
    m_session->Stop();
    event->accept();
}

// ---------------------------------------------------------------------------------------------------------------------
void MainWindow::SetPlayMode(bool play)
{
    if (play == m_play)
        return;
    if (play && !m_session->IsReady())
    {
        statusBar()->showMessage(tr("Start the game and show a page first, then switch to Play mode."), 6000);
        m_actEditMode->setChecked(true);
        return;
    }
    m_play = play;
    m_actEditMode->setChecked(!play);
    m_actPlayMode->setChecked(play);
    m_preview->SetPlayMode(play);
    m_paletteDock->setEnabled(!play);
    m_propertiesDock->setEnabled(!play);
    m_hierarchy->setEnabled(!play);
    if (play)
    {
        m_preview->SetSelection(QRectF());
        statusBar()->showMessage(tr("Play mode: use the page like a player. Click Edit mode in the toolbar to go back to editing (F7 only works while Studio has the keyboard, not the game)."), 8000);
        // Best: the game's own window, no copying. Fallback: shared frames shown in the preview, input sent as commands.
        m_embedded = m_session->IsReady() && m_embed->Attach(m_session->ProcessId());
        if (m_embedded)
        {
            m_session->Send("ui_mouse off");           // the real mouse and keyboard now reach the game directly
            m_previewStack->setCurrentWidget(m_embed);
            m_embed->SetActive(true);
        }
        else
            m_session->SetFrameSharing(true, 2);
    }
    else
    {
        if (m_embedded)
        {
            m_embed->SetActive(false);
            m_previewStack->setCurrentWidget(m_preview);
            m_embedded = false;
        }
        m_session->SetFrameSharing(false);
        if (m_session->IsReady())
        {
            m_session->Send("ui_mouse off");
            m_session->Send("ui_dump_json 64");        // the page may have changed while playing
            RefreshPreview(300);
        }
        statusBar()->showMessage(tr("Edit mode: click elements to select them."), 5000);
    }
}

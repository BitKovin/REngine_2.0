#include "TestPanel.h"

#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSplitter>
#include <QStandardPaths>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
    constexpr int RoleScript = Qt::UserRole + 1;
    constexpr int RoleLine = Qt::UserRole + 2;
    constexpr int SecondsPerBackend = 100;

    QColor StateColor(const QString& text)
    {
        if (text == "Passed") return QColor("#6bd48f");
        if (text == "Running") return QColor("#e8c46a");
        if (text == "Not run") return QColor("#8a8f98");
        return QColor("#f0716b");
    }
}

TestPanel::TestPanel(QWidget* parent) : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(6, 6, 6, 6);
    root->setSpacing(6);

    m_hint = new QLabel;
    m_hint->setWordWrap(true);
    m_hint->setText(tr("Tests drive the real game with a script and check what it shows. Pick a script and press Run."));
    root->addWidget(m_hint);

    auto* bar = new QHBoxLayout;
    m_run = new QPushButton(tr("▶  Run selected"));
    m_run->setToolTip(tr("Runs the selected script on the chosen backend."));
    m_runAll = new QPushButton(tr("Run all for this game"));
    m_runAll->setToolTip(tr("Runs every script listed here, one after another."));
    m_stop = new QPushButton(tr("■  Stop"));
    m_stop->setEnabled(false);
    m_backend = new QComboBox;
    m_backend->addItem(tr("DirectX 11 (fastest)"), "dx11");
    m_backend->addItem(tr("DirectX 12"), "dx12");
    m_backend->addItem(tr("Vulkan"), "vk");
    m_backend->addItem(tr("OpenGL"), "gl");
    m_backend->addItem(tr("All four backends"), "dx11,dx12,vk,gl");
    m_backend->setToolTip(tr("Which renderer the test runs on. Use All four to check they draw the same."));
    m_folder = new QPushButton(tr("Open results folder"));
    bar->addWidget(m_run);
    bar->addWidget(m_runAll);
    bar->addWidget(m_stop);
    bar->addSpacing(8);
    bar->addWidget(new QLabel(tr("Renderer:")));
    bar->addWidget(m_backend);
    bar->addStretch(1);
    bar->addWidget(m_folder);
    root->addLayout(bar);

    auto* split = new QSplitter(Qt::Horizontal);
    m_tree = new QTreeWidget;
    m_tree->setColumnCount(3);
    m_tree->setHeaderLabels({ tr("Script"), tr("Result"), tr("Last run") });
    m_tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tree->setRootIsDecorated(true);
    split->addWidget(m_tree);

    auto* right = new QSplitter(Qt::Vertical);
    m_failures = new QTreeWidget;
    m_failures->setColumnCount(3);
    m_failures->setHeaderLabels({ tr("Backend"), tr("Line"), tr("What failed") });
    m_failures->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_failures->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_failures->header()->setStretchLastSection(true);
    m_failures->setRootIsDecorated(false);
    m_failures->setToolTip(tr("Double-click a row to open that line of the script."));
    right->addWidget(m_failures);
    m_output = new QPlainTextEdit;
    m_output->setReadOnly(true);
    m_output->setMaximumBlockCount(3000);
    m_output->setPlaceholderText(tr("The test's output appears here while it runs."));
    QFont mono("Cascadia Mono");
    mono.setStyleHint(QFont::Monospace);
    mono.setPointSize(9);
    m_output->setFont(mono);
    right->addWidget(m_output);
    right->setSizes({ 90, 200 });
    split->addWidget(right);
    split->setSizes({ 380, 420 });
    root->addWidget(split, 1);

    connect(m_run, &QPushButton::clicked, this, &TestPanel::RunSelected);
    connect(m_runAll, &QPushButton::clicked, this, &TestPanel::RunAll);
    connect(m_stop, &QPushButton::clicked, this, &TestPanel::Cancel);
    connect(m_folder, &QPushButton::clicked, this, [this] {
        const QString dir = m_repo + "/Build/ui_runs/" + m_game;
        QDir().mkpath(dir);
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    });
    connect(m_tree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem* it, int) {
        const QString script = it->data(0, RoleScript).toString();
        if (!script.isEmpty())
            emit OpenScript(script, 0);
    });
    connect(m_tree, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* it, QTreeWidgetItem*) {
        if (it && !it->data(0, RoleScript).toString().isEmpty())
            ShowDetails(it->data(0, RoleScript).toString());
    });
    connect(m_failures, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem* it, int) {
        const QString script = it->data(0, RoleScript).toString();
        if (!script.isEmpty())
            emit OpenScript(script, it->data(0, RoleLine).toInt());
    });

    m_process.setProcessChannelMode(QProcess::MergedChannels);
    connect(&m_process, &QProcess::readyRead, this, &TestPanel::OnOutput);
    connect(&m_process, &QProcess::finished, this, &TestPanel::OnFinished);
    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError e) {
        if (e == QProcess::FailedToStart)
        {
            m_logs[m_current] << tr("Could not start Python. Install Python 3 and make sure the command \"python\" works.");
            Finish(State::CouldNotStart);
        }
    });
    m_timeout.setSingleShot(true);
    connect(&m_timeout, &QTimer::timeout, this, [this] {
        m_timedOut = true;
        m_logs[m_current] << tr("Stopped: the test took too long.");
        const qint64 pid = m_process.processId();
        if (pid > 0)
            QProcess::execute("taskkill", { "/PID", QString::number(pid), "/T", "/F" });
    });
    SetBusy(false);
}

TestPanel::~TestPanel()
{
    m_queue.clear();
    if (m_process.state() != QProcess::NotRunning)
    {
        const qint64 pid = m_process.processId();
        if (pid > 0)
            QProcess::execute("taskkill", { "/PID", QString::number(pid), "/T", "/F" });
        m_process.waitForFinished(3000);
    }
}

void TestPanel::SetProject(const QString& gameExe)
{
    m_exe = gameExe;
    m_repo.clear();
    m_game = QFileInfo(gameExe).completeBaseName();
    QDir dir = QFileInfo(gameExe).dir();
    for (int i = 0; i < 8; ++i)
    {
        if (QFileInfo::exists(dir.filePath("Tools/ui/run_ui_tests.py")))
        {
            m_repo = dir.absolutePath();
            break;
        }
        if (!dir.cdUp())
            break;
    }
    Discover();
}

void TestPanel::Discover()
{
    m_tree->clear();
    m_items.clear();
    const bool ok = !m_repo.isEmpty() && QFileInfo::exists(m_exe);
    m_run->setEnabled(ok);
    m_runAll->setEnabled(ok);
    m_folder->setEnabled(ok);
    if (!ok)
    {
        m_hint->setText(tr("Tests need the game's source folder (the one that contains Tools/ui) and a built game program. "
                           "Open the project from a normal checkout to use them."));
        return;
    }
    m_hint->setText(tr("Tests drive the real game with a script and check what it shows. Pick a script and press Run. "
                       "While a test runs, the live preview game is paused and comes back afterwards."));

    const QString groups[2] = { "Common", m_game };
    for (const QString& group : groups)
    {
        const QString root = m_repo + "/Tools/ui/scripts/" + group;
        if (!QDir(root).exists())
            continue;
        auto* top = new QTreeWidgetItem(m_tree, { group == "Common" ? tr("Shared by all games") : tr("%1 only").arg(group) });
        top->setFlags(top->flags() & ~Qt::ItemIsSelectable);
        QDirIterator it(root, { "*.uitest" }, QDir::Files, QDirIterator::Subdirectories);
        QStringList files;
        while (it.hasNext())
            files << QDir::cleanPath(it.next());
        files.sort();
        for (const QString& f : files)
        {
            auto* item = new QTreeWidgetItem(top, { QFileInfo(f).completeBaseName(), QString(), QString() });
            item->setData(0, RoleScript, f);
            item->setToolTip(0, f);
            m_items[f] = item;
            UpdateItem(f);
        }
        top->setExpanded(true);
    }
    if (m_items.isEmpty())
        new QTreeWidgetItem(m_tree, { tr("No .uitest scripts were found.") });
}

QString TestPanel::BackendList() const
{
    return m_backend->currentData().toString();
}

QString TestPanel::RunFolder(const QString& script, const QString& backend) const
{
    return m_repo + "/Build/ui_runs/" + m_game + "/" + QFileInfo(script).completeBaseName() + "/studio/" + backend;
}

QString TestPanel::StateText(State s)
{
    switch (s)
    {
    case State::NotRun: return "Not run";
    case State::Running: return "Running";
    case State::Passed: return "Passed";
    case State::Failed: return "Failed";
    case State::TimedOut: return "Timed out";
    case State::Cancelled: return "Stopped";
    case State::CouldNotStart: return "Could not start";
    }
    return QString();
}

void TestPanel::UpdateItem(const QString& script)
{
    QTreeWidgetItem* item = m_items.value(script);
    if (!item)
        return;
    const Result r = m_results.value(script);
    QString text = StateText(r.state);
    if (!r.backends.isEmpty() && r.state != State::Running && r.state != State::NotRun)
        text += "  (" + r.backends.join(", ") + ")";
    item->setText(1, text);
    item->setForeground(1, StateColor(StateText(r.state)));
    item->setText(2, r.when.isValid() ? r.when.toString("HH:mm:ss") : QString());
}

void TestPanel::SetBusy(bool busy)
{
    const bool ok = !m_repo.isEmpty() && QFileInfo::exists(m_exe);
    m_run->setEnabled(ok && !busy);
    m_runAll->setEnabled(ok && !busy);
    m_stop->setEnabled(busy);
    m_backend->setEnabled(!busy);
}

void TestPanel::RunSelected()
{
    QTreeWidgetItem* it = m_tree->currentItem();
    const QString script = it ? it->data(0, RoleScript).toString() : QString();
    if (script.isEmpty())
    {
        m_hint->setText(tr("Pick a script in the list first."));
        return;
    }
    Enqueue({ script });
}

void TestPanel::RunAll()
{
    Enqueue(m_items.keys());
}

void TestPanel::Enqueue(const QStringList& scripts)
{
    if (scripts.isEmpty() || IsRunning())
        return;
    if (QStandardPaths::findExecutable("python").isEmpty())
    {
        m_hint->setText(tr("Python 3 was not found. Install it and make sure the command \"python\" works, then try again."));
        return;
    }
    m_queue = scripts;
    m_cancelled = false;
    for (const QString& s : scripts)
    {
        m_results[s] = Result();
        m_logs.remove(s);
        UpdateItem(s);
    }
    SetBusy(true);
    emit RunStarting();
    StartNext();
}

void TestPanel::StartNext()
{
    if (m_queue.isEmpty())
    {
        SetBusy(false);
        m_current.clear();
        emit RunFinished();
        return;
    }
    m_current = m_queue.takeFirst();
    m_timedOut = false;
    m_backendStatus.clear();
    m_results[m_current].state = State::Running;
    UpdateItem(m_current);
    m_failures->clear();
    m_output->clear();
    if (QTreeWidgetItem* it = m_items.value(m_current))
        m_tree->setCurrentItem(it);

    const QString backends = BackendList();
    const QStringList args = { "-I", m_repo + "/Tools/ui/run_ui_tests.py", "run", "--game", m_game, "--script", m_current,
                               "--label", "studio", "--backends", backends };
    m_logs[m_current] << ("python " + args.join(' '));
    m_output->appendPlainText(m_logs[m_current].last());
    m_process.setWorkingDirectory(m_repo);
    m_timeout.start((backends.count(',') + 1) * SecondsPerBackend * 1000 + 30000);
    m_process.start("python", args);
}

void TestPanel::Cancel()
{
    m_cancelled = true;
    m_queue.clear();
    const qint64 pid = m_process.processId();
    if (pid > 0)
        QProcess::execute("taskkill", { "/PID", QString::number(pid), "/T", "/F" });
}

void TestPanel::OnOutput()
{
    static const QRegularExpression backendLine(R"(^\[(\w+)\]\s+(.*)$)");
    while (m_process.canReadLine())
    {
        const QString line = QString::fromLocal8Bit(m_process.readLine()).trimmed();
        if (line.isEmpty())
            continue;
        m_logs[m_current] << line;
        QTreeWidgetItem* cur = m_tree->currentItem();
        if (cur && cur->data(0, RoleScript).toString() == m_current)
            m_output->appendPlainText(line);

        const QRegularExpressionMatch m = backendLine.match(line);
        if (!m.hasMatch())
            continue;
        const QString backend = m.captured(1);
        const QString rest = m.captured(2);
        if (rest == "RESULT PASS") m_backendStatus[backend] = "passed";
        else if (rest.startsWith("RESULT FAIL")) m_backendStatus[backend] = "failed";
        else if (rest.startsWith("TIMEOUT")) m_backendStatus[backend] = "timed out";
        else if (rest.startsWith("FAILED")) m_backendStatus[backend] = "no output";
        else if (rest.startsWith("RESULT")) m_backendStatus[backend] = "failed";
    }
}

void TestPanel::OnFinished(int code, QProcess::ExitStatus)
{
    OnOutput();
    State state;
    if (m_cancelled)
        state = State::Cancelled;
    else if (m_timedOut)
        state = State::TimedOut;
    else if (m_backendStatus.isEmpty())
        state = State::CouldNotStart;     // the runner printed an error before any backend ran (for example: game not built)
    else
    {
        state = State::Passed;
        for (auto it = m_backendStatus.cbegin(); it != m_backendStatus.cend(); ++it)
        {
            if (it.value() == "timed out")
                state = State::TimedOut;
            else if (it.value() != "passed" && state == State::Passed)
                state = State::Failed;
        }
    }
    (void)code;
    Finish(state);
}

void TestPanel::Finish(State state)
{
    m_timeout.stop();
    if (m_current.isEmpty())
        return;
    Result& r = m_results[m_current];
    r.state = state;
    r.when = QDateTime::currentDateTime();
    r.backends.clear();
    for (auto it = m_backendStatus.cbegin(); it != m_backendStatus.cend(); ++it)
        r.backends << it.key() + " " + it.value();
    ReadFailures(m_current);
    UpdateItem(m_current);
    if (state == State::CouldNotStart)
        m_hint->setText(tr("The test could not start. Is the game built? The output below has the reason."));
    if (m_process.state() != QProcess::NotRunning)
        return;                              // errorOccurred fired first; finished() will not come
    StartNext();
}

void TestPanel::ReadFailures(const QString& script)
{
    // result.txt: one "FAIL <line> <text>" per failed command, then RESULT PASS / RESULT FAIL.
    m_failures->clear();
    for (const QString& backend : m_backendStatus.keys())
    {
        QFile f(RunFolder(script, backend) + "/result.txt");
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;
        const QStringList lines = QString::fromUtf8(f.readAll()).split('\n');
        for (const QString& raw : lines)
        {
            const QString line = raw.trimmed();
            if (!line.startsWith("FAIL "))
                continue;
            const QString rest = line.mid(5);
            const int space = rest.indexOf(' ');
            const int number = rest.left(space < 0 ? rest.size() : space).toInt();
            auto* it = new QTreeWidgetItem(m_failures, { backend, QString::number(number), space < 0 ? QString() : rest.mid(space + 1) });
            it->setData(0, RoleScript, script);
            it->setData(0, RoleLine, number);
            it->setForeground(2, StateColor("Failed"));
        }
    }
}

void TestPanel::ShowDetails(const QString& script)
{
    if (script == m_current && m_process.state() != QProcess::NotRunning)
        return;
    m_output->setPlainText(m_logs.value(script).join('\n'));
    // Failures belong to the last finished run of this script.
    m_failures->clear();
    const Result r = m_results.value(script);
    if (r.state == State::Failed || r.state == State::Passed)
    {
        const QMap<QString, QString> saved = m_backendStatus;
        m_backendStatus.clear();
        for (const QString& b : r.backends)
            m_backendStatus[b.section(' ', 0, 0)] = b.section(' ', 1);
        ReadFailures(script);
        m_backendStatus = saved;
    }
}

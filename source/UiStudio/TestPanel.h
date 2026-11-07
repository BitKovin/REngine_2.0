#pragma once

#include <QDateTime>
#include <QMap>
#include <QProcess>
#include <QTimer>
#include <QWidget>

class QComboBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

// Test runner. Finds the repository's *.uitest scripts, runs them through Tools/ui/run_ui_tests.py
// (the established runner) in a child process and shows pass / fail per script and backend.
// Failed commands from result.txt are listed with their script line; double-click opens the line.
class TestPanel : public QWidget
{
    Q_OBJECT
public:
    explicit TestPanel(QWidget* parent = nullptr);
    ~TestPanel() override;

    // Works out the repository, the game name and the script folders from the game program's path.
    void SetProject(const QString& gameExe);
    bool IsRunning() const { return m_process.state() != QProcess::NotRunning || !m_queue.isEmpty(); }

signals:
    void OpenScript(const QString& file, int line);
    void RunStarting();          // the game program is about to be used by a test; Studio stops its preview game
    void RunFinished();

private:
    enum class State { NotRun, Running, Passed, Failed, TimedOut, Cancelled, CouldNotStart };
    struct Result
    {
        State state = State::NotRun;
        QStringList backends;                  // "dx11 passed", "vk failed", ...
        QDateTime when;
    };

    void Discover();
    void RunSelected();
    void RunAll();
    void Enqueue(const QStringList& scripts);
    void StartNext();
    void Cancel();
    void OnOutput();
    void OnFinished(int code, QProcess::ExitStatus status);
    void Finish(State state);
    void ReadFailures(const QString& script);
    void UpdateItem(const QString& script);
    void ShowDetails(const QString& script);
    void SetBusy(bool busy);
    QString RunFolder(const QString& script, const QString& backend) const;
    QString BackendList() const;
    static QString StateText(State s);

    QTreeWidget* m_tree = nullptr;
    QTreeWidget* m_failures = nullptr;
    QPlainTextEdit* m_output = nullptr;
    QComboBox* m_backend = nullptr;
    QPushButton* m_run = nullptr;
    QPushButton* m_runAll = nullptr;
    QPushButton* m_stop = nullptr;
    QPushButton* m_folder = nullptr;
    QLabel* m_hint = nullptr;

    QString m_repo;
    QString m_game;
    QString m_exe;
    QMap<QString, Result> m_results;           // script file -> last result
    QMap<QString, QTreeWidgetItem*> m_items;
    QMap<QString, QStringList> m_logs;         // script file -> process output

    QProcess m_process;
    QTimer m_timeout;
    QStringList m_queue;
    QString m_current;
    bool m_cancelled = false;
    bool m_timedOut = false;
    QMap<QString, QString> m_backendStatus;    // backend -> "passed" / "failed" / "timed out" ... for the current script
};

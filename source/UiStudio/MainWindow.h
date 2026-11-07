#pragma once

#include <QDockWidget>
#include <QJsonDocument>
#include <QMainWindow>
#include <QMap>
#include <QRectF>
#include <QTimer>

class CodeEditor;
class GameSession;
class PreviewView;
class QAction;
class QFileSystemModel;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QStackedWidget;
class QTabWidget;
class QTableWidget;
class QTreeView;
class QTreeWidget;
class QTreeWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow() override;

    // UiStudio <GameData folder> [game program]: opens straight into the project.
    void OpenFromArguments(const QStringList& args);

protected:
    void closeEvent(QCloseEvent*) override;
    bool eventFilter(QObject*, QEvent*) override;

private slots:
    void OnOpenProject();

private:
    // setup
    QWidget* BuildWelcome();
    void BuildWorkspace();
    void BuildActions();
    void BuildMenus();
    void RebuildRecent();

    // project
    bool OpenProject(const QString& gameDataDir, const QString& exe);
    QString GuessExecutable(const QString& gameDataDir) const;
    QString GamePathOf(const QString& file) const;     // "GameData/ui/x.xml"
    QString FileOfGamePath(const QString& gamePath) const;

    // files
    CodeEditor* OpenFile(const QString& path, int line = 0, int column = 0);
    CodeEditor* CurrentEditor() const;
    bool SaveEditor(CodeEditor* editor);
    bool SaveAll();
    void CloseTab(int index);
    bool ConfirmDiscard(CodeEditor* editor);
    void NewFile();
    void CheckExternalChanges();
    void UpdateTabTitle(CodeEditor* editor);
    void CopyToGame(const QString& file);

    // game / preview
    void StartGame();
    void StopGame();
    void ShowInPreview();                       // load the current page into the game and refresh everything
    void RefreshPreview(int delayMs = 700);
    void SetGameState(const QString& text, const char* color);
    void RunLint();
    void OnJson(const QString& tag, const QJsonDocument& doc);
    void FillHierarchy(const QJsonDocument& doc);
    void FillInspector(const QJsonObject& computed);
    void FillProblems(const QJsonDocument& doc);
    void SelectByPath(const QString& path);
    void SelectItem(QTreeWidgetItem* item, bool jumpToSource);
    void ApplyProblemsToEditors();
    void AppendLog(const QString& line);
    void RunConsoleCommand();

    // command palette
    void ShowPalette();

    QStackedWidget* m_stack = nullptr;
    QStackedWidget* m_centerStack = nullptr;
    QWidget* m_welcome = nullptr;
    QLineEdit* m_projectEdit = nullptr;
    QLineEdit* m_exeEdit = nullptr;
    QListWidget* m_recent = nullptr;

    QString m_projectDir;      // the GameData folder being edited
    QString m_exe;
    QString m_previewPage;     // game path of the page currently shown

    QTabWidget* m_tabs = nullptr;
    QTreeView* m_tree = nullptr;
    QFileSystemModel* m_model = nullptr;

    PreviewView* m_preview = nullptr;
    QTreeWidget* m_hierarchy = nullptr;
    QTableWidget* m_inspector = nullptr;
    QTableWidget* m_problems = nullptr;
    QPlainTextEdit* m_log = nullptr;
    QLineEdit* m_console = nullptr;
    QStringList m_history;
    int m_historyPos = 0;
    QLabel* m_stateLabel = nullptr;
    QLabel* m_hintLabel = nullptr;
    QTabWidget* m_bottom = nullptr;

    GameSession* m_session = nullptr;
    QTimer m_refreshTimer;
    QTimer m_lintTimer;
    bool m_pendingShow = false;
    QString m_selectedPath;
    QMap<QString, QMap<int, QPair<bool, QString>>> m_problemsByFile;   // game path -> line -> (isError, message)

    QAction* m_actSave = nullptr;
    QAction* m_actSaveAll = nullptr;
    QAction* m_actNew = nullptr;
    QAction* m_actRun = nullptr;
    QAction* m_actPreview = nullptr;
    QAction* m_actLint = nullptr;
    QAction* m_actPalette = nullptr;
    QAction* m_actOpen = nullptr;
    QAction* m_actHelp = nullptr;
    QAction* m_actCloseTab = nullptr;
    QList<QAction*> m_paletteActions;
};

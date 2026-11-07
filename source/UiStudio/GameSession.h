#pragma once

#include <QDateTime>
#include <QImage>
#include <QJsonDocument>
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QVector>

// One running game process driven through its console. Studio never links game code:
// it starts the executable with -headless -console_stdin, writes console commands to stdin
// and reads the files the game writes (screenshots and JSON under SaveData).
class GameSession : public QObject
{
    Q_OBJECT
public:
    explicit GameSession(QObject* parent = nullptr);
    ~GameSession() override;

    // exe: path to the game executable. workDir: the folder the game expects as its working directory (holds GameData and SaveData).
    bool Start(const QString& exe, const QString& workDir, QString* error);
    void Stop();
    bool IsRunning() const { return m_process.state() == QProcess::Running; }
    bool IsReady() const { return m_ready; }

    void Send(const QString& command);

    // Each request deletes the old result file, sends the command, then waits for the new file.
    void RequestScreenshot();
    void RequestJson(const QString& command, const QString& relFile, const QString& tag);

    QString WorkDir() const { return m_workDir; }

signals:
    void Started();
    void Ready();                    // the game printed something that shows its console is live
    void Stopped(int exitCode, bool crashed);
    void LogLine(const QString& line);
    void ScreenshotReady(const QImage& image);
    void JsonReady(const QString& tag, const QJsonDocument& doc);
    void RequestFailed(const QString& what);

private slots:
    void OnStdout();
    void Poll();

private:
    struct Pending
    {
        QString command;
        QString file;
        QString tag;       // empty for a screenshot
        QDateTime since;
    };

    void Enqueue(const Pending& p);

    QProcess m_process;
    QTimer m_poll;
    QTimer m_probe;
    QString m_workDir;
    QByteArray m_buffer;
    QVector<Pending> m_pending;
    bool m_ready = false;
    bool m_stopping = false;
};

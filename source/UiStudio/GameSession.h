#pragma once

#include <QDateTime>
#include <QFile>
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
    qint64 ProcessId() const { return qint64(m_process.processId()); }
    bool IsRunning() const { return m_process.state() == QProcess::Running; }
    bool IsReady() const { return m_ready; }

    void Send(const QString& command);

    // Each request deletes the old result file, sends the command, then waits for the new file.
    // fast: a half-size uncompressed copy for live (Play mode) previews; otherwise a full-size PNG.
    void RequestScreenshot(bool fast = false);
    void RequestJson(const QString& command, const QString& relFile, const QString& tag);

    // Live view: the game copies every frame into SaveData/ui_frame.bin and Studio reads that file through a memory map.
    // No files per frame, no PNG, no polling of commands. FrameReady fires for each new frame.
    void SetFrameSharing(bool on, int downscale = 2);
    bool FrameSharing() const { return m_sharing; }

    QString WorkDir() const { return m_workDir; }

signals:
    void Started();
    void Ready();                    // the game printed something that shows its console is live
    void Stopped(int exitCode, bool crashed);
    void LogLine(const QString& line);
    void ScreenshotReady(const QImage& image);
    void FrameReady(const QImage& image);
    void JsonReady(const QString& tag, const QJsonDocument& doc);
    void RequestFailed(const QString& what);

private slots:
    void OnStdout();
    void Poll();
    void ReadFrame();

private:
    struct Pending
    {
        QString command;
        QString file;
        QString tag;       // empty for a screenshot
        QDateTime since;
    };

    void Enqueue(const Pending& p);
    void CloseFrameFile();

    QProcess m_process;
    QTimer m_poll;
    QTimer m_probe;
    QString m_workDir;
    QByteArray m_buffer;
    QVector<Pending> m_pending;
    bool m_ready = false;
    bool m_stopping = false;

    bool m_sharing = false;
    QTimer m_frameTimer;
    QFile m_frameFile;
    uchar* m_frameMap = nullptr;
    qint64 m_frameMapSize = 0;
    quint32 m_lastSequence = 0;
};

#include "GameSession.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

GameSession::GameSession(QObject* parent) : QObject(parent)
{
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &GameSession::OnStdout);
    connect(&m_process, &QProcess::started, this, &GameSession::Started);
    connect(&m_process, &QProcess::finished, this, [this](int code, QProcess::ExitStatus status) {
        m_ready = false;
        m_pending.clear();
        m_poll.stop();
        m_probe.stop();
        emit Stopped(code, status == QProcess::CrashExit && !m_stopping);
        m_stopping = false;
    });
    m_poll.setInterval(120);
    connect(&m_poll, &QTimer::timeout, this, &GameSession::Poll);
    // The console only answers once the engine has finished starting, so ask until it does.
    m_probe.setInterval(800);
    connect(&m_probe, &QTimer::timeout, this, [this] { Send("ui_stack"); });
}

GameSession::~GameSession()
{
    Stop();
}

bool GameSession::Start(const QString& exe, const QString& workDir, QString* error)
{
    if (IsRunning())
        return true;
    if (!QFileInfo::exists(exe))
    {
        if (error) *error = QString("The game executable was not found:\n%1").arg(exe);
        return false;
    }
    m_workDir = workDir;
    m_buffer.clear();
    m_ready = false;
    m_stopping = false;
    m_process.setWorkingDirectory(workDir);
    // -headless: hidden window, no audio, no input capture. -console_stdin: console commands come from this process.
    m_process.start(exe, { "-headless", "-console_stdin" });
    if (!m_process.waitForStarted(5000))
    {
        if (error) *error = "The game process could not be started: " + m_process.errorString();
        return false;
    }
    m_probe.start();
    return true;
}

void GameSession::Stop()
{
    if (m_process.state() == QProcess::NotRunning)
        return;
    m_stopping = true;
    m_process.write("quit\n");
    if (!m_process.waitForFinished(4000))
    {
        // The game did not leave on its own. It is our own child process, so ending it is safe.
        m_process.terminate();
        if (!m_process.waitForFinished(3000))
            m_process.kill();
    }
}

void GameSession::Send(const QString& command)
{
    if (!IsRunning())
        return;
    m_process.write(command.toUtf8() + "\n");
}

void GameSession::OnStdout()
{
    m_buffer += m_process.readAllStandardOutput();
    int nl;
    while ((nl = m_buffer.indexOf('\n')) >= 0)
    {
        const QString line = QString::fromUtf8(m_buffer.left(nl)).trimmed();
        m_buffer.remove(0, nl + 1);
        if (line.isEmpty())
            continue;
        if (!m_ready && line.contains("[UiStack]"))
        {
            m_ready = true;
            m_probe.stop();
            emit Ready();
        }
        emit LogLine(line);
    }
}

void GameSession::Enqueue(const Pending& p)
{
    const QString full = QDir(m_workDir).filePath(p.file);
    QFile::remove(full);
    Pending q = p;
    q.since = QDateTime::currentDateTime();
    m_pending.push_back(q);
    Send(p.command);
    if (!m_poll.isActive())
        m_poll.start();
}

void GameSession::RequestScreenshot()
{
    // "screenshot <name>" writes SaveData/screenshots/<name>.png on the next rendered frame.
    Enqueue({ "screenshot studio_preview", "SaveData/screenshots/studio_preview.png", QString(), {} });
}

void GameSession::RequestJson(const QString& command, const QString& relFile, const QString& tag)
{
    Enqueue({ command, relFile, tag, {} });
}

void GameSession::Poll()
{
    for (int i = 0; i < m_pending.size();)
    {
        const Pending& p = m_pending[i];
        const QString full = QDir(m_workDir).filePath(p.file);
        bool done = false;
        if (QFileInfo::exists(full))
        {
            if (p.tag.isEmpty())
            {
                QImage image(full);
                if (!image.isNull())
                {
                    emit ScreenshotReady(image);
                    done = true;
                }
            }
            else
            {
                QFile f(full);
                if (f.open(QIODevice::ReadOnly))
                {
                    QJsonParseError parseError;
                    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &parseError);
                    if (parseError.error == QJsonParseError::NoError)
                    {
                        emit JsonReady(p.tag, doc);
                        done = true;
                    }
                }
            }
        }
        if (done)
        {
            m_pending.removeAt(i);
            continue;
        }
        if (p.since.msecsTo(QDateTime::currentDateTime()) > 8000)
        {
            emit RequestFailed(p.tag.isEmpty() ? "screenshot" : p.tag);
            m_pending.removeAt(i);
            continue;
        }
        ++i;
    }
    if (m_pending.isEmpty())
        m_poll.stop();
}

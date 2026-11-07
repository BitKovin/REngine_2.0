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
        m_sharing = false;
        m_frameTimer.stop();
        CloseFrameFile();
        emit Stopped(code, status == QProcess::CrashExit && !m_stopping);
        m_stopping = false;
    });
    m_poll.setInterval(25);
    connect(&m_poll, &QTimer::timeout, this, &GameSession::Poll);
    m_frameTimer.setInterval(5);
    connect(&m_frameTimer, &QTimer::timeout, this, &GameSession::ReadFrame);
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
    // -no_map: the game opens no level, so the preview shows the interface on an empty world (games that ignore it open their usual map).
    // -lock_window: settings such as resolution or fullscreen must not resize the window Studio has embedded.
    // -free_cursor: the game keeps a normal mouse cursor instead of locking it while no menu is open.
    m_process.start(exe, { "-headless", "-console_stdin", "-no_map", "-free_cursor", "-lock_window" });
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

void GameSession::RequestScreenshot(bool fast)
{
    if (fast)
    {
        Enqueue({ "screenshot studio_play.bmp 2", "SaveData/screenshots/studio_play.bmp", QString(), {} });
        return;
    }
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
                bool complete = true;
                if (p.file.endsWith(".bmp"))
                {
                    // The game may still be writing: a BMP is complete when the file is as long as its header says.
                    QFile f(full);
                    QByteArray head;
                    if (f.open(QIODevice::ReadOnly))
                        head = f.read(6);
                    const quint32 expected = head.size() == 6
                        ? quint32(quint8(head[2])) | quint32(quint8(head[3])) << 8 | quint32(quint8(head[4])) << 16 | quint32(quint8(head[5])) << 24
                        : 0;
                    complete = expected > 0 && quint64(QFileInfo(full).size()) == expected;
                }
                QImage image = complete ? QImage(full) : QImage();
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

// ---------------------------------------------------------------------------------------------------------------------
void GameSession::SetFrameSharing(bool on, int downscale)
{
    if (on == m_sharing)
        return;
    m_sharing = on;
    if (on)
    {
        CloseFrameFile();
        m_lastSequence = 0;
        QFile::remove(QDir(m_workDir).filePath("SaveData/ui_frame.bin"));
        Send(QString("frame_share %1").arg(downscale));
        m_frameTimer.start();
    }
    else
    {
        m_frameTimer.stop();
        Send("frame_share off");
        CloseFrameFile();
    }
}

void GameSession::CloseFrameFile()
{
    if (m_frameMap)
        m_frameFile.unmap(m_frameMap);
    m_frameMap = nullptr;
    m_frameMapSize = 0;
    if (m_frameFile.isOpen())
        m_frameFile.close();
}

// Header: 'UIFR', width, height, stride, sequence (odd while the game writes), 12 spare bytes; BGRA rows follow.
void GameSession::ReadFrame()
{
    if (!m_frameFile.isOpen())
    {
        m_frameFile.setFileName(QDir(m_workDir).filePath("SaveData/ui_frame.bin"));
        if (!m_frameFile.open(QIODevice::ReadOnly))
            return;                                   // the game has not published the first frame yet
    }
    const qint64 size = m_frameFile.size();
    if (size < 32)
        return;
    if (!m_frameMap || m_frameMapSize != size)
    {
        if (m_frameMap)
            m_frameFile.unmap(m_frameMap);
        m_frameMap = m_frameFile.map(0, size);
        m_frameMapSize = m_frameMap ? size : 0;
        if (!m_frameMap)
            return;
    }
    const volatile quint32* header = reinterpret_cast<const volatile quint32*>(m_frameMap);
    if (header[0] != 0x52465549u)
        return;
    const quint32 w = header[1], h = header[2], stride = header[3];
    const quint32 before = header[4];
    if ((before & 1u) || before == m_lastSequence || w == 0 || h == 0 || stride < w * 4 || qint64(32) + qint64(stride) * h > size)
        return;
    // Copy out of the mapping, then keep the copy only if the game did not start another frame meanwhile.
    QImage image(int(w), int(h), QImage::Format_RGB32);
    for (quint32 y = 0; y < h; ++y)
        memcpy(image.scanLine(int(y)), m_frameMap + 32 + qint64(stride) * y, w * 4);
    if (header[4] != before)
        return;
    m_lastSequence = before;
    emit FrameReady(image);
}

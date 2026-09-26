#include "speedtestrunner.h"

#include "paths.h"

#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QProcess>
#include <QProcessEnvironment>

SpeedTestRunner::SpeedTestRunner(QObject *parent)
    : QObject(parent)
{
}

SpeedTestRunner::~SpeedTestRunner()
{
    cancel();
}

void SpeedTestRunner::setHistory(HistoryModel *history)
{
    if (m_history == history)
        return;
    m_history = history;
    emit historyChanged();
}

void SpeedTestRunner::setBackend(const QString &backend)
{
    if (m_backend == backend)
        return;
    m_backend = backend;
    emit optionsChanged();
}

void SpeedTestRunner::start()
{
    if (busy())
        return;
    detachProcess();
    reset();
    const QString helper = Paths::helper();
    if (!QFileInfo(helper).isExecutable()) {
        fail(tr("Helper not found or not executable: %1").arg(helper));
        return;
    }
    setPhase(Searching);
    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &SpeedTestRunner::drain);
    connect(m_process, &QProcess::finished, this, &SpeedTestRunner::onFinished);
    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart)
            return;
        fail(tr("Could not start the speed test helper"));
        m_process->deleteLater();
        m_process = nullptr;
    });
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("XSPEEDTEST_BACKEND"), m_backend);
    m_process->setProcessEnvironment(env);
    m_process->start(helper, {});
}

void SpeedTestRunner::detachProcess()
{
    if (!m_process)
        return;
    QProcess *process = m_process;
    m_process = nullptr;
    process->disconnect(this);
    process->kill();
    process->waitForFinished(1000);
    process->deleteLater();
    m_buffer.clear();
}

void SpeedTestRunner::cancel()
{
    detachProcess();
    if (busy()) {
        m_mbps = 0;
        m_progress = 0;
        emit progressChanged();
        setPhase(Idle);
    }
}

void SpeedTestRunner::reset()
{
    m_mbps = m_progress = m_ping = m_jitter = m_download = m_upload = 0;
    m_server.clear();
    m_isp.clear();
    m_error.clear();
    m_buffer.clear();
    emit progressChanged();
    emit resultsChanged();
}

void SpeedTestRunner::setPhase(Phase phase)
{
    if (m_phase == phase)
        return;
    m_phase = phase;
    emit phaseChanged();
}

void SpeedTestRunner::fail(const QString &message)
{
    m_error = message;
    if (m_phase == Error)
        emit phaseChanged();
    else
        setPhase(Error);
}

void SpeedTestRunner::drain()
{
    m_buffer += m_process->readAllStandardOutput();
    qsizetype newline;
    while ((newline = m_buffer.indexOf('\n')) >= 0) {
        parseLine(m_buffer.left(newline));
        m_buffer.remove(0, newline + 1);
    }
}

void SpeedTestRunner::onFinished()
{
    drain();
    if (!m_buffer.isEmpty())
        parseLine(m_buffer);
    m_buffer.clear();
    m_process->deleteLater();
    m_process = nullptr;
    if (m_phase != Done && m_phase != Error)
        fail(tr("Test ended unexpectedly"));
}

void SpeedTestRunner::parseLine(const QByteArray &line)
{
    const QJsonDocument doc = QJsonDocument::fromJson(line);
    if (!doc.isObject()) {
        qWarning() << "Ignoring unparseable helper output:" << line;
        return;
    }
    const QJsonObject o = doc.object();
    const QString phase = o.value(QStringLiteral("phase")).toString();
    if (phase.isEmpty())
        return;
    const auto num = [&o](const char *key) { return o.value(QLatin1String(key)).toDouble(); };
    const auto str = [&o](const char *key) { return o.value(QLatin1String(key)).toString(); };

    if (phase == QLatin1String("ping")) {
        m_progress = num("progress");
        emit progressChanged();
        setPhase(Searching);
    } else if (phase == QLatin1String("server")) {
        m_server = str("name");
        if (!str("country").isEmpty())
            m_server += QStringLiteral(", ") + str("country");
        if (!str("sponsor").isEmpty())
            m_server += QStringLiteral(" (") + str("sponsor") + QLatin1Char(')');
        m_ping = num("ping");
        emit resultsChanged();
    } else if (phase == QLatin1String("jitter")) {
        m_jitter = num("jitter");
        emit resultsChanged();
    } else if (phase == QLatin1String("download") || phase == QLatin1String("upload")) {
        m_mbps = num("mbps");
        m_progress = num("progress");
        const bool downloading = phase == QLatin1String("download");
        (downloading ? m_download : m_upload) = m_mbps;
        emit progressChanged();
        emit resultsChanged();
        setPhase(downloading ? Downloading : Uploading);
    } else if (phase == QLatin1String("done")) {
        m_ping = num("ping");
        m_jitter = num("jitter");
        m_download = num("download");
        m_upload = num("upload");
        m_server = str("server");
        m_isp = str("isp");
        m_mbps = m_download;
        m_progress = 1;
        emit progressChanged();
        emit resultsChanged();
        setPhase(Done);
        if (m_history) {
            m_history->add({{QStringLiteral("timestamp"), str("timestamp")},
                            {QStringLiteral("download"), m_download},
                            {QStringLiteral("upload"), m_upload},
                            {QStringLiteral("ping"), m_ping},
                            {QStringLiteral("jitter"), m_jitter},
                            {QStringLiteral("server"), m_server}});
        }
    } else if (phase == QLatin1String("error")) {
        fail(str("message"));
    }
}

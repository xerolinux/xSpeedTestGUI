#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml/qqmlregistration.h>

#include "historymodel.h"

class QProcess;

class SpeedTestRunner : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(Phase phase READ phase NOTIFY phaseChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY phaseChanged)
    Q_PROPERTY(double mbps READ mbps NOTIFY progressChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(double ping READ ping NOTIFY resultsChanged)
    Q_PROPERTY(double jitter READ jitter NOTIFY resultsChanged)
    Q_PROPERTY(double download READ download NOTIFY resultsChanged)
    Q_PROPERTY(double upload READ upload NOTIFY resultsChanged)
    Q_PROPERTY(QString server READ server NOTIFY resultsChanged)
    Q_PROPERTY(QString isp READ isp NOTIFY resultsChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY phaseChanged)
    Q_PROPERTY(QString backend READ backend WRITE setBackend NOTIFY optionsChanged)
    Q_PROPERTY(HistoryModel *history READ history WRITE setHistory NOTIFY historyChanged)

public:
    enum Phase { Idle, Searching, Downloading, Uploading, Done, Error };
    Q_ENUM(Phase)

    explicit SpeedTestRunner(QObject *parent = nullptr);
    ~SpeedTestRunner() override;

    Phase phase() const { return m_phase; }
    bool busy() const { return m_phase == Searching || m_phase == Downloading || m_phase == Uploading; }
    double mbps() const { return m_mbps; }
    double progress() const { return m_progress; }
    double ping() const { return m_ping; }
    double jitter() const { return m_jitter; }
    double download() const { return m_download; }
    double upload() const { return m_upload; }
    QString server() const { return m_server; }
    QString isp() const { return m_isp; }
    QString errorString() const { return m_error; }
    QString backend() const { return m_backend; }
    void setBackend(const QString &backend);
    HistoryModel *history() const { return m_history; }
    void setHistory(HistoryModel *history);

    Q_INVOKABLE void start();
    Q_INVOKABLE void cancel();
    void parseLine(const QByteArray &line);

signals:
    void phaseChanged();
    void progressChanged();
    void resultsChanged();
    void historyChanged();
    void optionsChanged();

private:
    void setPhase(Phase phase);
    void fail(const QString &message);
    void reset();
    void detachProcess();
    void drain();
    void onFinished();

    Phase m_phase = Idle;
    double m_mbps = 0;
    double m_progress = 0;
    double m_ping = 0;
    double m_jitter = 0;
    double m_download = 0;
    double m_upload = 0;
    QString m_server;
    QString m_isp;
    QString m_error;
    QString m_backend = QStringLiteral("auto");
    QPointer<HistoryModel> m_history;
    QProcess *m_process = nullptr;
    QByteArray m_buffer;
};

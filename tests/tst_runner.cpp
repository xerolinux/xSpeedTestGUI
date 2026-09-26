#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

#include "historymodel.h"
#include "speedtestrunner.h"

class RunnerTest : public QObject
{
    Q_OBJECT

    static void useStub(const char *mode, const QString &countFile = {})
    {
        qputenv("XSPEEDTEST_HELPER", STUB_HELPER);
        qputenv("STUB_MODE", mode);
        qputenv("STUB_COUNT", countFile.toUtf8());
    }

private slots:
    void parseLineTracksEachPhase()
    {
        SpeedTestRunner r;
        r.parseLine("{\"phase\":\"ping\",\"progress\":0.4}");
        QCOMPARE(r.phase(), SpeedTestRunner::Searching);
        QCOMPARE(r.progress(), 0.4);

        r.parseLine("{\"phase\":\"server\",\"name\":\"Frankfurt\",\"sponsor\":\"Hetzner\",\"country\":\"Germany\",\"ping\":12.5}");
        QCOMPARE(r.server(), QString("Frankfurt, Germany (Hetzner)"));
        QCOMPARE(r.ping(), 12.5);

        r.parseLine("{\"phase\":\"jitter\",\"jitter\":1.5}");
        QCOMPARE(r.jitter(), 1.5);

        r.parseLine("{\"phase\":\"download\",\"mbps\":250.5,\"progress\":0.3}");
        QCOMPARE(r.phase(), SpeedTestRunner::Downloading);
        QCOMPARE(r.mbps(), 250.5);
        QCOMPARE(r.progress(), 0.3);
        QCOMPARE(r.download(), 250.5);

        r.parseLine("{\"phase\":\"upload\",\"mbps\":80.0,\"progress\":0.9}");
        QCOMPARE(r.phase(), SpeedTestRunner::Uploading);
        QCOMPARE(r.mbps(), 80.0);
        QCOMPARE(r.upload(), 80.0);
        QCOMPARE(r.download(), 250.5);
    }

    void parseLineDoneStoresResults()
    {
        SpeedTestRunner r;
        r.parseLine("{\"phase\":\"done\",\"ping\":10,\"jitter\":2,\"download\":300,\"upload\":100,\"server\":\"S\",\"isp\":\"I\",\"timestamp\":\"t\"}");
        QCOMPARE(r.phase(), SpeedTestRunner::Done);
        QCOMPARE(r.download(), 300.0);
        QCOMPARE(r.upload(), 100.0);
        QCOMPARE(r.isp(), QString("I"));
        QCOMPARE(r.progress(), 1.0);
    }

    void serverStringOmitsMissingParts()
    {
        SpeedTestRunner r;
        r.parseLine("{\"phase\":\"server\",\"name\":\"ATH\",\"sponsor\":\"Cloudflare\",\"country\":\"\",\"ping\":11}");
        QCOMPARE(r.server(), QString("ATH (Cloudflare)"));
    }

    void garbageLinesAreIgnored()
    {
        SpeedTestRunner r;
        const QRegularExpression warning("Ignoring unparseable helper output");
        QTest::ignoreMessage(QtWarningMsg, warning);
        r.parseLine("not json at all");
        QTest::ignoreMessage(QtWarningMsg, warning);
        r.parseLine("{\"phase\":\"downl");
        r.parseLine("{}");
        QTest::ignoreMessage(QtWarningMsg, warning);
        r.parseLine("[1,2,3]");
        r.parseLine("{\"phase\":\"unknown\"}");
        QCOMPARE(r.phase(), SpeedTestRunner::Idle);
        QCOMPARE(r.mbps(), 0.0);
    }

    void successfulRunReachesDoneAndRecordsHistory()
    {
        QTemporaryDir dir;
        HistoryModel history;
        history.setPath(dir.filePath("h.json"));
        useStub("ok");
        SpeedTestRunner r;
        r.setHistory(&history);
        r.start();
        QVERIFY(r.busy());
        QTRY_COMPARE_WITH_TIMEOUT(r.phase(), SpeedTestRunner::Done, 5000);
        QVERIFY(!r.busy());
        QCOMPARE(r.download(), 310.2);
        QCOMPARE(history.rowCount(), 1);
        QCOMPARE(history.lastDownload(), 310.2);
    }

    void errorLineMovesToError()
    {
        useStub("error");
        SpeedTestRunner r;
        r.start();
        QTRY_COMPARE_WITH_TIMEOUT(r.phase(), SpeedTestRunner::Error, 5000);
        QCOMPARE(r.errorString(), QString("No network"));
    }

    void exitWithoutDoneIsUnexpected()
    {
        useStub("silent");
        SpeedTestRunner r;
        r.start();
        QTRY_COMPARE_WITH_TIMEOUT(r.phase(), SpeedTestRunner::Error, 5000);
        QCOMPARE(r.errorString(), QString("Test ended unexpectedly"));
    }

    void missingHelperNamesThePath()
    {
        qputenv("XSPEEDTEST_HELPER", "/nonexistent/xspeedtest-helper");
        SpeedTestRunner r;
        r.start();
        QCOMPARE(r.phase(), SpeedTestRunner::Error);
        QVERIFY(r.errorString().contains("/nonexistent/xspeedtest-helper"));
    }

    void startWhileBusyRunsOneProcess()
    {
        QTemporaryDir dir;
        const QString counter = dir.filePath("count");
        useStub("hang", counter);
        SpeedTestRunner r;
        r.start();
        r.start();
        QTRY_COMPARE_WITH_TIMEOUT(r.phase(), SpeedTestRunner::Downloading, 5000);
        QFile f(counter);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QCOMPARE(f.readAll().count('\n'), 1);
        r.cancel();
    }

    void passesBackendToHelper()
    {
        QTemporaryDir dir;
        const QString envFile = dir.filePath("env");
        useStub("ok");
        qputenv("STUB_ENV", envFile.toUtf8());
        SpeedTestRunner r;
        r.setBackend("cloudflare");
        r.start();
        QTRY_COMPARE_WITH_TIMEOUT(r.phase(), SpeedTestRunner::Done, 5000);
        QFile f(envFile);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QCOMPARE(f.readAll().trimmed(), QByteArray("cloudflare"));
        qunsetenv("STUB_ENV");
    }

    void restartWhileHelperStillExitingDoesNotCrash()
    {
        QTemporaryDir dir;
        const QString counter = dir.filePath("count");
        useStub("lingering", counter);
        SpeedTestRunner r;
        r.start();
        QTRY_COMPARE_WITH_TIMEOUT(r.phase(), SpeedTestRunner::Done, 5000);
        r.start();
        QCOMPARE(r.phase(), SpeedTestRunner::Searching);
        QTRY_COMPARE_WITH_TIMEOUT(r.phase(), SpeedTestRunner::Done, 5000);
        QTest::qWait(1500);
        QCOMPARE(r.phase(), SpeedTestRunner::Done);
        QFile f(counter);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QCOMPARE(f.readAll().count('\n'), 2);
    }

    void failedStartDoesNotBlockNextRun()
    {
        QTemporaryDir dir;
        QFile broken(dir.filePath("broken.sh"));
        QVERIFY(broken.open(QIODevice::WriteOnly));
        broken.write("#!/nonexistent/interpreter\n");
        broken.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
        broken.close();
        qputenv("XSPEEDTEST_HELPER", broken.fileName().toUtf8());
        SpeedTestRunner r;
        r.start();
        QTRY_COMPARE_WITH_TIMEOUT(r.phase(), SpeedTestRunner::Error, 5000);

        useStub("ok");
        r.start();
        QTRY_COMPARE_WITH_TIMEOUT(r.phase(), SpeedTestRunner::Done, 5000);
    }

    void cancelReturnsToIdleWithoutHistory()
    {
        QTemporaryDir dir;
        HistoryModel history;
        history.setPath(dir.filePath("h.json"));
        useStub("hang");
        SpeedTestRunner r;
        r.setHistory(&history);
        r.start();
        QTRY_COMPARE_WITH_TIMEOUT(r.phase(), SpeedTestRunner::Downloading, 5000);
        r.cancel();
        QCOMPARE(r.phase(), SpeedTestRunner::Idle);
        QVERIFY(!r.busy());
        QTest::qWait(300);
        QCOMPARE(r.phase(), SpeedTestRunner::Idle);
        QCOMPARE(history.rowCount(), 0);
    }
};

QTEST_GUILESS_MAIN(RunnerTest)
#include "tst_runner.moc"

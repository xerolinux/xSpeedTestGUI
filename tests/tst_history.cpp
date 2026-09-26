#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <QtTest>

#include "historymodel.h"

class HistoryTest : public QObject
{
    Q_OBJECT

    static QVariantMap entry(double download)
    {
        return {{"timestamp", "2026-09-26T10:00:00+00:00"}, {"download", download}, {"upload", download / 2},
                {"ping", 12.5}, {"jitter", 1.5}, {"server", "Frankfurt, Germany (Hetzner)"}};
    }

private slots:
    void addPersistsAndReloads()
    {
        QTemporaryDir dir;
        const QString file = dir.filePath("history.json");
        HistoryModel first;
        first.setPath(file);
        first.add(entry(300));

        HistoryModel second;
        second.setPath(file);
        QCOMPARE(second.rowCount(), 1);
        QCOMPARE(second.index(0).data(HistoryModel::DownloadRole).toDouble(), 300.0);
        QCOMPARE(second.index(0).data(HistoryModel::ServerRole).toString(), QString("Frankfurt, Germany (Hetzner)"));
    }

    void capsAtFiftyNewestFirst()
    {
        QTemporaryDir dir;
        HistoryModel model;
        model.setPath(dir.filePath("history.json"));
        for (int i = 0; i < 55; ++i)
            model.add(entry(i));
        QCOMPARE(model.rowCount(), 50);
        QCOMPARE(model.index(0).data(HistoryModel::DownloadRole).toDouble(), 54.0);
        QCOMPARE(model.index(49).data(HistoryModel::DownloadRole).toDouble(), 5.0);
    }

    void corruptFileLoadsEmptyThenRewrites()
    {
        QTemporaryDir dir;
        const QString file = dir.filePath("history.json");
        QFile bad(file);
        QVERIFY(bad.open(QIODevice::WriteOnly));
        bad.write("{{ not json");
        bad.close();

        HistoryModel model;
        model.setPath(file);
        QCOMPARE(model.rowCount(), 0);
        model.add(entry(100));

        QFile good(file);
        QVERIFY(good.open(QIODevice::ReadOnly));
        const QJsonDocument doc = QJsonDocument::fromJson(good.readAll());
        QVERIFY(doc.isArray());
        QCOMPARE(doc.array().size(), 1);
    }

    void unwritablePathKeepsInMemoryEntry()
    {
        HistoryModel model;
        model.setPath("/proc/xspeedtest-denied/history.json");
        model.add(entry(42));
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.lastDownload(), 42.0);
    }

    void externalChangeReloads()
    {
        QTemporaryDir dir;
        const QString file = dir.filePath("history.json");
        HistoryModel watcher;
        watcher.setPath(file);
        QCOMPARE(watcher.rowCount(), 0);

        HistoryModel writer;
        writer.setPath(file);
        writer.add(entry(500));
        QTRY_COMPARE(watcher.rowCount(), 1);
        QCOMPARE(watcher.lastDownload(), 500.0);
    }

    void watchesWhenDirectoryDidNotExist()
    {
        QTemporaryDir dir;
        const QString file = dir.filePath("fresh/history.json");
        HistoryModel watcher;
        watcher.setPath(file);
        HistoryModel writer;
        writer.setPath(file);
        writer.add(entry(700));
        QTRY_COMPARE(watcher.rowCount(), 1);
        writer.add(entry(710));
        QTRY_COMPARE(watcher.rowCount(), 2);
    }

    void addKeepsEntriesFromOtherInstance()
    {
        QTemporaryDir dir;
        const QString file = dir.filePath("history.json");
        HistoryModel first;
        first.setPath(file);
        HistoryModel second;
        second.setPath(file);
        first.add(entry(100));
        second.add(entry(200));

        HistoryModel reader;
        reader.setPath(file);
        QCOMPARE(reader.rowCount(), 2);
        QCOMPARE(reader.index(0).data(HistoryModel::DownloadRole).toDouble(), 200.0);
        QCOMPARE(reader.index(1).data(HistoryModel::DownloadRole).toDouble(), 100.0);
    }

    void clearEmptiesModelAndFile()
    {
        QTemporaryDir dir;
        const QString file = dir.filePath("history.json");
        HistoryModel model;
        model.setPath(file);
        model.add(entry(10));
        model.add(entry(20));
        model.clear();
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.lastDownload(), 0.0);

        HistoryModel other;
        other.setPath(file);
        QCOMPARE(other.rowCount(), 0);
    }

    void lastValuesFollowNewestEntry()
    {
        QTemporaryDir dir;
        HistoryModel model;
        model.setPath(dir.filePath("history.json"));
        QCOMPARE(model.lastDownload(), 0.0);
        model.add(entry(200));
        model.add(entry(400));
        QCOMPARE(model.lastDownload(), 400.0);
        QCOMPARE(model.lastUpload(), 200.0);
        QCOMPARE(model.lastPing(), 12.5);
    }
};

QTEST_GUILESS_MAIN(HistoryTest)
#include "tst_history.moc"

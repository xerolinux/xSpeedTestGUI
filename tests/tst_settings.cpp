#include <QDir>
#include <QFile>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

#include "speedsettings.h"

class SettingsTest : public QObject
{
    Q_OBJECT

private slots:
    void defaults()
    {
        QTemporaryDir dir;
        SpeedSettings s;
        s.setPath(dir.filePath("s.conf"));
        QCOMPARE(s.speedUnit(), SpeedSettings::Mbps);
        QCOMPARE(s.backend(), QString("auto"));
        QVERIFY(!s.autoStart());
        QVERIFY(s.showHistory());
        QVERIFY(s.animate());
        QCOMPARE(s.glassOpacity(), 0.36);
        QCOMPARE(s.style(), QString("downpour"));
    }

    void convertsEveryUnit()
    {
        QTemporaryDir dir;
        SpeedSettings s;
        s.setPath(dir.filePath("s.conf"));
        s.setSpeedUnit(SpeedSettings::Kbps);
        QCOMPARE(s.convert(300.0), 300000.0);
        QCOMPARE(s.unitLabel(), QString("kbps"));
        s.setSpeedUnit(SpeedSettings::Mbps);
        QCOMPARE(s.convert(300.0), 300.0);
        QCOMPARE(s.unitLabel(), QString("Mbps"));
        s.setSpeedUnit(SpeedSettings::KBps);
        QCOMPARE(s.convert(300.0), 37500.0);
        QCOMPARE(s.unitLabel(), QString("KBps"));
        s.setSpeedUnit(SpeedSettings::MBps);
        QCOMPARE(s.convert(300.0), 37.5);
        QCOMPARE(s.unitLabel(), QString("MBps"));
    }

    void persistsAcrossInstances()
    {
        QTemporaryDir dir;
        const QString file = dir.filePath("s.conf");
        {
            SpeedSettings s;
            s.setPath(file);
            s.setSpeedUnit(SpeedSettings::MBps);
            s.setBackend("cloudflare");
            s.setAutoStart(true);
            s.setShowHistory(false);
            s.setAnimate(false);
            s.setGlassOpacity(0.5);
            s.setStyle("aurora-arc");
        }
        SpeedSettings s;
        s.setPath(file);
        QCOMPARE(s.speedUnit(), SpeedSettings::MBps);
        QCOMPARE(s.backend(), QString("cloudflare"));
        QVERIFY(s.autoStart());
        QVERIFY(!s.showHistory());
        QVERIFY(!s.animate());
        QCOMPARE(s.glassOpacity(), 0.5);
        QCOMPARE(s.style(), QString("aurora-arc"));
    }

    void invalidValuesFallBack()
    {
        QTemporaryDir dir;
        const QString file = dir.filePath("s.conf");
        {
            QSettings raw(file, QSettings::IniFormat);
            raw.setValue("backend", "nonsense");
            raw.setValue("speedUnit", 42);
            raw.setValue("glassOpacity", 7.0);
        }
        SpeedSettings s;
        s.setPath(file);
        QCOMPARE(s.backend(), QString("auto"));
        QCOMPARE(s.speedUnit(), SpeedSettings::Mbps);
        QCOMPARE(s.glassOpacity(), 1.0);
    }

    void setterClampsAndRejects()
    {
        QTemporaryDir dir;
        SpeedSettings s;
        s.setPath(dir.filePath("s.conf"));
        s.setStyle("");
        QCOMPARE(s.style(), QString("downpour"));
        s.setBackend("bogus");
        QCOMPARE(s.backend(), QString("auto"));
        s.setGlassOpacity(0.0);
        QCOMPARE(s.glassOpacity(), 0.1);
    }

    void changeNotifiesOnlyWhenValueDiffers()
    {
        QTemporaryDir dir;
        SpeedSettings s;
        s.setPath(dir.filePath("s.conf"));
        QSignalSpy spy(&s, &SpeedSettings::changed);
        s.setSpeedUnit(SpeedSettings::Mbps);
        QCOMPARE(spy.count(), 0);
        s.setSpeedUnit(SpeedSettings::Kbps);
        QCOMPARE(spy.count(), 1);
    }

    void appAndPlasmoidProfilesAreIndependent()
    {
        QTemporaryDir dir;
        qputenv("XDG_CONFIG_HOME", dir.path().toUtf8());
        SpeedSettings app;
        app.setProfile("app");
        SpeedSettings widget;
        widget.setProfile("plasmoid");
        QVERIFY(app.path().endsWith("/xspeedtest/app.conf"));
        QVERIFY(widget.path().endsWith("/xspeedtest/plasmoid.conf"));

        app.setSpeedUnit(SpeedSettings::MBps);
        app.setStyle("spectrum-deck");
        widget.setGlassOpacity(0.8);
        QTest::qWait(200);
        QCOMPARE(widget.speedUnit(), SpeedSettings::Mbps);
        QCOMPARE(widget.style(), QString("downpour"));
        QCOMPARE(app.glassOpacity(), 0.36);

        SpeedSettings again;
        again.setProfile("plasmoid");
        QCOMPARE(again.glassOpacity(), 0.8);
        QCOMPARE(again.speedUnit(), SpeedSettings::Mbps);
        qunsetenv("XDG_CONFIG_HOME");
    }

    void externalChangeReloads()
    {
        QTemporaryDir dir;
        const QString file = dir.filePath("fresh/s.conf");
        SpeedSettings watcher;
        watcher.setPath(file);
        SpeedSettings writer;
        writer.setPath(file);
        writer.setSpeedUnit(SpeedSettings::KBps);
        QTRY_COMPARE(watcher.speedUnit(), SpeedSettings::KBps);
    }
};

QTEST_GUILESS_MAIN(SettingsTest)
#include "tst_settings.moc"

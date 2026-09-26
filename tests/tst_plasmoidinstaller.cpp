#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include "plasmoidinstaller.h"

class PlasmoidInstallerTest : public QObject
{
    Q_OBJECT

    QTemporaryDir m_root;
    QString m_home;
    QString m_system;
    QString m_source;
    QString m_module;

    static void write(const QString &path, const QByteArray &content)
    {
        QVERIFY(QDir().mkpath(QFileInfo(path).absolutePath()));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(content);
    }

    static QString metadata(const QString &dataDir)
    {
        return dataDir + "/plasma/plasmoids/org.xspeedtest.widget/metadata.json";
    }

private slots:
    void init()
    {
        m_home = m_root.filePath(QString::number(QRandomGenerator::global()->generate()) + "-home");
        m_system = m_home + "-system";
        m_source = m_home + "-source";
        m_module = m_home + "-module";
        QDir().mkpath(m_home);
        QDir().mkpath(m_system);
        write(m_source + "/metadata.json", "{}");
        write(m_source + "/contents/ui/main.qml", "import QtQuick\nItem {}\n");
        write(m_source + "/contents/icons/xspeedtest.svg", "<svg/>");
        write(m_module + "/qmldir", "module org.xspeedtest\n");
        write(m_module + "/libxspeedtestplugin.so", "plugin");
        write(m_home + "-lib/libxspeedtest.so", "backing");
        write(m_home + "-helper/xspeedtest-helper", "#!/bin/sh\n");
        qputenv("XDG_DATA_HOME", m_home.toUtf8());
        qputenv("XDG_DATA_DIRS", m_system.toUtf8());
        qputenv("XSPEEDTEST_PLASMOID_SRC", m_source.toUtf8());
        qputenv("XSPEEDTEST_MODULE_SRC", m_module.toUtf8());
        qputenv("XSPEEDTEST_LIBRARY", (m_home + "-lib/libxspeedtest.so").toUtf8());
        qputenv("XSPEEDTEST_HELPER", (m_home + "-helper/xspeedtest-helper").toUtf8());
    }

    void notInstalledByDefault()
    {
        PlasmoidInstaller installer;
        QVERIFY(!installer.installed());
    }

    void installCopiesTreeAndMarksInstalled()
    {
        PlasmoidInstaller installer;
        QSignalSpy spy(&installer, &PlasmoidInstaller::installedChanged);
        installer.install();
        QVERIFY2(installer.errorString().isEmpty(), qPrintable(installer.errorString()));
        QVERIFY(installer.installed());
        QCOMPARE(spy.count(), 1);
        QVERIFY(QFile::exists(metadata(m_home)));
        QVERIFY(QFile::exists(m_home + "/plasma/plasmoids/org.xspeedtest.widget/contents/ui/main.qml"));
    }

    void installBundlesModuleLibraryHelperAndIcon()
    {
        PlasmoidInstaller installer;
        installer.install();
        QVERIFY2(installer.errorString().isEmpty(), qPrintable(installer.errorString()));
        const QString lib = m_home + "/plasma/plasmoids/org.xspeedtest.widget/contents/lib/org/xspeedtest";
        QVERIFY(QFile::exists(lib + "/qmldir"));
        QVERIFY(QFile::exists(lib + "/libxspeedtestplugin.so"));
        QVERIFY(QFile::exists(lib + "/libxspeedtest.so"));
        QVERIFY(QFile::exists(lib + "/xspeedtest-helper"));
        QVERIFY(QFile::exists(m_home + "/icons/hicolor/scalable/apps/xspeedtest.svg"));
    }

    void missingModuleReportsErrorAndInstallsNothing()
    {
        qputenv("XSPEEDTEST_MODULE_SRC", "/nonexistent/module");
        PlasmoidInstaller installer;
        installer.install();
        QVERIFY(!installer.installed());
        QVERIFY(installer.errorString().contains("/nonexistent/module"));
        QVERIFY(!QFile::exists(metadata(m_home)));
    }

    void detectsSystemWideInstall()
    {
        write(metadata(m_system), "{}");
        PlasmoidInstaller installer;
        QVERIFY(installer.installed());
    }

    void missingSourceReportsError()
    {
        qputenv("XSPEEDTEST_PLASMOID_SRC", "/nonexistent/plasmoid");
        PlasmoidInstaller installer;
        installer.install();
        QVERIFY(!installer.installed());
        QVERIFY(installer.errorString().contains("/nonexistent/plasmoid"));
        QVERIFY(!QFile::exists(metadata(m_home)));
    }

    void userInstalledIgnoresSystemCopy()
    {
        write(metadata(m_system), "{}");
        PlasmoidInstaller installer;
        QVERIFY(installer.installed());
        QVERIFY(!installer.userInstalled());
        installer.install();
        QVERIFY(installer.userInstalled());
    }

    void notOutdatedRightAfterInstall()
    {
        PlasmoidInstaller installer;
        QVERIFY(!installer.outdated());
        installer.install();
        QVERIFY(!installer.outdated());
    }

    void outdatedWhenABundledFileChanges()
    {
        PlasmoidInstaller installer;
        installer.install();
        QSignalSpy spy(&installer, &PlasmoidInstaller::outdatedChanged);
        write(m_source + "/contents/ui/main.qml", "import QtQuick\nItem { width: 10 }\n");
        installer.refresh();
        QVERIFY(installer.outdated());
        QCOMPARE(spy.count(), 1);
    }

    void outdatedWhenTheBundledPluginChanges()
    {
        PlasmoidInstaller installer;
        installer.install();
        write(m_module + "/libxspeedtestplugin.so", "newer plugin");
        installer.refresh();
        QVERIFY(installer.outdated());
    }

    void systemCopyIsNeverOutdated()
    {
        write(metadata(m_system), "{}");
        PlasmoidInstaller installer;
        write(m_source + "/contents/ui/main.qml", "changed");
        installer.refresh();
        QVERIFY(!installer.outdated());
    }

    void updateReinstallsAndRestartsTheShell()
    {
        const QString marker = m_home + "-restarted";
        const QString script = m_home + "-restart.sh";
        write(script, ("#!/bin/sh\ntouch " + marker + "\n").toUtf8());
        QVERIFY(QFile::setPermissions(script, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner));
        qputenv("XSPEEDTEST_SHELL_RESTART", script.toUtf8());

        PlasmoidInstaller installer;
        installer.install();
        write(m_source + "/contents/ui/main.qml", "import QtQuick\nItem { width: 20 }\n");
        installer.refresh();
        QVERIFY(installer.outdated());

        installer.update();
        QVERIFY(!installer.outdated());
        QVERIFY(QFile::exists(m_home + "/plasma/plasmoids/org.xspeedtest.widget/contents/ui/main.qml"));
        QTRY_VERIFY(QFile::exists(marker));
        qunsetenv("XSPEEDTEST_SHELL_RESTART");
    }

    void failedUpdateDoesNotRestartTheShell()
    {
        const QString marker = m_home + "-restarted";
        const QString script = m_home + "-restart.sh";
        write(script, ("#!/bin/sh\ntouch " + marker + "\n").toUtf8());
        QVERIFY(QFile::setPermissions(script, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner));
        qputenv("XSPEEDTEST_SHELL_RESTART", script.toUtf8());
        qputenv("XSPEEDTEST_MODULE_SRC", "/nonexistent/module");

        PlasmoidInstaller installer;
        installer.update();
        QVERIFY(!installer.errorString().isEmpty());
        QTest::qWait(300);
        QVERIFY(!QFile::exists(marker));
        qunsetenv("XSPEEDTEST_SHELL_RESTART");
    }

    void refreshNoticesExternalInstall()
    {
        PlasmoidInstaller installer;
        QSignalSpy spy(&installer, &PlasmoidInstaller::installedChanged);
        write(metadata(m_home), "{}");
        installer.refresh();
        QVERIFY(installer.installed());
        QCOMPARE(spy.count(), 1);
    }
};

QTEST_GUILESS_MAIN(PlasmoidInstallerTest)
#include "tst_plasmoidinstaller.moc"

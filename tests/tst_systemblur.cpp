#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include "systemblur.h"

class SystemBlurTest : public QObject
{
    Q_OBJECT

    QTemporaryDir m_dir;

    QString script(const QString &name, const QByteArray &body)
    {
        const QString path = m_dir.filePath(name);
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly))
            qFatal("cannot write %s", qPrintable(path));
        f.write("#!/bin/sh\n" + body);
        f.close();
        f.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
        return path;
    }

    void useTools(const QByteArray &readBody, const QByteArray &writeBody)
    {
        qputenv("XSPEEDTEST_KREADCONFIG", script("read.sh", readBody).toUtf8());
        qputenv("XSPEEDTEST_KWRITECONFIG", script("write.sh", writeBody).toUtf8());
        qputenv("XSPEEDTEST_KWIN_RELOAD", script("reload.sh", "touch " + m_dir.filePath("reloaded").toUtf8() + "\n").toUtf8());
        QFile::remove(m_dir.filePath("reloaded"));
        QFile::remove(m_dir.filePath("written"));
    }

private slots:
    void readsTheCurrentKWinStrength()
    {
        useTools("echo 11\n", "exit 0\n");
        SystemBlur blur;
        QVERIFY(blur.available());
        QCOMPARE(blur.strength(), 11);
    }

    void missingToolsMakeItUnavailable()
    {
        qputenv("XSPEEDTEST_KREADCONFIG", "/nonexistent/kreadconfig");
        qputenv("XSPEEDTEST_KWRITECONFIG", "/nonexistent/kwriteconfig");
        SystemBlur blur;
        QVERIFY(!blur.available());
        QCOMPARE(blur.strength(), 8);
    }

    void applyWritesTheClampedValueAndReloadsKWin()
    {
        useTools("echo 8\n", "echo \"$@\" >> " + m_dir.filePath("written").toUtf8() + "\n");
        SystemBlur blur;
        QSignalSpy spy(&blur, &SystemBlur::strengthChanged);
        blur.apply(99);
        QCOMPARE(blur.strength(), 15);
        QCOMPARE(spy.count(), 1);
        QVERIFY(blur.errorString().isEmpty());
        QFile f(m_dir.filePath("written"));
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray args = f.readAll();
        QVERIFY(args.contains("--file kwinrc"));
        QVERIFY(args.contains("--group Effect-blur"));
        QVERIFY(args.contains("--key BlurStrength"));
        QVERIFY(args.trimmed().endsWith("15"));
        QTRY_VERIFY(QFile::exists(m_dir.filePath("reloaded")));
    }

    void failedWriteReportsAnErrorAndKeepsTheOldValue()
    {
        useTools("echo 8\n", "exit 1\n");
        SystemBlur blur;
        blur.apply(3);
        QCOMPARE(blur.strength(), 8);
        QVERIFY(!blur.errorString().isEmpty());
        QVERIFY(!QFile::exists(m_dir.filePath("reloaded")));
    }
};

QTEST_GUILESS_MAIN(SystemBlurTest)
#include "tst_systemblur.moc"

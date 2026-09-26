#include <QQmlComponent>
#include <QTemporaryDir>
#include <QQmlEngine>
#include <QStandardPaths>
#include <QtTest>

class QmlLoadTest : public QObject
{
    Q_OBJECT

    static void load(const char *body)
    {
        QTest::failOnWarning(QRegularExpression(".*"));
        QQmlEngine engine;
        engine.addImportPath(QStringLiteral(QML_IMPORT_DIR));
        QQmlComponent component(&engine);
        component.setData(QByteArray("import QtQuick\nimport org.xspeedtest\n") + body, QUrl());
        QVERIFY2(component.status() == QQmlComponent::Ready, qPrintable(component.errorString()));
        QScopedPointer<QObject> object(component.create());
        QVERIFY2(object, qPrintable(component.errorString()));
    }

    QTemporaryDir m_dataHome;

private slots:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
        qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
        qputenv("XDG_DATA_HOME", m_dataHome.path().toUtf8());
        qputenv("XDG_CONFIG_HOME", m_dataHome.path().toUtf8());
    }

    void catalogHasThirtyFiveValidStyles()
    {
        QQmlEngine engine;
        engine.addImportPath(QStringLiteral(QML_IMPORT_DIR));
        QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport org.xspeedtest\nItem {\n property int count: Styles.list.length\n property bool valid: Styles.valid()\n property string fallback: Styles.resolve(\"nonsense\").id\n}", QUrl());
        QScopedPointer<QObject> object(component.create());
        QVERIFY2(object, qPrintable(component.errorString()));
        QCOMPARE(object->property("count").toInt(), 35);
        QVERIFY(object->property("valid").toBool());
        QCOMPARE(object->property("fallback").toString(), QString("downpour"));
    }

    void everyStyleLoadsInItsOwnLayoutAndInThePopupLayout()
    {
        QQmlEngine engine;
        engine.addImportPath(QStringLiteral(QML_IMPORT_DIR));
        QQmlComponent listComponent(&engine);
        listComponent.setData("import QtQuick\nimport org.xspeedtest\nItem { property var ids: Styles.list.map(function (s) { return s.id; }) }", QUrl());
        QScopedPointer<QObject> holder(listComponent.create());
        QVERIFY2(holder, qPrintable(listComponent.errorString()));
        const QStringList ids = holder->property("ids").toStringList();
        QCOMPARE(ids.size(), 35);
        for (const QString &id : ids) {
            for (const char *compact : {"false", "true"}) {
                load(qPrintable(QStringLiteral("SpeedView {\n runner: SpeedTestRunner {}\n settings: SpeedSettings { style: \"%1\" }\n history: HistoryModel {}\n compact: %2\n}").arg(id, QLatin1String(compact))));
            }
        }
    }

    void speedPanelLoadsWithAndWithoutInstaller()
    {
        load("SpeedPanel {\n runner: SpeedTestRunner {}\n settings: SpeedSettings {}\n history: HistoryModel {}\n installer: PlasmoidInstaller {}\n}");
        load("SpeedPanel {\n runner: SpeedTestRunner {}\n settings: SpeedSettings {}\n history: HistoryModel {}\n compact: true\n settingsOpen: true\n}");
    }

    void windowBlurLoads() { load("Item { WindowBlur { radius: 12 } }"); }

    void settingsPageLoads()
    {
        load("SettingsPage {\n settings: SpeedSettings {}\n history: HistoryModel {}\n installer: PlasmoidInstaller {}\n}");
    }

    void plasmoidInstallerLoads() { load("PlasmoidInstaller {}"); }
};

QTEST_MAIN(QmlLoadTest)
#include "tst_qmlload.moc"

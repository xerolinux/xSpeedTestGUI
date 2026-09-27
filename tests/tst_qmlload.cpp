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

    static QSizeF implicitSizeOf(const char *body)
    {
        QQmlEngine engine;
        engine.addImportPath(QStringLiteral(QML_IMPORT_DIR));
        QQmlComponent component(&engine);
        component.setData(QByteArray("import QtQuick\nimport org.xspeedtest\n") + body, QUrl());
        QScopedPointer<QObject> object(component.create());
        if (!object)
            return {-1, -1};
        return {object->property("implicitWidth").toReal(), object->property("implicitHeight").toReal()};
    }

    static qreal implicitHeightOf(const char *body)
    {
        QQmlEngine engine;
        engine.addImportPath(QStringLiteral(QML_IMPORT_DIR));
        QQmlComponent component(&engine);
        component.setData(QByteArray("import QtQuick\nimport org.xspeedtest\n") + body, QUrl());
        QScopedPointer<QObject> object(component.create());
        if (!object)
            return -1;
        return object->property("implicitHeight").toReal();
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
                load(qPrintable(QStringLiteral("SpeedView {\n runner: SpeedTestRunner {}\n settings: SpeedSettings { style: \"%1\" }\n compact: %2\n}").arg(id, QLatin1String(compact))));
            }
        }
    }

    void speedPanelLoadsEveryPage()
    {
        for (const char *page : {"main", "history", "settings"}) {
            load(qPrintable(QStringLiteral("SpeedPanel {\n runner: SpeedTestRunner {}\n settings: SpeedSettings {}\n history: HistoryModel {}\n installer: PlasmoidInstaller {}\n page: \"%1\"\n}").arg(QLatin1String(page))));
            load(qPrintable(QStringLiteral("SpeedPanel {\n runner: SpeedTestRunner {}\n settings: SpeedSettings {}\n history: HistoryModel {}\n compact: true\n page: \"%1\"\n}").arg(QLatin1String(page))));
        }
    }

    void historyPageLoads()
    {
        load("HistoryPage {\n settings: SpeedSettings {}\n history: HistoryModel {}\n}");
    }

    void windowBlurLoads() { load("Item { WindowBlur { radius: 12 } }"); }

    void compactSettingsAndHistoryAreShorterThanFull()
    {
        const qreal fullSettings = implicitHeightOf("SettingsPage {\n settings: SpeedSettings {}\n}");
        const qreal compactSettings = implicitHeightOf("SettingsPage {\n settings: SpeedSettings {}\n compact: true\n}");
        QVERIFY(fullSettings > 0);
        QVERIFY(compactSettings > 0);
        QVERIFY2(compactSettings < fullSettings * 0.95, qPrintable(QStringLiteral("compact %1 vs full %2").arg(compactSettings).arg(fullSettings)));

        const qreal fullHistory = implicitHeightOf("HistoryPage {\n settings: SpeedSettings {}\n history: HistoryModel {}\n}");
        const qreal compactHistory = implicitHeightOf("HistoryPage {\n settings: SpeedSettings {}\n history: HistoryModel {}\n compact: true\n}");
        QVERIFY(fullHistory > 0);
        QVERIFY(compactHistory > 0);
        QVERIFY2(compactHistory < fullHistory * 0.6, qPrintable(QStringLiteral("compact %1 vs full %2").arg(compactHistory).arg(fullHistory)));
    }

    void compactPopupFitsTheCurrentPageInsteadOfTheTallestOne()
    {
        // Each page's height must come from that page alone, not from Math.max() across all three
        // (the old bug that stretched settings/history to match whatever the main view needed).
        const qreal standaloneViewHeight = implicitHeightOf("SpeedView {\n runner: SpeedTestRunner {}\n settings: SpeedSettings {}\n compact: true\n}");
        const qreal standaloneSettingsHeight = implicitHeightOf("SettingsPage {\n settings: SpeedSettings {}\n compact: true\n}");
        QVERIFY(standaloneViewHeight > 0);
        QVERIFY(standaloneSettingsHeight > 0);

        const qreal mainHeight = implicitHeightOf(
            "SpeedPanel {\n runner: SpeedTestRunner {}\n settings: SpeedSettings {}\n history: HistoryModel {}\n compact: true\n page: \"main\"\n}");
        const qreal settingsHeight = implicitHeightOf(
            "SpeedPanel {\n runner: SpeedTestRunner {}\n settings: SpeedSettings {}\n history: HistoryModel {}\n compact: true\n page: \"settings\"\n}");
        // Each, minus the panel's header+padding overhead, should track its own page's bare content
        // height. The tolerance covers the header's title label, shown on settings/history but not
        // on main -- the old bug instead locked both pages to whichever was tallest, a gap of 100px+.
        const qreal overhead = mainHeight - standaloneViewHeight;
        QVERIFY2(qAbs((settingsHeight - overhead) - standaloneSettingsHeight) < 60,
                 qPrintable(QStringLiteral("settings %1 vs its own standalone height %2 (overhead %3)")
                                .arg(settingsHeight).arg(standaloneSettingsHeight).arg(overhead)));
    }

    void settingsPageLoads()
    {
        load("SettingsPage {\n settings: SpeedSettings {}\n installer: PlasmoidInstaller {}\n}");
    }

    void compactMainPageHasASaneAspectRatio()
    {
        // Regression: CompactLayout used to be a bare ColumnLayout with implicitWidth/Height set
        // directly on it. QtQuick.Layouts recomputes those on a container layout itself, silently
        // discarding the manual override, so the popup ended up ~3x taller than wide (144x420).
        const QSizeF size = implicitSizeOf(
            "SpeedView {\n runner: SpeedTestRunner {}\n settings: SpeedSettings { style: \"lava-orb\" }\n compact: true\n}");
        QVERIFY(size.width() > 0);
        QVERIFY(size.height() > 0);
        QVERIFY2(size.height() < size.width() * 1.4,
                 qPrintable(QStringLiteral("compact popup is too tall: %1x%2").arg(size.width()).arg(size.height())));
    }

    void desktopWidgetSettingsDoNotStretchToMatchATallMainStyle()
    {
        // The desktop widget keeps the main view's own full style layout (compact: false), but its
        // settings/history pages must size to their own content (pagesCompact: true) instead of being
        // stretched via Math.max() to whatever height the selected style's main view happens to need.
        const char *panel =
            "SpeedPanel {\n runner: SpeedTestRunner {}\n settings: SpeedSettings { style: \"lava-orb\" }\n history: HistoryModel {}\n"
            " compact: false\n pagesCompact: true\n page: \"%1\"\n}";
        const qreal mainHeight = implicitHeightOf(qPrintable(QString::fromLatin1(panel).arg("main")));
        const qreal settingsHeight = implicitHeightOf(qPrintable(QString::fromLatin1(panel).arg("settings")));
        const qreal historyHeight = implicitHeightOf(qPrintable(QString::fromLatin1(panel).arg("history")));
        QVERIFY(mainHeight > 0);
        QVERIFY(settingsHeight > 0);
        QVERIFY(historyHeight > 0);
        QVERIFY2(settingsHeight < mainHeight,
                 qPrintable(QStringLiteral("settings %1 should not be stretched to match main view %2").arg(settingsHeight).arg(mainHeight)));
        QVERIFY2(historyHeight < mainHeight,
                 qPrintable(QStringLiteral("history %1 should not be stretched to match main view %2").arg(historyHeight).arg(mainHeight)));
    }

    void plasmoidInstallerLoads() { load("PlasmoidInstaller {}"); }

    void copyResultsOnlyOffersItselfOnTheFinishedMainPage()
    {
        QQmlEngine engine;
        engine.addImportPath(QStringLiteral(QML_IMPORT_DIR));
        QQmlComponent component(&engine);
        component.setData(
            "import QtQuick\nimport org.xspeedtest\n"
            "SpeedPanel {\n runner: SpeedTestRunner {}\n settings: SpeedSettings {}\n history: HistoryModel {}\n}",
            QUrl());
        QScopedPointer<QObject> object(component.create());
        QVERIFY2(object, qPrintable(component.errorString()));

        QCOMPARE(object->property("canCopyResults").toBool(), false);

        QObject *runner = qvariant_cast<QObject *>(object->property("runner"));
        QVERIFY(runner);
        QMetaObject::invokeMethod(runner, "parseLine",
            Q_ARG(QByteArray, QByteArrayLiteral(R"({"phase":"done","ping":10,"jitter":1,"download":300,"upload":100,"server":"S","isp":"I","timestamp":"t"})")));
        QCOMPARE(object->property("canCopyResults").toBool(), true);

        QVERIFY(object->setProperty("page", QStringLiteral("history")));
        QCOMPARE(object->property("canCopyResults").toBool(), false);
    }
};

QTEST_MAIN(QmlLoadTest)
#include "tst_qmlload.moc"

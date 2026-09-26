#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

int main(int argc, char *argv[])
{
    QQuickWindow::setDefaultAlphaBuffer(true);
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("xspeedtest"));
    app.setDesktopFileName(QStringLiteral("org.xspeedtest.app"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("xspeedtest")));

    QQmlApplicationEngine engine;
    engine.loadFromModule("org.xspeedtest.app", "Main");
    if (engine.rootObjects().isEmpty())
        return 1;
    return app.exec();
}

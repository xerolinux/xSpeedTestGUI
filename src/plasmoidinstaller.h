#pragma once

#include <QList>
#include <QObject>
#include <QPair>
#include <QtQml/qqmlregistration.h>

class PlasmoidInstaller : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool installed READ installed NOTIFY installedChanged)
    Q_PROPERTY(bool userInstalled READ userInstalled NOTIFY userInstalledChanged)
    Q_PROPERTY(bool outdated READ outdated NOTIFY outdatedChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)

public:
    explicit PlasmoidInstaller(QObject *parent = nullptr);

    bool installed() const { return m_installed; }
    bool userInstalled() const { return m_userInstalled; }
    bool outdated() const { return m_outdated; }
    QString errorString() const { return m_error; }

    Q_INVOKABLE void install();
    Q_INVOKABLE void update();
    Q_INVOKABLE void refresh();

signals:
    void installedChanged();
    void userInstalledChanged();
    void outdatedChanged();
    void errorStringChanged();

private:
    using CopyPlan = QList<QPair<QString, QString>>;

    static bool detect();
    static bool detectUser();
    static QString userDir();
    static void restartShell();
    bool buildPlan(CopyPlan &plan);
    bool detectOutdated();
    void setError(const QString &message);

    bool m_installed = false;
    bool m_userInstalled = false;
    bool m_outdated = false;
    QString m_error;
};

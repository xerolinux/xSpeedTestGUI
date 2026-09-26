#include "plasmoidinstaller.h"

#include "paths.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

namespace
{
const QString appletPath = QStringLiteral("plasma/plasmoids/org.xspeedtest.widget");
const QString iconFile = QStringLiteral("icons/hicolor/scalable/apps/xspeedtest.svg");
const QString bundleDir = QStringLiteral("/contents/lib/org/xspeedtest");

QString dataDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
}

void addTree(QList<QPair<QString, QString>> &plan, const QString &from, const QString &to, bool recursive)
{
    const QDir root(from);
    QDirIterator it(from, QDir::Files, recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        it.next();
        plan.append({it.filePath(), to + QLatin1Char('/') + root.relativeFilePath(it.filePath())});
    }
}

bool sameContent(const QString &a, const QString &b)
{
    QFile first(a);
    QFile second(b);
    return first.open(QIODevice::ReadOnly) && second.open(QIODevice::ReadOnly) && first.readAll() == second.readAll();
}
}

PlasmoidInstaller::PlasmoidInstaller(QObject *parent)
    : QObject(parent)
    , m_installed(detect())
    , m_userInstalled(detectUser())
{
    m_outdated = m_userInstalled && detectOutdated();
}

bool PlasmoidInstaller::detect()
{
    return !QStandardPaths::locate(QStandardPaths::GenericDataLocation, appletPath + QStringLiteral("/metadata.json")).isEmpty();
}

QString PlasmoidInstaller::userDir()
{
    return dataDir() + QLatin1Char('/') + appletPath;
}

bool PlasmoidInstaller::detectUser()
{
    return QFileInfo::exists(userDir() + QStringLiteral("/metadata.json"));
}

void PlasmoidInstaller::setError(const QString &message)
{
    if (m_error == message)
        return;
    m_error = message;
    emit errorStringChanged();
}

bool PlasmoidInstaller::buildPlan(CopyPlan &plan)
{
    const QString source = Paths::plasmoidSource();
    const QString module = Paths::moduleSource();
    const QString library = Paths::libraryFile();
    const QString helper = Paths::helper();
    for (const QString &required : {source + QStringLiteral("/metadata.json"), module + QStringLiteral("/qmldir"), library, helper}) {
        if (!QFileInfo::exists(required)) {
            setError(tr("Missing file for the plasmoid: %1").arg(required));
            return false;
        }
    }

    const QString target = userDir();
    const QString bundle = target + bundleDir;
    addTree(plan, source, target, true);
    addTree(plan, module, bundle, false);
    plan.append({library, bundle + QLatin1Char('/') + QFileInfo(library).fileName()});
    plan.append({helper, bundle + QStringLiteral("/xspeedtest-helper")});
    const QString icon = source + QStringLiteral("/contents/icons/xspeedtest.svg");
    if (QFileInfo::exists(icon))
        plan.append({icon, dataDir() + QLatin1Char('/') + iconFile});
    return true;
}

bool PlasmoidInstaller::detectOutdated()
{
    const QString previous = m_error;
    CopyPlan plan;
    const bool ready = buildPlan(plan);
    setError(previous);
    if (!ready)
        return false;
    for (const auto &[from, to] : plan) {
        if (!QFileInfo::exists(to) || !sameContent(from, to))
            return true;
    }
    return false;
}

void PlasmoidInstaller::refresh()
{
    const bool installed = detect();
    const bool userInstalled = detectUser();
    const bool outdated = userInstalled && detectOutdated();
    const bool installedDiffers = installed != m_installed;
    const bool userDiffers = userInstalled != m_userInstalled;
    const bool outdatedDiffers = outdated != m_outdated;
    m_installed = installed;
    m_userInstalled = userInstalled;
    m_outdated = outdated;
    if (installedDiffers)
        emit installedChanged();
    if (userDiffers)
        emit userInstalledChanged();
    if (outdatedDiffers)
        emit outdatedChanged();
}

void PlasmoidInstaller::install()
{
    setError({});
    CopyPlan plan;
    if (!buildPlan(plan))
        return;

    QDir(userDir()).removeRecursively();
    for (const auto &[from, to] : plan) {
        QFile::remove(to);
        if (!QDir().mkpath(QFileInfo(to).absolutePath()) || !QFile::copy(from, to)) {
            QDir(userDir()).removeRecursively();
            setError(tr("Could not write %1").arg(to));
            return;
        }
    }
    refresh();
}

void PlasmoidInstaller::restartShell()
{
    QString program = QString::fromLocal8Bit(qgetenv("XSPEEDTEST_SHELL_RESTART"));
    QStringList arguments;
    if (program.isEmpty()) {
        const QStringList unit = {QStringLiteral("--user"), QStringLiteral("is-active"), QStringLiteral("--quiet"), QStringLiteral("plasma-plasmashell.service")};
        if (QProcess::execute(QStringLiteral("systemctl"), unit) == 0) {
            program = QStringLiteral("systemctl");
            arguments = {QStringLiteral("--user"), QStringLiteral("restart"), QStringLiteral("plasma-plasmashell.service")};
        } else {
            program = QStringLiteral("plasmashell");
            arguments = {QStringLiteral("--replace")};
        }
    }
    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);
    process.setStandardOutputFile(QProcess::nullDevice());
    process.setStandardErrorFile(QProcess::nullDevice());
    process.startDetached();
}

void PlasmoidInstaller::update()
{
    install();
    if (m_error.isEmpty())
        restartShell();
}

#include "paths.h"

#include <QFileInfo>
#include <dlfcn.h>

namespace
{
QString fromEnv(const char *name)
{
    return QString::fromLocal8Bit(qgetenv(name));
}
}

namespace Paths
{
QString libraryFile()
{
    const QString override = fromEnv("XSPEEDTEST_LIBRARY");
    if (!override.isEmpty())
        return override;
    Dl_info info;
    if (dladdr(reinterpret_cast<void *>(&Paths::libraryFile), &info) && info.dli_fname)
        return QFileInfo(QString::fromLocal8Bit(info.dli_fname)).canonicalFilePath();
    return {};
}

QString helper()
{
    const QString override = fromEnv("XSPEEDTEST_HELPER");
    if (!override.isEmpty())
        return override;
    const QString sibling = QFileInfo(libraryFile()).absolutePath() + QStringLiteral("/xspeedtest-helper");
    if (QFileInfo(sibling).isExecutable())
        return sibling;
    return QStringLiteral(XSPEEDTEST_HELPER_INSTALL_PATH);
}

QString moduleSource()
{
    const QString override = fromEnv("XSPEEDTEST_MODULE_SRC");
    if (!override.isEmpty())
        return override;
    const QString sibling = QFileInfo(libraryFile()).absolutePath() + QStringLiteral("/org/xspeedtest");
    if (QFileInfo::exists(sibling + QStringLiteral("/qmldir")))
        return sibling;
    return QStringLiteral(XSPEEDTEST_MODULE_INSTALL_PATH);
}

QString plasmoidSource()
{
    const QString override = fromEnv("XSPEEDTEST_PLASMOID_SRC");
    if (!override.isEmpty())
        return override;
    const QString sibling = QFileInfo(libraryFile()).absolutePath() + QStringLiteral("/plasmoid");
    if (QFileInfo::exists(sibling + QStringLiteral("/metadata.json")))
        return sibling;
    return QStringLiteral(XSPEEDTEST_PLASMOID_INSTALL_PATH);
}
}

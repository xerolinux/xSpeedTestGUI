#include "systemblur.h"

#include <QProcess>
#include <QStandardPaths>
#include <algorithm>

namespace
{
QString tool(const char *overrideVariable, const QString &name)
{
    const QString override = QString::fromLocal8Bit(qgetenv(overrideVariable));
    return override.isEmpty() ? name : override;
}

QStringList configArguments()
{
    return {QStringLiteral("--file"), QStringLiteral("kwinrc"), QStringLiteral("--group"), QStringLiteral("Effect-blur"),
            QStringLiteral("--key"), QStringLiteral("BlurStrength")};
}
}

SystemBlur::SystemBlur(QObject *parent)
    : QObject(parent)
{
    QProcess reader;
    reader.start(tool("XSPEEDTEST_KREADCONFIG", QStringLiteral("kreadconfig6")),
                 configArguments() + QStringList{QStringLiteral("--default"), QString::number(DefaultStrength)});
    if (!reader.waitForFinished(3000) || reader.exitCode() != 0)
        return;
    bool ok = false;
    const int value = QString::fromUtf8(reader.readAllStandardOutput()).trimmed().toInt(&ok);
    if (!ok)
        return;
    m_strength = std::clamp(value, MinStrength, MaxStrength);
    m_available = true;
}

void SystemBlur::setError(const QString &message)
{
    if (m_error == message)
        return;
    m_error = message;
    emit errorStringChanged();
}

void SystemBlur::apply(int strength)
{
    setError({});
    const int value = std::clamp(strength, MinStrength, MaxStrength);
    if (QProcess::execute(tool("XSPEEDTEST_KWRITECONFIG", QStringLiteral("kwriteconfig6")),
                          configArguments() + QStringList{QString::number(value)}) != 0) {
        setError(tr("Could not change the KWin blur strength"));
        return;
    }
    QProcess reload;
    const QString override = QString::fromLocal8Bit(qgetenv("XSPEEDTEST_KWIN_RELOAD"));
    if (override.isEmpty()) {
        reload.setProgram(QStringLiteral("dbus-send"));
        reload.setArguments({QStringLiteral("--session"), QStringLiteral("--dest=org.kde.KWin"), QStringLiteral("/KWin"),
                             QStringLiteral("org.kde.KWin.reconfigure")});
    } else {
        reload.setProgram(override);
    }
    reload.setStandardOutputFile(QProcess::nullDevice());
    reload.setStandardErrorFile(QProcess::nullDevice());
    reload.startDetached();

    m_strength = value;
    m_available = true;
    emit strengthChanged();
}

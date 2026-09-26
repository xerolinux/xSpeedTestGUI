#include "speedsettings.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <algorithm>

namespace
{
const QStringList backends = {QStringLiteral("auto"), QStringLiteral("speedtest"), QStringLiteral("cloudflare")};

QString pathFor(const QString &profile)
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/xspeedtest/") + profile + QStringLiteral(".conf");
}

QString validBackend(const QString &backend)
{
    return backends.contains(backend) ? backend : backends.first();
}
}

SpeedSettings::SpeedSettings(QObject *parent)
    : QObject(parent)
{
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [this] {
        load();
        rewatch();
    });
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this] {
        load();
        rewatch();
    });
    setPath(pathFor(m_profile));
}

void SpeedSettings::setProfile(const QString &profile)
{
    const QString name = profile.isEmpty() ? QStringLiteral("app") : profile;
    if (name == m_profile)
        return;
    m_profile = name;
    setPath(pathFor(name));
}

void SpeedSettings::setPath(const QString &path)
{
    if (path == m_path)
        return;
    m_path = path;
    const QStringList watched = m_watcher.files() + m_watcher.directories();
    if (!watched.isEmpty())
        m_watcher.removePaths(watched);
    load();
    rewatch();
    emit changed();
}

QString SpeedSettings::unitLabel() const
{
    switch (m_values.unit) {
    case Kbps:
        return QStringLiteral("kbps");
    case KBps:
        return QStringLiteral("KBps");
    case MBps:
        return QStringLiteral("MBps");
    case Mbps:
        break;
    }
    return QStringLiteral("Mbps");
}

double SpeedSettings::convert(double mbps) const
{
    switch (m_values.unit) {
    case Kbps:
        return mbps * 1000;
    case KBps:
        return mbps * 125;
    case MBps:
        return mbps / 8;
    case Mbps:
        break;
    }
    return mbps;
}

void SpeedSettings::setSpeedUnit(SpeedUnit unit)
{
    Values v = m_values;
    v.unit = unit;
    update(v);
}

void SpeedSettings::setBackend(const QString &backend)
{
    Values v = m_values;
    v.backend = validBackend(backend);
    update(v);
}

void SpeedSettings::setAutoStart(bool enabled)
{
    Values v = m_values;
    v.autoStart = enabled;
    update(v);
}

void SpeedSettings::setAnimate(bool enabled)
{
    Values v = m_values;
    v.animate = enabled;
    update(v);
}

void SpeedSettings::setStyle(const QString &style)
{
    Values v = m_values;
    v.style = style.isEmpty() ? Values().style : style;
    update(v);
}

void SpeedSettings::setGlassOpacity(double opacity)
{
    Values v = m_values;
    v.opacity = std::clamp(opacity, MinOpacity, MaxOpacity);
    update(v);
}

void SpeedSettings::update(const Values &values)
{
    if (values == m_values)
        return;
    m_values = values;
    save();
    emit changed();
}

void SpeedSettings::load()
{
    QSettings raw(m_path, QSettings::IniFormat);
    Values v;
    const int unit = raw.value(QStringLiteral("speedUnit"), int(v.unit)).toInt();
    v.unit = unit >= Kbps && unit <= MBps ? SpeedUnit(unit) : Mbps;
    v.backend = validBackend(raw.value(QStringLiteral("backend"), v.backend).toString());
    v.autoStart = raw.value(QStringLiteral("autoStart"), v.autoStart).toBool();
    v.animate = raw.value(QStringLiteral("animate"), v.animate).toBool();
    v.opacity = std::clamp(raw.value(QStringLiteral("glassOpacity"), v.opacity).toDouble(), MinOpacity, MaxOpacity);
    const QString style = raw.value(QStringLiteral("style")).toString();
    if (!style.isEmpty())
        v.style = style;
    if (v == m_values)
        return;
    m_values = v;
    emit changed();
}

void SpeedSettings::save()
{
    if (!QDir().mkpath(QFileInfo(m_path).absolutePath()))
        return;
    QSettings raw(m_path, QSettings::IniFormat);
    raw.setValue(QStringLiteral("speedUnit"), int(m_values.unit));
    raw.setValue(QStringLiteral("backend"), m_values.backend);
    raw.setValue(QStringLiteral("autoStart"), m_values.autoStart);
    raw.setValue(QStringLiteral("animate"), m_values.animate);
    raw.setValue(QStringLiteral("glassOpacity"), m_values.opacity);
    raw.setValue(QStringLiteral("style"), m_values.style);
    raw.sync();
    rewatch();
}

void SpeedSettings::rewatch()
{
    QDir dir(QFileInfo(m_path).absolutePath());
    while (!dir.exists() && dir.cdUp()) {
    }
    if (!m_watcher.directories().contains(dir.path()))
        m_watcher.addPath(dir.path());
    if (!m_watcher.files().contains(m_path) && QFileInfo::exists(m_path))
        m_watcher.addPath(m_path);
}

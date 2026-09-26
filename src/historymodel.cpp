#include "historymodel.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

namespace
{
QString defaultPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/xspeedtest/history.json";
}
}

HistoryModel::HistoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [this] {
        reload();
        rewatch();
    });
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this] {
        reload();
        rewatch();
    });
    setPath(defaultPath());
}

void HistoryModel::setPath(const QString &path)
{
    if (path == m_path)
        return;
    m_path = path;
    const QStringList watched = m_watcher.files() + m_watcher.directories();
    if (!watched.isEmpty())
        m_watcher.removePaths(watched);
    reload();
    rewatch();
    emit pathChanged();
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_entries.size());
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount())
        return {};
    const Entry &e = m_entries[index.row()];
    switch (role) {
    case TimestampRole:
        return e.timestamp;
    case DownloadRole:
        return e.download;
    case UploadRole:
        return e.upload;
    case PingRole:
        return e.ping;
    case JitterRole:
        return e.jitter;
    case ServerRole:
        return e.server;
    }
    return {};
}

QHash<int, QByteArray> HistoryModel::roleNames() const
{
    return {{TimestampRole, "timestamp"}, {DownloadRole, "download"}, {UploadRole, "upload"},
            {PingRole, "ping"},           {JitterRole, "jitter"},     {ServerRole, "server"}};
}

double HistoryModel::lastDownload() const
{
    return m_entries.empty() ? 0.0 : m_entries.front().download;
}

double HistoryModel::lastUpload() const
{
    return m_entries.empty() ? 0.0 : m_entries.front().upload;
}

double HistoryModel::lastPing() const
{
    return m_entries.empty() ? 0.0 : m_entries.front().ping;
}

void HistoryModel::add(const QVariantMap &result)
{
    reload();

    Entry e;
    e.timestamp = result.value("timestamp", QDateTime::currentDateTimeUtc().toString(Qt::ISODate)).toString();
    e.download = result.value("download").toDouble();
    e.upload = result.value("upload").toDouble();
    e.ping = result.value("ping").toDouble();
    e.jitter = result.value("jitter").toDouble();
    e.server = result.value("server").toString();

    beginInsertRows({}, 0, 0);
    m_entries.insert(m_entries.begin(), e);
    endInsertRows();
    if (m_entries.size() > MaxEntries) {
        beginRemoveRows({}, MaxEntries, static_cast<int>(m_entries.size()) - 1);
        m_entries.resize(MaxEntries);
        endRemoveRows();
    }
    emit countChanged();
    save();
}

void HistoryModel::clear()
{
    if (m_entries.empty())
        return;
    beginResetModel();
    m_entries.clear();
    endResetModel();
    emit countChanged();
    save();
}

void HistoryModel::reload()
{
    std::vector<Entry> loaded;
    QFile file(m_path);
    if (file.open(QIODevice::ReadOnly)) {
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isArray()) {
            for (const QJsonValue &v : doc.array()) {
                const QJsonObject o = v.toObject();
                loaded.push_back({o["timestamp"].toString(), o["download"].toDouble(), o["upload"].toDouble(),
                                  o["ping"].toDouble(), o["jitter"].toDouble(), o["server"].toString()});
                if (loaded.size() == MaxEntries)
                    break;
            }
        }
    }
    if (loaded == m_entries)
        return;
    beginResetModel();
    m_entries = std::move(loaded);
    endResetModel();
    emit countChanged();
}

void HistoryModel::save()
{
    if (!QDir().mkpath(QFileInfo(m_path).absolutePath()))
        return;
    QJsonArray array;
    for (const Entry &e : m_entries) {
        array.append(QJsonObject{{"timestamp", e.timestamp}, {"download", e.download}, {"upload", e.upload},
                                 {"ping", e.ping}, {"jitter", e.jitter}, {"server", e.server}});
    }
    QSaveFile file(m_path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(array).toJson(QJsonDocument::Compact));
        file.commit();
    }
    rewatch();
}

void HistoryModel::rewatch()
{
    QDir dir(QFileInfo(m_path).absolutePath());
    while (!dir.exists() && dir.cdUp()) {
    }
    if (!m_watcher.directories().contains(dir.path()))
        m_watcher.addPath(dir.path());
    if (!m_watcher.files().contains(m_path) && QFileInfo::exists(m_path))
        m_watcher.addPath(m_path);
}

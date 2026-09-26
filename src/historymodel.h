#pragma once

#include <QAbstractListModel>
#include <QFileSystemWatcher>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>
#include <vector>

class HistoryModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(double lastDownload READ lastDownload NOTIFY countChanged)
    Q_PROPERTY(double lastUpload READ lastUpload NOTIFY countChanged)
    Q_PROPERTY(double lastPing READ lastPing NOTIFY countChanged)

public:
    enum Role { TimestampRole = Qt::UserRole + 1, DownloadRole, UploadRole, PingRole, JitterRole, ServerRole };
    static constexpr int MaxEntries = 50;

    explicit HistoryModel(QObject *parent = nullptr);

    QString path() const { return m_path; }
    void setPath(const QString &path);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    double lastDownload() const;
    double lastUpload() const;
    double lastPing() const;

    Q_INVOKABLE void add(const QVariantMap &result);
    Q_INVOKABLE void clear();

signals:
    void pathChanged();
    void countChanged();

private:
    struct Entry {
        QString timestamp;
        double download = 0;
        double upload = 0;
        double ping = 0;
        double jitter = 0;
        QString server;
        bool operator==(const Entry &) const = default;
    };

    void reload();
    void save();
    void rewatch();

    QString m_path;
    std::vector<Entry> m_entries;
    QFileSystemWatcher m_watcher;
};

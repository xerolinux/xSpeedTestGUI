#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <QtQml/qqmlregistration.h>

class SpeedSettings : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString profile READ profile WRITE setProfile NOTIFY changed)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY changed)
    Q_PROPERTY(SpeedUnit speedUnit READ speedUnit WRITE setSpeedUnit NOTIFY changed)
    Q_PROPERTY(QString unitLabel READ unitLabel NOTIFY changed)
    Q_PROPERTY(QString backend READ backend WRITE setBackend NOTIFY changed)
    Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart NOTIFY changed)
    Q_PROPERTY(bool animate READ animate WRITE setAnimate NOTIFY changed)
    Q_PROPERTY(QString style READ style WRITE setStyle NOTIFY changed)
    Q_PROPERTY(double glassOpacity READ glassOpacity WRITE setGlassOpacity NOTIFY changed)

public:
    enum SpeedUnit { Kbps, Mbps, KBps, MBps };
    Q_ENUM(SpeedUnit)

    static constexpr double MinOpacity = 0.1;
    static constexpr double MaxOpacity = 1.0;

    explicit SpeedSettings(QObject *parent = nullptr);

    QString profile() const { return m_profile; }
    void setProfile(const QString &profile);
    QString path() const { return m_path; }
    void setPath(const QString &path);

    SpeedUnit speedUnit() const { return m_values.unit; }
    void setSpeedUnit(SpeedUnit unit);
    QString unitLabel() const;
    QString backend() const { return m_values.backend; }
    void setBackend(const QString &backend);
    bool autoStart() const { return m_values.autoStart; }
    void setAutoStart(bool enabled);
    bool animate() const { return m_values.animate; }
    void setAnimate(bool enabled);
    QString style() const { return m_values.style; }
    void setStyle(const QString &style);
    double glassOpacity() const { return m_values.opacity; }
    void setGlassOpacity(double opacity);

    Q_INVOKABLE double convert(double mbps) const;

signals:
    void changed();

private:
    struct Values {
        SpeedUnit unit = Mbps;
        QString backend = QStringLiteral("auto");
        bool autoStart = false;
        bool animate = true;
        double opacity = 0.36;
        QString style = QStringLiteral("downpour");
        bool operator==(const Values &) const = default;
    };

    void update(const Values &values);
    void load();
    void save();
    void rewatch();

    QString m_profile = QStringLiteral("app");
    QString m_path;
    Values m_values;
    QFileSystemWatcher m_watcher;
};

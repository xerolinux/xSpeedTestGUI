#pragma once

#include <QObject>
#include <QPointer>
#include <QQuickWindow>
#include <QtQml/qqmlregistration.h>

class WindowBlur : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QQuickWindow *window READ window WRITE setWindow NOTIFY windowChanged)
    Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit WindowBlur(QObject *parent = nullptr);

    QQuickWindow *window() const { return m_window; }
    void setWindow(QQuickWindow *window);
    qreal radius() const { return m_radius; }
    void setRadius(qreal radius);
    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled);

signals:
    void windowChanged();
    void radiusChanged();
    void enabledChanged();

private:
    void apply();

    QPointer<QQuickWindow> m_window;
    qreal m_radius = 0;
    bool m_enabled = true;
};

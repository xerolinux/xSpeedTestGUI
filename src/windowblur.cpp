#include "windowblur.h"

#include <KWindowEffects>
#include <QPainterPath>
#include <QRegion>

WindowBlur::WindowBlur(QObject *parent)
    : QObject(parent)
{
}

void WindowBlur::setWindow(QQuickWindow *window)
{
    if (m_window == window)
        return;
    if (m_window)
        m_window->disconnect(this);
    m_window = window;
    if (m_window) {
        connect(m_window, &QWindow::widthChanged, this, &WindowBlur::apply);
        connect(m_window, &QWindow::heightChanged, this, &WindowBlur::apply);
        connect(m_window, &QWindow::visibleChanged, this, &WindowBlur::apply);
    }
    apply();
    emit windowChanged();
}

void WindowBlur::setRadius(qreal radius)
{
    if (qFuzzyCompare(m_radius, radius))
        return;
    m_radius = radius;
    apply();
    emit radiusChanged();
}

void WindowBlur::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    apply();
    emit enabledChanged();
}

void WindowBlur::apply()
{
    if (!m_window)
        return;
    QPainterPath path;
    path.addRoundedRect(QRectF(QPointF(0, 0), QSizeF(m_window->size())), m_radius, m_radius);
    KWindowEffects::enableBlurBehind(m_window, m_enabled, QRegion(path.toFillPolygon().toPolygon()));
}

#include "monitor.h"

Monitor::Monitor(QObject *parent)
    : QObject(parent)
    , m_changeTimer(new QTimer (this) )
{
    m_changeTimer->setInterval(500);
    m_changeTimer->setSingleShot(true);
    connect(m_changeTimer, &QTimer::timeout, this, &Monitor::geometryChanged);
}

void Monitor::setX(const int x)
{
    if (m_x == x)
        return;

    m_x = x;

    Q_EMIT xChanged(m_x);
    restartTimer();
}

void Monitor::setY(const int y)
{
    if (m_y == y)
        return;

    m_y = y;

    Q_EMIT yChanged(m_y);
    restartTimer();
}

void Monitor::setW(const int w)
{
    if (m_w == w)
        return;

    m_w = w;

    Q_EMIT wChanged(m_w);
    restartTimer();
}

void Monitor::setH(const int h)
{
    if (m_h == h)
        return;

    m_h = h;

    Q_EMIT hChanged(m_h);
    restartTimer();
}

void Monitor::setPrimary(const QString &primaryName)
{
    m_primary = primaryName;
}

void Monitor::setName(const QString &name)
{
    m_name = name;
}

void Monitor::setPath(const QString &path)
{
    m_path = path;
}

void Monitor::setMonitorEnable(bool enable)
{
    if (m_enable == enable)
        return;

    m_enable = enable;
    Q_EMIT enableChanged(enable);
}

void Monitor::setMonitorModes(const ResolutionList &rl)
{
    m_modes = rl;
    Q_EMIT modesChanged(rl);
}

void Monitor::restartTimer()
{
    if (m_changeTimer->isActive()) {
        m_changeTimer->stop();
    }
    m_changeTimer->start();
}

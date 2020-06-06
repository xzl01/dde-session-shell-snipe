#ifndef MONITOR_H
#define MONITOR_H

#include <QObject>
#include <QRect>

#include <com_deepin_daemon_display_monitor.h>

using MonitorInter = com::deepin::daemon::display::Monitor;

class Monitor : public QObject
{
    Q_OBJECT
    friend class MultiScreenManager;
public:
    explicit Monitor(QObject *parent = nullptr);

    inline const ResolutionList &modes() const { return m_modes; }
    inline int x() const { return m_x; }
    inline int y() const { return m_y; }
    inline int w() const { return m_w; }
    inline int h() const { return m_h; }
    inline const QRect rect() const { return QRect(m_x, m_y, m_w, m_h); }
    inline const QString name() const { Q_ASSERT(!m_name.isEmpty()); return m_name; }
    inline const QString path() const { return m_path; }
    inline bool enable() const { return m_enable; }

Q_SIGNALS:
    void geometryChanged() const;
    void xChanged(const int x) const;
    void yChanged(const int y) const;
    void wChanged(const int w) const;
    void hChanged(const int h) const;
    void enableChanged(bool enable) const;
    void modesChanged(const ResolutionList& rl) const;


private Q_SLOTS:
    void setX(const int x);
    void setY(const int y);
    void setW(const int w);
    void setH(const int h);
    void setPrimary(const QString &primaryName);
    void setName(const QString &name);
    void setPath(const QString &path);
    void setMonitorEnable(bool enable);
    void setMonitorModes(const ResolutionList& rl);

private:
    int m_x;
    int m_y;
    int m_w;
    int m_h;
    QString m_name;
    QString m_path;
    QString m_primary;
    bool m_enable;
    ResolutionList m_modes;
};

#endif // MONITOR_H

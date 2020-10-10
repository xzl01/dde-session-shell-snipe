#include "greeterscreenmanager.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QDesktopWidget>

#include <com_deepin_daemon_display.h>
using DisplayInter=com::deepin::daemon::Display;


GreeterScreenManager::GreeterScreenManager(QObject *parent)
    : QObject(parent)
    , m_registerFunction(nullptr)
    , m_raiseContentFrameTimer(new QTimer(this))
{
    // 在sw平台存在复制模式显示问题，使用延迟来置顶一个Frame
    m_raiseContentFrameTimer->setInterval(80);
    m_raiseContentFrameTimer->setSingleShot(true);

    connect(m_raiseContentFrameTimer, &QTimer::timeout, this, &GreeterScreenManager::raiseContentFrame);
}

void GreeterScreenManager::register_for_mutil_screen(std::function<QWidget *(QScreen *)> function)
{
    m_registerFunction = function;

    DisplayInter inter("com.deepin.daemon.Display", "/com/deepin/daemon/Display", QDBusConnection::sessionBus(), nullptr);
    // update all screen
    for (QScreen *screen : qApp->screens()) {
        qDebug() << __FILE__ << __LINE__ << __func__ << ": screen info ---- " <<  screen << screen->geometry();
        if (qGuiApp->primaryScreen() == screen)
            continue;
        onScreenAdded(screen);
    }
    onScreenAdded(qGuiApp->primaryScreen());
}

void GreeterScreenManager::startRaiseContentFrame()
{
    m_raiseContentFrameTimer->start();
}

void GreeterScreenManager::onScreenAdded(QScreen *screen)
{
    qDebug() << __FILE__ << __LINE__ << __func__ << ": screen add ---- " <<  screen << screen->geometry();
    if (!m_registerFunction) {
        return;
    }

    m_frames[screen] = m_registerFunction(screen);

    startRaiseContentFrame();
}

void GreeterScreenManager::raiseContentFrame()
{
    for (auto it = m_frames.constBegin(); it != m_frames.constEnd(); ++it) {
        if (it.value()->property("contentVisible").toBool()) {
            it.value()->raise();
            return;
        }
    }
}

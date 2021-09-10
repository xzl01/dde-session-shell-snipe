
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

#include "multiscreenmanager.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QDesktopWidget>

MultiScreenManager::MultiScreenManager(QObject *parent)
    : QObject(parent)
    , m_registerFunction(nullptr)
    , m_raiseContentFrameTimer(new QTimer(this))
{
    TRACE_ME_IN;	//<<==--TracePoint!
    connect(qApp, &QGuiApplication::screenAdded, this, &MultiScreenManager::onScreenAdded, Qt::QueuedConnection);
    connect(qApp, &QGuiApplication::screenRemoved, this, &MultiScreenManager::onScreenRemoved, Qt::QueuedConnection);

    // 在sw平台存在复制模式显示问题，使用延迟来置顶一个Frame
    m_raiseContentFrameTimer->setInterval(50);
    m_raiseContentFrameTimer->setSingleShot(true);

    connect(m_raiseContentFrameTimer, &QTimer::timeout, this, &MultiScreenManager::raiseContentFrame);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void MultiScreenManager::register_for_mutil_screen(std::function<QWidget *(QScreen *)> function)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_registerFunction = function;

    // update all screen
    for (QScreen *screen : qApp->screens()) {
        onScreenAdded(screen);
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void MultiScreenManager::startRaiseContentFrame()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_raiseContentFrameTimer->start();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void MultiScreenManager::onScreenAdded(QScreen *screen)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (!m_registerFunction) {
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    }

    m_frames[screen] = m_registerFunction(screen);

    startRaiseContentFrame();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void MultiScreenManager::onScreenRemoved(QScreen *screen)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (!m_registerFunction) {
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    }

    m_frames[screen]->deleteLater();
    m_frames.remove(screen);

    startRaiseContentFrame();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void MultiScreenManager::raiseContentFrame()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    for (auto it = m_frames.constBegin(); it != m_frames.constEnd(); ++it) {
        if (it.value()->property("contentVisible").toBool()) {
            it.value()->raise();
            TRACE_ME_OUT;	//<<==--TracePoint!
            return;
        }
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

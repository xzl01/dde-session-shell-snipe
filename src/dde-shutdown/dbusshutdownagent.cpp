
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

#include "dbusshutdownagent.h"

DBusShutdownAgent::DBusShutdownAgent(QObject *parent) : QObject(parent)
{

}

void DBusShutdownAgent::addFrame(ShutdownFrame *frame)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_frames << frame;
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusShutdownAgent::removeFrame(ShutdownFrame *frame)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_frames.removeOne(frame);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusShutdownAgent::Ping()
{

}

void DBusShutdownAgent::Shutdown()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    for (ShutdownFrame *frame : m_frames) {
        frame->setConfirm(true);
        frame->powerAction(Actions::Shutdown);
        frame->show();
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusShutdownAgent::Restart()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    for (ShutdownFrame *frame : m_frames) {
        frame->setConfirm(true);
        frame->powerAction(Actions::Restart);
        frame->show();
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusShutdownAgent::Logout()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    for (ShutdownFrame *frame : m_frames) {
        frame->setConfirm(true);
        frame->powerAction(Actions::Logout);
        frame->show();
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusShutdownAgent::Suspend()
{
    // action未成功执行时，可能是被其它应用程序阻塞，此时应当确保主窗口显示，将阻塞程序的列表显示出来
    TRACE_ME_IN;	//<<==--TracePoint!
    if (!m_frames.first()->powerAction(Actions::Suspend)) {
        for (ShutdownFrame *frame : m_frames) {
            frame->show();
        }
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusShutdownAgent::Hibernate()
{
    // action未成功执行时，可能是被其它应用程序阻塞，此时应当确保主窗口显示，将阻塞程序的列表显示出来
    TRACE_ME_IN;	//<<==--TracePoint!
    if (!m_frames.first()->powerAction(Actions::Hibernate)) {
        for (ShutdownFrame *frame : m_frames) {
            frame->show();
        }
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusShutdownAgent::SwitchUser()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    for (ShutdownFrame *frame : m_frames) {
        frame->setConfirm(true);
        frame->powerAction(Actions::SwitchUser);
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusShutdownAgent::Show()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    for (ShutdownFrame *frame : m_frames) {
        frame->show();
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusShutdownAgent::sync(Actions action)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    for (ShutdownFrame *frame : m_frames) {
        if(!frame->isVisible())
            frame->show();

        frame->powerAction(action);
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

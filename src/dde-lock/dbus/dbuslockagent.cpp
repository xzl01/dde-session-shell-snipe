
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

#include "dbuslockagent.h"
#include "src/session-widgets/sessionbasemodel.h"

DBusLockAgent::DBusLockAgent(QObject *parent) : QObject(parent), m_model(nullptr)
{

}

void DBusLockAgent::setModel(SessionBaseModel *const model)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_model = model;
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusLockAgent::Show()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_model->setIsBlackModel(false);
    m_model->setIsHibernateModel(false);
    m_model->setIsShow(true);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusLockAgent::ShowAuth(bool active)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Show();
    emit m_model->activeAuthChanged(!active);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

// 待机，enable=true：进入待机；enable=false：待机恢复
void DBusLockAgent::Suspend(bool enable)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (enable) {
        m_model->setIsBlackModel(true);
        m_model->setIsShow(true);
    } else {
        QDBusInterface infc("com.deepin.daemon.Power","/com/deepin/daemon/Power","com.deepin.daemon.Power");
        // 待机恢复需要密码
        bool bSuspendLock = infc.property("SleepLock").toBool();

        if (bSuspendLock) {
            m_model->setIsBlackModel(false);
            m_model->setIsShow(true);
        } else {
            m_model->setIsShow(false);
            emit m_model->visibleChanged(false);
        }
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusLockAgent::Hibernate(bool enable)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_model->setIsHibernateModel(enable);
    m_model->setIsShow(true);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusLockAgent::ShowUserList()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    emit m_model->showUserList();
    TRACE_ME_OUT;	//<<==--TracePoint!

}



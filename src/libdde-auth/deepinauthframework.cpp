
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

#include "deepinauthframework.h"
#include "interface/deepinauthinterface.h"
#include "src/session-widgets/userinfo.h"

#include <QThread>
#include <QTimer>
#include <QVariant>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>

DeepinAuthFramework::DeepinAuthFramework(DeepinAuthInterface *inter, QObject *parent)
    : QObject(parent)
    , m_interface(inter)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_authagent = new AuthAgent(this);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

DeepinAuthFramework::~DeepinAuthFramework()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (!m_authagent.isNull()) {
        if (m_pamAuth != 0) {
            //先取消上次验证请求
            m_authagent->setCancelAuth(true);
            pthread_cancel(m_pamAuth);
            pthread_join(m_pamAuth, nullptr);
            m_pamAuth = 0;
        }

        delete m_authagent;
        m_authagent = nullptr;
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

bool DeepinAuthFramework::isAuthenticate() const
{
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return m_pamAuth != 0;
}

int DeepinAuthFramework::GetAuthType()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return m_authagent->GetAuthType();
}

void* DeepinAuthFramework::pamAuthWorker(void *arg)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    DeepinAuthFramework* deepin_auth = static_cast<DeepinAuthFramework*>(arg);
    if(deepin_auth != nullptr && deepin_auth->m_currentUser != nullptr) {
        //开始验证,并重置变量等待输入密码
        deepin_auth->m_authagent->setCancelAuth(false);
        deepin_auth->m_authagent->Authenticate(deepin_auth->m_currentUser->name());
    } else {
        qDebug() << "pam auth worker deepin framework is nullptr";
    }

    TRACE_ME_OUT;	//<<==--TracePoint!
    return nullptr;
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DeepinAuthFramework::Authenticate(std::shared_ptr<User> user)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (user->isLock() || user->name().isEmpty())
{
    TRACE_ME_OUT;	//<<==--TracePoint!
    return;
}

    m_password.clear();

    if (m_pamAuth != 0) {
        qDebug() << Q_FUNC_INFO << "pam auth cancel" << m_authagent->thread()->loopLevel();
        //先取消上次验证请求
        m_authagent->setCancelAuth(true);
        //发送退出线程消息
        pthread_cancel(m_pamAuth);
        //等待线程退出
        pthread_join(m_pamAuth, nullptr);
        m_pamAuth = 0;
    }

    m_currentUser = user;

    //创建验证线程,等待输入密码
    int rc = pthread_create(&m_pamAuth, nullptr, &pamAuthWorker, this);
    if (rc != 0) {
        qDebug() << "failed to create the authentication thread: %s" << strerror(errno);
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DeepinAuthFramework::Responsed(const QString &password)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if(m_authagent.isNull() || m_pamAuth == 0) {
        qDebug() << "pam auth agent is not start";
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    }

    m_password = password;
    if (m_currentUser->isNoPasswdGrp() || (!m_currentUser->isNoPasswdGrp() && !m_password.isEmpty())) {
        qDebug() << "pam responsed password";

        m_authagent->Responsed(password);
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

const QString DeepinAuthFramework::RequestEchoOff(const QString &msg)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Q_UNUSED(msg);

    TRACE_ME_OUT;	//<<==--TracePoint!
    return m_password;
}

const QString DeepinAuthFramework::RequestEchoOn(const QString &msg)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return msg;
}

void DeepinAuthFramework::DisplayErrorMsg(const QString &msg)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_interface->onDisplayErrorMsg(msg);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DeepinAuthFramework::DisplayTextInfo(const QString &msg)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_interface->onDisplayTextInfo(msg);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DeepinAuthFramework::RespondResult(const QString &msg)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_interface->onPasswordResult(msg);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

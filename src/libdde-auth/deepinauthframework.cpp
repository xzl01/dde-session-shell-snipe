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
    m_authagent = new AuthAgent(this);
}

DeepinAuthFramework::~DeepinAuthFramework()
{
    if (!m_authagent.isNull()) {
        delete m_authagent;
        m_authagent = nullptr;
    }
}

int DeepinAuthFramework::GetAuthType()
{
    return m_authagent->GetAuthType();
}

void* DeepinAuthFramework::pamAuthWorker(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    tagAuthArg *pArg = static_cast<tagAuthArg*>(arg);
    pArg->pFrameWork->m_authagent->Authenticate(pArg->strAuthUserName);
    delete pArg;
    return nullptr;
}

void DeepinAuthFramework::Authenticate(std::shared_ptr<User> user)
{
    if (user->isLock()) return;

    qDebug() << "DeepinAuthFramework::Authenticate: pam auth start, loopLevel =" << m_authagent->thread()->loopLevel();
    m_password.clear();
    m_currentUser = user;
    //m_authagent->NotifyCancelAuth();

    tagAuthArg *pArg = new tagAuthArg();
    pArg->pFrameWork = this;
    pArg->strAuthUserName = user->name();

    pthread_t m_pamAuth = 0;
    int rc = pthread_create(&m_pamAuth, nullptr, &pamAuthWorker, pArg);
    if (rc != 0) {
        char buferr[256];
        qDebug() << "failed to create the authentication thread: %s" << strerror_r(errno, buferr, sizeof(buferr));
        m_pamAuth = 0;
        return;
    }
    pthread_detach(m_pamAuth);
}

bool DeepinAuthFramework::Responsed(const QString &password)
{
    if(m_authagent.isNull() || !m_authagent->IsWaitingPassword()) {
        qDebug() << "Responsed: pam auth agent is not start";
        return false;
    }

    m_password = password;
    if (m_currentUser->isNoPasswdGrp() || (!m_currentUser->isNoPasswdGrp() && !password.isEmpty())) {
        qDebug() << "Responsed: pam responsed password";
        m_authagent->Responsed(password);
        return true;
    }
    qDebug() << "Responsed: password error! not start auth!";
    return false;
}

const QString DeepinAuthFramework::RequestEchoOff(const QString &msg)
{
    Q_UNUSED(msg);

    return m_password;
}

const QString DeepinAuthFramework::RequestEchoOn(const QString &msg)
{
    return msg;
}

void DeepinAuthFramework::DisplayErrorMsg(const QString &msg)
{
    m_interface->onDisplayErrorMsg(msg);
}

void DeepinAuthFramework::DisplayTextInfo(const QString &msg)
{
    m_interface->onDisplayTextInfo(msg);
}

void DeepinAuthFramework::RespondResult(const QString &msg)
{
    m_interface->onPasswordResult(msg);
}

void DeepinAuthFramework::CancelCurrentAuth()
{
    m_authagent->NotifyCancelAuth();
}
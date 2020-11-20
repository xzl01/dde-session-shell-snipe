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

        forceCancelLastAuth();
    }
}

bool DeepinAuthFramework::isAuthenticate() const
{
    return m_pamAuth != 0;
}

int DeepinAuthFramework::GetAuthType()
{
    return m_authagent->GetAuthType();
}

void* DeepinAuthFramework::pamAuthWorker(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, nullptr);

    DeepinAuthFramework* deepin_auth = static_cast<DeepinAuthFramework*>(arg);
    if(deepin_auth != nullptr && deepin_auth->m_currentUser != nullptr) {
        deepin_auth->m_authagent->Authenticate(deepin_auth->m_currentUser->name());
    } else {
        qDebug() << "pam auth worker deepin framework is nullptr";
    }

    deepin_auth->setLastAuthFinished();
    return nullptr;
}

void DeepinAuthFramework::Authenticate(std::shared_ptr<User> user)
{
    if (user->isLock()) return;

    m_password.clear();
    if (!isLastAuthFinished()) {
        qDebug() << "Authenticate: failed to create the authentication thread, last not finished";
        return ;
    }

    qDebug() << Q_FUNC_INFO << "pam auth start" << m_authagent->thread()->loopLevel();

    m_currentUser = user;

    std::lock_guard<std::mutex> guard(m_pamMutex);
    int rc = pthread_create(&m_pamAuth, nullptr, &pamAuthWorker, this);
    if (rc != 0) {
        qDebug() << "failed to create the authentication thread: %s" << strerror(errno);
    }
    pthread_detach(m_pamAuth);
}

void DeepinAuthFramework::Responsed(const QString &password)
{
    if(m_authagent.isNull() || m_pamAuth == 0) {
        qDebug() << "pam auth agent is not start";
        return;
    }

    m_password = password;
    if (m_currentUser->isNoPasswdGrp() || (!m_currentUser->isNoPasswdGrp() && !m_password.isEmpty())) {
        qDebug() << "pam responsed password";

        m_authagent->Responsed(password);
    }
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

bool DeepinAuthFramework::isLastAuthFinished()
{
    std::lock_guard<std::mutex> guard(m_pamMutex);
    return m_pamAuth == 0;
}

void DeepinAuthFramework::setLastAuthFinished()
{
    std::lock_guard<std::mutex> guard(m_pamMutex);
    m_pamAuth = 0;
}

void DeepinAuthFramework::forceCancelLastAuth()
{
    std::lock_guard<std::mutex> guard(m_pamMutex);
    if (m_pamAuth == 0) {
        return;
    }
    pthread_cancel(m_pamAuth);
    m_pamAuth = 0;
}

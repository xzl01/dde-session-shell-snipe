
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

#include "lockworker.h"

#include "src/session-widgets/sessionbasemodel.h"
#include "src/session-widgets/userinfo.h"

#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <libintl.h>
#include <QDebug>
#include <QApplication>
#include <QProcess>
#include <QRegularExpression>
#include <DSysInfo>
#include <pwd.h>

#include <com_deepin_daemon_power.h>

#define LOCKSERVICE_PATH "/com/deepin/dde/LockService"
#define LOCKSERVICE_NAME "com.deepin.dde.LockService"

using PowerInter = com::deepin::daemon::Power;
using namespace Auth;
DCORE_USE_NAMESPACE

LockWorker::LockWorker(SessionBaseModel *const model, QObject *parent)
    : AuthInterface(model, parent)
    , m_authenticating(false)
    , m_isThumbAuth(false)
    , m_lockInter(new DBusLockService(LOCKSERVICE_NAME, LOCKSERVICE_PATH, QDBusConnection::systemBus(), this))
    , m_hotZoneInter(new DBusHotzone("com.deepin.daemon.Zone", "/com/deepin/daemon/Zone", QDBusConnection::sessionBus(), this))
    , m_sessionManager(new SessionManager("com.deepin.SessionManager", "/com/deepin/SessionManager", QDBusConnection::sessionBus(), this))
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_currentUserUid = getuid();
    m_authFramework = new DeepinAuthFramework(this, this);
    m_sessionManager->setSync(false);

    //当前用户m_currentUserUid是已登录用户,直接按AuthInterface::onLoginUserListChanged中的流程处理
    std::shared_ptr<User> u(new ADDomainUser(m_currentUserUid));
    u->setisLogind(true);
    struct passwd *pws;
    pws = getpwuid(m_currentUserUid);
    static_cast<ADDomainUser *>(u.get())->setUserDisplayName(pws->pw_name);
    static_cast<ADDomainUser *>(u.get())->setUserName(pws->pw_name);
    m_model->userAdd(u);
    m_model->setCurrentUser(u);

    //该信号用来处理初始化切换用户(锁屏+锁屏)或者切换用户(锁屏+登陆)两种种场景的指纹认证
    connect(m_lockInter, &DBusLockService::UserChanged, this, &LockWorker::onCurrentUserChanged);

    connect(model, &SessionBaseModel::onPowerActionChanged, this, [ = ](SessionBaseModel::PowerAction poweraction) {
        switch (poweraction) {
        case SessionBaseModel::PowerAction::RequireSuspend:
            m_sessionManager->RequestSuspend();
            break;
        case SessionBaseModel::PowerAction::RequireHibernate:
            m_sessionManager->RequestHibernate();
            break;
        case SessionBaseModel::PowerAction::RequireRestart:
            m_authFramework->Authenticate(m_model->currentUser());
            model->setPowerAction(SessionBaseModel::PowerAction::RequireRestart);
            TRACE_ME_OUT;	//<<==--TracePoint!
            return;
        case SessionBaseModel::PowerAction::RequireShutdown:
            m_authFramework->Authenticate(m_model->currentUser());
            model->setPowerAction(SessionBaseModel::PowerAction::RequireShutdown);
            TRACE_ME_OUT;	//<<==--TracePoint!
            return;
        default:
            break;
        }

        model->setPowerAction(SessionBaseModel::PowerAction::None);
    });

    connect(model, &SessionBaseModel::lockLimitFinished, this, [ = ] {
        auto user = m_model->currentUser();
        if (user != nullptr && !user->isLock()) {
            m_password.clear();
            m_authFramework->Authenticate(user);
        }
    });

    connect(model, &SessionBaseModel::visibleChanged, this, [ = ](bool isVisible) {
        if (!isVisible || model->currentType() != SessionBaseModel::AuthType::LockType)
{
    TRACE_ME_OUT;	//<<==--TracePoint!
    return;
}

        std::shared_ptr<User> user_ptr = model->currentUser();
        if (!user_ptr.get())
{
    TRACE_ME_OUT;	//<<==--TracePoint!
    return;
}

        if (user_ptr->type() == User::ADDomain && user_ptr->uid() == 0)
{
    TRACE_ME_OUT;	//<<==--TracePoint!
    return;
}

        m_authFramework->Authenticate(user_ptr);
    });

    connect(m_lockInter, &DBusLockService::Event, this, &LockWorker::lockServiceEvent);
    connect(model, &SessionBaseModel::onStatusChanged, this, [ = ](SessionBaseModel::ModeStatus status) {
        if (status == SessionBaseModel::ModeStatus::PowerMode) {
            checkPowerInfo();
        }
    });

    connect(m_sessionManager, &SessionManager::Unlock, this, [ = ] {
        m_authenticating = false;
        m_password.clear();
        emit m_model->authFinished(true);
    });

    connect(m_login1SessionSelf, &Login1SessionSelf::ActiveChanged, this, [ = ](bool active) {
        if (!active) {
            m_canAuthenticate = true;
        } else if(m_canAuthenticate) {
            m_canAuthenticate = false;
            m_authenticating = false;
            m_authFramework->Authenticate(m_model->currentUser());
        }
    });

    //因为部分机器在待机休眠唤醒后无法发出Login1SessionSelf::ActiveChange信号，从而无法请求验证密码，增加此信号连接重新请求验证
    connect(m_login1Inter, &DBusLogin1Manager::PrepareForSleep, this, [ = ](bool sleep) {
        if (sleep) {
            m_canAuthenticate = true;
        } else if(m_canAuthenticate) {
            m_canAuthenticate = false;
            m_authenticating = false;
            m_authFramework->Authenticate(m_model->currentUser());
        }
    });

    const bool &LockNoPasswordValue { valueByQSettings<bool>("", "lockNoPassword", false) };
    m_model->setIsLockNoPassword(LockNoPasswordValue);

    const QString &switchUserButtonValue { valueByQSettings<QString>("Lock", "showSwitchUserButton", "ondemand") };
    m_model->setAlwaysShowUserSwitchButton(switchUserButtonValue == "always");
    m_model->setAllowShowUserSwitchButton(switchUserButtonValue == "ondemand");

    {
        initDBus();
        initData();
    }

    // init ADDomain User
    if (DSysInfo::deepinType() == DSysInfo::DeepinServer || valueByQSettings<bool>("", "loginPromptInput", false)) {
        std::shared_ptr<User> user = std::make_shared<ADDomainUser>(INT_MAX);
        static_cast<ADDomainUser *>(user.get())->setUserDisplayName("...");
        static_cast<ADDomainUser *>(user.get())->setIsServerUser(true);
        m_model->setIsServerModel(true);
        m_model->userAdd(user);
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockWorker::switchToUser(std::shared_ptr<User> user)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    qWarning() << "switch user from" << m_model->currentUser()->name() << " to " << user->name();

    // clear old password
    m_password.clear();
    m_authenticating = false;

    // if type is lock, switch to greeter
    QJsonObject json;
    json["Uid"] = static_cast<int>(user->uid());
    json["Type"] = user->type();

    m_lockInter->SwitchToUser(QString(QJsonDocument(json).toJson(QJsonDocument::Compact))).waitForFinished();

    if (user->isLogin()) {
        QProcess::startDetached("dde-switchtogreeter", QStringList() << user->name());
    } else {
        QProcess::startDetached("dde-switchtogreeter");
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockWorker::authUser(const QString &password)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (m_authenticating)
{
    TRACE_ME_OUT;	//<<==--TracePoint!
    return;
}

    m_authenticating = true;

    // auth interface
    std::shared_ptr<User> user = m_model->currentUser();

    m_password = password;

    qWarning() << "start authentication of user: " << user->name();

    // 服务器登录输入用户与当前用户不同时给予提示
    if (m_currentUserUid != user->uid()) {
        QTimer::singleShot(800, this, [ = ] {
            onUnlockFinished(false);
        });
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    }

    if(!m_authFramework->isAuthenticate())
        m_authFramework->Authenticate(user);

    m_authFramework->Responsed(password);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockWorker::enableZoneDetected(bool disable)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_hotZoneInter->EnableZoneDetected(disable);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockWorker::onDisplayErrorMsg(const QString &msg)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    emit m_model->authFaildTipsMessage(msg);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockWorker::onDisplayTextInfo(const QString &msg)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_authenticating = false;
    emit m_model->authFaildMessage(msg);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockWorker::onPasswordResult(const QString &msg)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    onUnlockFinished(!msg.isEmpty());

    if(msg.isEmpty()) {
        m_authFramework->Authenticate(m_model->currentUser());
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockWorker::onUserAdded(const QString &user)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    std::shared_ptr<NativeUser> user_ptr(new NativeUser(user));

    if (!user_ptr->isUserIsvalid()) {
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    }

    user_ptr->setisLogind(isLogined(user_ptr->uid()));

    if (user_ptr->uid() == m_currentUserUid) {
        m_model->setCurrentUser(user_ptr);

        // AD domain account auth will not be activated for the first time
        connect(user_ptr->getUserInter(), &UserInter::UserNameChanged, this, [ = ] {
            updateLockLimit(user_ptr);
        });
    }

    if (user_ptr->uid() == m_lastLogoutUid) {
        m_model->setLastLogoutUser(user_ptr);
    }

    m_model->userAdd(user_ptr);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockWorker::lockServiceEvent(quint32 eventType, quint32 pid, const QString &username, const QString &message)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (!m_model->currentUser())
{
    TRACE_ME_OUT;	//<<==--TracePoint!
    return;
}

    if (username != m_model->currentUser()->name()) {
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    }

    // Don't show password prompt from standard pam modules since
    // we'll provide our own prompt or just not.
    const QString msg = message.simplified() == "Password:" ? "" : message;

    m_authenticating = false;

    if (msg == "Verification timed out") {
        m_isThumbAuth = true;
        emit m_model->authFaildMessage(tr("Fingerprint verification timed out, please enter your password manually"));
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    }

    switch (eventType) {
    case DBusLockService::PromptQuestion:
        qWarning() << "prompt quesiton from pam: " << message;
        emit m_model->authFaildMessage(message);
        break;
    case DBusLockService::PromptSecret:
        qWarning() << "prompt secret from pam: " << message;
        if (m_isThumbAuth && !msg.isEmpty()) {
            emit m_model->authFaildMessage(msg);
        }
        break;
    case DBusLockService::ErrorMsg:
        qWarning() << "error message from pam: " << message;
        if (msg == "Failed to match fingerprint") {
            emit m_model->authFaildTipsMessage(tr("Failed to match fingerprint"));
            emit m_model->authFaildMessage("");
        }
        break;
    case DBusLockService::TextInfo:
        emit m_model->authFaildMessage(QString(dgettext("fprintd", message.toLatin1())));
        break;
    case DBusLockService::Failure:
        onUnlockFinished(false);
        break;
    case DBusLockService::Success:
        onUnlockFinished(true);
        break;
    default:
        break;
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockWorker::onUnlockFinished(bool unlocked)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    qWarning() << "LockWorker::onUnlockFinished -- unlocked status : " << unlocked;
    emit m_model->authFinished(unlocked);

    m_authenticating = false;

    if (!unlocked && m_authFramework->GetAuthType() == AuthFlag::Password) {
        qWarning() << "Authorization password failed!";
        emit m_model->authFaildTipsMessage(tr("Wrong Password"));
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    }

    switch (m_model->powerAction()) {
    case SessionBaseModel::PowerAction::RequireRestart:
        m_model->setPowerAction(SessionBaseModel::PowerAction::RequireRestart);
        if (unlocked) {
            m_sessionManager->RequestReboot();
        }
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    case SessionBaseModel::PowerAction::RequireShutdown:
        m_model->setPowerAction(SessionBaseModel::PowerAction::RequireShutdown);
        if (unlocked) {
            m_sessionManager->RequestShutdown();
        }
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    default:
        break;
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockWorker::onCurrentUserChanged(const QString &user)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    qWarning() << "LockWorker::onCurrentUserChanged -- change to :" << user;
    const QJsonObject obj = QJsonDocument::fromJson(user.toUtf8()).object();
    auto user_cur = static_cast<uint>(obj["Uid"].toInt());
    if (user_cur == m_currentUserUid) {
        for (std::shared_ptr<User> user_ptr : m_model->userList()) {
            if (user_ptr->uid() == m_currentUserUid) {
                m_authFramework->Authenticate(user_ptr);
                break;
            }
        }
    }
    emit m_model->switchUserFinished();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

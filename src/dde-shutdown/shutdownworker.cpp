
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

#include "shutdownworker.h"
#include "src/session-widgets/userinfo.h"
#include "src/session-widgets/sessionbasemodel.h"
#include <unistd.h>

using namespace Auth;

ShutdownWorker::ShutdownWorker(SessionBaseModel * const model, QObject *parent)
    : AuthInterface(model, parent)
    , m_hotZoneInter(new DBusHotzone("com.deepin.daemon.Zone", "/com/deepin/daemon/Zone", QDBusConnection::sessionBus(), this))
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (valueByQSettings<bool>("", "loginPromptAvatar", true)) {
        initDBus();
        initData();

        m_currentUserUid = getuid();
        onUserAdded(ACCOUNTS_DBUS_PREFIX + QString::number(m_currentUserUid));
        model->setCurrentUser(model->findUserByUid(getuid()));
    }

    connect(model, &SessionBaseModel::onStatusChanged, this, [ = ](SessionBaseModel::ModeStatus status) {
        if (status == SessionBaseModel::ModeStatus::PowerMode) {
            checkPowerInfo();
        }
    });
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownWorker::switchToUser(std::shared_ptr<User> user)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Q_UNUSED(user);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownWorker::authUser(const QString &password)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Q_UNUSED(password);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownWorker::enableZoneDetected(bool enable)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_hotZoneInter->EnableZoneDetected(enable);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

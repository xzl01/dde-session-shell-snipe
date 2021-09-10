
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

#include "lockcontent.h"

#include "src/widgets/controlwidget.h"
#include "sessionbasemodel.h"
#include "userframe.h"
#include "src/widgets/shutdownwidget.h"
#include "src/widgets/virtualkbinstance.h"
#include "src/widgets/logowidget.h"
#include "src/widgets/timewidget.h"
#include "userlogininfo.h"
#include "userloginwidget.h"
#include "userframelist.h"

#include <QMouseEvent>
#include <DDBusSender>

LockContent::LockContent(SessionBaseModel *const model, QWidget *parent)
    : SessionBaseWindow(parent)
    , m_model(model)
    , m_virtualKB(nullptr)
    , m_translator(new QTranslator)
    , m_userLoginInfo(new UserLoginInfo(model))
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_controlWidget = new ControlWidget;
    m_shutdownFrame = new ShutdownWidget;
    m_logoWidget = new LogoWidget;

    m_timeWidget = new TimeWidget();
    // 处理时间制跳转策略，获取到时间制再显示时间窗口
    m_timeWidget->setVisible(false);

    m_mediaWidget = nullptr;

    m_shutdownFrame->setModel(model);
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);

    setCenterTopWidget(m_timeWidget);
    setLeftBottomWidget(m_logoWidget);
    connect(model, &SessionBaseModel::currentUserChanged, this, [ = ](std::shared_ptr<User> user) {
        if (user.get()) {
            m_logoWidget->updateLocale(user->locale().split(".").first());
        }
    });

    switch (model->currentType()) {
    case SessionBaseModel::AuthType::LockType:
        setMPRISEnable(true);
        break;
    default:
        break;
    }
    setRightBottomWidget(m_controlWidget);

    // init connect
    connect(model, &SessionBaseModel::currentUserChanged, this, &LockContent::onCurrentUserChanged);
    connect(m_controlWidget, &ControlWidget::requestSwitchUser, this, [ = ] {
        if (m_model->currentModeState() == SessionBaseModel::ModeStatus::UserMode)
{
    TRACE_ME_OUT;	//<<==--TracePoint!
    return;
}
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::UserMode);
    });
    connect(m_controlWidget, &ControlWidget::requestShutdown, this, [ = ] {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PowerMode);
    });
    connect(m_controlWidget, &ControlWidget::requestSwitchVirtualKB, this, &LockContent::toggleVirtualKB);

    //lixin
    //connect(m_userLoginInfo, &UserLoginInfo::requestAuthUser, this, &LockContent::restoreMode);
    connect(m_userLoginInfo, &UserLoginInfo::requestAuthUser, this, &LockContent::requestAuthUser);
    connect(m_userLoginInfo, &UserLoginInfo::hideUserFrameList, this, &LockContent::restoreMode);
    connect(m_userLoginInfo, &UserLoginInfo::requestSwitchUser, this, &LockContent::requestSwitchToUser);
    connect(m_userLoginInfo, &UserLoginInfo::switchToCurrentUser, this, &LockContent::restoreMode);
    connect(m_userLoginInfo, &UserLoginInfo::requestSetLayout, this, &LockContent::requestSetLayout);
    connect(m_userLoginInfo, &UserLoginInfo::unlockActionFinish, this, [&]{
        emit unlockActionFinish();
    });
    connect(m_shutdownFrame, &ShutdownWidget::abortOperation, this, [ = ] {
        if (m_model->powerAction() != SessionBaseModel::RequireShutdown &&
                m_model->powerAction() != SessionBaseModel::RequireRestart)
            restoreMode();
    });

    if (m_model->currentType() == SessionBaseModel::LockType) {
        // 锁屏，点击关机，需要提示“输入密码以关机”。登录不需要这个提示
        connect(m_shutdownFrame, &ShutdownWidget::abortOperation, m_userLoginInfo, [ = ] {
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ConfirmPasswordMode);
        });
    }

    connect(model, &SessionBaseModel::onStatusChanged, this, &LockContent::onStatusChanged);

    //在锁屏显示时，启动onborad进程，锁屏结束时结束onboard进程
    auto initVirtualKB = [&](bool hasvirtualkb) {
        if (hasvirtualkb && !m_virtualKB) {
            connect(&VirtualKBInstance::Instance(), &VirtualKBInstance::initFinished, this, [&] {
                m_virtualKB = VirtualKBInstance::Instance().virtualKBWidget();
                m_controlWidget->setVirtualKBVisible(true);
            }, Qt::QueuedConnection);
            VirtualKBInstance::Instance().init();
        } else {
            VirtualKBInstance::Instance().stopVirtualKBProcess();
            m_virtualKB = nullptr;
            m_controlWidget->setVirtualKBVisible(false);
        }
    };

    connect(model, &SessionBaseModel::hasVirtualKBChanged, this, initVirtualKB, Qt::QueuedConnection);
    connect(model, &SessionBaseModel::onUserListChanged, this, &LockContent::onUserListChanged);
    connect(model, &SessionBaseModel::userListLoginedChanged, this, &LockContent::onUserListChanged);
    connect(model, &SessionBaseModel::authFinished, this, &LockContent::restoreMode);
    connect(model, &SessionBaseModel::switchUserFinished, this, [ = ] {
        QTimer::singleShot(100, this, [ = ] {
            emit LockContent::restoreMode();
        });
    });

    QTimer::singleShot(0, this, [ = ] {
        onCurrentUserChanged(model->currentUser());
        onUserListChanged(model->isServerModel() ? model->logindUser() : model->userList());
    });
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::onCurrentUserChanged(std::shared_ptr<User> user)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (user.get() == nullptr)
{
    TRACE_ME_OUT;	//<<==--TracePoint!
    return;
} // if dbus is async

    // set user language
    qApp->removeTranslator(m_translator);
    const QString locale { user->locale().split(".").first() };
    m_translator->load("/usr/share/dde-session-shell/translations/dde-session-shell_" + QLocale(locale.isEmpty() ? "en_US" : locale).name());
    qApp->installTranslator(m_translator);

    m_logoWidget->updateLocale(user->locale());
    m_timeWidget->updateLocale(user->locale());

    for (auto connect : m_currentUserConnects) {
        m_user.get()->disconnect(connect);
    }

    m_currentUserConnects.clear();

    m_user = user;

    std::shared_ptr<NativeUser> nativeUser = std::dynamic_pointer_cast<NativeUser>(user);
    UserInter *userInter = nullptr;
    if (!nativeUser) {
        m_timeWidget->setVisible(true);
        qDebug() << "trans to NativeUser user failed, user:" << user->metaObject()->className();
    } else {
        userInter = nativeUser->getUserInter();
    }

    m_currentUserConnects << connect(user.get(), &User::greeterBackgroundPathChanged, this, &LockContent::requestBackground, Qt::UniqueConnection)
                          << connect(userInter, &UserInter::Use24HourFormatChanged, this, &LockContent::updateTimeFormat, Qt::UniqueConnection)
                          << connect(userInter, &UserInter::WeekdayFormatChanged, m_timeWidget, &TimeWidget::setWeekdayFormatType)
                          << connect(userInter, &UserInter::ShortDateFormatChanged, m_timeWidget, &TimeWidget::setShortDateFormat)
                          << connect(userInter, &UserInter::ShortTimeFormatChanged, m_timeWidget, &TimeWidget::setShortTimeFormat);

    //lixin
    m_userLoginInfo->setUser(user);

    //TODO: refresh blur image
    QTimer::singleShot(0, this, [ = ] {
        if (userInter) {
            userInter->weekdayFormat();
            userInter->shortDateFormat();
            userInter->shortTimeFormat();
        }

        // 异步刷新界面时间格式
        user->is24HourFormat();

        m_user->greeterBackgroundPath();
    });

    m_logoWidget->updateLocale(m_user->locale());
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::pushPasswordFrame()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (m_model->currentModeState() == SessionBaseModel::ModeStatus::ConfirmPasswordMode) {
        m_model->setAbortConfirm(false);
    } else {
        restoreMode();
        setCenterContent(m_userLoginInfo->getUserLoginWidget());
    }

    // hide keyboardlayout widget
    m_userLoginInfo->hideKBLayout();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::pushUserFrame()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if(m_model->isServerModel())
        m_controlWidget->setUserSwitchEnable(false);

    //设置用户列表大小为中间区域的大小
    UserFrameList * userFrameList = m_userLoginInfo->getUserFrameList();
    userFrameList->setFixedSize(getCenterContentSize());
    setCenterContent(userFrameList);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::pushConfirmFrame()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    setCenterContent(m_userLoginInfo->getUserLoginWidget());
    m_model->setAbortConfirm(true);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::pushShutdownFrame()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QSize size = getCenterContentSize();
    m_shutdownFrame->setFixedSize(size);
    setCenterContent(m_shutdownFrame);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::setMPRISEnable(const bool state)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (m_mediaWidget) {
        m_mediaWidget->setVisible(state);
    } else {
        m_mediaWidget = new MediaWidget;
        m_mediaWidget->initMediaPlayer();
        setCenterBottomWidget(m_mediaWidget);
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::beforeUnlockAction(bool is_finish)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_userLoginInfo->beforeUnlockAction(is_finish);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::onStatusChanged(SessionBaseModel::ModeStatus status)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if(m_model->isServerModel())
        onUserListChanged(m_model->logindUser());
    switch (status) {
    case SessionBaseModel::ModeStatus::PasswordMode:
        restoreCenterContent();
        break;
    case SessionBaseModel::ModeStatus::ConfirmPasswordMode:
        pushConfirmFrame();
        break;
    case SessionBaseModel::ModeStatus::UserMode:
        pushUserFrame();
        break;
    case SessionBaseModel::ModeStatus::PowerMode:
        pushShutdownFrame();
        break;
    default:
        break;
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::mouseReleaseEvent(QMouseEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    pushPasswordFrame();

    TRACE_ME_OUT;	//<<==--TracePoint!
    return SessionBaseWindow::mouseReleaseEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::showEvent(QShowEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    onStatusChanged(m_model->currentModeState());

    QFrame::showEvent(event);
    tryGrabKeyboard();
    TRACE_ME_OUT;	//<<==--TracePoint!
 
}

void LockContent::hideEvent(QHideEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return QFrame::hideEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::resizeEvent(QResizeEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QTimer::singleShot(0, this, [ = ] {
        if (m_virtualKB && m_virtualKB->isVisible())
        {
            updateVirtualKBPosition();
        }
    });

    TRACE_ME_OUT;	//<<==--TracePoint!
    return QFrame::resizeEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::restoreCenterContent()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    auto current_user = m_model->currentUser();
    if ((m_model->powerAction() == SessionBaseModel::RequireShutdown)
            || (m_model->powerAction() == SessionBaseModel::RequireRestart)) {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ConfirmPasswordMode);
    } else {
        restoreMode();
    }
    setCenterContent(m_userLoginInfo->getUserLoginWidget());
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::restoreMode()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::updateTimeFormat(bool use24)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (m_user != nullptr) {
        m_timeWidget->updateLocale(m_user->locale());
        m_timeWidget->set24HourFormat(use24);
        m_timeWidget->setVisible(true);
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::toggleVirtualKB()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (!m_virtualKB) {
        VirtualKBInstance::Instance();
        QTimer::singleShot(500, this, [ = ] {
            m_virtualKB = VirtualKBInstance::Instance().virtualKBWidget();
            qDebug() << "init virtualKB over." << m_virtualKB;
            toggleVirtualKB();
        });
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    }

    m_virtualKB->setParent(this);
    m_virtualKB->raise();
    m_userLoginInfo->getUserLoginWidget()->setPassWordEditFocus();

    updateVirtualKBPosition();
    m_virtualKB->setVisible(!m_virtualKB->isVisible());
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::updateVirtualKBPosition()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    const QPoint point = mapToParent(QPoint((width() - m_virtualKB->width()) / 2, height() - m_virtualKB->height() - 50));
    m_virtualKB->move(point);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::onUserListChanged(QList<std::shared_ptr<User> > list)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    const bool allowShowUserSwitchButton = m_model->allowShowUserSwitchButton();
    const bool alwaysShowUserSwitchButton = m_model->alwaysShowUserSwitchButton();
    bool haveLogindUser = true;

    if (m_model->isServerModel() && m_model->currentType() == SessionBaseModel::LightdmType) {
        haveLogindUser = !m_model->logindUser().isEmpty();
    }
    m_controlWidget->setUserSwitchEnable((alwaysShowUserSwitchButton ||
                                          (allowShowUserSwitchButton &&
                                           (list.size() > (m_model->isServerModel() ? 0 : 1)))) &&
                                         haveLogindUser);
                                         TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::tryGrabKeyboard()
{
TRACE_ME_IN;	//<<==--TracePoint!
#ifdef QT_DEBUG
    TRACE_ME_OUT;	//<<==--TracePoint!
    return;
#endif

    if (window()->windowHandle() && isVisible() && window()->windowHandle()->setKeyboardGrabEnabled(true)) {
        m_failures = 0;
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    }

    m_failures++;

    if (m_failures == 15) {
        qDebug() << "Trying grabkeyboard has exceeded the upper limit. dde-lock will quit.";
        m_model->setLocked(false);

        DDBusSender()
                .service("org.freedesktop.Notifications")
                .path("/org/freedesktop/Notifications")
                .interface("org.freedesktop.Notifications")
                .method(QString("Notify"))
                .arg(tr("Lock Screen"))
                .arg(static_cast<uint>(0))
                .arg(QString(""))
                .arg(QString(""))
                .arg(tr("Failed to lock screen"))
                .arg(QStringList())
                .arg(QVariantMap())
                .arg(5000)
                .call();

        qApp->quit();
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    }

    QTimer::singleShot(100, this, &LockContent::tryGrabKeyboard);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockContent::keyPressEvent(QKeyEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    switch (event->key()) {
    case Qt::Key_Escape: {
        if (m_model->currentModeState() == SessionBaseModel::ModeStatus::ConfirmPasswordMode)
            m_model->setAbortConfirm(false);
        break;
    }
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

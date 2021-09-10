
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

#include "userframe.h"

#include "src/widgets/userbutton.h"
#include "src/session-widgets/sessionbasemodel.h"
#include "src/session-widgets/userinfo.h"
#include "src/session-widgets/framedatabind.h"

#define USER_WIDGET_HEIGHT 360
#define USER_HEIGHT 180

UserFrame::UserFrame(QWidget *parent)
    : QWidget(parent)
    , m_isExpansion(false)
    , m_frameDataBind(FrameDataBind::Instance())
{
    TRACE_ME_IN;	//<<==--TracePoint!
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    std::function<void (QVariant)> function = std::bind(&UserFrame::onOtherPageChanged, this, std::placeholders::_1);
    int index = m_frameDataBind->registerFunction("UserFrame", function);

    connect(this, &UserFrame::destroyed, this, [ = ] {
        m_frameDataBind->unRegisterFunction("UserFrame", index);
    });
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserFrame::setModel(SessionBaseModel *model)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_model = model;

    connect(model, &SessionBaseModel::onUserAdded, this, &UserFrame::userAdded);
    connect(model, &SessionBaseModel::onUserRemoved, this, &UserFrame::userRemoved);

    QList<std::shared_ptr<User>> userList = m_model->userList();

    for (auto user : userList) {
        userAdded(user);
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserFrame::resizeEvent(QResizeEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QTimer::singleShot(0, this, &UserFrame::refreshPosition);

    TRACE_ME_OUT;	//<<==--TracePoint!
    return QWidget::resizeEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserFrame::mouseReleaseEvent(QMouseEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    emit hideFrame();

    expansion(false);

    hide();

    TRACE_ME_OUT;	//<<==--TracePoint!
    return QWidget::mouseReleaseEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserFrame::hideEvent(QHideEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    expansion(false);

    TRACE_ME_OUT;	//<<==--TracePoint!
    return QWidget::hideEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserFrame::keyPressEvent(QKeyEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    switch (event->key()) {
    case Qt::Key_Left:
        switchPreviousUser();
        break;
    case Qt::Key_Right:
        switchNextUser();
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        for (auto it = m_userBtns.constBegin(); it != m_userBtns.constEnd(); ++it) {
            if (it.value()->selected()) {
                emit it.value()->clicked(true);
                break;
            }
        }
        break;
    case Qt::Key_Escape:
        emit hideFrame();
        break;
    default:
        break;
    }
    TRACE_ME_OUT;	//<<==--TracePoint!
    return QWidget::keyPressEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserFrame::userAdded(std::shared_ptr<User> user)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    UserButton *button = new UserButton(user, this);
    m_userBtns[user->uid()] = button;

    connect(button, &UserButton::clicked, this, &UserFrame::onUserClicked);

    refreshPosition();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserFrame::userRemoved(const uint uid)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    UserButton *button = m_userBtns[uid];
    m_userBtns.remove(uid);
    button->deleteLater();

    refreshPosition();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserFrame::expansion(bool expansion)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_isExpansion = expansion;

    refreshPosition();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserFrame::refreshPosition()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (m_isExpansion) {
        const int cout = m_userBtns.size();
        const int maxLineCap = width() / USER_ICON_WIDTH - 1; // 1 for left-offset and right-offset.
        const int offset = (width() - USER_ICON_WIDTH * std::min(cout, maxLineCap)) / 2;

        int row = 0;
        int index = 0;

        for (auto it = m_userBtns.constBegin(); it != m_userBtns.constEnd(); ++it) {
            UserButton *button = it.value();
            button->stopAnimation();
            button->show();
            button->setImageSize(UserButton::AvatarSmallSize);
            // If the current value is the maximum, need to change the line.
            if (index >= maxLineCap) {
                index = 0;
                ++row;
            }

            button->move(QPoint(offset + index * USER_ICON_WIDTH, USER_HEIGHT * row));
            index++;
        }

        setFixedHeight(USER_HEIGHT * (row + 1));
    } else {
        for (auto it = m_userBtns.constBegin(); it != m_userBtns.constEnd(); ++it) {
            it.value()->move(QPoint((width() - it.value()->width()) / 2, 0));
        }
    }

    std::shared_ptr<User> user = m_model->currentUser();
    if (user.get() == nullptr)
{
    TRACE_ME_OUT;	//<<==--TracePoint!
    return;
}

    for (auto it = m_userBtns.constBegin(); it != m_userBtns.constEnd(); ++it) {
        it.value()->setSelected(it.key() == user->uid());
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserFrame::onUserClicked()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    UserButton *button = static_cast<UserButton *>(sender());

    expansion(false);

    emit requestSwitchUser(m_model->findUserByUid(m_userBtns.key(button)));
    emit hideFrame();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserFrame::switchNextUser()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QList<UserButton *> btns = m_userBtns.values();

    for (int i = 0; i != btns.size(); ++i) {
        if (btns[i]->selected()) {
            btns[i]->setSelected(false);
            if (i == (btns.size() - 1)) {
                btns.first()->setSelected(true);
                m_frameDataBind->updateValue("UserFrame", m_userBtns.key(m_userBtns.first()));
            } else {
                btns[i + 1]->setSelected(true);
                m_frameDataBind->updateValue("UserFrame", m_userBtns.key(btns[i + 1]));
            }
            break;
        }
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserFrame::switchPreviousUser()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QList<UserButton *> btns = m_userBtns.values();

    for (int i = 0; i != btns.size(); ++i) {
        if (btns[i]->selected()) {
            btns[i]->setSelected(false);
            if (i == 0) {
                btns.last()->setSelected(true);
                m_frameDataBind->updateValue("UserFrame", m_userBtns.key(m_userBtns.last()));
            } else {
                btns[i - 1]->setSelected(true);
                m_frameDataBind->updateValue("UserFrame", m_userBtns.key(btns[i - 1]));
            }
            break;
        }
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserFrame::onOtherPageChanged(const QVariant &value)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QList<UserButton *> btns = m_userBtns.values();

    for (UserButton *btn : btns) {
        btn->setSelected(false);
    }

    m_userBtns[value.toInt()]->setSelected(true);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

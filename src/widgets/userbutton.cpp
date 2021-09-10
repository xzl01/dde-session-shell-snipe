
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtGui/QPainter>
#include <QDebug>
#include <QTimer>
#include <QSettings>

#include "dhidpihelper.h"
#include "userbutton.h"
#include "src/session-widgets/userinfo.h"

DWIDGET_USE_NAMESPACE

UserButton::UserButton(std::shared_ptr<User> user, QWidget *parent)
    : QPushButton(parent)

    , m_user(user)
    , m_selected(false)
    , m_opacity(0)
#ifndef DISABLE_ANIMATIONS
    , m_moveAni(new QPropertyAnimation(this, "pos"))
    , m_showAnimation(new QPropertyAnimation(this, "opacity"))
    , m_hideAnimation(new QPropertyAnimation(this, "opacity"))
#endif
{
    TRACE_ME_IN;	//<<==--TracePoint!
    initUI();
    initConnect();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserButton::initConnect()
{
TRACE_ME_IN;	//<<==--TracePoint!
#ifndef DISABLE_ANIMATIONS
    connect(m_hideAnimation, &QPropertyAnimation::finished, this, &QPushButton::hide);
#endif
    connect(m_userAvatar, &UserAvatar::clicked, this, &UserButton::click);
    connect(m_user.get(), &User::displayNameChanged, m_userNameLabel, &QLabel::setText);
    connect(m_user.get(), &User::avatarChanged, this, [=] (const QString avatar) {
        m_userAvatar->setIcon(avatar);
    });
    connect(m_user.get(), &User::logindChanged, m_checkedMark, &QLabel::setVisible);

    m_checkedMark->setVisible(m_user.get()->isLogin());
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserButton::initUI()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    setFixedSize(USER_ICON_WIDTH, USER_ICON_HEIGHT);
    setFocusPolicy(Qt::NoFocus);

    m_userAvatar = new UserAvatar;
    m_userAvatar->setAvatarSize(UserAvatar::AvatarLargeSize);
    m_userAvatar->setFixedSize(120, 120);

    m_userNameLabel = new QLabel;

    QFont font(m_userNameLabel->font());
    font.setPixelSize(16);
    m_userNameLabel->setFont(font);
    m_userNameLabel->setText(m_user->displayName());
    m_userNameLabel->setStyleSheet("QLabel {"
                                   "text-align:center; "
                                   "font-size: 16px;"
                                   "font-weight: normal;"
                                   "font-style: normal;"
                                   "line-height: normal;"
                                   "text-align: center;"
                                   "color: #ffffff;"
                                   "}");

    m_userAvatar->setIcon(m_user->avatarPath());

    m_checkedMark = new QLabel;

    QPixmap pixmap = DHiDPIHelper::loadNxPixmap(":img/select.svg");
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    m_checkedMark->setPixmap(pixmap);

    m_nameLayout = new QHBoxLayout;
    m_nameLayout->setSpacing(5);
    m_nameLayout->setMargin(0);
    m_nameLayout->addStretch();
    m_nameLayout->addWidget(m_checkedMark);
    m_nameLayout->addWidget(m_userNameLabel);
    m_nameLayout->addStretch();

    m_centralLayout = new QVBoxLayout;
    m_centralLayout->setMargin(0);
    m_centralLayout->setSpacing(0);
    m_centralLayout->addWidget(m_userAvatar);
    m_centralLayout->setAlignment(m_userAvatar, Qt::AlignHCenter);
    m_centralLayout->addLayout(m_nameLayout);
    m_centralLayout->addStretch();

    setStyleSheet("QPushButton {"
                  "background-color: transparent;"
                  "border: none;"
                  "}");

    setLayout(m_centralLayout);

    m_opacityEffect = new QGraphicsOpacityEffect;

    connect(this, &UserButton::opacityChanged, &UserButton::setCustomEffect);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserButton::setImageSize(const AvatarSize &avatarsize) {
    TRACE_ME_IN;	//<<==--TracePoint!
    if (avatarsize==AvatarLargerSize) {
        m_userAvatar->setAvatarSize(m_userAvatar->AvatarLargeSize);
    } else {
        m_userAvatar->setAvatarSize(m_userAvatar->AvatarSmallSize);
    }

    m_avatarsize = avatarsize;
    update();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserButton::show()
{
TRACE_ME_IN;	//<<==--TracePoint!
#ifndef DISABLE_ANIMATIONS
    m_showAnimation->stop();
    m_showAnimation->setStartValue(0.0);
    m_showAnimation->setEndValue(1.0);
    m_showAnimation->setDuration(800);
    m_showAnimation->start();

    connect(m_showAnimation, &QPropertyAnimation::finished, [&]{
        QTimer::singleShot(500, this, SLOT(addTextShadowAfter()));
    });
#endif
    QPushButton::show();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserButton::addTextShadowAfter()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_opacityEffect->setEnabled(false);
    addTextShadow(true);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserButton::hide()
{
TRACE_ME_IN;	//<<==--TracePoint!
#ifndef DISABLE_ANIMATIONS
    m_hideAnimation->setStartValue(1);
    m_hideAnimation->setEndValue(0);
    m_hideAnimation->start();
#else
    QPushButton::hide();
#endif

#ifndef DISABLE_TEXT_SHADOW
    addTextShadow(false);
#endif

    m_opacityEffect->setEnabled(true);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserButton::move(const QPoint &position, bool immediately)
{
TRACE_ME_IN;	//<<==--TracePoint!
#ifndef DISABLE_ANIMATIONS
    const bool play_ani = !immediately;

    if (play_ani)
    {
        m_moveAni->stop();
        m_moveAni->setDuration(200);
        m_moveAni->setStartValue(pos());
        m_moveAni->setEndValue(position);

        m_moveAni->start();
    } else {
        QPushButton::move(position);
    }
#else
   Q_UNUSED(immediately);
   QPushButton::move(position);
#endif
TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserButton::addTextShadow(bool isEffective)
{
TRACE_ME_IN;	//<<==--TracePoint!
#ifndef DISABLE_TEXT_SHADOW
    QGraphicsDropShadowEffect *nameShadow = new QGraphicsDropShadowEffect;
    nameShadow->setBlurRadius(16);
    nameShadow->setColor(QColor(0, 0, 0, 85));
    nameShadow->setOffset(0, 4);
    nameShadow->setEnabled(isEffective);
    m_userNameLabel->setGraphicsEffect(nameShadow);
#endif
TRACE_ME_OUT;	//<<==--TracePoint!

}

bool UserButton::selected() const
{
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return m_selected;
}

void UserButton::setSelected(bool selected)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_selected = selected;
    update();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserButton::paintEvent(QPaintEvent* event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QPushButton::paintEvent(event);

    if (!m_selected) {
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;}

    QPainter painter(this);
    painter.setPen(QPen(QColor(255, 255, 255, 51), 2));
    painter.setBrush(QColor(0, 0 , 0, 76));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawRoundedRect(QRect(2, 2, width() - 4, height() - 4), 10, 10, Qt::RelativeSize);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserButton::stopAnimation()
{
TRACE_ME_IN;	//<<==--TracePoint!
#ifndef DISABLE_ANIMATIONS
    m_moveAni->stop();
#endif
TRACE_ME_OUT;	//<<==--TracePoint!

}

double UserButton::opacity() {
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return m_opacity;
}

void UserButton::setOpacity(double opa) {
    TRACE_ME_IN;	//<<==--TracePoint!
    if (m_opacity != opa) {
        m_opacity = opa;
        emit opacityChanged();
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserButton::setCustomEffect() {
    TRACE_ME_IN;	//<<==--TracePoint!
    m_opacityEffect->setOpacity(m_opacity);
    setGraphicsEffect(m_opacityEffect);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

UserButton::~UserButton()
{}

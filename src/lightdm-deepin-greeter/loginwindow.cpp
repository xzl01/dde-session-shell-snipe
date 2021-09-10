
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
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

#include "loginwindow.h"
#include "logincontent.h"
#include "src/session-widgets/userinfo.h"

#include <QWindow>

LoginWindow::LoginWindow(SessionBaseModel *const model, QWidget *parent)
    : FullscreenBackground(parent)
    , m_loginContent(new LoginContent(model, this))
    , m_model(model)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QPixmap image(this->size());
    image.fill(Qt::black);
    updateBackground(image);

    QTimer::singleShot(0, this, [ = ] {
        auto user = model->currentUser();
        if (user != nullptr) updateBackground(QPixmap(user->greeterBackgroundPath()));
    });

    setContent(m_loginContent);
    m_loginContent->hide();

    connect(m_loginContent, &LockContent::requestBackground, this, [ = ](const QString & wallpaper) {
        updateBackground(wallpaper);
#ifdef DISABLE_LOGIN_ANI
        // 在认证成功以后会通过更改背景来实现登录动画，但是禁用登录动画的情况下，会立即调用startSession，
        // 导致当前进程被lightdm退掉，X上会残留上一帧的画面，可以看到输入框等画面。使用repaint()强制刷新背景来避免这个问题。
        repaint();
#endif
    });

    connect(model, &SessionBaseModel::authFinished, this, [ = ](bool successd) {
        enableEnterEvent(!successd);
        if (successd) {
            m_loginContent->setVisible(!successd);
        }

#ifdef DISABLE_LOGIN_ANI
        // 在认证成功以后会通过更改背景来实现登录动画，但是禁用登录动画的情况下，会立即调用startSession，
        // 导致当前进程被lightdm退掉，X上会残留上一帧的画面，可以看到输入框等画面。使用repaint()强制刷新背景来避免这个问题。
        repaint();
#endif
    });

    connect(m_loginContent, &LockContent::requestAuthUser, this, &LoginWindow::requestAuthUser);
    connect(m_loginContent, &LockContent::requestSwitchToUser, this, &LoginWindow::requestSwitchToUser);
    connect(m_loginContent, &LockContent::requestSetLayout, this, &LoginWindow::requestSetLayout);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LoginWindow::resizeEvent(QResizeEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return FullscreenBackground::resizeEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LoginWindow::showEvent(QShowEvent *event)
{
    //greeter界面显示时，需要调用虚拟键盘
    TRACE_ME_IN;	//<<==--TracePoint!
    FullscreenBackground::showEvent(event);

    m_model->setHasVirtualKB(true);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LoginWindow::hideEvent(QHideEvent *event)
{
    //greeter界面隐藏时，需要结束虚拟键盘
    TRACE_ME_IN;	//<<==--TracePoint!
    FullscreenBackground::hideEvent(event);

    m_model->setHasVirtualKB(false);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

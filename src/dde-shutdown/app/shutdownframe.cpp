
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

#include <QDebug>

#include "shutdownframe.h"
#include "src/session-widgets/sessionbasemodel.h"
#include "src/dde-shutdown/dbusshutdownagent.h"

const QString WallpaperKey = "pictureUri";

ShutdownFrame::ShutdownFrame(SessionBaseModel *const model, QWidget *parent)
    : FullscreenBackground(parent)
    , m_model(model)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QTimer::singleShot(0, this, [ = ] {
        auto user = model->currentUser();
        if (user != nullptr) updateBackground(QPixmap(user->desktopBackgroundPath()));
    });

    m_shutdownFrame = new ContentWidget(this);
    m_shutdownFrame->setModel(model);
    setContent(m_shutdownFrame);
    m_shutdownFrame->hide();

    connect(m_shutdownFrame, &ContentWidget::requestBackground,
            this, static_cast<void (ShutdownFrame::*)(const QString &)>(&ShutdownFrame::updateBackground));

    connect(m_shutdownFrame, &ContentWidget::buttonClicked, this, &ShutdownFrame::buttonClicked);

    connect(model, &SessionBaseModel::visibleChanged, this, [ = ](bool visible) {
        if (visible) {
            // refresh hibernate and sleep function.
            emit model->onStatusChanged(SessionBaseModel::PowerMode);
        }
    });

    m_shutdownFrame->initBackground();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

bool ShutdownFrame::powerAction(const Actions action)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return m_shutdownFrame->powerAction(action);
}

void ShutdownFrame::setConfirm(const bool confirm)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_shutdownFrame->setConfirm(confirm);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownFrame::showEvent(QShowEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Q_EMIT requestEnableHotzone(false);

    m_model->setIsShow(true);

    TRACE_ME_OUT;	//<<==--TracePoint!
    return FullscreenBackground::showEvent(event);

}

void ShutdownFrame::hideEvent(QHideEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Q_EMIT requestEnableHotzone(true);

    m_model->setIsShow(false);

    m_shutdownFrame->recoveryLayout();

    TRACE_ME_OUT;	//<<==--TracePoint!
    return FullscreenBackground::hideEvent(event);

}

bool ShutdownFrame::eventFilter(QObject *watched, QEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (event->type() == QEvent::KeyRelease) {
        QString  keyValue = "";
        switch (static_cast<QKeyEvent *>(event)->key()) {
        // 需要捕捉poweroff键进行处理
        case Qt::Key_PowerOff: {
            if (!handlePoweroffKey()) {
                keyValue = "power-off";
            }
            break;
        }
        }
        if (keyValue != "") {
            emit sendKeyValue(keyValue);
        }
    }
    TRACE_ME_OUT;	//<<==--TracePoint!
    return QObject::eventFilter(watched, event);
}

/**
 * @brief ShutdownFrame::handlePoweroffKey
 * 根据Power键的行为对捕捉到Power键后做特殊处理
 * @return
 */
bool ShutdownFrame::handlePoweroffKey()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QDBusInterface powerInter("com.deepin.daemon.Power","/com/deepin/daemon/Power","com.deepin.daemon.Power");
    if (!powerInter.isValid()) {
        TRACE_ME_OUT;	//<<==--TracePoint!
        return false;
    }
    int action = powerInter.property("LinePowerPressPowerBtnAction").toInt();
    // 需要特殊处理：无任何操作(4)
    if (action == 4) {
       TRACE_ME_OUT;	//<<==--TracePoint!
       return true;
    }
    TRACE_ME_OUT;	//<<==--TracePoint!
    return false;
}

ShutdownFrame::~ShutdownFrame()
{
}

ShutdownFrontDBus::ShutdownFrontDBus(DBusShutdownAgent *parent,SessionBaseModel* model)
    :QDBusAbstractAdaptor(parent)
    ,m_parent(parent)
    ,m_model(model)
{
}

ShutdownFrontDBus::~ShutdownFrontDBus()
{
}

void ShutdownFrontDBus::Shutdown()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_parent->Shutdown();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownFrontDBus::Restart()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_parent->Restart();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownFrontDBus::Logout()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_parent->Logout();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownFrontDBus::Suspend()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_parent->Suspend();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownFrontDBus::Hibernate()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_parent->Hibernate();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownFrontDBus::SwitchUser()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_parent->SwitchUser();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownFrontDBus::Show()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if ( m_model != nullptr && !m_model->isLocked() )
        m_parent->Show();
        TRACE_ME_OUT;	//<<==--TracePoint!

}

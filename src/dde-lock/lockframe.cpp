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

#include "lockframe.h"
#include "src/session-widgets/lockcontent.h"
#include "src/session-widgets/sessionbasemodel.h"
#include "src/session-widgets/userinfo.h"
#include "src/session-widgets/hibernatewidget.h"

#include <QApplication>
#include <QWindow>
#include <QDBusInterface>


LockFrame::LockFrame(SessionBaseModel *const model, QWidget *parent)
    : FullscreenBackground(parent)
    , m_model(model)
    , m_login1Inter(new DBusLogin1Manager("org.freedesktop.login1", "/org/freedesktop/login1", QDBusConnection::systemBus(), this))
    , m_preparingSleep(false)
    , m_prePreparingSleep(false)
{
    qDebug() << "LockFrame geometry:" << geometry();

    QTimer::singleShot(0, this, [ = ] {
        auto user = model->currentUser();
        if (user != nullptr) updateBackground(QPixmap(user->greeterBackgroundPath()));
    });

    Hibernate = new HibernateWidget(this);
    Hibernate->hide();
    m_content = new LockContent(model);
    m_content->hide();
    setContent(m_content);

    connect(m_content, &LockContent::requestSwitchToUser, this, &LockFrame::requestSwitchToUser);
    connect(m_content, &LockContent::requestAuthUser, this, &LockFrame::requestAuthUser);
    connect(m_content, &LockContent::requestSetLayout, this, &LockFrame::requestSetLayout);
    connect(m_content, &LockContent::requestBackground, this, static_cast<void (LockFrame::*)(const QString &)>(&LockFrame::updateBackground));
    connect(model, &SessionBaseModel::blackModeChanged, this, &FullscreenBackground::setIsBlackMode);
    connect(model, &SessionBaseModel::HibernateModeChanged, this, [&](bool is_hibernate){
        if(is_hibernate){
            m_content->hide();
            setContent(Hibernate);
            setIsHibernateMode();     //更新大小 现示动画
        }else{
            Hibernate->hide();
            setContent(m_content);
        }
    });
    connect(model, &SessionBaseModel::showUserList, this, &LockFrame::showUserList);
    connect(model, &SessionBaseModel::authFinished, this, [ = ](bool success){
        m_content->beforeUnlockAction(success);

        if (success) {
            Q_EMIT requestEnableHotzone(true);
            hide();
        }
    });

    connect(m_login1Inter, &DBusLogin1Manager::PrepareForSleep, this, [this](bool isSleep){
        //初始化时，m_prePreparingSleep = false,m_preparingSleep = false
        //开始待机时，isSleep为true,那么m_prePreparingSleep = false,m_preparingSleep = true
        //待机唤醒后，isSleep为false,那么m_prePreparingSleep = true,m_preparingSleep = false
        m_prePreparingSleep = m_preparingSleep;
        m_preparingSleep = isSleep;
        //记录待机和唤醒时间
        m_preparingSleepTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

        //待机休眠唤醒后将界面切换到锁屏状态
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
    });
}

bool LockFrame::event(QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        QString  keyValue = "";
        switch (static_cast<QKeyEvent *>(event)->key()) {
        case Qt::Key_PowerOff: {
            if (!handlePoweroffKey()) {
                keyValue = "power-off";
            }
            break;
        }
        case Qt::Key_NumLock: {
            keyValue = "numlock";
            break;
        }
        case Qt::Key_TouchpadOn: {
            keyValue = "touchpad-on";
            break;
        }
        case Qt::Key_TouchpadOff: {
            keyValue = "touchpad-off";
            break;
        }
        case Qt::Key_TouchpadToggle: {
            keyValue = "touchpad-toggle";
            break;
        }
        case Qt::Key_CapsLock: {
            keyValue = "capslock";
            break;
        }
        case Qt::Key_VolumeDown: {
            keyValue = "audio-lower-volume";
            break;
        }
        case Qt::Key_VolumeUp: {
            keyValue = "audio-raise-volume";
            break;
        }
        case Qt::Key_VolumeMute: {
            keyValue = "audio-mute";
            break;
        }
        case Qt::Key_MonBrightnessUp: {
            keyValue = "mon-brightness-up";
            break;
        }
        case Qt::Key_MonBrightnessDown: {
            keyValue = "mon-brightness-down";
            break;
        }
        }

        if (keyValue != "") {
            emit sendKeyValue(keyValue);
        }
    }
    return FullscreenBackground::event(event);
}

bool LockFrame::handlePoweroffKey()
{
    QDBusInterface powerInter("com.deepin.daemon.Power","/com/deepin/daemon/Power","com.deepin.daemon.Power");
    if (!powerInter.isValid()) {
        qDebug() << "powerInter is not valid";
        return false;
    }
    bool isBattery = powerInter.property("OnBattery").toBool();
    int action = 0;
    if (isBattery) {
        action = powerInter.property("BatteryPressPowerBtnAction").toInt();
    } else {
        action = powerInter.property("LinePowerPressPowerBtnAction").toInt();
    }
    qDebug() << "battery is: " << isBattery << "," << action;
    // 需要特殊处理：关机(0)和无任何操作(4)
    if (action == 0) {
        m_model->setPowerAction(SessionBaseModel::PowerAction::RequireShutdown);
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ConfirmPasswordMode);
        m_content->pushConfirmFrame();
        return true;
    } else if (action == 4) {
        //初始化时，m_prePreparingSleep = false,m_preparingSleep = false
        //开始待机时，isSleep为true,那么m_prePreparingSleep = false,m_preparingSleep = true
        //待机唤醒后，isSleep为false,那么m_prePreparingSleep = true,m_preparingSleep = false

        // 先检查当前是不是准备待机
        if (!m_preparingSleep) {
            //有些机器使用电源唤醒时，除了会唤醒机器外还会发送按键消息，会将锁屏界面切换成电源选项界面,增加唤醒时500毫秒时间检测
            //如果系统刚唤醒 ，则500毫秒内不响应电源按钮事件
            if (m_prePreparingSleep && QDateTime::currentDateTime().toMSecsSinceEpoch() - m_preparingSleepTime < 500) {
                return true;
            }
            //无任何操作时，如果是锁定时显示小关机界面
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PowerMode);
        }
        return true;
    }
    return false;
}

void LockFrame::showUserList()
{
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::UserMode);
    show();
}

void LockFrame::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
#ifdef QT_DEBUG
    case Qt::Key_Escape:    qApp->quit();       break;
#endif
    }
}

void LockFrame::showEvent(QShowEvent *event)
{
    emit requestEnableHotzone(false);

    m_model->setIsShow(true);

    return FullscreenBackground::showEvent(event);
}

void LockFrame::hideEvent(QHideEvent *event)
{
    emit requestEnableHotzone(true);

    m_model->setIsShow(false);

    return FullscreenBackground::hideEvent(event);
}

LockFrame::~LockFrame()
{

}

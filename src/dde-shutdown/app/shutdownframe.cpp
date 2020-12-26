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
    m_shutdownFrame = new ContentWidget(this);
    m_shutdownFrame->setModel(model);
    setContent(m_shutdownFrame);

    this->setWindowFlag(Qt::WindowMinMaxButtonsHint, false);

    connect(m_shutdownFrame, &ContentWidget::requestBackground,
            this, static_cast<void (ShutdownFrame::*)(const QString &)>(&ShutdownFrame::updateBackground));

    connect(m_shutdownFrame, &ContentWidget::buttonClicked, this, &ShutdownFrame::buttonClicked);

    connect(m_shutdownFrame, &ContentWidget::quitApplication, this, [ = ] {
        this->globalShortcutsChanged(false);
        qApp->quit();
    });

    connect(model, &SessionBaseModel::visibleChanged, this, [ = ](bool visible) {
        if (visible) {
            // refresh hibernate and sleep function.
            qDebug() << __FILE__ << __LINE__ << __FUNCTION__ << ": visibleChanged to " << visible;
            emit model->onStatusChanged(SessionBaseModel::PowerMode);
        }
    });
    m_sessionInter = new SessionManagerInter("com.deepin.SessionManager", "/com/deepin/SessionManager",
                                QDBusConnection::sessionBus(), this);
    connect(m_sessionInter, &SessionManagerInter::LockedChanged, this, [this](){
        qDebug() << __FILE__ << __LINE__ << __FUNCTION__ << ": LockedChanged, ShutdownFrame hide";
        qApp->quit();
        //com.deepin.dde.lockFront

    });

    m_shutdownFrame->initBackground();
}

bool ShutdownFrame::powerAction(const Actions action)
{
    return m_shutdownFrame->powerAction(action);
}

void ShutdownFrame::setConfirm(const bool confrim)
{
    m_shutdownFrame->setConfirm(confrim);
}

void ShutdownFrame::visibleChangedFrame(bool isVisible)
{
    globalShortcutsChanged(isVisible);

    if (isVisible && m_monitor->enable()) {
        SessionManagerInter sessionInter("com.deepin.SessionManager", "/com/deepin/SessionManager",
                                    QDBusConnection::sessionBus(), nullptr);
        if (sessionInter.locked())
            return;
        qDebug() << __FILE__ << __LINE__ << ": shutdown showFullScreen, locked :" << sessionInter.locked();
        show();
        updateMonitorGeometry();
    } else {
        qDebug() << __FILE__ << __LINE__ << ": shutdown setVisible false";
        setVisible(false);
    }
}

void ShutdownFrame::monitorEnableChanged(bool isEnable)
{
    qDebug() << "ShutdownFrame::monitorEnableChanged:" << m_monitor->name() << isEnable;
    this->setVisible(isEnable && m_model->isShow());
}

void ShutdownFrame::globalShortcutsChanged(bool isEnable)
{
    QDBusInterface *inter = nullptr;
    QDBusInterface *inter1 = nullptr;
    if (qEnvironmentVariable("XDG_SESSION_TYPE").toLower().contains("wayland")) {
        inter = new QDBusInterface("org.kde.KWin", "/kglobalaccel", "org.kde.KGlobalAccel",
                                                  QDBusConnection::sessionBus(), this);
        inter1 = new QDBusInterface("org.kde.KWin", "/KWin", "org.kde.KWin",
                                                  QDBusConnection::sessionBus(), this);
    }
    if (inter) {
        auto req = inter->call("blockGlobalShortcuts", isEnable);
        auto req1 = inter1->call("disableHotKeysForClient", isEnable);
    }
}

void ShutdownFrame::showEvent(QShowEvent *event)
{
    Q_EMIT requestEnableHotzone(false);

    m_model->setIsShow(true);

    if (!m_monitor->enable()) {
        setVisible(false);
        return;
    }

    return FullscreenBackground::showEvent(event);
}

void ShutdownFrame::hideEvent(QHideEvent *event)
{
    Q_EMIT requestEnableHotzone(true);

    m_shutdownFrame->recoveryLayout();

    return FullscreenBackground::hideEvent(event);
}

ShutdownFrame::~ShutdownFrame()
{
}

ShutdownFrontDBus::ShutdownFrontDBus(DBusShutdownAgent *parent,SessionBaseModel* model)
    :QDBusAbstractAdaptor(parent)
    ,m_parent(parent)
    ,m_model(model)
{
    m_sessionInter = new SessionManagerInter("com.deepin.SessionManager", "/com/deepin/SessionManager",
                                QDBusConnection::sessionBus(), this);
    connect(m_sessionInter, &SessionManagerInter::LockedChanged, this, [this](){
        qDebug() << __FILE__ << __LINE__ << __FUNCTION__ << ": LockedChanged, ShutdownFrame hide";
        m_parent->Hide();
        //com.deepin.dde.lockFront

    });
}

ShutdownFrontDBus::~ShutdownFrontDBus()
{
}

void ShutdownFrontDBus::Shutdown()
{
    m_parent->Shutdown();
}

void ShutdownFrontDBus::Restart()
{
    m_parent->Restart();
}

void ShutdownFrontDBus::Logout()
{
    m_parent->Logout();
}

void ShutdownFrontDBus::Suspend()
{
    m_parent->Suspend();
}

void ShutdownFrontDBus::Hibernate()
{
    m_parent->Hibernate();
}

void ShutdownFrontDBus::SwitchUser()
{
    m_parent->SwitchUser();
}

void ShutdownFrontDBus::Show()
{
    if ( m_model != nullptr && !m_model->isLocked() )
        m_parent->Show();
}


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

#include "shutdownwidget.h"

#if 0 // storage i10n
QT_TRANSLATE_NOOP("ShutdownWidget", "Shut down"),
                  QT_TRANSLATE_NOOP("ShutdownWidget", "Reboot"),
                  QT_TRANSLATE_NOOP("ShutdownWidget", "Suspend"),
                  QT_TRANSLATE_NOOP("ShutdownWidget", "Hibernate")
#endif

                  ShutdownWidget::ShutdownWidget(QWidget *parent)
                      : QFrame(parent)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_frameDataBind = FrameDataBind::Instance();
    initUI();
    initConnect();

    std::function<void (QVariant)> function = std::bind(&ShutdownWidget::onOtherPageChanged, this, std::placeholders::_1);
    int index = m_frameDataBind->registerFunction("ShutdownWidget", function);

    connect(this, &ShutdownWidget::destroyed, this, [ = ] {
        m_frameDataBind->unRegisterFunction("ShutdownWidget", index);
    });

    QTimer::singleShot(0, this, [ = ] {
        m_frameDataBind->refreshData("ShutdownWidget");
    });
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownWidget::initConnect()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    connect(m_requireRestartButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireRestartButton;
        shutdownAction();
    });
    connect(m_requireShutdownButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireShutdownButton;
        shutdownAction();
    });
    connect(m_requireSuspendButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireSuspendButton;
        shutdownAction();
    });
    connect(m_requireHibernateButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireHibernateButton;
        shutdownAction();
    });
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownWidget::updateTr(RoundItemButton *widget, const QString &tr)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_trList << std::pair<std::function<void (QString)>, QString>(std::bind(&RoundItemButton::setText, widget, std::placeholders::_1), tr);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownWidget::onOtherPageChanged(const QVariant &value)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_index = value.toInt();

    for (auto it = m_btnList.constBegin(); it != m_btnList.constEnd(); ++it) {
        (*it)->updateState(RoundItemButton::Normal);
    }

    m_currentSelectedBtn = m_btnList.at(m_index);
    m_currentSelectedBtn->updateState(RoundItemButton::Checked);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownWidget::initUI()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    setFocusPolicy(Qt::StrongFocus);
    m_requireShutdownButton = new RoundItemButton(this);
    m_requireShutdownButton->setFocusPolicy(Qt::NoFocus);
    m_requireShutdownButton->setObjectName("RequireShutdownButton");
    m_requireShutdownButton->setAutoExclusive(true);
    updateTr(m_requireShutdownButton, "Shut down");
    m_actionMap[m_requireShutdownButton] = SessionBaseModel::RequireShutdown;

    m_requireRestartButton = new RoundItemButton(tr("Reboot"), this);
    m_requireRestartButton->setFocusPolicy(Qt::NoFocus);
    m_requireRestartButton->setObjectName("RequireRestartButton");
    m_requireRestartButton->setAutoExclusive(true);
    updateTr(m_requireRestartButton, "Reboot");
    m_actionMap[m_requireRestartButton] = SessionBaseModel::RequireRestart;

    m_requireSuspendButton = new RoundItemButton(tr("Suspend"), this);
    m_requireSuspendButton->setFocusPolicy(Qt::NoFocus);
    m_requireSuspendButton->setObjectName("RequireSuspendButton");
    m_requireSuspendButton->setAutoExclusive(true);
    updateTr(m_requireSuspendButton, "Suspend");
    m_actionMap[m_requireSuspendButton] = SessionBaseModel::RequireSuspend;

    m_requireHibernateButton = new RoundItemButton(tr("Hibernate"), this);
    m_requireHibernateButton->setFocusPolicy(Qt::NoFocus);
    m_requireHibernateButton->setObjectName("RequireHibernateButton");
    m_requireHibernateButton->setAutoExclusive(true);
    updateTr(m_requireHibernateButton, "Hibernate");
    m_actionMap[m_requireHibernateButton] = SessionBaseModel::RequireHibernate;

    m_currentSelectedBtn = m_requireShutdownButton;
    m_currentSelectedBtn->updateState(RoundItemButton::Hover);

    m_btnList.append(m_requireShutdownButton);
    m_btnList.append(m_requireRestartButton);
    m_btnList.append(m_requireSuspendButton);
    m_btnList.append(m_requireHibernateButton);

    m_Layout = new QHBoxLayout;
    m_Layout->setMargin(0);
    m_Layout->setSpacing(10);
    m_Layout->addStretch(0);
    m_Layout->addWidget(m_requireShutdownButton);
    m_Layout->addWidget(m_requireRestartButton);
    m_Layout->addWidget(m_requireSuspendButton);
    m_Layout->addWidget(m_requireHibernateButton);
    m_Layout->addStretch(0);
    setLayout(m_Layout);

    updateStyle(":/skin/requireshutdown.qss", this);

    // refresh language
    for (auto it = m_trList.constBegin(); it != m_trList.constEnd(); ++it) {
        it->first(qApp->translate("ShutdownWidget", it->second.toUtf8()));
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownWidget::leftKeySwitch()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_btnList.at(m_index)->updateState(RoundItemButton::Normal);
    if (m_index == 0) {
        m_index = m_btnList.length() - 1;
    } else {
        m_index  -= 1;
    }

    for (int i = m_btnList.size(); i != 0; --i) {
        int index = (m_index + i) % m_btnList.size();

        if (m_btnList[index]->isVisible()) {
            m_index = index;
            break;
        }
    }

    m_currentSelectedBtn = m_btnList.at(m_index);
    m_currentSelectedBtn->updateState(RoundItemButton::Checked);

    m_frameDataBind->updateValue("ShutdownWidget", m_index);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownWidget::rightKeySwitch()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_btnList.at(m_index)->updateState(RoundItemButton::Normal);

    if (m_index == m_btnList.size() - 1) {
        m_index = 0;
    } else {
        ++m_index;
    }

    for (int i = 0; i < m_btnList.size(); ++i) {
        int index = (m_index + i) % m_btnList.size();

        if (m_btnList[index]->isVisible()) {
            m_index = index;
            break;
        }
    }

    m_currentSelectedBtn = m_btnList.at(m_index);
    m_currentSelectedBtn->updateState(RoundItemButton::Checked);

    m_frameDataBind->updateValue("ShutdownWidget", m_index);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownWidget::shutdownAction()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    qDebug() << "emit m_currentSelectedBtn clicked";

    m_model->setPowerAction(m_actionMap[m_currentSelectedBtn]);

    emit abortOperation();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ShutdownWidget::keyPressEvent(QKeyEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    switch (event->key()) {
    case Qt::Key_Escape:
        hide();
        emit abortOperation();
        break;
    case Qt::Key_Left:
        leftKeySwitch();
        break;
    case Qt::Key_Right:
        rightKeySwitch();
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        shutdownAction();
        break;
    default:
        break;
    }

    QFrame::keyReleaseEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

bool ShutdownWidget::event(QEvent *e)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (e->type() == QEvent::LanguageChange) {
        // refresh language
        for (auto it = m_trList.constBegin(); it != m_trList.constEnd(); ++it) {
            it->first(qApp->translate("ShutdownWidget", it->second.toUtf8()));
        }
    }

    if (e->type() == QEvent::FocusOut) {
        m_btnList.at(m_index)->updateState(RoundItemButton::Normal);
    }

    if (e->type() == QEvent::FocusIn) {
        m_index = 0;
        m_btnList.at(m_index)->updateState(RoundItemButton::Checked);
    }

    TRACE_ME_OUT;	//<<==--TracePoint!
    return QFrame::event(e);
}

ShutdownWidget::~ShutdownWidget()
{
}

void ShutdownWidget::setModel(SessionBaseModel *const model)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_model = model;

    connect(model, &SessionBaseModel::onHasSwapChanged, m_requireHibernateButton, &RoundItemButton::setVisible);
    connect(model, &SessionBaseModel::canSleepChanged, m_requireSuspendButton, &RoundItemButton::setVisible);

    m_requireHibernateButton->setVisible(model->hasSwap());
    m_requireSuspendButton->setVisible(model->canSleep());
    TRACE_ME_OUT;	//<<==--TracePoint!

}

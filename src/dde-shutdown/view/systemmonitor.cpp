
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

#include "systemmonitor.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QPainter>
#include <QtSvg/QSvgRenderer>
#include <QPaintEvent>
#include <QApplication>
#include <QScreen>
#include <QDebug>

SystemMonitor::SystemMonitor(QWidget *parent) : QWidget(parent)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_state = Leave;

    m_icon = new QWidget;
    m_icon->installEventFilter(this);
    m_icon->setFixedSize(24, 24);

    m_text = new QLabel(tr("Start system monitor"));
    m_text->setStyleSheet("color: white;"
                          "font-weight: 400;");

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setContentsMargins(30, 0, 30, 0);

    layout->addWidget(m_icon, 0, Qt::AlignVCenter);
    layout->addSpacing(10);
    layout->addWidget(m_text, 0, Qt::AlignVCenter);

    setLayout(layout);
    setMinimumHeight(40);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void SystemMonitor::setState(SystemMonitor::State state)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_state = state;
    update();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void SystemMonitor::enterEvent(QEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QWidget::enterEvent(event);

    m_text->setStyleSheet("color: white;"
                          "font-weight: 400;");

    m_state = Enter;
    update();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void SystemMonitor::leaveEvent(QEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QWidget::leaveEvent(event);

    m_text->setStyleSheet("color: white;"
                          "font-weight: 400;");

    m_state = Leave;
    update();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void SystemMonitor::mouseReleaseEvent(QMouseEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QWidget::mouseReleaseEvent(event);

    m_text->setStyleSheet("color: white;"
                          "font-weight: 400;");

    m_state = Release;
    update();

    emit clicked();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void SystemMonitor::mousePressEvent(QMouseEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QWidget::mousePressEvent(event);

    m_text->setStyleSheet("color: #2ca7f8;"
                          "font-weight: 400;");

    m_state = Press;
    update();

    event->accept();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

bool SystemMonitor::eventFilter(QObject *watched, QEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (watched == m_icon) {
        if (event->type() == QEvent::Paint) {
            QPainter painter(m_icon);
            QSvgRenderer render(QString(":/img/deepin-system-monitor.svg"), m_icon);
            render.render(&painter, m_icon->rect());
        }
    }

    TRACE_ME_OUT;	//<<==--TracePoint!
    return false;
}

void SystemMonitor::paintEvent(QPaintEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setBrush(QColor(0, 0 , 0, 76));
    painter.setRenderHint(QPainter::Antialiasing, true);

    switch(m_state) {
    case Enter:
    case Release:{
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 105));
        painter.drawRoundedRect(QRect(1, 1, width() - 2, height() - 2), 10, 10);
        break;
    }
    case Press: {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 105));
        painter.drawRoundedRect(QRect(1, 1, width() - 2, height() - 2), 10, 10);
        break;
    }
    case Leave:
    default:
        break;
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

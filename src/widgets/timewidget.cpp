
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

#include "timewidget.h"

#include <QVBoxLayout>
#include <QDateTime>
#include <QFontDatabase>
#include <QSettings>

const QStringList weekdayFormat = {"dddd", "ddd"};
const QStringList shortDateFormat = { "yyyy/M/d", "yyyy-M-d", "yyyy.M.d",
                                      "yyyy/MM/dd", "yyyy-MM-dd", "yyyy.MM.dd",
                                      "yy/M/d", "yy-M-d", "yy.M.d" };
const QStringList shortTimeFormat = { "h:mm", "hh:mm"};

TimeWidget::TimeWidget(QWidget *parent)
    : QWidget(parent)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QFont timeFont;
    timeFont.setFamily("Noto Sans CJK SC-Thin");

    m_timeLabel = new QLabel;
    timeFont.setWeight(QFont::ExtraLight);
    m_timeLabel->setFont(timeFont);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    QPalette palette = m_timeLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_timeLabel->setPalette(palette);
    DFontSizeManager::instance()->bind(m_timeLabel, DFontSizeManager::T1);

    m_dateLabel = new QLabel;
    m_dateLabel->setAlignment(Qt::AlignCenter);
    palette = m_dateLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_dateLabel->setPalette(palette);
    DFontSizeManager::instance()->bind(m_dateLabel, DFontSizeManager::T6);

    refreshTime();

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(1000);
    m_refreshTimer->start();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(m_timeLabel);
    vLayout->addWidget(m_dateLabel);
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(vLayout);

    connect(m_refreshTimer, &QTimer::timeout, this, &TimeWidget::refreshTime);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void TimeWidget::set24HourFormat(bool use24HourFormat)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_use24HourFormat = use24HourFormat;
    refreshTime();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void TimeWidget::updateLocale(const QLocale &locale)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_locale = locale;
    refreshTime();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void TimeWidget::refreshTime()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (m_use24HourFormat) {
        m_timeLabel->setText(m_locale.toString(QDateTime::currentDateTime(), shortTimeFormat.at(m_shortTimeIndex)));
    } else {
        m_timeLabel->setText(m_locale.toString(QDateTime::currentDateTime(), shortTimeFormat.at(m_shortTimeIndex) + " AP"));
    }

    QString date_format = shortDateFormat.at(m_shortDateIndex) + " " + weekdayFormat.at(m_weekdayIndex);
    m_dateLabel->setText(m_locale.toString(QDateTime::currentDateTime(), date_format));
    TRACE_ME_OUT;	//<<==--TracePoint!

}

/**
 * @brief TimeWidget::setWeekdayFormatType 根据类型来设置周显示格式
 * @param type 自定义类型
 */
void TimeWidget::setWeekdayFormatType(int type)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if(type >= weekdayFormat.size() || type < 0)
{
    TRACE_ME_OUT;	//<<==--TracePoint!
    return;
}

    m_weekdayIndex = type;
    refreshTime();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

/**
 * @brief TimeWidget::setShortDateFormat 根据类型来设置短日期显示格式
 * @param type 自定义格式
 */
void TimeWidget::setShortDateFormat(int type)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if(type >= shortDateFormat.size() || type < 0)
{
    TRACE_ME_OUT;	//<<==--TracePoint!
    return;
}

    m_shortDateIndex = type;
    refreshTime();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

/**
 * @brief TimeWidget::setShortTimeFormat 根据类型来设置短时间显示格式
 * @param type
 */
void TimeWidget::setShortTimeFormat(int type)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if(type >= shortTimeFormat.size() || type < 0)
{
    TRACE_ME_OUT;	//<<==--TracePoint!
    return;
}

    m_shortTimeIndex = type;
    refreshTime();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     zhaoyingzhen <zhaoyingzhen@uniontech.com>
 *
 * Maintainer: zhaoyingzhen <zhaoyingzhen@uniontech.com>
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
#include "roundpopupwidget.h"

#include <QPainter>
#include <QDebug>
#include <QVBoxLayout>

const int MARGIN = 5;

RoundPopupWidget::RoundPopupWidget(QWidget *parent)
    : QWidget(parent)
    , m_pContentWidget(nullptr)
    , m_mainLayout(new QVBoxLayout())
{
    initUI();
}

void RoundPopupWidget::initUI()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    setAttribute(Qt::WA_TranslucentBackground);

    m_mainLayout->setMargin(0);
    m_mainLayout->setContentsMargins(MARGIN, MARGIN, MARGIN, MARGIN);
    setLayout(m_mainLayout);
}

void RoundPopupWidget::setContent(QWidget *widget)
{
    if (!widget)
        return;

    setFixedSize(widget->size() + QSize(MARGIN * 2, MARGIN * 2));

    if (m_pContentWidget != widget) {
        if (m_pContentWidget) {
          m_mainLayout->removeWidget(m_pContentWidget);
          m_pContentWidget->setVisible(false);
        }

        m_mainLayout->addWidget(widget);
    }

    m_pContentWidget = widget;
    widget->setVisible(true);
}

QWidget *RoundPopupWidget::getContent() const
{
    return m_pContentWidget;
}

void RoundPopupWidget::hideEvent(QHideEvent *event)
{
    if (m_pContentWidget)
        m_pContentWidget->hide();

    QWidget::hideEvent(event);
}

void RoundPopupWidget::paintEvent(QPaintEvent *event)
{
    const int radius = 18;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);
    painter.setBrush(QColor(238, 238, 238, int(0.8 * 255)));

    QRect rect = this->rect();
    rect.setWidth(rect.width() - 1);
    rect.setHeight(rect.height() - 1);
    painter.drawRoundedRect(rect, radius, radius);
}
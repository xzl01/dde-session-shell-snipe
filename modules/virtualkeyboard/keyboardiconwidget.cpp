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

#include "keyboardiconwidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QSvgRenderer>

const static QSize iconSize = QSize(30, 30);

KeyboardIconWidget::KeyboardIconWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(iconSize);
    setAttribute(Qt::WA_TranslucentBackground, true);
}

void KeyboardIconWidget::setIconPath(const QString &iconPath)
{
    m_iconPath = iconPath;
    update();
}

void KeyboardIconWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPixmap pixmap(iconSize);
    pixmap.load(m_iconPath);
    pixmap.fill(Qt::transparent);

    QSvgRenderer renderer(m_iconPath);
    renderer.render(&painter);

    int x = (rect().width() - iconSize.width()) / 2;
    int y = (rect().height() - iconSize.height()) / 2;
    painter.drawPixmap(x, y, iconSize.width(), iconSize.height(), pixmap);

    QWidget::paintEvent(event);
}

void KeyboardIconWidget::hideEvent(QHideEvent *event)
{
    Q_EMIT iconWidgetHided();

    if (this->topLevelWidget() && !topLevelWidget()->isVisible()) {
        // 通过判断顶层窗口隐藏来触发
        Q_EMIT topLevelWidgetHided();
    }

    QWidget::hideEvent(event);
}

void KeyboardIconWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QWidget *topLevelWidget = this->topLevelWidget();
        if (topLevelWidget) {
            // 获取顶层窗口，虚拟键盘窗口中需要知道顶层窗口以便移动位置
            Q_EMIT clicked(topLevelWidget);
        }
    }

    QWidget::mousePressEvent(event);
}

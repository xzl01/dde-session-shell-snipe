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
#ifndef ROUND_POPUP_WIDGET_H
#define ROUND_POPUP_WIDGET_H

#include <QWidget>

class QVBoxLayout;

/*!
 * \brief The RoundPopupWidget class
 * 圆角弹窗页面
 */
class RoundPopupWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RoundPopupWidget(QWidget *parent = nullptr);

public:
    void setContent(QWidget *widget);
    QWidget *getContent() const;

protected:
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    void initUI();

private:
    QWidget *m_pContentWidget;
    QVBoxLayout *m_mainLayout;
};


#endif // ROUND_POPUP_WIDGET_H

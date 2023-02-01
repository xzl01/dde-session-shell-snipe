/*
* Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co.,Ltd.
*
* Author:     fanpengcheng <fanpengcheng@uniontech.com>
*
* Maintainer: fanpengcheng <fanpengcheng@uniontech.com>
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

#ifndef PASSWORDLEVELWIDGET_H
#define PASSWORDLEVELWIDGET_H

#include <QWidget>

#include <dtkwidget_global.h>
#include <dwidgetstype.h>

#include <DGuiApplicationHelper>

#include "pwqualitymanager.h"

class QHBoxLayout;
DWIDGET_BEGIN_NAMESPACE
class DLabel;
DWIDGET_END_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class PasswordLevelWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PasswordLevelWidget(QWidget *parent = nullptr);

public Q_SLOTS:
    void reset();
    void setLevel(PASSWORD_LEVEL_TYPE level);

private:
    void initUI();
    void initConnections();

    QString defaultIcon();

private Q_SLOTS:
    void onThemeTypeChanged(DGuiApplicationHelper::ColorType type);

private:
    PASSWORD_LEVEL_TYPE m_level;
    QHBoxLayout *m_layout;
    DLabel *m_tips;
    DLabel *m_lowIcon;
    DLabel *m_mediumIcon;
    DLabel *m_highIcon;
};
#endif // PASSWORDLEVELWIDGET_H

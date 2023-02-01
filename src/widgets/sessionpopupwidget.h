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
#ifndef SESSIONPOPUPWIDGET_H
#define SESSIONPOPUPWIDGET_H

#include <DListView>

DWIDGET_USE_NAMESPACE

class QStandardItemModel;
class SessionPopupWidget : public DListView
{
    Q_OBJECT
public:
    explicit SessionPopupWidget(QWidget *parent = nullptr);

public:
    void setSessionInfo(const QMap<QString, QString> &infos, const QString &curSession);
    void updateCurrentSession(const QString &curSession);

signals:
    void sessionItemClicked(const QString &session);

private:
    void initUI();
    void addItem(const QString &icon, const QString &itemName);
    void itemClicked(const QModelIndex &index);
    void updateSelectedState();

private:
    QStandardItemModel *m_model;
    QString m_curSession;
};

#endif // SESSIONPOPUPWIDGET_H

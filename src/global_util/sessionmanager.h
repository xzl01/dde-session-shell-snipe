/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *             zhaoyingzhen <zhaoyingzhen@uniontech.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *             zhaoyingzhen <zhaoyingzhen@uniontech.com>
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

#ifndef SESSIONWIDGET_H
#define SESSIONWIDGET_H

#include <QLightDM/SessionsModel>
#include <QLightDM/UsersModel>

class SessionBaseModel;
class SessionManager : public QObject
{
    Q_OBJECT

public:
    static SessionManager &Reference();

public:
    void setModel(SessionBaseModel * const model);
    int sessionCount() const;
    QString currentSession() const;
    QMap<QString, QString> sessionInfo() const;
    void updateSession(const QString &userName);
    void switchSession(const QString &session);

private:
    QString getSessionKey(const QString &sessionName) const;
    QString lastLoggedInSession(const QString &userName) const;
    bool isWaylandExisted() const;
    QString defaultConfigSession() const;

private:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager() override;

private:
    SessionBaseModel *m_model;
    QLightDM::SessionsModel *m_sessionModel;
    QLightDM::UsersModel *m_userModel;

    bool m_allowSwitchingToWayland;
};

#endif // SESSIONWIDGET_H

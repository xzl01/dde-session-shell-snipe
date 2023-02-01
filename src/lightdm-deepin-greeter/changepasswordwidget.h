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

#ifndef CHANGEPASSWORDWIDGET_H
#define CHANGEPASSWORDWIDGET_H

#include <QWidget>

#include <dtkwidget_global.h>
#include <dwidgetstype.h>

#include <memory>

#include "userinfo.h"

class DLineEditEx;
class QVBoxLayout;
class UserAvatar;
class PasswordLevelWidget;
DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DLabel;
DWIDGET_END_NAMESPACE
DWIDGET_USE_NAMESPACE

class ChangePasswordWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChangePasswordWidget(std::shared_ptr<User> user, QWidget *parent = nullptr);

private Q_SLOTS:
    void onNewPasswordTextChanged(const QString &text);
    void onRepeatPasswordTextEdited(const QString &text);
    void onRepeatPasswordEditFinished();
    void onPasswordHintsChanged(const QString &text);
    void onOkClicked();

Q_SIGNALS:
    void changePasswordSuccessed();

private:
    void initUI();
    void initConnections();
    bool isInfoValid();
    void parseProcessResult(int exitCode, const QString &output);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::shared_ptr<User> m_user;

    QVBoxLayout *m_mainLayout;
    DLabel *m_lockBtn;
    UserAvatar *m_avatar;
    DLabel *m_nameLabel;
    DLabel *m_tipsLabel;
    DPasswordEdit *m_oldPasswdEdit;
    DPasswordEdit *m_newPasswdEdit;
    PasswordLevelWidget *m_levelWidget;
    DPasswordEdit *m_repeatPasswdEdit;
    DLineEditEx *m_passwordHints;
    DPushButton *m_okBtn;
};

#endif // CHANGEPASSWORDWIDGET_H

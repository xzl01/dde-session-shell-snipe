// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEEPINAUTHFRAMEWORK_H
#define DEEPINAUTHFRAMEWORK_H

#include <QObject>
#include <QPointer>

#include <memory>

#include "authenticate_interface.h"
#include "authenticatesession2_interface.h"

#define AUTHRNTICATESERVICE "org.deepin.dde.Authenticate1"
#define AUTHRNTICATEINTERFACE "org.deepin.dde.Authenticate1.Session"

using AuthInter = org::deepin::dde::Authenticate1;
using AuthControllerInter = org::deepin::dde::authenticate1::Session;


class DeepinAuthFramework : public QObject
{
    Q_OBJECT

public:
    enum AuthQuitFlag {
        AutoQuit,  // 自动退出（默认）
        ManualQuit // 手动退出
    };
    Q_ENUM(AuthQuitFlag)

    explicit DeepinAuthFramework(QObject *parent = nullptr);
    ~DeepinAuthFramework() override;

    /* Compatible with old authentication methods */
    void CreateAuthenticate(const QString &account);
    void SendToken(const QString &token);
    void DestroyAuthenticate();

    /* org.deepin.dde.Authenticate1 */
    int GetFrameworkState() const;
    int GetSupportedMixAuthFlags() const;
    QString GetPreOneKeyLogin(const int flag) const;
    QString GetLimitedInfo(const QString &account) const;
    QString GetSupportedEncrypts() const;
    /* org.deepin.dde.Authenticate1.Session */
    int GetFuzzyMFA(const QString &account) const;
    int GetMFAFlag(const QString &account) const;
    int GetPINLen(const QString &account) const;
    MFAInfoList GetFactorsInfo(const QString &account) const;
    QString GetPrompt(const QString &account) const;
    bool SetPrivilegesEnable(const QString &account, const QString &path);
    void SetPrivilegesDisable(const QString &account);

    QString AuthSessionPath(const QString &account) const;
    void setEncryption(const int type, const ArrayInt method);
    bool authSessionExist(const QString &account) const;
    bool isDeepinAuthValid() const;

signals:
    /* org.deepin.dde.Authenticate1 */
    void LimitsInfoChanged(const QString &);
    void SupportedMixAuthFlagsChanged(const int);
    void FramworkStateChanged(const int);
    void SupportedEncryptsChanged(const QString &);
    /* org.deepin.dde.Authenticate1.Session */
    void MFAFlagChanged(const bool);
    void FuzzyMFAChanged(const bool);
    void PromptChanged(const QString &);
    void FactorsInfoChanged(const MFAInfoList &);
    void PINLenChanged(const int);
    void AuthStateChanged(const int, const int, const QString &);
    void SessionCreated();

public slots:
    /* New authentication framework */
    void CreateAuthController(const QString &account, const int authType, const int appType);
    void DestroyAuthController(const QString &account);
    void StartAuthentication(const QString &account, const int authType, const int timeout);
    void EndAuthentication(const QString &account, const int authType);
    void SendTokenToAuth(const QString &account, const int authType, const QString &token);
    void SetAuthQuitFlag(const QString &account, const int flag = AutoQuit);

private:
    /* Compatible with old authentication methods */
    static void *PAMAuthWorker(void *arg);
    void PAMAuthentication(const QString &account);
    static int PAMConversation(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *app_data);
    void UpdateAuthState(const int state, const QString &message);

private:
    AuthInter *m_authenticateInter;
    pthread_t m_PAMAuthThread;
    QString m_account;
    QString m_message;
    QString m_token;
    int m_encryptType;
    QString m_publicKey;
    QString m_symmetricKey;
    ArrayInt m_encryptMethod;
    QMap<QString, AuthControllerInter *> *m_authenticateControllers;
    bool m_cancelAuth;
    bool m_waitToken;
};

#endif // DEEPINAUTHFRAMEWORK_H

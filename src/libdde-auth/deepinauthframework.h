#ifndef DEEPINAUTHFRAMEWORK_H
#define DEEPINAUTHFRAMEWORK_H

#include "authagent.h"

#include <QObject>
#include <QPointer>
#include <memory>
#include <mutex>

class DeepinAuthInterface;
class QThread;
class User;

class DeepinAuthFramework : public QObject
{
    Q_OBJECT
public:
    explicit DeepinAuthFramework(DeepinAuthInterface *inter, QObject *parent = nullptr);
    ~DeepinAuthFramework();

    friend class AuthAgent;
    int GetAuthType();
    void CancelCurrentAuth();

public slots:
    void Authenticate(std::shared_ptr<User> user);
    bool Responsed(const QString &password);

private:
    typedef struct tagAuthArg {
        DeepinAuthFramework *pFrameWork = nullptr;
        QString strAuthUserName;
    };
    static void* pamAuthWorker(void *arg);
    const QString RequestEchoOff(const QString &msg);
    const QString RequestEchoOn(const QString &msg);
    void DisplayErrorMsg(const QString &msg);
    void DisplayTextInfo(const QString &msg);
    void RespondResult(const QString &msg);

    void setEditReadOnly(bool isReadOnly);

private:
    DeepinAuthInterface *m_interface;
    QPointer<AuthAgent> m_authagent;
    std::shared_ptr<User> m_currentUser = nullptr;
    QString m_password;
};

#endif // DEEPINAUTHFRAMEWORK_H

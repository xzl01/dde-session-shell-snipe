#ifndef AUTHAGENT_H
#define AUTHAGENT_H

#include <QObject>
#include <mutex>
#include <atomic>
#include <map>

#define MAX_VERIFY_FAILED 5

class DeepinAuthFramework;
typedef struct pam_handle pam_handle_t;


class AuthAgent : public QObject {
    Q_OBJECT
public:
    enum AuthFlag {
        UnknowAuthType = 0,
        Password = 1 << 0,
        Fingerprint = 1 << 1,
        Face = 1 << 2,
        ActiveDirectory = 1 << 3
    };

    enum FingerprintStatus {
        MATCH = 0,
        NO_MATCH,
        ERROR,
        RETRY,
        DISCONNECTED
    };

    enum FpRetryStatus {
        SWIPE_TOO_SHORT = 1,
        FINGER_NOT_CENTERED,
        REMOVE_AND_RETRY
    };

    enum AuthStatus {
        Auth_NotStart = 0,
        Auth_WaitPasswd,
        Auth_Canceled,
        Auth_Failed,
        Auth_Success,
        Auth_WaitDelete, //只有在开启了新的一轮认证且状态为这个时，才可以删除，此时上层已经不需要缓存的信息了
    };

    typedef struct tagPAM_AUTH_DATA {
        //std::atomic<int> m_refCount;
        int m_nRefCount2;
        pthread_t m_workThread = 0;
        AuthAgent *m_pAuthAgent = nullptr;
        pam_handle_t* m_pamHandle = nullptr;
        int  m_nAuthNumber;
        int  m_pamFuncRetCode = 0;
        volatile bool m_hasFeedPassword = false;
        AuthStatus m_authStatus = Auth_NotStart;
        QString m_userName;
        QString m_password;
        AuthFlag m_authType = UnknowAuthType;
        std::mutex m_refMutex;
        int addRef() {
            m_refMutex.lock();
            m_nRefCount2++;
            m_refMutex.unlock();
            return m_nRefCount2;
        }
        int subRef() {
            m_refMutex.lock();
            m_nRefCount2--;
            m_refMutex.unlock();
            return m_nRefCount2;
        }
        void storeRef(int ref) {
            m_refMutex.lock();
            m_nRefCount2 = ref;
            m_refMutex.unlock();
        }
        int loadRef() {
            m_refMutex.lock();
            int ref = m_nRefCount2;
            m_refMutex.unlock();
            return ref;
        }

        bool isAuthFinished() {
            return m_authStatus > Auth_WaitPasswd;
        }
    }PAM_AUTH_DATA, *LPPAM_AUTH_DATA;

public:
    explicit AuthAgent(DeepinAuthFramework *deepin);
    ~AuthAgent();

    void Authenticate(const QString& username);
    void Responsed(const QString& password);
    int GetAuthType();
    DeepinAuthFramework *deepinAuth() { return m_deepinauth; }
    void NotifyCancelAuth();
    bool IsWaitingPassword();

signals:
    void displayErrorMsg(const QString &msg);
    void displayTextInfo(const QString &msg);
    void respondResult(const QString &msg);

    void setEditReadOnly(bool isReadOnly);
private:
    void CalcelCurAuthNolock();
    void ClearAuthDataNolock();

private:
    static int pamConversation(int num,
                               const struct pam_message** msg,
                               struct pam_response** resp,
                               void* app_data);

private:
    DeepinAuthFramework* m_deepinauth = nullptr;
    std::atomic<int> m_nextAuthNumber;
    std::map<int, LPPAM_AUTH_DATA> m_mAuthData;
    LPPAM_AUTH_DATA m_curAuthData = nullptr;
    std::mutex m_mutex;
};

#endif // AUTHAGENT_H

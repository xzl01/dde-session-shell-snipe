#include "authagent.h"
#include "deepinauthframework.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <unistd.h>
#include <security/pam_appl.h>

#include <syslog.h>

#ifdef PAM_SUN_CODEBASE
#define PAM_MSG_MEMBER(msg, n, member) ((*(msg))[(n)].member)
#else
#define PAM_MSG_MEMBER(msg, n, member) ((msg)[(n)]->member)
#endif

#define PAM_SERVICE_NAME "common-auth"

static AuthAgent* g_pAgentPtr = nullptr;

AuthAgent::AuthAgent(DeepinAuthFramework *deepin)
    : m_deepinauth(deepin)
{
    g_pAgentPtr = this;
    m_nextAuthNumber.store(1);
    connect(this, &AuthAgent::displayErrorMsg, deepin, &DeepinAuthFramework::DisplayErrorMsg, Qt::QueuedConnection);
    connect(this, &AuthAgent::displayTextInfo, deepin, &DeepinAuthFramework::DisplayTextInfo, Qt::QueuedConnection);
    connect(this, &AuthAgent::respondResult, deepin, &DeepinAuthFramework::RespondResult, Qt::QueuedConnection);
}

AuthAgent::~AuthAgent()
{

}

void AuthAgent::Responsed(const QString &password)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    if (m_curAuthData == nullptr) {
        return;
    }
    LPPAM_AUTH_DATA pAuthData = m_curAuthData;
    if (pAuthData->isAuthFinished()) {
        //todo log
        return;
    }
    qDebug() << "AuthAgent Responsed password, authNumber=" << pAuthData->m_nAuthNumber;
    pAuthData->m_password = password;
    pAuthData->m_hasFeedPassword = true;
}

//每次认证都会开启新线程执行
void AuthAgent::Authenticate(const QString& username)
{
    //新的认证数据
    LPPAM_AUTH_DATA pAuthData = new PAM_AUTH_DATA;
    pAuthData->m_nAuthNumber = m_nextAuthNumber.fetch_add(1);
    pAuthData-> m_workThread = pthread_self();
    pAuthData->m_pAuthAgent = this;
    pAuthData->m_userName = username;
    pAuthData->storeRef(1);
    qDebug() << "AuthAgent::Authenticate, startNewAuth, thread=" << pthread_self();
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        CalcelCurAuthNolock();
        m_curAuthData = pAuthData;
        m_mAuthData[pAuthData->m_nAuthNumber] = pAuthData;
        ClearAuthDataNolock();

        //开始认证流程
        pam_conv conv = { pamConversation, (void*)(pAuthData->m_nAuthNumber) };
        pAuthData->m_pamFuncRetCode = pam_start(PAM_SERVICE_NAME, username.toLocal8Bit().data(), &conv, &pAuthData->m_pamHandle);

        if (pAuthData->m_pamFuncRetCode != PAM_SUCCESS) {
            qDebug() << "AuthAgent::Authenticate startNewAuth failed:" << pAuthData->m_nAuthNumber << pam_strerror(pAuthData->m_pamHandle, pAuthData->m_pamFuncRetCode);
            pAuthData->m_authStatus = Auth_Failed;
        } else {
            qDebug() << "AuthAgent::Authenticate, startNewAuth, authNumber=" << pAuthData->m_nAuthNumber << ", thread=" << pthread_self();
            pAuthData->m_authStatus = Auth_WaitPasswd;
        }
    }
    //启动失败就直接结束
    if (pAuthData->m_authStatus != Auth_WaitPasswd) {
        syslog(LOG_INFO, "zl: %s %s %d ", __FILE__, __func__, __LINE__);
        pam_end(pAuthData->m_pamHandle, pAuthData->m_pamFuncRetCode);
        emit respondResult("failed to start authentication");
        pAuthData->m_authStatus = Auth_WaitDelete;
        pAuthData->subRef();
        return;
    }

    //这里会阻塞到成功或失败
    int rc = pam_authenticate(pAuthData->m_pamHandle, 0);
    qDebug() << "AuthAgent pam_authenticate return=" << rc <<  "authNumber=" << pAuthData->m_nAuthNumber << ", thread=" << pthread_self();;

    //息屏状态下亮屏，由于后端没有亮屏信号，只能用此临时办法
    system("xset dpms force on");
    bool emitResult = false;
    QString msg;
    pAuthData->m_pamFuncRetCode = rc;
    if (pAuthData->m_pamFuncRetCode == PAM_SUCCESS) {
        msg = "succes";
        pAuthData->m_authStatus = Auth_Success;
        qDebug() << "AuthAgent::Authenticate success, authNumber= " << pAuthData->m_nAuthNumber;
    } else {
        pAuthData->m_authStatus = Auth_Failed;
        qDebug() << "AuthAgent::Authenticate failed, authNumber=" << pAuthData->m_nAuthNumber << pam_strerror(pAuthData->m_pamHandle, pAuthData->m_pamFuncRetCode);
    }

    pAuthData->m_pamFuncRetCode = pam_end(pAuthData->m_pamHandle, pAuthData->m_pamFuncRetCode);
    pAuthData->m_pamHandle = nullptr;
    if (pAuthData->m_pamFuncRetCode != PAM_SUCCESS) {
        qDebug() << "AuthAgent authNumber=" << pAuthData->m_nAuthNumber << "pam_end() failed " << pam_strerror(pAuthData->m_pamHandle, pAuthData->m_pamFuncRetCode);
    }
    emitResult = pAuthData->m_nAuthNumber == m_curAuthData->m_nAuthNumber;

    //这里多线程下还是会有卡点错误emit的问题，但需要改的上层太多，先这样
    if (emitResult) {
        syslog(LOG_INFO, "zl: %s %d ", __func__, __LINE__);
        qDebug() << "AuthAgent::Authenticate emitResult, authNumber=" << pAuthData->m_nAuthNumber;
        emit respondResult(msg);
    }
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        pAuthData->m_authStatus = Auth_WaitDelete;
        pAuthData->m_workThread = 0;
    }

    pAuthData->subRef();
}

int AuthAgent::GetAuthType()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    if (m_curAuthData == nullptr) {
        return 0;
    }
    return m_curAuthData->m_authType;
}

void AuthAgent::NotifyCancelAuth()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    CalcelCurAuthNolock();
}

bool AuthAgent::IsWaitingPassword()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    if (m_curAuthData == nullptr) {
        return false;
    }
    return m_curAuthData->m_authStatus == Auth_WaitPasswd;
}

//在pam线程中执行
int AuthAgent::pamConversation(int num_msg, const struct pam_message **msg,
                               struct pam_response **resp, void *app_data)
{
    //参数检查
    if (resp != nullptr) {
        *resp = nullptr;
    }
    if (num_msg <= 0 || num_msg > PAM_MAX_NUM_MSG) {
        return PAM_CONV_ERR;
    }

    //准备上下文数据
    const int nAuthNumber = (int)(int64_t)app_data;
    AuthAgent *app_ptr = g_pAgentPtr;
    struct AutoSubRef
    {
        LPPAM_AUTH_DATA pAuthData = nullptr;
        ~AutoSubRef() {
            if (pAuthData) {
                int refNum = pAuthData->subRef();
                //qDebug() << "pamConversation: SubRef, refNum =" << refNum << ", authNumber=" <<  pAuthData->m_nAuthNumber <<", thread = " << pthread_self();
            }
        }
    };
    AutoSubRef subref;
    LPPAM_AUTH_DATA pAuthData = nullptr;
    {
        std::lock_guard<std::mutex> guard(app_ptr->m_mutex);
        auto it_find = app_ptr->m_mAuthData.find(nAuthNumber);
        if (it_find == app_ptr->m_mAuthData.end()) {
            qDebug() << "pamConversation: not find auth data, authNumber=" << nAuthNumber << ", thread = " << pthread_self();
            return PAM_CONV_ERR;
        }
        pAuthData = it_find->second;
        int refNum = pAuthData->addRef();
        //qDebug() << "pamConversation: Addref, refNum =" << refNum << ", authNumber=" <<  pAuthData->m_nAuthNumber <<", thread = " << pthread_self();
        subref.pAuthData = pAuthData;
    }
    if (pAuthData == nullptr) {
        return PAM_CONV_ERR;
    }

    //申请返回的数据空间
    const int bufSize = num_msg * sizeof(pam_response);
    struct pam_response *pResponseBuf = static_cast<struct pam_response*>(malloc(bufSize));
    if (pResponseBuf == nullptr) {
        return PAM_BUF_ERR;
    }
    memset(pResponseBuf, 0, bufSize);

    //处理消息
    int nReturn = PAM_SUCCESS;
    AuthFlag auth_type = AuthFlag::Password;
    for (int idx = 0; idx < num_msg; ++idx) {
        int msgStyle = PAM_MSG_MEMBER(msg, idx, msg_style);
        qDebug() << "pamConversation Message styles =" << msgStyle << ", authNumber=" << pAuthData->m_nAuthNumber;
    }
    for (int idx = 0; idx < num_msg; ++idx) {
        if (nReturn == PAM_CONV_ERR) {
            break;
        }
        const int msgStyle = PAM_MSG_MEMBER(msg, idx, msg_style);
        switch(msgStyle) {
        case PAM_PROMPT_ECHO_OFF: {
            while (true) {
                usleep(10*1000);
                std::lock_guard<std::mutex> guard(app_ptr->m_mutex);
                if (pAuthData->isAuthFinished()) {
                    //qDebug() << "pamConversation canceled, authNumber=" << pAuthData->m_nAuthNumber << ", thread=" << pthread_self();
                    nReturn = PAM_CONV_ERR;//PAM_CONV_ERR,
                    pResponseBuf[idx].resp_retcode = PAM_SUCCESS;//PAM_AUTH_ERR，文档说这个字段未起作用，必须赋0值
                    pResponseBuf[idx].resp = NULL;
                    break;
                }
                if (pAuthData->m_hasFeedPassword) {
                    qDebug() << "pamConversation getPassword, authNumber=" << pAuthData->m_nAuthNumber << ", thread=" << pthread_self();
                    auth_type = AuthFlag::Password;
                    pResponseBuf[idx].resp = strdup(pAuthData->m_password.toLocal8Bit().data());
                    if (pResponseBuf[idx].resp == nullptr) {
                        nReturn = PAM_CONV_ERR;//PAM_BUF_ERR,
                        pResponseBuf[idx].resp_retcode = 0;//PAM_AUTH_ERR，文档说这个字段未起作用，必须赋0值
                        pResponseBuf[idx].resp = new char(0);
                        qDebug() << "pamConversation getPassword, strdup return nullptr, authNumber=" << pAuthData->m_nAuthNumber;
                    } else {
                        pResponseBuf[idx].resp_retcode = PAM_SUCCESS;
                    }
                    break;
                }
            }
            break;
        }

        case PAM_PROMPT_ECHO_ON:
        case PAM_ERROR_MSG:
            qDebug() << "pamConversation error: " << PAM_MSG_MEMBER(msg, idx, msg) << ", authNumber=" << pAuthData->m_nAuthNumber;
            app_ptr->displayErrorMsg(QString::fromLocal8Bit(PAM_MSG_MEMBER(msg, idx, msg)));
            auth_type = AuthFlag::Fingerprint;
            pResponseBuf[idx].resp_retcode = PAM_SUCCESS;
            break;
        case PAM_TEXT_INFO:
            qDebug() << "pamConversation info: " << PAM_MSG_MEMBER(msg, idx, msg) << ", authNumber=" << pAuthData->m_nAuthNumber;
            app_ptr->displayTextInfo(QString::fromLocal8Bit(PAM_MSG_MEMBER(msg, idx, msg)));
            pResponseBuf[idx].resp_retcode = PAM_SUCCESS;
            break;
        default:
            qDebug() << "pamConversation default: msgStyle=" << msgStyle << ", msg=" << PAM_MSG_MEMBER(msg, idx, msg) << ", authNumber=" << pAuthData->m_nAuthNumber;
            nReturn = PAM_CONV_ERR;
            break;
        }
    }

    //处理返回数据
    if (auth_type == AuthFlag::Password) {
        pAuthData->m_authType = AuthFlag::Password;
    } else if (auth_type == AuthFlag::Fingerprint) {
        pAuthData->m_authType = AuthFlag::Fingerprint;
    }

    if (resp == nullptr || nReturn == PAM_CONV_ERR) {
        for (int idx = 0; idx < num_msg; idx++) {
            if (pResponseBuf[idx].resp == nullptr) {
                continue;
            }
            free(pResponseBuf[idx].resp);
        }
        free(pResponseBuf);
    } else {
        *resp = pResponseBuf;
    }
    //qDebug() << "pamConversation return, authNumber=" << pAuthData->m_nAuthNumber << ", nReturn=" << nReturn << ", thread=" << pthread_self();
    return nReturn;
}

void AuthAgent::CalcelCurAuthNolock()
{
    if (m_curAuthData == nullptr) {
        return;
    }

    LPPAM_AUTH_DATA pAuthData = m_curAuthData;
    qDebug() << "CalcelCurAuthNolock: force pam_end, authNumber=" << pAuthData->m_nAuthNumber;

    //m_curAuthData = nullptr; //这里注释掉但不删作个提醒，上层可能还需要缓存的信息
    if (pAuthData->m_workThread != 0) {
        //pthread_cancel(pAuthData->m_workThread);
        //pAuthData->m_workThread = 0; //防止多次取消
    }
    if (pAuthData->isAuthFinished()) {
        return;
    }
    pAuthData->m_authStatus = Auth_Canceled;
}

void AuthAgent::ClearAuthDataNolock()
{
    auto it = m_mAuthData.begin();
    for (; it != m_mAuthData.end(); ) {
        LPPAM_AUTH_DATA itAuthData = it->second;
        if (itAuthData == m_curAuthData
        || itAuthData->m_authStatus != Auth_WaitDelete
        || itAuthData->loadRef() > 0) {
            ++it;
            continue;
        }
        it = m_mAuthData.erase(it);
        qDebug() << "Auth: delete data, authNumber=" << itAuthData->m_nAuthNumber;
        delete itAuthData;
    }
}

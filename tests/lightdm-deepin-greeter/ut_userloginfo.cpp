#include <gtest/gtest.h>
#include <QTest>
#include <pwd.h>
#include <QDebug>

#include "userlogininfo.h"
#include "sessionbasemodel.h"
#include "userframelist.h"
#include "userinfo.h"
#include "lockcontent.h"

class UT_UserLoginInfo : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    SessionBaseModel *m_sessionBaseModel;
    UserLoginInfo *m_userLoginInfo;
    LockContent *m_lockContent;
    std::shared_ptr<User> m_user;
};

void UT_UserLoginInfo::SetUp()
{
    m_sessionBaseModel = new SessionBaseModel(SessionBaseModel::AuthType::LightdmType);
    if (m_sessionBaseModel == nullptr) {
        return;
    }
    m_user = std::make_shared<User>();
    m_sessionBaseModel->updateCurrentUser(m_user);
    m_lockContent = new LockContent(m_sessionBaseModel);
    m_userLoginInfo = new UserLoginInfo(m_sessionBaseModel);
    if (m_userLoginInfo == nullptr) {
        return;
    }
    m_userLoginInfo->getUserFrameList()->setModel(m_sessionBaseModel);
}

void UT_UserLoginInfo::TearDown()
{
    delete m_sessionBaseModel;
    delete m_userLoginInfo;
    delete m_lockContent;
}

TEST_F(UT_UserLoginInfo, Validity)
{
    ASSERT_NE(m_lockContent, nullptr);
    ASSERT_NE(m_userLoginInfo, nullptr);
}

TEST_F(UT_UserLoginInfo, init)
{

}


#include "sessionmanager.h"

#include <QLabel>
#include <QTest>

#include <gtest/gtest.h>

class UT_SessionManager : public testing::Test
{

};


TEST_F(UT_SessionManager, BasicTest)
{
    SessionManager &sessionManager = SessionManager::Reference();

    EXPECT_TRUE(sessionManager.sessionCount() >= 0);
    EXPECT_FALSE(sessionManager.currentSession().isEmpty());
    EXPECT_FALSE(sessionManager.sessionInfo().isEmpty());
}

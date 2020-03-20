#include <stdafx.h>
#include "procfixture.h"
#include "kdlib/dbgengine.h"

using namespace kdlib;
using namespace testing;

class DbgEngineTest : public ProcessFixture
{
};

TEST_F(DbgEngineTest, DbgCommandSuppress)
{
    EXPECT_EQ(L"", kdlib::debugCommand(kdlib::debugCommand(L".printf /ow \"warning\""), true));

    EXPECT_EQ(L"normal", kdlib::debugCommand(L".printf /on \"normal\"", true, OutputFlag::Normal));
    EXPECT_EQ(L"", kdlib::debugCommand(L".printf /oe \"error\"", true, OutputFlag::Normal));

    EXPECT_EQ(L"", kdlib::debugCommand(L".printf /on \"normal\"", true, OutputFlag::Error));
    EXPECT_EQ(L"error", kdlib::debugCommand(L".printf /oe \"error\"", true, OutputFlag::Error));

    EXPECT_EQ(L"", kdlib::debugCommand(L".printf /os \"symbols\"", true, OutputFlag::Normal | OutputFlag::Warning));
}

TEST_F(DbgEngineTest, DbgCommandUnicode)
{
    EXPECT_EQ(L"💩", kdlib::debugCommand(kdlib::debugCommand(L".printf \"💩\""), true));
}

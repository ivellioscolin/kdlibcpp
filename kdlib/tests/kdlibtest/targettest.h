
#pragma once

#include "basefixture.h"

#include "kdlib/process.h"

using namespace kdlib;

class TargetTest : public BaseFixture
{
public:
};

TEST_F(TargetTest, getTargetProcess )
{
    ASSERT_NO_THROW( startProcess(L"targetapp.exe") );

    TargetProcessPtr  targetProcess;
    ASSERT_NO_THROW( targetProcess = TargetProcess::getCurrent() );
    EXPECT_TRUE( 0 != targetProcess );
    ASSERT_NO_THROW( targetProcess = TargetProcess::getByIndex(0) );
    EXPECT_TRUE( 0 != targetProcess );
}

TEST_F(TargetTest, getSystemId)
{
    ASSERT_NO_THROW( startProcess(L"targetapp.exe") );
    TargetProcessPtr  targetProcess;
    ASSERT_NO_THROW( targetProcess = TargetProcess::getCurrent() );
    ASSERT_TRUE( 0 != targetProcess );
    ASSERT_NE( 0, targetProcess->getSystemId() );
}

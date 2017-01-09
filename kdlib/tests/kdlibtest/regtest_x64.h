#pragma once

#include "procfixture.h"

using namespace kdlib;
using namespace testing;

class RegTest : public ProcessFixture 
{
};


TEST_F(RegTest, GetRegIndex)
{
    EXPECT_NO_THROW(getRegsiterIndex(L"rax"));
    EXPECT_NO_THROW(getRegsiterIndex(L"rsi"));

    EXPECT_THROW(getRegsiterIndex(L"RAX"), CPUException);
    EXPECT_THROW(getRegsiterIndex(L"RSI"), CPUException);
    EXPECT_THROW(getRegsiterIndex(L"abc"), CPUException);
}



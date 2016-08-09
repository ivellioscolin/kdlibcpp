#pragma once

#include "procfixture.h"
#include "kdlib/cpucontext.h"

using namespace kdlib;

class CPUContextTest : public ProcessFixture 
{
public:

    CPUContextTest() : ProcessFixture( L"cputest" ) {}
};

TEST_F( CPUContextTest, GetRegisters )
{
    CPUContextPtr  cpu;
    ASSERT_NO_THROW( cpu = loadCPUContext() );

    MEMDISPLACEMENT  displ;
    EXPECT_EQ( L"breakOnRun", findSymbol( cpu->getIP(), displ ) );
    EXPECT_NE( 0, cpu->getSP() );
}

TEST_F( CPUContextTest, RestoreContext )
{
    CPUContextPtr  cpu;
    ASSERT_NO_THROW( cpu = loadCPUContext() );

    ASSERT_NO_THROW( setRegisterByIndex( 10, getRegisterByIndex(10) + 10 ) );
    ASSERT_NO_THROW( setRegisterByName( L"eax", getRegisterByName(L"eax") + 121 ) );

    EXPECT_NE( cpu->getRegisterByIndex(10), getRegisterByIndex(10) );

    EXPECT_NO_THROW( cpu->restore() );

    EXPECT_EQ( cpu->getRegisterByIndex(10), getRegisterByIndex(10) );
    EXPECT_EQ( cpu->getRegisterByName(L"eax"), getRegisterByName(L"eax") );
}


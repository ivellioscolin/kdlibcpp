#pragma once

#include "processtest.h"
#include "kdlib/cpucontext.h"

using namespace kdlib;

class CPUContextTest : public ProcessTest 
{
public:

    CPUContextTest() : ProcessTest( L"cputest" ) {}
};

TEST_F( CPUContextTest, GetRegisters )
{
    CPUContextPtr  cpu;
    ASSERT_NO_THROW( cpu = loadCPUContext() );

    MEMDISPLACEMENT  displ;
    EXPECT_EQ( L"breakOnRun", findSymbol( cpu->getIP(), displ ) );
    EXPECT_NE( 0, cpu->getSP() );
    EXPECT_NE( 0, cpu->getFP() );
}


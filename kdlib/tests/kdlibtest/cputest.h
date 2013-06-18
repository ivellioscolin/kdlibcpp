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
}


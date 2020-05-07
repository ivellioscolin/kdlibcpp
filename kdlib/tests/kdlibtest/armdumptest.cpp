#include <stdafx.h>

#include "memdumpfixture.h"

#include "kdlib/typeinfo.h"
#include "kdlib/memaccess.h"
#include "kdlib/module.h"
#include "kdlib/stack.h"

using namespace kdlib;


class ARMKernelMiniDump : public MemDumpFixture
{
public:
    ARMKernelMiniDump()
        : MemDumpFixture( makeDumpFullName(MemDumps::WIN10_ARM) )
    {
    }
};

TEST_F(ARMKernelMiniDump, Test)
{
    // dbgmem
    EXPECT_EQ( 0, addr64(0) );
    EXPECT_EQ( 0xffffffff80000000, addr64(0x80000000) );

    // dbgeng
    EXPECT_EQ( CPU_ARM, getCPUType() );
    EXPECT_EQ( CPU_ARM, getCPUMode() );

    EXPECT_EQ( false, is64bitSystem() );

    EXPECT_EQ( addr64(evaluate(L"@$retreg").asULongLong()), addr64(getReturnReg()) );

    // typeinfo
    {
        const auto typeInfo{ loadType(L"nt!_IRP*") };
        ASSERT_TRUE( typeInfo.get() );

        EXPECT_EQ( 4, typeInfo->getSize() );
        EXPECT_EQ( 4, typeInfo->getPtrSize() );
    }

    // MS DIA
    {
        const std::wstring symName{ L"nt!KiUpdateThreadState" };
        EXPECT_EQ( addr64(evaluate(symName).asULongLong()), addr64(getSymbolOffset(symName)) );
    }

    // stack & CPU context
    {
        const auto stack = getStack();
        ASSERT_TRUE( stack.get() );
        ASSERT_TRUE( stack->getFrameCount() >= 1 );

        {
            auto frame0 = stack->getFrame(0);
            ASSERT_TRUE( frame0.get() );

            EXPECT_EQ( std::wstring(L"FxRequest::CompleteInternal"), findSymbol(frame0->getIP()) );

            const auto currentContext = frame0->getCPUContext();
            ASSERT_TRUE( currentContext.get() );

            EXPECT_EQ( addr64(evaluate(L"@$csp").asULongLong()), addr64(currentContext->getSP()) );
            EXPECT_EQ( addr64(evaluate(L"@$fp").asULongLong()), addr64(currentContext->getFP()) );
        }

        {
            auto frameWithVars = stack->getFrame(1);
            ASSERT_TRUE( frameWithVars.get() );

            EXPECT_EQ( std::wstring(L"imp_WdfRequestCompleteWithInformation"), findSymbol(frameWithVars->getIP()) );

            EXPECT_EQ( 0x102, frameWithVars->getTypedParam(L"RequestStatus")->getValue() );
        }
    }
}

TEST_F(ARMKernelMiniDump, DumpType)
{
    EXPECT_TRUE(isDumpAnalyzing());
    EXPECT_TRUE(isKernelDebugging());
    EXPECT_EQ(DumpType::KernelSmall, getDumpType());
}


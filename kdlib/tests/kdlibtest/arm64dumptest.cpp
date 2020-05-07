#include <stdafx.h>

#include "memdumpfixture.h"

#include "kdlib/typeinfo.h"
#include "kdlib/memaccess.h"
#include "kdlib/module.h"
#include "kdlib/stack.h"

using namespace kdlib;

class ARM64KernelMiniDump : public MemDumpFixture
{
public:
    ARM64KernelMiniDump()
        : MemDumpFixture( makeDumpFullName(MemDumps::WIN10_ARM64) )
    {
    }
};

TEST_F(ARM64KernelMiniDump, Test)
{
    // dbgmem
    EXPECT_EQ( 0, addr64(0) );
    EXPECT_EQ( 0x80000000, addr64(0x80000000) );

    // dbgeng
    EXPECT_EQ( CPU_ARM64, getCPUType() );
    EXPECT_EQ( CPU_ARM64, getCPUMode() );

    EXPECT_EQ( true, is64bitSystem() );

    EXPECT_EQ( evaluate(L"@$retreg").asULongLong(), getReturnReg() );

    // typeinfo
    {
        const auto typeInfo{ loadType(L"nt!_IRP*") };
        ASSERT_TRUE( typeInfo.get() );

        EXPECT_EQ( 8, typeInfo->getSize() );
        EXPECT_EQ( 8, typeInfo->getPtrSize() );
    }

    // MS DIA
    {
        const std::wstring symName{ L"nt!KiUpdateThreadState" };
        EXPECT_EQ( evaluate(symName).asULongLong(), getSymbolOffset(symName) );
    }

    // symexport
    {
        const auto clipsp{ loadModule(L"clipsp") };
        ASSERT_TRUE( clipsp.get() );

        EXPECT_TRUE( clipsp->getSymFile() == L"export symbols" );

        EXPECT_EQ( evaluate(L"clipsp!ClipSpInitialize").asULongLong(), clipsp->getSymbolVa(L"ClipSpInitialize") );
    }

    // stack & CPU context
    {
        const auto stack = getStack();
        ASSERT_TRUE( stack.get() );
        ASSERT_TRUE( stack->getFrameCount() >= 11 );

        {
            auto frame0 = stack->getFrame(0);
            ASSERT_TRUE( frame0.get() );

            EXPECT_EQ( std::wstring(L"KeBugCheck2"), findSymbol(frame0->getIP()) );

            const auto currentContext = frame0->getCPUContext();
            ASSERT_TRUE( currentContext.get() );

            EXPECT_EQ( evaluate(L"@$ip").asULongLong(), currentContext->getIP() );
            EXPECT_EQ( evaluate(L"@$csp").asULongLong(), currentContext->getSP() );
            EXPECT_EQ( evaluate(L"@$fp").asULongLong(), currentContext->getFP() );
        }

        {
            auto frameWithVars = stack->getFrame(11);
            ASSERT_TRUE( frameWithVars.get() );

            EXPECT_EQ( std::wstring(L"FxRequest::CompleteInternal"), findSymbol(frameWithVars->getIP()) );

            // sp-based variable
            EXPECT_EQ( 0xffffc18eaa798940, frameWithVars->getLocalVar(L"irp")->getElement(L"m_Irp")->getValue() );
            EXPECT_EQ( 0, frameWithVars->getTypedParam(L"Status")->getValue() );
        }
    }
}

TEST_F(ARM64KernelMiniDump, DumpType)
{
    EXPECT_TRUE(isDumpAnalyzing());
    EXPECT_TRUE(isKernelDebugging());
    EXPECT_EQ(DumpType::KernelSmall, getDumpType());
}

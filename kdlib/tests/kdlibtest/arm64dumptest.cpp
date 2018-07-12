#include <stdafx.h>

#include "memdumpfixture.h"

#include "kdlib/typeinfo.h"
#include "kdlib/memaccess.h"
#include "kdlib/module.h"

#include <Windows.h>

using namespace kdlib;

class ARM64KernelMiniDump : public MemDumpFixture
{
public:
    ARM64KernelMiniDump()
        : MemDumpFixture( MemDumpFixture::getARM64KernelDumpName())
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
        EXPECT_EQ(evaluate(symName).asULongLong(), getSymbolOffset(symName));
    }

    // symexport
    {
        const auto clipsp{ loadModule(L"clipsp") };
        ASSERT_TRUE( clipsp.get() );

        EXPECT_TRUE( clipsp->getSymFile() == L"export symbols" );

        EXPECT_EQ( evaluate(L"clipsp!ClipSpInitialize").asULongLong(), clipsp->getSymbolVa(L"ClipSpInitialize") );
    }
}

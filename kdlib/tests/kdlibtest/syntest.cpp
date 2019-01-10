#include <stdafx.h>

#include "procfixture.h"

using namespace kdlib;

class SyntheticsTest : public ProcessFixture
{
public:

    SyntheticsTest()
        : ProcessFixture(L"syntest")
    {
    }
};

TEST_F(SyntheticsTest, Symbol)
{
    SyntheticSymbol synSym{};
    EXPECT_NO_THROW(synSym = addSyntheticSymbol(m_targetModule->getBase() + 1, 1, L"artificial_symbol"));
    EXPECT_NO_THROW(removeSyntheticSymbol(synSym));

    // exception: symbol outside module
    EXPECT_THROW(addSyntheticSymbol(32 * 1024, 1, L"impossible_symbol"), DbgException);
}

TEST_F(SyntheticsTest, Module)
{
    constexpr MEMOFFSET_64 base = 64 * 1024;
    EXPECT_NO_THROW(addSyntheticModule(base, 1024, L"artificial_module"));
        SyntheticSymbol synSym{};
        EXPECT_NO_THROW(synSym = addSyntheticSymbol(base + 2, 1, L"artificial_symbol"));
        EXPECT_NO_THROW(removeSyntheticSymbol(synSym));
    EXPECT_NO_THROW(removeSyntheticModule(base));
}

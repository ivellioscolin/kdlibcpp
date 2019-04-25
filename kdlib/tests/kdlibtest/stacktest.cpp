#include <stdafx.h>

#include "memdumpfixture.h"
#include "eventhandlermock.h"

#include "kdlib/kdlib.h"

using namespace kdlib;

enum class StackTestMode
{
    Process,
    Wow64Dump,
};

class StackTest : public ::testing::TestWithParam<StackTestMode>
{
public:
    StackTest()
    {
    }

    virtual void SetUp()
    {
        switch (GetParam())
        {
        case StackTestMode::Process:
            m_processId = kdlib::startProcess(L"targetapp.exe stacktest");
            kdlib::targetGo(); // go to work break point
            break;

        case StackTestMode::Wow64Dump:
            // path to targetapp.pdb
            m_oldSymPath = kdlib::getSymbolPath();
            kdlib::appendSymbolPath(makeDumpDirName(MemDumps::STACKTEST_WOW64));

            m_dumpId = kdlib::loadDump(makeDumpFullName(MemDumps::STACKTEST_WOW64));
            break;

        default:
            assert(false);
        }
    }

    virtual void TearDown()
    {
        switch (GetParam())
        {
        case StackTestMode::Process:
            try {
                kdlib::terminateProcess(m_processId);
            }
            catch (const kdlib::DbgException &)
            {
            }
            break;

        case StackTestMode::Wow64Dump:
            try {
                kdlib::closeDump(m_dumpId);
                kdlib::setSymbolPath(m_oldSymPath);
            }
            catch (const kdlib::DbgException &)
            {
            }
            break;

        default:
            assert(false);
        }
    }

private:
    kdlib::PROCESS_DEBUG_ID m_processId = -1;
    kdlib::PROCESS_DEBUG_ID m_dumpId = -1;
    std::wstring  m_oldSymPath;
};

TEST_P( StackTest, Ctor )
{
    EXPECT_NO_THROW( getStack() );
    //    EXPECT_NO_THROW( getStack( loadCPUContext() ) );
}

TEST_P( StackTest, StackLength )
{
    StackPtr  stack;
    ASSERT_NO_THROW( stack = getStack() );
    EXPECT_LT( 3UL, stack->getFrameCount() );
}

//TEST_P( StackTest, GetFunction )
//{
//    StackFramePtr  frame;
//    TypedVarPtr  function;
//
//    ASSERT_NO_THROW( frame =  getStack()->getFrame(0) );
//    ASSERT_NO_THROW( function = frame->getFunction() );
//    EXPECT_EQ( std::wstring(L"stackTestClass::stackMethod"), findSymbol( function->getAddress() ) );
//
//    ASSERT_NO_THROW( frame =  getStack()->getFrame(1) );
//    ASSERT_NO_THROW( function = frame->getFunction() );
//    EXPECT_EQ( std::wstring(L"stackTestRun2"), findSymbol( function->getAddress() ) );
//
//    ASSERT_NO_THROW( frame =  getStack()->getFrame(2) );
//    ASSERT_NO_THROW( function = frame->getFunction() );
//    EXPECT_EQ( std::wstring(L"stackTestRun1"), findSymbol( function->getAddress() ) );
//}

TEST_P( StackTest, TypedParam )
{
    StackFramePtr  frame;
    ASSERT_NO_THROW( frame = getCurrentStackFrame() );

    EXPECT_EQ( 3, frame->getTypedParamCount() ); 

    TypedVarPtr  param;
    ASSERT_NO_THROW( param = frame->getTypedParam(0) );
    EXPECT_EQ(1234, param->deref()->getElement(L"m_param")->getValue());

    ASSERT_NO_THROW( param = frame->getTypedParam(1) );
    EXPECT_FLOAT_EQ( 0.8f, param->getValue().asFloat() );

    ASSERT_NO_THROW(param = frame->getTypedParam(2));
    EXPECT_EQ(10, param->getValue() );

    EXPECT_THROW(frame->getTypedParam(3), IndexException);
    EXPECT_THROW(frame->getTypedParam(-1), IndexException);

    EXPECT_EQ( 0, getStack()->getFrame(5)->getTypedParamCount() );
}

TEST_P( StackTest, FastcallParam )
{
    StackFramePtr  frame;
    ASSERT_NO_THROW(frame = getStack()->getFrame(1) );

    TypedVarPtr  param;

    ASSERT_NO_THROW(param = frame->getTypedParam(L"a"));
    EXPECT_EQ(10, param->getValue());

    ASSERT_NO_THROW(param = frame->getTypedParam(L"b"));
    EXPECT_FLOAT_EQ(0.8f, param->getValue().asFloat());
}

TEST_P( StackTest, TypedParamByName )
{
    StackFramePtr  frame;
    ASSERT_NO_THROW( frame = getCurrentStackFrame() );

    TypedVarPtr  param;
    ASSERT_NO_THROW( param = frame->getTypedParam(L"this") );
    EXPECT_EQ(1234, param->deref()->getElement(L"m_param")->getValue());

    ASSERT_NO_THROW( param = frame->getTypedParam(L"a") );
    EXPECT_FLOAT_EQ( 0.8f, param->getValue().asFloat() );

    ASSERT_NO_THROW( param = frame->getTypedParam(L"b") );
    EXPECT_EQ(10, param->getValue());
   
    EXPECT_THROW( frame->getTypedParam(L""), DbgException );
    EXPECT_THROW( frame->getTypedParam(L"notexist"), SymbolException );
    EXPECT_THROW( frame->getTypedParam(L"THIS"), SymbolException );
}

TEST_P( StackTest, LocalVars )
{
    StackFramePtr  frame;

    ASSERT_NO_THROW( frame = getStack()->getFrame(2) );
    EXPECT_EQ( 3, frame->getLocalVarCount() );

    EXPECT_FLOAT_EQ(0.0f, frame->getLocalVar(0)->getValue().asFloat());
    EXPECT_FLOAT_EQ(0.8f, frame->getLocalVar(L"localFloat")->getValue().asFloat());
    EXPECT_NO_THROW(frame->getLocalVar(L"localChars"));

    EXPECT_THROW(frame->getLocalVar(-1), IndexException);
    EXPECT_THROW(frame->getLocalVar(3), IndexException);
    EXPECT_THROW(frame->getLocalVar(L"Notexist"), SymbolException);

    EXPECT_FALSE(frame->findStaticVar(L"localChars"));
}

TEST_P( StackTest, StaticVars )
{
    StackFramePtr  frame;
    ASSERT_NO_THROW(frame = getStack()->getFrame(2));

    EXPECT_EQ( 2, frame->getStaticVarCount() );
    EXPECT_EQ(1, *frame->getStaticVar(0));
    EXPECT_EQ(1, *frame->getStaticVar(L"staticLong"));
    EXPECT_EQ(L"staticLong", frame->getStaticVarName(0));

    EXPECT_THROW(frame->getStaticVar(2), IndexException);
    EXPECT_THROW(frame->getStaticVar(-1), IndexException);
    EXPECT_THROW(frame->getStaticVarName(2), IndexException);
    EXPECT_THROW(frame->getStaticVar(L"Notexist"), SymbolException);
}

TEST_P( StackTest, ChangeCurrentFrame )
{
    StackPtr  stack;
    StackFramePtr  frame1;

    ASSERT_NO_THROW(stack = getStack());
    ASSERT_NO_THROW( frame1 = stack->getFrame(1) );
    ASSERT_NO_THROW( setCurrentStackFrame(frame1) );

    StackFramePtr  frame2;
    ASSERT_NO_THROW( frame2 = getCurrentStackFrame() );
    EXPECT_TRUE( frame1->getIP() == frame2->getIP() && 
        frame1->getRET() == frame2->getRET() &&
        frame1->getFP() == frame2->getFP() &&
        frame1->getSP() == frame2->getSP() );

    EXPECT_EQ( 1, getCurrentStackFrameNumber() );
    
    EXPECT_NO_THROW( resetCurrentStackFrame() );

    ASSERT_NO_THROW( frame1 = getStack()->getFrame(0) );

    ASSERT_NO_THROW( frame2 = getCurrentStackFrame() );
    EXPECT_TRUE( frame1->getIP() == frame2->getIP() && 
        frame1->getRET() == frame2->getRET() &&
        frame1->getFP() == frame2->getFP() &&
        frame1->getSP() == frame2->getSP() );

    EXPECT_EQ( 0, getCurrentStackFrameNumber() );

    ASSERT_NO_THROW(setCurrentStackFrameByIndex(2));
    EXPECT_EQ(2, getCurrentStackFrameNumber());

    ASSERT_NO_THROW(debugCommand(L".frame 3"));
    EXPECT_EQ(3, getCurrentStackFrameNumber());
}

TEST_P(StackTest, SwitchTo)
{
    StackPtr  stack;

    ASSERT_NO_THROW(stack = getStack());

    stack->getFrame(3)->switchTo();
    EXPECT_EQ(3, getCurrentStackFrameNumber());
}


TEST_P( StackTest, ChangeScopeEvent )
{
    using namespace testing;

    EventHandlerMock  eventHandler;

    EXPECT_CALL(eventHandler, onChangeLocalScope()).Times(2);
    EXPECT_CALL(eventHandler, onDebugOutput(_,_)).Times(AnyNumber());

    StackFramePtr  frame1;
    ASSERT_NO_THROW(frame1 = getStack()->getFrame(1));
    ASSERT_NO_THROW(setCurrentStackFrame(frame1));

    debugCommand(L".frame 2");
}

INSTANTIATE_TEST_CASE_P(Stack, StackTest, ::testing::Values(StackTestMode::Process, StackTestMode::Wow64Dump));

class Wow64StackTest : public MemDumpFixture
{
public:
    Wow64StackTest() :
        MemDumpFixture(makeDumpFullName(MemDumps::STACKTEST_WOW64))
    {
    }
};

TEST_F(Wow64StackTest, NotCurrentThread)
{
    ASSERT_NO_THROW(setCurrentThreadById(2));

    EXPECT_EQ(std::wstring(L"NtWaitForWorkViaWorkerFactory"), findSymbol(getCurrentStackFrame()->getIP()));

    StackPtr stack;
    ASSERT_NO_THROW(stack = getStack());
    ASSERT_TRUE(stack->getFrameCount() > 2);

    EXPECT_EQ(std::wstring(L"TppWorkerThread"), findSymbol(stack->getFrame(1)->getIP()));
}


class InlineStackTest : public ::testing::WithParamInterface<const wchar_t*>, public MemDumpFixture
{

public:

    InlineStackTest() : MemDumpFixture( makeDumpFullName(GetParam()) )
    {}

    void SetUp() override
    {
        MemDumpFixture::SetUp();

        // path to targetapp.pdb
        m_oldSymPath = kdlib::getSymbolPath();
        kdlib::appendSymbolPath(makeDumpDirName(GetParam()));
    }

    void TearDown() override 
    {
        kdlib::setSymbolPath(m_oldSymPath);

        MemDumpFixture::TearDown();
    }

protected:

    std::wstring  m_oldSymPath;
};

TEST_P(InlineStackTest, GetStack)
{
    EXPECT_NO_THROW(getStack(true));
}

TEST_P(InlineStackTest, InlineFrame)
{
    auto stk = getStack(true);
    EXPECT_TRUE(stk->getFrame(0)->isInline());
    EXPECT_FALSE(stk->getFrame(2)->isInline());
}

TEST_P(InlineStackTest, InlineFrameOffset)
{
    auto stk = getStack(true);
    EXPECT_TRUE(stk->getFrame(0)->getRET() == stk->getFrame(1)->getRET());
    EXPECT_TRUE(stk->getFrame(1)->getRET() == stk->getFrame(2)->getRET());
}

TEST_P(InlineStackTest, InlineFunc)
{
    auto func = loadTypedVar(L"stackTestRun1");
    auto stack = getStack(true);
    auto offset = stack->getFrame(2)->getIP();

    EXPECT_EQ(0, func->getInlineFunctions(func->getDebugStart()).size());
    EXPECT_EQ(2, func->getInlineFunctions(offset).size());

    EXPECT_EQ(L"stackTestClass::stackMethod", func->getInlineFunctions(offset).front()->getName());
    EXPECT_EQ(L"Void(__thiscall stackTestClass::)(Double, Int4B)", func->getInlineFunctions(offset).front()->getType()->getName());
}

TEST_P(InlineStackTest, InlineFuncSource)
{
    auto func = loadTypedVar(L"stackTestRun1");
    auto stack = getStack(true);
    auto offset = stack->getFrame(2)->getIP();
    
    std::wstring  fileName;
    unsigned long  lineno;
    auto inlineFunc = *std::next(func->getInlineFunctions(offset).begin());
    inlineFunc->getSourceLine(offset, fileName, lineno);
    EXPECT_EQ(144, lineno);
    EXPECT_NE(std::string::npos, fileName.find(L"targetapp.cpp"));
}

TEST_P(InlineStackTest, InlineStackGetFunc)
{
    auto stack = getStack(true);
    EXPECT_EQ(L"stackTestClass::stackMethod", stack->getFrame(0)->getFunction()->getName());
    EXPECT_EQ(L"stackTestRun2", stack->getFrame(1)->getFunction()->getName());
    EXPECT_EQ(L"stackTestRun1", stack->getFrame(2)->getFunction()->getName());
    EXPECT_EQ(L"stackTestRun", stack->getFrame(3)->getFunction()->getName());
}

TEST_P(InlineStackTest, InlineStackGetSymbol)
{
    auto stack = getStack(true);
    MEMDISPLACEMENT  displacement;
    EXPECT_EQ(L"stackTestClass::stackMethod", stack->getFrame(0)->findSymbol(displacement) );
    EXPECT_EQ(L"stackTestRun2", stack->getFrame(1)->findSymbol(displacement) );
    EXPECT_EQ(L"stackTestRun1", stack->getFrame(2)->findSymbol(displacement) );
    EXPECT_EQ(L"stackTestRun", stack->getFrame(3)->findSymbol(displacement));

    displacement = 0;
    stack->getFrame(2)->findSymbol(displacement);
    EXPECT_NE(0, displacement);
}

TEST_P(InlineStackTest, InlineStackSourceLine)
{
    auto stack = getStack(true);
    std::wstring  fileName;
    unsigned long  lineNumber;

    stack->getFrame(0)->getSourceLine(fileName, lineNumber);
    EXPECT_EQ(129, lineNumber);
    EXPECT_NE(std::string::npos, fileName.find(L"targetapp.cpp"));

    stack->getFrame(1)->getSourceLine(fileName, lineNumber);
    EXPECT_EQ(144, lineNumber);
    EXPECT_NE(std::string::npos, fileName.find(L"targetapp.cpp"));

    stack->getFrame(2)->getSourceLine(fileName, lineNumber);
    EXPECT_EQ(172, lineNumber);
    EXPECT_NE(std::string::npos, fileName.find(L"targetapp.cpp"));

    stack->getFrame(3)->getSourceLine(fileName, lineNumber);
    EXPECT_EQ(185, lineNumber);
    EXPECT_NE(std::string::npos, fileName.find(L"targetapp.cpp"));
}

INSTANTIATE_TEST_CASE_P(ReleaseStackDumps, InlineStackTest, ::testing::Values(
    MemDumps::STACKTEST_X64_RELEASE
    ,MemDumps::STACKTEST_WOW64_RELEASE
));

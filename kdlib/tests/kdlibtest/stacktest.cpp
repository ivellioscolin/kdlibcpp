#include <stdafx.h>

#include <math.h>

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
            m_dumpId = kdlib::loadDump(MemDumpFixture::getWow64UserDumpName());
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
    kdlib::PROCESS_DEBUG_ID m_processId{};
    kdlib::PROCESS_DEBUG_ID m_dumpId{};
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
    EventHandlerMock  eventHandler;

    EXPECT_CALL(eventHandler, onChangeLocalScope()).Times(2);

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
        MemDumpFixture{MemDumpFixture::getWow64UserDumpName()}
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

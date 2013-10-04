#pragma once

#include <math.h>

#include "processtest.h"

#include "kdlib/stack.h"

using namespace kdlib;

class StackTest : public ProcessTest 
{
public:

    StackTest() : ProcessTest( L"stacktest" ) {}
};

TEST_F( StackTest, Ctor )
{
    EXPECT_NO_THROW( getStack() );
    EXPECT_NO_THROW( getStack( loadCPUContext() ) );
}

TEST_F( StackTest, StackLength )
{
    StackPtr  stack;
    ASSERT_NO_THROW( stack = getStack() );
    EXPECT_LT( 3UL, stack->getFrameCount() );
}

TEST_F( StackTest, GetFunction )
{
    StackFramePtr  frame;
    ASSERT_NO_THROW( frame = getStackFrame() );

    TypedVarPtr  function;
    ASSERT_NO_THROW( function = frame->getFunction() );

    EXPECT_EQ( std::wstring(L"stackTestRun2"), findSymbol( function->getAddress() ) );
}

TEST_F( StackTest, TypedParam )
{
    StackFramePtr  frame;
    ASSERT_NO_THROW( frame = getStackFrame() );

    EXPECT_EQ( 3, frame->getTypedParamCount() ); 

    TypedVarPtr  param;
    ASSERT_NO_THROW( param = frame->getTypedParam(0) );
    EXPECT_EQ( 10, param->deref()->getValue() );

    ASSERT_NO_THROW( param = frame->getTypedParam(1) );
    EXPECT_FLOAT_EQ( 0.8f, param->getValue().asFloat() );

    ASSERT_NO_THROW( param = frame->getTypedParam(2) );
    EXPECT_EQ( "Bye", loadCStr( param->getValue().asULongLong() ) );


    ASSERT_NO_THROW( frame = getStack()->getFrame(1) );

    EXPECT_EQ( 3, frame->getTypedParamCount() ); 

    ASSERT_NO_THROW( param = frame->getTypedParam(0) );
    EXPECT_EQ( 10, param->getValue() );

    ASSERT_NO_THROW( param = frame->getTypedParam(1) );
    EXPECT_FLOAT_EQ( 0.8f, param->deref()->getValue().asFloat() );
}

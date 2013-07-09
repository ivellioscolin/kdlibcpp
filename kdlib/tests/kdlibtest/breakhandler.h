#pragma once


#include "kdlib/dbgengine.h"

#include "processtest.h"
#include "eventhandler.h"

using namespace kdlib;
using namespace testing;

class BreakHandlerTest : public ProcessTest 
{
public:

    BreakHandlerTest() : ProcessTest( L"breakhandlertest" ) {}

    virtual void TearDown() {
    }
};


TEST_F( BreakHandlerTest, StopOnBreak )
{
    EventHandlerMock    eventHandler;

    DefaultValue<kdlib::DebugCallbackResult>::Set( DebugCallbackNoChange );

    EXPECT_CALL( eventHandler, onBreakpoint( _ ) ).Times(1);
    EXPECT_CALL( eventHandler, onExecutionStatusChange( kdlib::DebugStatusGo) ).Times(1);
    EXPECT_CALL( eventHandler, onExecutionStatusChange( kdlib::DebugStatusBreak) ).Times(1);

    ASSERT_NO_THROW( softwareBreakPointSet( m_targetModule->getSymbolVa( L"CdeclFunc" ) ) );

    targetGo();
}

TEST_F( BreakHandlerTest, RemoveBreak )
{
    EventHandlerMock    eventHandler;

    DefaultValue<kdlib::DebugCallbackResult>::Set( DebugCallbackNoChange );

    EXPECT_CALL( eventHandler, onBreakpoint( _ ) ).Times( 0 );
    EXPECT_CALL( eventHandler, onExecutionStatusChange( kdlib::DebugStatusGo) ).Times(1);
    EXPECT_CALL( eventHandler, onExecutionStatusChange( kdlib::DebugStatusNoDebuggee) ).Times(1);


    BREAKPOINT_ID  bpid;
    ASSERT_NO_THROW( bpid = softwareBreakPointSet( m_targetModule->getSymbolVa( L"CdeclFunc" ) ) );
    ASSERT_NO_THROW( breakPointRemove( bpid ) );

    targetGo();
}






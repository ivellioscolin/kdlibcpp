#pragma once

#include "kdlib/breakpoint.h"

#include "procfixture.h"
#include "eventhandlermock.h"

using namespace kdlib;
using namespace testing;

class BreakPointTest : public ProcessFixture
{
public:

    BreakPointTest() : ProcessFixture( L"breakhandlertest" ) {}

    virtual void TearDown() {
    }
};


TEST_F( BreakPointTest, StopOnBreak )
{
    EventHandlerMock    eventHandler;

    DefaultValue<kdlib::DebugCallbackResult>::Set( DebugCallbackNoChange );

    EXPECT_CALL( eventHandler, onBreakpoint( _ ) ).Times(1);
    EXPECT_CALL( eventHandler, onExecutionStatusChange( kdlib::DebugStatusGo) ).Times(1);
    EXPECT_CALL( eventHandler, onExecutionStatusChange( kdlib::DebugStatusBreak) ).Times(1);

    ASSERT_NO_THROW( softwareBreakPointSet( m_targetModule->getSymbolVa( L"CdeclFunc" ) ) );

    targetGo();
}

TEST_F( BreakPointTest, RemoveBreak )
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


class BreakpointMock : public kdlib::BaseBreakpoint
{
public:

    MOCK_METHOD0( onHit, kdlib::DebugCallbackResult () );
};


TEST_F( BreakPointTest, BreakpointObject )
{
    BreakpointPtr  bp;

    DefaultValue<kdlib::DebugCallbackResult>::Set( DebugCallbackBreak );

    ASSERT_NO_THROW( bp = setBp<BreakpointMock>( m_targetModule->getSymbolVa( L"CdeclFunc" ) ) );

    EXPECT_CALL( static_cast<BreakpointMock&>(*bp), onHit() ).Times(1);

    targetGo();
}

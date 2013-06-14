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
};


TEST_F( BreakHandlerTest, Stop )
{
    EventHandlerMock    eventHandler;

    DefaultValue<kdlib::DebugCallbackResult>::Set( DebugCallbackNoChange );

    EXPECT_CALL( eventHandler, onBreakpoint( _ ) ).Times( AtLeast(1) );

    ASSERT_NO_THROW( softwareBreakPointSet( m_targetModule->getSymbolVa( L"CdeclFunc" ) ) );

    targetGo();
}





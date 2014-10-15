#pragma once

#include "basefixture.h"
#include "eventhandlermock.h"

using namespace kdlib;
using namespace testing;

class EventHandlerTest : public BaseFixture 
{
public:
};


TEST_F(EventHandlerTest, KillProcessTargetChanged)
{

    EventHandlerMock    eventHandler;

    DefaultValue<kdlib::DebugCallbackResult>::Set( DebugCallbackNoChange );

    EXPECT_CALL(eventHandler, onExecutionStatusChange(DebugStatusGo) ).Times(1);
    EXPECT_CALL(eventHandler, onExecutionStatusChange(DebugStatusBreak) ).Times(1);
    EXPECT_CALL(eventHandler, onExecutionStatusChange(DebugStatusNoDebuggee) ).Times(1);

    EXPECT_CALL(eventHandler, onModuleLoad(_, _) ).Times(AnyNumber());
    EXPECT_CALL(eventHandler, onModuleUnload(_, _) ).Times(AnyNumber());

    PROCESS_DEBUG_ID   process_id;

    ASSERT_NO_THROW( process_id = startProcess(L"targetapp.exe") );

    EXPECT_NO_THROW( terminateProcess(process_id) );
}


TEST_F(EventHandlerTest, DetachProcessTargetChanged)
{

    EventHandlerMock    eventHandler;

    DefaultValue<kdlib::DebugCallbackResult>::Set( DebugCallbackNoChange );

    EXPECT_CALL(eventHandler, onExecutionStatusChange(DebugStatusGo) ).Times(1);
    EXPECT_CALL(eventHandler, onExecutionStatusChange(DebugStatusBreak) ).Times(1);
    EXPECT_CALL(eventHandler, onExecutionStatusChange(DebugStatusNoDebuggee) ).Times(1);

    EXPECT_CALL(eventHandler, onModuleLoad(_, _) ).Times(AnyNumber());
    EXPECT_CALL(eventHandler, onModuleUnload(_, _) ).Times(AnyNumber());

    PROCESS_DEBUG_ID   process_id;

    ASSERT_NO_THROW( process_id = startProcess(L"targetapp.exe") );

    EXPECT_NO_THROW( detachProcess(process_id) );
}


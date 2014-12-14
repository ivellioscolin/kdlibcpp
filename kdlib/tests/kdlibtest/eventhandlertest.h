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
    EXPECT_CALL(eventHandler, onCurrentThreadChange(_)).Times(AnyNumber());

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
    EXPECT_CALL(eventHandler, onCurrentThreadChange(_)).Times(AnyNumber());

    PROCESS_DEBUG_ID   process_id;

    ASSERT_NO_THROW( process_id = startProcess(L"targetapp.exe") );

    EXPECT_NO_THROW( detachProcess(process_id) );
}


TEST_F(EventHandlerTest, KillProcessesTargetChanged)
{

    EventHandlerMock    eventHandler;

    DefaultValue<kdlib::DebugCallbackResult>::Set( DebugCallbackNoChange );

    EXPECT_CALL(eventHandler, onExecutionStatusChange(DebugStatusGo) ).Times(AnyNumber());
    EXPECT_CALL(eventHandler, onExecutionStatusChange(DebugStatusBreak) ).Times(AnyNumber());
    EXPECT_CALL(eventHandler, onExecutionStatusChange(DebugStatusNoDebuggee) ).Times(1);

    EXPECT_CALL(eventHandler, onModuleLoad(_, _) ).Times(AnyNumber());
    EXPECT_CALL(eventHandler, onModuleUnload(_, _) ).Times(AnyNumber());
    EXPECT_CALL(eventHandler, onCurrentThreadChange(_)).Times(AnyNumber());

    ASSERT_NO_THROW( startProcess(L"targetapp.exe") );
    ASSERT_NO_THROW( startProcess(L"targetapp.exe") );
    
    EXPECT_NO_THROW( terminateAllProcesses() );
}


TEST_F(EventHandlerTest, DetachProcessesTargetChanged)
{
    EventHandlerMock    eventHandler;

    DefaultValue<kdlib::DebugCallbackResult>::Set( DebugCallbackNoChange );

    EXPECT_CALL(eventHandler, onExecutionStatusChange(DebugStatusGo) ).Times(AnyNumber());
    EXPECT_CALL(eventHandler, onExecutionStatusChange(DebugStatusBreak) ).Times(AnyNumber());
    EXPECT_CALL(eventHandler, onExecutionStatusChange(DebugStatusNoDebuggee) ).Times(1);

    EXPECT_CALL(eventHandler, onModuleLoad(_, _) ).Times(AnyNumber());
    EXPECT_CALL(eventHandler, onModuleUnload(_, _) ).Times(AnyNumber());
    EXPECT_CALL(eventHandler, onCurrentThreadChange(_)).Times(AnyNumber());

    ASSERT_NO_THROW( startProcess(L"targetapp.exe") );
    ASSERT_NO_THROW( startProcess(L"targetapp.exe") );
    
    EXPECT_NO_THROW( detachAllProcesses() );
}

TEST_F(EventHandlerTest, ChangeCurrentThread)
{
    ASSERT_NO_THROW(startProcess(L"targetapp.exe  processtest"));

    EventHandlerMock    eventHandler;
    DefaultValue<kdlib::DebugCallbackResult>::Set(DebugCallbackNoChange);

    EXPECT_CALL(eventHandler, onCurrentThreadChange(_)).Times(2);
    EXPECT_CALL(eventHandler, onExecutionStatusChange(_)).Times(AnyNumber());

    unsigned long  threadNumber = kdlib::getNumberThreads();

    for (unsigned int i = 0; i < threadNumber; ++i)
    {
        kdlib::THREAD_DEBUG_ID  id = getThreadIdByIndex(i);
        kdlib::setCurrentThreadById(id);
        kdlib::setCurrentThreadById(id);
    }

    EXPECT_NO_THROW( terminateAllProcesses() );
}

TEST_F(EventHandlerTest, ScopeChange)
{
    ASSERT_NO_THROW(startProcess(L"targetapp.exe breakhandlertest") );

    targetGo();

    EventHandlerMock    eventHandler;

    EXPECT_CALL(eventHandler, onExecutionStatusChange(_)).Times(AnyNumber());
    EXPECT_CALL(eventHandler, onChangeLocalScope()).Times(AtLeast(2));

    EXPECT_NO_THROW( debugCommand(L".frame 2") );
    EXPECT_NO_THROW( debugCommand(L".frame 3") );

    EXPECT_NO_THROW( terminateAllProcesses() );
}

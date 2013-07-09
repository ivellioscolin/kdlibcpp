#pragma once

#include "kdlib\eventhandler.h"

#include "gmock\gmock.h"

class EventHandlerMock : public kdlib::EventHandler 
{
public:
    MOCK_METHOD1( onBreakpoint, kdlib::DebugCallbackResult ( kdlib::BREAKPOINT_ID bpId ) );
    MOCK_METHOD1( onException, kdlib::DebugCallbackResult ( kdlib::EXCEPTION_INFO exceptionInfo ) );
    MOCK_METHOD1( onExecutionStatusChange, void ( kdlib::ExecutionStatus executionStatus ) );
};

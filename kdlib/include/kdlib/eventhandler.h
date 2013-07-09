#pragma once

#include "kdlib/dbgtypedef.h"
#include "kdlib/dbgcallbacks.h"
namespace kdlib {


///////////////////////////////////////////////////////////////////////////////

class EventHandler : public DebugEventsCallback
{
public:

    virtual DebugCallbackResult onBreakpoint( BREAKPOINT_ID bpId ) {
        return DebugCallbackNoChange;
    }

    virtual DebugCallbackResult onException( EXCEPTION_INFO exceptionInfo ) {
        return DebugCallbackNoChange;
    }

    virtual void onExecutionStatusChange( ExecutionStatus executionStatus ) 
    {}

    EventHandler() {
       registerEventsCallback(this);
    }

    virtual ~EventHandler() {
       removeEventsCallback( this );
    }
};

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end

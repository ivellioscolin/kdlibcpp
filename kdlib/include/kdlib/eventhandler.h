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

    virtual DebugCallbackResult onException( const ExceptionInfo &exceptionInfo ) {
        return DebugCallbackNoChange;
    }

    virtual void onExecutionStatusChange( ExecutionStatus executionStatus ) 
    {}

    virtual DebugCallbackResult onModuleLoad( MEMOFFSET_64 offset, const std::wstring &name ) {
        return DebugCallbackNoChange;
    }

    virtual DebugCallbackResult onModuleUnload( MEMOFFSET_64 offset, const std::wstring &name ) {
        return DebugCallbackNoChange;
    }

    EventHandler() {
       registerEventsCallback(this);
    }

    virtual ~EventHandler() {
       removeEventsCallback( this );
    }
};

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end

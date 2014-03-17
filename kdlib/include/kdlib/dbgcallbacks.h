#pragma once

#include <string>

#include "kdlib/dbgtypedef.h"

namespace kdlib {

//////////////////////////////////////////////////////////////////////////////

struct DebugEventsCallback {

    virtual DebugCallbackResult onBreakpoint( BREAKPOINT_ID bpId ) = 0;
    virtual DebugCallbackResult onException( const ExceptionInfo &exceptionInfo ) = 0;
    virtual void onExecutionStatusChange( ExecutionStatus executionStatus ) = 0;
    virtual DebugCallbackResult onModuleLoad( MEMOFFSET_64 offset, const std::wstring &name ) = 0;
    virtual DebugCallbackResult onModuleUnload( MEMOFFSET_64 offset, const std::wstring &name ) = 0;
    virtual DebugCallbackResult onProcessExit( PROCESS_DEBUG_ID processid, ProcessExitReason  reason, unsigned long exitCode ) = 0;

   // virtual void onSymbolsLoaded(MEMOFFSET_64 modBase) = 0;
   // virtual void onSymbolsUnloaded(MEMOFFSET_64 modBase OPTIONAL) = 0;
};

void registerEventsCallback( DebugEventsCallback *callback );
void removeEventsCallback( DebugEventsCallback *callback );

//////////////////////////////////////////////////////////////////////////////

} //kdlib namespace end

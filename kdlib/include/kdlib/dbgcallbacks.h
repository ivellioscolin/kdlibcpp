#pragma once

#include <string>

#include "kdlib/dbgtypedef.h"

namespace kdlib {

//////////////////////////////////////////////////////////////////////////////

enum DEBUG_CALLBACK_RESULT {
    DebugCallbackProceed = 0,
    DebugCallbackNoChange = 1,
    DebugCallbackBreak = 2,
    DebugCallbackMax = 3
};

enum EXECUTION_STATUS {
    DebugStatusNoChange             = 0,
    DebugStatusGo                   = 1,
    DebugStatusBreak                = 6,
    DebugStatusNoDebuggee           = 7
};


struct DEBUG_EVENT_CALLBACK {
    virtual DEBUG_CALLBACK_RESULT OnBreakpoint( BREAKPOINT_ID bpId ) = 0;
    virtual DEBUG_CALLBACK_RESULT OnModuleLoad( MEMOFFSET_64 offset, const std::wstring &name ) = 0;
    virtual DEBUG_CALLBACK_RESULT OnModuleUnload( MEMOFFSET_64 offset, const std::wstring &name ) = 0;
   // virtual DEBUG_CALLBACK_RESULT OnException( ExceptionInfoPtr exceptInfo ) = 0;
    virtual void onExecutionStatusChange( EXECUTION_STATUS executionStatus ) = 0;
    virtual void onSymbolsLoaded(MEMOFFSET_64 modBase) = 0;
    virtual void onSymbolsUnloaded(MEMOFFSET_64 modBase OPTIONAL) = 0;
};

void eventRegisterCallbacks( const DEBUG_EVENT_CALLBACK *callbacks );
void eventRemoveCallbacks( const DEBUG_EVENT_CALLBACK *callbacks );

BREAKPOINT_ID breakPointSet( MEMOFFSET_64 offset, bool hardware = false, unsigned long size = 0, unsigned long accessType = 0 );
void breakPointRemove( BREAKPOINT_ID bpId );

//////////////////////////////////////////////////////////////////////////////

} //kdlib namespace end

#pragma once

#include "kdlib/dbgtypedef.h"
#include "kdlib/module.h"
#include "kdlib/breakpoint.h"
#include "kdlib/dbgcallbacks.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class ProcessMonitor {

public: // init and deinit

    static void init();
    static void deinit();

public: // notification

    static DebugCallbackResult processStart(PROCESS_DEBUG_ID id);
    static DebugCallbackResult processStop(PROCESS_DEBUG_ID id, ProcessExitReason reason, unsigned int ExitCode);
    static DebugCallbackResult moduleLoad(PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, const std::wstring &moduleName);
    static DebugCallbackResult moduleUnload(PROCESS_DEBUG_ID id, MEMOFFSET_64  offset, const std::wstring &moduleName);
    static DebugCallbackResult breakpointHit(PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, BreakpointType breakpointType);
    static void currentThreadChange(THREAD_DEBUG_ID id);
    static void executionStatusChange(ExecutionStatus status);
    static void changeLocalScope();
    static DebugCallbackResult exceptionHit(const ExceptionInfo& excinfo);

public: // process manipulation

    static unsigned int getNumberProcesses();
    static void processAllStop();

public: // module manipulation

    static ModulePtr getModule( MEMOFFSET_64  offset, PROCESS_DEBUG_ID id = -1 );
    static void insertModule( ModulePtr& module, PROCESS_DEBUG_ID id = -1 );

public: //breakpoint manipulation

    static BREAKPOINT_ID insertBreakpoint( BreakpointPtr& breakpoint, PROCESS_DEBUG_ID id = -1 );
    static void removeBreakpoint( BREAKPOINT_ID  bpid );
    static unsigned long getNumberBreakpoints(PROCESS_DEBUG_ID id = -1);
    static BreakpointPtr getBreakpointByIndex(unsigned long index, PROCESS_DEBUG_ID id = -1);
    static BreakpointPtr getBreakpointById(BREAKPOINT_ID  bpid );
    
public: //callbacks
    static void registerEventsCallback(DebugEventsCallback *callback);
    static void removeEventsCallback(DebugEventsCallback *callback);
};

///////////////////////////////////////////////////////////////////////////////

} // namespace kdlib

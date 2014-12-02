#pragma once

#include "kdlib/dbgtypedef.h"
#include "kdlib/module.h"
#include "kdlib/breakpoint.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class ProcessMonitor {

public:

    static void init();
    static void deinit();
    static void processStart( PROCESS_DEBUG_ID id );
    static void processStop( PROCESS_DEBUG_ID id );
    static unsigned int getNumberProcesses();

    static void moduleLoad( PROCESS_DEBUG_ID id, MEMOFFSET_64  offset );
    static void moduleUnload( PROCESS_DEBUG_ID id, MEMOFFSET_64  offset );
    static DebugCallbackResult breakpointHit( PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, BreakpointType breakpointType);

public:

    static void processAllStop();

    static ModulePtr  getModule( MEMOFFSET_64  offset, PROCESS_DEBUG_ID id = -1 );
    static void insertModule( ModulePtr& module, PROCESS_DEBUG_ID id = -1 );

    static BREAKPOINT_ID insertBreakpoint( BreakpointPtr& breakpoint, PROCESS_DEBUG_ID id = -1 );
    static void removeBreakpoint( BREAKPOINT_ID  bpid );
    static unsigned long getNumberBreakpoints();
    static BreakpointPtr getBreakpointByIndex(unsigned long index);
    static BreakpointPtr getBreakpointById(BREAKPOINT_ID  bpid );
};

///////////////////////////////////////////////////////////////////////////////

} // namespace kdlib

#pragma once

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

typedef unsigned long           PROCESS_ID;
typedef unsigned long           PROCESS_DEBUG_ID;
typedef unsigned long           THREAD_ID;
typedef unsigned long           BREAKPOINT_ID;
typedef unsigned long long      MEMOFFSET_64;
typedef unsigned long           MEMOFFSET_32;
typedef long                    MEMDISPLACEMENT;
typedef unsigned long           BITOFFSET;
typedef void*                   EXCEPTION_INFO;
typedef unsigned long           ACCESS_TYPE;

enum DebugCallbackResult {
    DebugCallbackProceed = 0,
    DebugCallbackNoChange = 1,
    DebugCallbackBreak = 2,
    DebugCallbackMax = 3
};

enum ExecutionStatus {
    DebugStatusNoChange             = 0,
    DebugStatusGo                   = 1,
    DebugStatusBreak                = 6,
    DebugStatusNoDebuggee           = 7
};

enum CPUType {
    CPU_I386 = 0,
    CPU_AMD64 = 1,
    CPU_MAX = 2
};

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end



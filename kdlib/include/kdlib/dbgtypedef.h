#pragma once

#include <string>

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

typedef unsigned long           PROCESS_ID;
typedef unsigned long           PROCESS_DEBUG_ID;
typedef unsigned long           THREAD_ID;
typedef unsigned long           THREAD_DEBUG_ID;
typedef unsigned long           BREAKPOINT_ID;
typedef unsigned long long      MEMOFFSET_64;
typedef unsigned long           MEMOFFSET_32;
typedef long                    MEMOFFSET_REL;
typedef long                    MEMDISPLACEMENT;
typedef unsigned long           BITOFFSET;
typedef unsigned long           ACCESS_TYPE;
typedef unsigned long long      EXTENSION_ID;
typedef unsigned long           RELREG_ID;

enum DebugCallbackResult {
    DebugCallbackProceed = 0,
    DebugCallbackNoChange = 1,
    DebugCallbackBreak = 2,
    DebugCallbackMax = 3
};

enum ExecutionStatus {
    DebugStatusNoChange,
    DebugStatusGo,
    DebugStatusBreak,
    DebugStatusNoDebuggee
};

enum CPUType {
    CPU_I386 = 0,
    CPU_AMD64 = 1,
    CPU_MAX = 2
};

enum CPURegType {
    RegInt8,
    RegInt16,
    RegInt32,
    RegInt64,
    RegFloat32,
    RegFloat64,
    RegFloat80,
    RegFloat128,
    RegVector64,
    RegVector128
};

struct SystemInfo {
    unsigned long  majorVersion;
    unsigned long  minorVersion;
    unsigned long  buildNumber;
    std::wstring  buildDescription;
};

struct ExceptionInfo {
    bool  firstChance;
    unsigned long  exceptionCode;
    unsigned long  exceptionFlags;
    MEMOFFSET_64  exceptionRecord;
    MEMOFFSET_64  exceptionAddress;

    unsigned long parameterCount;
    unsigned long long parameters[0x10];
};

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end



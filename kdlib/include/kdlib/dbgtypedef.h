#pragma once

#include <string>

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

typedef unsigned long           PROCESS_ID;
typedef unsigned long           PROCESS_DEBUG_ID;
typedef unsigned long           THREAD_ID;
typedef unsigned long           THREAD_DEBUG_ID;
typedef unsigned long           SYSTEM_DEBUG_ID;
typedef unsigned long           BREAKPOINT_ID;
typedef unsigned long long      MEMOFFSET_64;
typedef unsigned long           MEMOFFSET_32;
typedef long                    MEMOFFSET_REL;
typedef long                    MEMDISPLACEMENT;
typedef unsigned long           BITOFFSET;
typedef unsigned long           ACCESS_TYPE;
typedef unsigned long long      EXTENSION_ID;
typedef unsigned long           RELREG_ID;
typedef unsigned long long      SYMBOL_ID;

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


enum BreakpointType {

    SoftwareBreakpoint,
    DataAccessBreakpoint
};

enum CPUType {
    CPU_I386 = 0,
    CPU_AMD64 = 1,
    CPU_ARM64 = 2,
    CPU_ARM = 3,
    CPU_MAX = 4
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


enum EventType {
    EventTypeNoEvent,
    EventTypeBreakpoint,
    EventTypeException,
    EventTypeCreateThread,
    EventTypeExitThread,
    EventTypeCreateProcess,
    EventTypeExitProcess,
    EventTypeLoadModule,
    EventTypeUnloadModule,
    EventTypeSystemError,
    EventTypeSessionStatus,
    EventTypeChangeDebuggeeState,
    EventTypeChangeEngineState,
    EventTypeChangeSymbolState
};

struct SystemInfo {
    unsigned long  majorVersion;
    unsigned long  minorVersion;
    unsigned long  servicePackNumber;
    unsigned long  buildNumber;
    std::wstring  buildDescription;
};

struct SystemCrashInfo {
    unsigned long long  code;
    unsigned long paramterCount;
    unsigned long long parameters[0x10];
};

struct ExceptionInfo {
    bool  firstChance;
    unsigned long  exceptionCode;
    unsigned long  exceptionFlags;
    MEMOFFSET_64  exceptionRecord;
    MEMOFFSET_64  exceptionAddress;
    PROCESS_DEBUG_ID  process;
    THREAD_DEBUG_ID  thread;
    unsigned long parameterCount;
    unsigned long long parameters[0x10];
};

struct FixedFileInfo {
    unsigned long Signature;
    unsigned long StrucVersion;
    unsigned long FileVersionMS;
    unsigned long FileVersionLS;
    unsigned long ProductVersionMS;
    unsigned long ProductVersionLS;
    unsigned long FileFlagsMask;
    unsigned long FileFlags;
    unsigned long FileOS;
    unsigned long FileType;
    unsigned long FileSubtype;
    unsigned long FileDateMS;
    unsigned long FileDateLS;
};

enum FileFlag {
    FileFlagDebug           = 0x00000001,
    FileFlagPreRelease      = 0x00000002,
    FileFlagPatched         = 0x00000004,
    FileFlagPrivateBuild    = 0x00000008,
    FileFlagInfoInferred    = 0x00000010,
    FileFlagSpecialBuild    = 0x00000020,
};

enum MemoryProtect {
    PageNoAccess            = 0x00000001,
    PageReadOnly            = 0x00000002,
    PageReadWrite           = 0x00000004,
    PageWriteCopy           = 0x00000008,
    PageExecute             = 0x00000010,
    PageExecuteRead         = 0x00000020,
    PageExecuteReadWrite    = 0x00000040,
    PageExecuteWriteCopy    = 0x00000080
};

enum MemoryState {
    MemCommit               = 0x1000,
    MemReserve              = 0x2000,
    MemFree                 = 0x10000
};

enum MemoryType {
    MemPrivate              = 0x20000,
    MemMapped               = 0x40000,
    MemImage                = 0x1000000    
};

enum ProcessExitReason {
    ProcessExit,
    ProcessTerminate,
    ProcessDetach
};

enum ProcessDebugOptions {
    ProcessBreakOnStart    = 0x00000001,
    ProcessBreakOnStop     = 0x00000002,
    ProcessDebugChildren   = 0x00000004,
    ProcessNoDebugHeap     = 0x00000008,

    ProcessDebugDefault = ProcessBreakOnStart
};

typedef unsigned long  ProcessDebugFlags;


struct FrameDesc {
    MEMOFFSET_64            instructionOffset;
    MEMOFFSET_64            returnOffset;
    MEMOFFSET_64            frameOffset;
    MEMOFFSET_64            stackOffset; 
};

struct SyntheticSymbol {
    MEMOFFSET_64    moduleBase;
    SYMBOL_ID       symbolId;
};

enum VarStorage {
    UnknownVar,
    MemoryVar,
    RegisterVar
};


struct LiveProcessInfo {
    PROCESS_ID  pid;
    PROCESS_ID  parentPid;
    std::wstring  name;
    std::wstring  user;
    std::wstring  commandLine;
};

enum DebugOptions {
    AllowNetworkPaths = 0x00000004,
    DisallowNetworkPaths = 0x00000008,
    InitialBreak = 0x00000020,
    FinalBreak = 0x00000040,
    FailIncompleteInformation = 0x00000200,
    DisableModuleSymbolLoad = 0x00008000,
    DisallowImageFileMapping = 0x00020000,
    PreferDml =  0x00040000
};

typedef unsigned long  DebugOptionsSet;

enum BreakpointAccess {
    Read = 0x00000001,
    Write = 0x00000002,
    Execute =  0x00000004
};


enum OutputFlag
{
    Normal = 0x00000001,
    Error = 0x00000002,
    Warning = 0x00000004,
    Verbose = 0x00000008,
    Prompt = 0x00000010,
    PromptRegister = 0x00000020,
    ExtensionWarning = 0x00000040,
    Debuggee = 0x00000080,
    DebuggeePrompt = 0x00000100,
    Symbols = 0x00000200,
    Status = 0x00000400,
    All = 0x000007FF
};

typedef unsigned long  OutputFlagsSet;

enum DumpType
{
    Small = 1024,
    Default = 1025,
    Full = 1026,
    Image = 1027,
    KernelSmall = Small,
    Kernel = Default,
    KernelFull = Full
};

enum DumpFormat
{
    UserSmallFullMemory = 0x1,
    UserSmallHandleData = 0x2,
    UserSmallUnloadedModules = 0x4,
    UserSmallIndirectMemory = 0x8,
    UserSmallDataSegments = 0x10,
    UserSmallFilterMemory = 0x20,
    UserSmallFilterPaths = 0x40,
    UserSmallProcessThreadData = 0x80,
    UserSmallPrivateReadWriteMemory = 0x100,
    UserSmallNoOptionalData = 0x200,
    UserSmallFullMemoryInfo = 0x400,
    UserSmallThreadInfo = 0x800,
    UserSmallCodeSegments = 0x1000,
    UserSmallNoAuxiliaryState = 0x2000,
    UserSmallFullAuxiliaryState = 0x4000,
    UserSmallModuleHeaders = 0x8000,
    UserSmallFilterTriage = 0x10000,
    UserSmallAddAvxXStateContext = 0x20000,
    UserSmallIptTrace = 0x40000,
    UserSmallIgnoreInaccessibleMem = 0x08000000
};

typedef unsigned long  DumpFormatFlagsSet;

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end



#pragma once

#include <string>
#include <vector>

#include "kdlib/dbgtypedef.h"
#include "kdlib/variant.h"

namespace kdlib {

//////////////////////////////////////////////////////////////////////////////

// Initialization/Uninitialization
bool initialize();
void uninitialize();

// manage debug target

PROCESS_DEBUG_ID startProcess( const std::wstring  &processName, bool debugChildren = false );
PROCESS_DEBUG_ID attachProcess( PROCESS_ID pid );
void detachProcess( PROCESS_DEBUG_ID processId = -1);
void terminateProcess( PROCESS_DEBUG_ID processId = -1);
void detachAllProcesses();
void terminateAllProcesses();

void loadDump( const std::wstring &fileName );
void writeDump( const std::wstring &fileNamem, bool smallDump );

bool isLocalKernelDebuggerEnabled();
void attachKernel( const std::wstring &connectOptions = L"" );

bool isDumpAnalyzing();
bool isKernelDebugging();

std::wstring debugCommand( const std::wstring &command );
NumVariant evaluate( const std::wstring  &expression, bool cplusplus=false );

// system properties
size_t ptrSize();
bool is64bitSystem();
size_t getPageSize();
unsigned long getSystemUptime();
unsigned long getCurrentTime();
void getSystemInfo( SystemInfo &systemInfo );
void getSystemCrashInfo( SystemCrashInfo &crashInfo );

//manage target modules
unsigned long getNumberModules();
std::vector<MEMOFFSET_64> getModuleBasesList();
MEMOFFSET_64 findModuleBase( const std::wstring &moduleName );
MEMOFFSET_64 findModuleBase( MEMOFFSET_64 offset );
MEMOFFSET_64 findModuleBySymbol( const std::wstring &symbolName );
MEMOFFSET_32 getModuleSize( MEMOFFSET_64 baseOffset );
std::wstring getModuleName( MEMOFFSET_64 baseOffset );
std::wstring getModuleImageName( MEMOFFSET_64 baseOffset ); 
unsigned long getModuleTimeStamp( MEMOFFSET_64 baseOffset );
unsigned long getModuleCheckSum( MEMOFFSET_64 baseOffset );
bool isModuleUnloaded( MEMOFFSET_64 baseOffset );
bool isModuleUserMode( MEMOFFSET_64 baseOffset );
std::wstring getModuleSymbolFileName( MEMOFFSET_64 baseOffset );
std::string getModuleVersionInfo( MEMOFFSET_64 baseOffset, const std::string &value );
void getModuleFixedFileInfo( MEMOFFSET_64 baseOffset, FixedFileInfo &fixedFileInfo );

// Symbol path
std::wstring getSymbolPath();
void setSymbolPath(const std::wstring &symPath);
void appendSymbolPath(const std::wstring &symPath);

std::wstring getSourceFile( MEMOFFSET_64 offset = 0);
void getSourceLine( std::wstring &fileName, unsigned long &lineno, long &displacement, MEMOFFSET_64 offset = 0 );

// processes end threads
unsigned long getNumberThreads();
THREAD_DEBUG_ID getCurrentThreadId();
THREAD_DEBUG_ID getThreadIdByOffset(MEMOFFSET_64 offset);
THREAD_DEBUG_ID getThreadIdBySystemId(THREAD_ID tid = -1);
THREAD_DEBUG_ID getThreadIdByIndex(unsigned long index);

THREAD_ID getThreadSystemId(THREAD_DEBUG_ID id = -1);
MEMOFFSET_64 getThreadOffset(THREAD_DEBUG_ID id = -1);

void setCurrentThreadById( PROCESS_DEBUG_ID id );
void setCurrentThreadByOffset( MEMOFFSET_64 offset );

void setImplicitThread(MEMOFFSET_64 offset); 
MEMOFFSET_64 getImplicitThreadOffset();

MEMOFFSET_64 getCurrentThread();
void setCurrentThread(MEMOFFSET_64 offset);

unsigned long getNumberProcesses();
PROCESS_DEBUG_ID getCurrentProcessId();
PROCESS_DEBUG_ID getProcessIdByOffset( MEMOFFSET_64 offset );
PROCESS_DEBUG_ID getProcessIdBySystemId( PROCESS_ID pid = -1 );
PROCESS_DEBUG_ID getProcessIdByIndex(unsigned long index);

PROCESS_ID  getProcessSystemId( PROCESS_DEBUG_ID id = -1);
MEMOFFSET_64 getProcessOffset( PROCESS_DEBUG_ID id  = -1);

void setCurrentProcessById( PROCESS_DEBUG_ID id );
void setCurrentProcessByOffset( MEMOFFSET_64 offset );

void setImplicitProcess( MEMOFFSET_64 offset ); 
MEMOFFSET_64 getImplicitProcessOffset();

MEMOFFSET_64 getCurrentProcess();
void setCurrentProcess( MEMOFFSET_64  offset );

std::wstring getCurrentProcessExecutableName();

ExecutionStatus targetExecutionStatus();
ExecutionStatus targetGo();
ExecutionStatus targetStep();
ExecutionStatus targetStepIn();
void targetBreak();

// events
EventType getLastEventType();
PROCESS_DEBUG_ID getLastEventProcessId();
THREAD_DEBUG_ID getLastEventThreadId(); 
ExceptionInfo  getLastException();

// registers
MEMOFFSET_64 getInstructionOffset( THREAD_ID id = -1 );
MEMOFFSET_64 getStackOffset( THREAD_ID id = -1);
MEMOFFSET_64 getFrameOffset( THREAD_ID id = -1);

unsigned long getRegisterNumber( THREAD_ID id );
unsigned long getRegsiterIndex( THREAD_ID id, const std::wstring &name );
CPURegType getRegisterType( THREAD_ID id, unsigned long index );
std::wstring getRegisterName( THREAD_ID id, unsigned long index );
void getRegisterValue( THREAD_ID id, unsigned long index, void* buffer, size_t bufferSize );
CPUType getCPUType( THREAD_ID id );
CPUType getCPUMode( THREAD_ID id );
void setCPUMode( THREAD_ID id, CPUType mode );
unsigned long long loadMSR( THREAD_ID id, unsigned long msrIndex );
void setMSR( THREAD_ID id, unsigned long msrIndex, unsigned long long value );
unsigned long getNumberFrames(THREAD_ID id);
void getStackFrame( THREAD_ID id, unsigned long frameIndex, MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp );

//
// Extensions
std::wstring getExtensionSearchPath();
EXTENSION_ID loadExtension(const std::wstring &extPath );
void removeExtension( EXTENSION_ID extHandle );
void removeExtension( const std::wstring &extPath );
EXTENSION_ID getExtension(const std::wstring &extPath );
std::wstring callExtension( EXTENSION_ID extHandle, const std::wstring command, const std::wstring  &params  );

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end


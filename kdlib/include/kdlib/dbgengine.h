#pragma once

#include <string>
#include <vector>

#include "kdlib/dbgtypedef.h"

namespace kdlib {

//////////////////////////////////////////////////////////////////////////////

// Initialization/Uninitialization
bool initialize();
void uninitialize();

// manage debug target

PROCESS_DEBUG_ID startProcess( const std::wstring  &processName );
PROCESS_DEBUG_ID attachProcess( PROCESS_ID pid );
void detachProcess( PROCESS_DEBUG_ID processId = -1);
void terminateProcess( PROCESS_DEBUG_ID processId = -1);

void loadDump( const std::wstring &fileName );
void writeDump( const std::wstring &fileNamem, bool smallDump );

bool isDumpAnalyzing();
bool isKernelDebugging();

std::wstring debugCommand( const std::wstring &command );
unsigned long long evaluate( const std::wstring  &expression );

// system properties
size_t ptrSize();
bool is64bitSystem();
size_t getPageSize();
unsigned long getSystemUptime();
unsigned long getCurrentTime();
void getSystemInfo( SystemInfo &systemInfo );

//manage target modules
size_t getNumberModules();
std::vector<MEMOFFSET_64> getModuleBasesList();
MEMOFFSET_64 findModuleBase( const std::wstring &moduleName );
MEMOFFSET_64 findModuleBase( MEMOFFSET_64 offset );
MEMOFFSET_64 findModuleBySymbol( const std::wstring &symbolName );
MEMOFFSET_32 getModuleSize( MEMOFFSET_64 baseOffset );
std::wstring getModuleName( MEMOFFSET_64 baseOffset );
std::wstring getModuleImageName( MEMOFFSET_64 baseOffset ); 
unsigned long getModuleTimeStamp( MEMOFFSET_64 baseOffset );
unsigned long getModuleCheckSum( MEMOFFSET_64 baseOffset );
std::wstring getModuleSymbolFileName( MEMOFFSET_64 baseOffset );

// Symbol path
std::wstring getSymbolPath();
void setSymbolPath(const std::wstring &symPath);
void appendSymbolPath(const std::wstring &symPath);

std::wstring getSourceFile( MEMOFFSET_64 offset = 0);
void getSourceLine( std::wstring &fileName, unsigned long &lineno, long &displacement, MEMOFFSET_64 offset = 0 );

// processes end threads
size_t getNumberThreads();
THREAD_ID getCurrentThreadId();
void setCurrentThread( THREAD_ID id );
THREAD_ID getThreadSystemId( THREAD_ID id );

size_t getNumberProcesses();
PROCESS_DEBUG_ID getCurrentProcessId();
void setCurrentProcess( PROCESS_DEBUG_ID id );
MEMOFFSET_64 getProcessOffset( PROCESS_DEBUG_ID id );
PROCESS_DEBUG_ID getProcessIdByOffset( MEMOFFSET_64 offset );
PROCESS_ID getProcessSystemId( PROCESS_DEBUG_ID id );
PROCESS_DEBUG_ID getProcessIdBySystemId( PROCESS_ID pid );


//breakpoints
BREAKPOINT_ID softwareBreakPointSet( MEMOFFSET_64 offset );
BREAKPOINT_ID hardwareBreakPointSet( MEMOFFSET_64 offset, size_t = 0, ACCESS_TYPE accessType = 0 );
void breakPointRemove( BREAKPOINT_ID id );
void breakPointRemoveAll();

ExecutionStatus targetExecutionStatus();
ExecutionStatus targetGo();
ExecutionStatus targetStep();
ExecutionStatus targetStepIn();
void targetBreak();

// registers

MEMOFFSET_64 getInstructionOffset( THREAD_ID id = -1 );
MEMOFFSET_64 getStackOffset( THREAD_ID id = -1);
MEMOFFSET_64 getFrameOffset( THREAD_ID id = -1);

size_t getRegisterNumber( THREAD_ID id );
size_t getRegsiterIndex( THREAD_ID id, const std::wstring &name );
CPURegType getRegisterType( THREAD_ID id, size_t index );
std::wstring getRegisterName( THREAD_ID id, size_t index );
void getRegisterValue( THREAD_ID id, size_t index, void* buffer, size_t bufferSize );
CPUType getCPUType( THREAD_ID id );
CPUType getCPUMode( THREAD_ID id );
unsigned long long loadMSR( THREAD_ID id, size_t msrIndex );
void setMSR( THREAD_ID id, size_t msrIndex, unsigned long long value );

//
// Extensions
EXTENSION_ID loadExtension(const std::wstring &extPath );
void removeExtension( EXTENSION_ID extHandle );
std::wstring callExtension( EXTENSION_ID extHandle, const std::wstring command, const std::wstring  &params  );



///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end


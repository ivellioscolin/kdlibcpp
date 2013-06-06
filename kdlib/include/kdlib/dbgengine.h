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

void targetGo();
void targetStep();
void targetStepIn();
void targetBreak();

bool isDumpAnalyzing();
bool isKernelDebugging();

// system properties
size_t ptrSize();
bool is64bitSystem();
size_t getPageSize();
unsigned long getSystemUptime();
unsigned long getCurrentTime();

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
PROCESS_ID getCurrentProcessId();
THREAD_ID getCurrentThreadId();

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end


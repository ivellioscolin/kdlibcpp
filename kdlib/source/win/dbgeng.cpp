#include "stdafx.h"

#include "kdlib/dbgengine.h"
#include "win/exceptions.h"
#include "win/dbgmgr.h"

#include <vector>


namespace  kdlib {

///////////////////////////////////////////////////////////////////////////////

bool initialize()
{
    g_dbgMgr = new DebugManager();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void uninitialize()
{
    delete g_dbgMgr;
    g_dbgMgr = NULL;
}

///////////////////////////////////////////////////////////////////////////////

PROCESS_DEBUG_ID startProcess( const std::wstring  &processName )
{
    HRESULT     hres;

    ULONG       opt;
    hres = g_dbgMgr->control->GetEngineOptions( &opt );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetEngineOptions failed", hres  );

    opt |= DEBUG_ENGOPT_INITIAL_BREAK;
    hres = g_dbgMgr->control->SetEngineOptions( opt );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::SetEngineOptions failed", hres );

    std::vector< std::wstring::value_type >      cmdLine( processName.size() + 1 );
    wcscpy_s( &cmdLine[0], cmdLine.size(), processName.c_str() );

    hres = g_dbgMgr->client->CreateProcessWide( 0, &cmdLine[0], DEBUG_PROCESS | DETACHED_PROCESS );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient4::CreateProcessWide failed", hres );

    hres = g_dbgMgr->control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::WaitForEvent failed", hres );

    ULONG processId = -1;
    hres = g_dbgMgr->system->GetCurrentProcessId( &processId );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessId failed", hres );

    return processId;
}

///////////////////////////////////////////////////////////////////////////////

void terminateProcess( PROCESS_DEBUG_ID processId )
{
    HRESULT     hres;

    if ( processId != -1 )
    {
        hres = g_dbgMgr->system->SetCurrentProcessId(processId);
        if ( FAILED(hres) )
            throw DbgEngException( L"IDebugSystemObjects::SetCurrentProcessId failed", hres );
    }

    hres = g_dbgMgr->client->TerminateCurrentProcess();
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient::TerminateCurrentProcess", hres );
}

///////////////////////////////////////////////////////////////////////////////

PROCESS_DEBUG_ID attachProcess( PROCESS_ID pid )
{
    HRESULT     hres;

    ULONG       opt;
    hres = g_dbgMgr->control->GetEngineOptions( &opt );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetEngineOptions", hres );

    opt |= DEBUG_ENGOPT_INITIAL_BREAK;
    hres = g_dbgMgr->control->SetEngineOptions( opt );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::SetEngineOptions", hres );
    
    hres = g_dbgMgr->client->AttachProcess( 0, pid, 0 );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient::AttachProcess", hres );

    hres = g_dbgMgr->control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::WaitForEvent", hres );

    ULONG processId = -1;
    hres = g_dbgMgr->system->GetCurrentProcessId( &processId );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessId", hres );

    return processId;
}

///////////////////////////////////////////////////////////////////////////////

void detachProcess( PROCESS_DEBUG_ID processId )
{
    HRESULT  hres;

    if ( processId != -1 )
    {
        hres = g_dbgMgr->system->SetCurrentProcessId(processId);
        if ( FAILED(hres) )
            throw DbgEngException( L"IDebugSystemObjects::SetCurrentProcessId failed", hres );
    }

    hres = g_dbgMgr->client->DetachCurrentProcess();
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient::DetachCurrentProcess failed", hres );
}


///////////////////////////////////////////////////////////////////////////////

void loadDump( const std::wstring &fileName )
{
    HRESULT     hres;
     
    hres = g_dbgMgr->client->OpenDumpFileWide( fileName.c_str(), NULL );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient4::OpenDumpFileWide failed", hres );

    hres = g_dbgMgr->control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::WaitForEvent failed", hres );
}

///////////////////////////////////////////////////////////////////////////////

void writeDump( const std::wstring &fileName, bool smallDump )
{
    HRESULT     hres;

    ULONG       debugClass, debugQualifier;
    
    hres = g_dbgMgr->control->GetDebuggeeType( &debugClass, &debugQualifier );
    
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetDebuggeeType  failed", hres );

    hres = g_dbgMgr->client->WriteDumpFileWide(
        fileName.c_str(), 
        NULL,
        smallDump ? DEBUG_DUMP_SMALL : ( debugClass == DEBUG_CLASS_KERNEL ? DEBUG_DUMP_FULL : DEBUG_DUMP_DEFAULT ),
        DEBUG_FORMAT_DEFAULT,
        NULL );

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugClient4::WriteDumpFileWide failed", hres );
}

///////////////////////////////////////////////////////////////////////////////////

bool isDumpAnalyzing()
{
    HRESULT         hres;
    ULONG           debugClass, debugQualifier;
    
    hres = g_dbgMgr->control->GetDebuggeeType( &debugClass, &debugQualifier );
    
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetDebuggeeType  failed", hres );
         
    return debugQualifier >= DEBUG_DUMP_SMALL;
}

///////////////////////////////////////////////////////////////////////////////////

bool isKernelDebugging()
{
    HRESULT     hres;
    ULONG       debugClass, debugQualifier;
    
    hres = g_dbgMgr->control->GetDebuggeeType( &debugClass, &debugQualifier );
    
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetDebuggeeType  failed", hres );
         
    return debugClass == DEBUG_CLASS_KERNEL;
}

///////////////////////////////////////////////////////////////////////////////////

void targetGo()
{
    HRESULT     hres;

    hres = g_dbgMgr->control->SetExecutionStatus( DEBUG_STATUS_GO );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::SetExecutionStatus failed", hres );

    ULONG    currentStatus;

    do {
        hres = g_dbgMgr->control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugControl::WaitForEvent failed", hres );

        hres = g_dbgMgr->control->GetExecutionStatus( &currentStatus );

        if ( FAILED( hres ) )
            throw  DbgEngException( L"IDebugControl::GetExecutionStatus  failed", hres ); 

    } while( currentStatus != DEBUG_STATUS_BREAK && currentStatus != DEBUG_STATUS_NO_DEBUGGEE );
}

///////////////////////////////////////////////////////////////////////////////////

void targetStep()
{
    HRESULT     hres;

    hres = g_dbgMgr->control->SetExecutionStatus( DEBUG_STATUS_STEP_OVER );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::SetExecutionStatus failed", hres );

    ULONG    currentStatus;

    do {
        hres = g_dbgMgr->control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugControl::WaitForEvent failed", hres );

        hres = g_dbgMgr->control->GetExecutionStatus( &currentStatus );

        if ( FAILED( hres ) )
            throw  DbgEngException( L"IDebugControl::GetExecutionStatus  failed", hres ); 

    } while( currentStatus != DEBUG_STATUS_BREAK && currentStatus != DEBUG_STATUS_NO_DEBUGGEE );
}

///////////////////////////////////////////////////////////////////////////////

void targetStepIn()
{
    HRESULT     hres;

    hres = g_dbgMgr->control->SetExecutionStatus( DEBUG_STATUS_STEP_INTO );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::SetExecutionStatus", hres );

    ULONG    currentStatus;

    do {
        hres = g_dbgMgr->control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugControl::WaitForEvent", hres );

        hres = g_dbgMgr->control->GetExecutionStatus( &currentStatus );

        if ( FAILED( hres ) )
            throw  DbgEngException( L"IDebugControl::GetExecutionStatus", hres ); 

    } while( currentStatus != DEBUG_STATUS_BREAK && currentStatus != DEBUG_STATUS_NO_DEBUGGEE );
}

///////////////////////////////////////////////////////////////////////////////

void targetBreak()
{
    HRESULT  hres;

    hres = g_dbgMgr->control->SetInterrupt( DEBUG_INTERRUPT_ACTIVE );

    if ( FAILED( hres ) )
        throw  DbgEngException( L"IDebugControl::SetInterrupt", hres ); 
}

///////////////////////////////////////////////////////////////////////////////

size_t ptrSize()
{
    HRESULT  hres;

    hres = g_dbgMgr->control->IsPointer64Bit();

    if ( FAILED( hres ) )
        throw  DbgEngException( L"IDebugControl::IsPointer64Bit", hres );
    
    return S_OK == hres ? 8 : 4;
}

///////////////////////////////////////////////////////////////////////////////////

bool is64bitSystem()
{
    HRESULT  hres;
    ULONG  procType;

    hres = g_dbgMgr->control->GetActualProcessorType( &procType );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetActualProcessorType", hres );
        
    return procType == IMAGE_FILE_MACHINE_AMD64;
}

///////////////////////////////////////////////////////////////////////////////

size_t getPageSize()
{
    HRESULT  hres;
    ULONG  pageSize;

    hres = g_dbgMgr->control->GetPageSize( &pageSize );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetPageSize", hres );

    return pageSize;
}

///////////////////////////////////////////////////////////////////////////////

ULONG getSystemUptime()
{
    HRESULT  hres;
    ULONG  time;

    hres = g_dbgMgr->control->GetCurrentSystemUpTime( &time );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl2::GetCurrentSystemUpTime", hres );

    return time;
}

///////////////////////////////////////////////////////////////////////////////

ULONG getCurrentTime()
{
    HRESULT  hres;
    ULONG  time;

    hres = g_dbgMgr->control->GetCurrentTimeDate( &time );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl2::GetCurrentTimeDate", hres );

    return time;
}

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end 





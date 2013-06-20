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

ExecutionStatus targetChangeStatus( ULONG status )
{
    HRESULT     hres;

    hres = g_dbgMgr->control->SetExecutionStatus( status );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::SetExecutionStatus failed", hres );

    ULONG    currentStatus;

    do {
        hres = g_dbgMgr->control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
        if ( FAILED( hres ) )
        {
            hres = g_dbgMgr->control->GetExecutionStatus( &currentStatus );
            if ( FAILED( hres ) )
                throw  DbgEngException( L"IDebugControl::GetExecutionStatus  failed", hres ); 

            if ( currentStatus == DEBUG_STATUS_NO_DEBUGGEE )
                return DebugStatusNoDebuggee;
            
            throw DbgEngException( L"IDebugControl::WaitForEvent failed", hres );
        }

        hres = g_dbgMgr->control->GetExecutionStatus( &currentStatus );

        if ( FAILED( hres ) )
            throw  DbgEngException( L"IDebugControl::GetExecutionStatus  failed", hres ); 

        if ( currentStatus == DEBUG_STATUS_BREAK )
            return DebugStatusBreak;

    } while( TRUE );
}


ExecutionStatus targetGo()
{
    return targetChangeStatus( DEBUG_STATUS_GO );
}


ExecutionStatus targetStep()
{
    return targetChangeStatus( DEBUG_STATUS_STEP_OVER );
}

ExecutionStatus targetStepIn()
{
    return targetChangeStatus( DEBUG_STATUS_STEP_INTO );
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

PROCESS_ID getCurrentProcessId()
{
    HRESULT      hres;
    ULONG        id;

    hres = g_dbgMgr->system->GetCurrentProcessId( &id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects2::GetCurrentProcessSystemId", hres ); 

    return  PROCESS_ID(id);
}

///////////////////////////////////////////////////////////////////////////////

THREAD_ID getCurrentThreadId()
{
    HRESULT      hres;
    ULONG        id;

    hres = g_dbgMgr->system->GetCurrentThreadId( &id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects2::GetCurrentThreadId", hres ); 
        
     return THREAD_ID(id);
}

///////////////////////////////////////////////////////////////////////////////

size_t getNumberThreads()
{
    HRESULT     hres;
    ULONG       number;

    hres = g_dbgMgr->system->GetNumberThreads( &number );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetNumberThreads", hres ); 
        
    return number;
}

///////////////////////////////////////////////////////////////////////////////

BREAKPOINT_ID softwareBreakPointSet( MEMOFFSET_64 offset )
{
    HRESULT  hres;

    IDebugBreakpoint  *bp;
    hres = g_dbgMgr->control->AddBreakpoint(
        DEBUG_BREAKPOINT_CODE,
        DEBUG_ANY_ID,
        &bp);

    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::AddBreakpoint", hres);

    hres = bp->SetOffset(offset);
    if (S_OK != hres)
    {
        g_dbgMgr->control->RemoveBreakpoint(bp);
        throw DbgEngException(L"IDebugBreakpoint::SetOffset", hres);
    }

    ULONG bpFlags;
    hres = bp->GetFlags(&bpFlags);
    if (S_OK != hres)
    {
        g_dbgMgr->control->RemoveBreakpoint(bp);
        throw DbgEngException(L"IDebugBreakpoint::GetFlags", hres);
    }


    bpFlags |= DEBUG_BREAKPOINT_ENABLED | DEBUG_BREAKPOINT_ADDER_ONLY | DEBUG_BREAKPOINT_GO_ONLY;
    hres = bp->SetFlags(bpFlags);
    if (S_OK != hres)
    {
        g_dbgMgr->control->RemoveBreakpoint(bp);
        throw DbgEngException( L"IDebugBreakpoint::SetFlags", hres);
    }

    ULONG  breakId;
    hres = bp->GetId(&breakId);
    if (S_OK != hres)
    {
        g_dbgMgr->control->RemoveBreakpoint(bp);
        throw DbgEngException( L"IDebugBreakpoint::GetId", hres);
    }

    return breakId;
}

///////////////////////////////////////////////////////////////////////////////

void breakPointRemove( BREAKPOINT_ID id )
{
    IDebugBreakpoint *bp;
    HRESULT hres = g_dbgMgr->control->GetBreakpointById(id, &bp);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::GetBreakpointById", hres);

    hres = g_dbgMgr->control->RemoveBreakpoint(bp);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::RemoveBreakpoint", hres);
}

///////////////////////////////////////////////////////////////////////////////

void registerEventsCallback( DebugEventsCallback *callback )
{
    g_dbgMgr->registerEventsCallback( callback );
}

///////////////////////////////////////////////////////////////////////////////

void removeEventsCallback( DebugEventsCallback *callback )
{
    g_dbgMgr->removeEventsCallback( callback );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getInstructionOffset( THREAD_ID id )
{
    HRESULT      hres;
    ULONG        previousId;

    hres = g_dbgMgr->system->GetCurrentThreadId( &previousId );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentThreadId", hres ); 

    hres = g_dbgMgr->system->SetCurrentThreadId( id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres ); 

    MEMOFFSET_64 offset;
    hres =  g_dbgMgr->registers->GetInstructionOffset( &offset );

    g_dbgMgr->system->SetCurrentThreadId( previousId );

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugRegisters::GetInstructionOffset", hres ); 

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getStackOffset( THREAD_ID id )
{
    HRESULT      hres;
    ULONG        previousId;

    hres = g_dbgMgr->system->GetCurrentThreadId( &previousId );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentThreadId", hres ); 

    hres = g_dbgMgr->system->SetCurrentThreadId( id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres ); 

    MEMOFFSET_64 offset;
    hres =  g_dbgMgr->registers->GetStackOffset( &offset );

    g_dbgMgr->system->SetCurrentThreadId( previousId );

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugRegisters::GetStackOffset", hres ); 

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getFrameOffset( THREAD_ID id )
{
    HRESULT      hres;
    ULONG        previousId;

    hres = g_dbgMgr->system->GetCurrentThreadId( &previousId );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentThreadId", hres ); 

    hres = g_dbgMgr->system->SetCurrentThreadId( id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres ); 

    MEMOFFSET_64 offset;
    hres =  g_dbgMgr->registers->GetFrameOffset( &offset );

    g_dbgMgr->system->SetCurrentThreadId( previousId );

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugRegisters::GetFrameOffset", hres ); 

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end 





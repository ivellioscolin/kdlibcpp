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

std::wstring debugCommand( const std::wstring &command )
{
    HRESULT         hres;
    OutputReader    outReader( g_dbgMgr->client );

    hres = g_dbgMgr->control->ExecuteWide( DEBUG_OUTCTL_THIS_CLIENT, command.c_str(), 0 );

    if ( FAILED( hres ) )
        throw  DbgEngException( L"IDebugControl::Execute", hres ); 

    return std::wstring( outReader.Line() ); 
}

///////////////////////////////////////////////////////////////////////////////////

unsigned long long evaluate( const std::wstring  &expression )
{
    HRESULT             hres;
    ULONG64             value = 0;
    DEBUG_VALUE         debugValue = {};
    ULONG               remainderIndex = 0;

    hres = g_dbgMgr->control->IsPointer64Bit();
    if ( FAILED( hres ) )
        throw  DbgEngException( L"IDebugControl::IsPointer64Bit", hres );
    
    if ( hres == S_OK )
    {
        hres = g_dbgMgr->control->EvaluateWide( 
            expression.c_str(), 
            DEBUG_VALUE_INT64,
            &debugValue,
            &remainderIndex );
            
        if ( FAILED( hres ) )
            throw  DbgEngException( L"IDebugControl::Evaluate", hres );
            
        if ( remainderIndex == expression.length() )
            value = debugValue.I64;
    }
    else
    {
        hres = g_dbgMgr->control->EvaluateWide( 
            expression.c_str(), 
            DEBUG_VALUE_INT32,
            &debugValue,
            &remainderIndex );
            
        if (  FAILED( hres ) )
            throw  DbgEngException( L"IDebugControl::Evaluated", hres );
            
        if ( remainderIndex == expression.length() )
            value = debugValue.I32;
    }

    return value;
}

///////////////////////////////////////////////////////////////////////////////////

ExecutionStatus targetExecutionStatus()
{
    HRESULT     hres;

    ULONG    currentStatus;

    hres = g_dbgMgr->control->GetExecutionStatus( &currentStatus );

    if ( FAILED( hres ) )
        throw  DbgEngException( L"IDebugControl::GetExecutionStatus  failed", hres ); 

    return ConvertDbgEngineExecutionStatus( currentStatus );
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

void getSystemInfo( SystemInfo &systemInfo )
{
    HRESULT  hres;

    ULONG platformId, servicePackNumber;

    systemInfo.buildNumber = 0;
    systemInfo.majorVersion = 0;
    systemInfo.minorVersion = 0;

    hres = g_dbgMgr->control->GetSystemVersion(
        &platformId,
        &systemInfo.majorVersion,
        &systemInfo.minorVersion,
        NULL,
        0,
        NULL,
        &servicePackNumber,
        NULL,
        0,
        NULL );

    if ( FAILED( hres ) )
         throw DbgEngException( L"IDebugControl::GetSystemVersion", hres );

    ULONG strSize = 0;

    hres = g_dbgMgr->control->GetSystemVersionStringWide( DEBUG_SYSVERSTR_BUILD, NULL, 0, &strSize );

    if ( strSize == 0 )
         throw DbgEngException( L"IDebugControl4::GetSystemVersionStringWide", hres );

    std::vector<wchar_t> buffer(strSize);

    hres = g_dbgMgr->control->GetSystemVersionStringWide( DEBUG_SYSVERSTR_BUILD, &buffer[0], buffer.size(), NULL );
    if ( FAILED( hres ) )
         throw DbgEngException( L"IDebugControl::GetSystemVersion", hres );

    systemInfo.buildDescription = std::wstring( &buffer[0] );

}

///////////////////////////////////////////////////////////////////////////////

PROCESS_ID getCurrentProcessId()
{
    HRESULT      hres;
    ULONG        id;

    hres = g_dbgMgr->system->GetCurrentProcessId( &id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessId", hres ); 

    return  PROCESS_ID(id);
}

///////////////////////////////////////////////////////////////////////////////

PROCESS_ID getProcessSystemId( PROCESS_DEBUG_ID id )
{
    HRESULT      hres;
    ULONG        old_id;

    hres = g_dbgMgr->system->GetCurrentProcessId( &old_id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessId", hres );

    if ( old_id != id )
    {
        hres = g_dbgMgr->system->SetCurrentProcessId( id );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugSystemObjects::SetCurrentProcessId", hres );
    }

    ULONG  systemId;
    hres = g_dbgMgr->system->GetCurrentProcessSystemId( &systemId );

    g_dbgMgr->system->SetCurrentProcessId( old_id );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessSystemId", hres );

    return systemId;
}

///////////////////////////////////////////////////////////////////////////////

PROCESS_DEBUG_ID getProcessIdBySystemId( PROCESS_ID pid )
{
    HRESULT  hres;
    ULONG  id;

    hres = g_dbgMgr->system->GetProcessIdBySystemId( pid, &id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetProcessIdBySystemId", hres );

    return id;
}

///////////////////////////////////////////////////////////////////////////////

PROCESS_DEBUG_ID getProcessIdByOffset( MEMOFFSET_64 offset )
{
    HRESULT  hres;
    ULONG  id;

    hres = g_dbgMgr->system->GetProcessIdByDataOffset( offset, &id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetProcessIdBySystemId", hres );

    return id;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getProcessOffset( PROCESS_DEBUG_ID id )
{
    HRESULT      hres;
    ULONG        old_id;

    hres = g_dbgMgr->system->GetCurrentProcessId( &old_id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessId", hres );

    if ( old_id != id )
    {
        hres = g_dbgMgr->system->SetCurrentProcessId( id );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugSystemObjects::SetCurrentProcessId", hres );
    }

    MEMOFFSET_64  offset;
    hres = g_dbgMgr->system->GetCurrentProcessDataOffset( &offset );

    g_dbgMgr->system->SetCurrentProcessId( old_id );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessSystemId", hres );

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

size_t getNumberProcesses()
{
    HRESULT  hres;
    ULONG  number;

    hres = g_dbgMgr->system->GetNumberProcesses( &number );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetNumberProcesses", hres );

    return number;
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

THREAD_DEBUG_ID getThreadIdBySystemId( THREAD_ID tid )
{
    HRESULT  hres;
    ULONG  id;

    hres = g_dbgMgr->system->GetThreadIdBySystemId( tid, &id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetThreadIdBySystemId", hres );

    return id;
}

///////////////////////////////////////////////////////////////////////////////

THREAD_DEBUG_ID getThreadIdByOffset( MEMOFFSET_64 offset )
{
    HRESULT  hres;
    ULONG  id;

    hres = g_dbgMgr->system->GetThreadIdByDataOffset( offset, &id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetThreadIdBySystemId", hres );

    return id;
}

///////////////////////////////////////////////////////////////////////////////

THREAD_ID getThreadSystemId( THREAD_DEBUG_ID id )
{
    HRESULT      hres;
    ULONG        old_id;

    hres = g_dbgMgr->system->GetCurrentThreadId( &old_id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetThreadProcessId", hres );

    if ( old_id != id )
    {
        hres = g_dbgMgr->system->SetCurrentThreadId( id );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres );
    }

    ULONG  systemId;
    hres = g_dbgMgr->system->GetCurrentThreadSystemId( &systemId );

    g_dbgMgr->system->SetCurrentThreadId( old_id );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentThreadSystemId", hres );

    return systemId;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getThreadOffset( THREAD_DEBUG_ID id )
{
    HRESULT      hres;
    ULONG        old_id;

    hres = g_dbgMgr->system->GetCurrentThreadId( &old_id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentThreadId", hres );

    if ( old_id != id )
    {
        hres = g_dbgMgr->system->SetCurrentThreadId( id );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres );
    }

    MEMOFFSET_64  offset;
    hres = g_dbgMgr->system->GetCurrentThreadDataOffset( &offset );

    g_dbgMgr->system->SetCurrentThreadId( old_id );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentThreadSystemId", hres );

    return offset;
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

class CurrentThreadGuard {

public:

    CurrentThreadGuard() {

        HRESULT      hres;

        hres = g_dbgMgr->system->GetCurrentThreadId( &m_previousId );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugSystemObjects::GetCurrentThreadId", hres ); 
    }

    ~CurrentThreadGuard() {
        g_dbgMgr->system->SetCurrentThreadId( m_previousId );
    }

private:


    THREAD_ID   m_previousId;

};

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getInstructionOffset( THREAD_ID id )
{
    HRESULT                 hres;
    CurrentThreadGuard      previousThread;

    if ( id != -1 )
    {
        hres = g_dbgMgr->system->SetCurrentThreadId( id );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres ); 
    }

    MEMOFFSET_64 offset;
    hres =  g_dbgMgr->registers->GetInstructionOffset( &offset );

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugRegisters::GetInstructionOffset", hres ); 

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getStackOffset( THREAD_ID id )
{
    HRESULT                 hres;
    CurrentThreadGuard      previousThread;

    if ( id != -1 )
    {
        hres = g_dbgMgr->system->SetCurrentThreadId( id );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres ); 
    }

    MEMOFFSET_64 offset;
    hres =  g_dbgMgr->registers->GetStackOffset( &offset );

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugRegisters::GetStackOffset", hres ); 

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getFrameOffset( THREAD_ID id )
{
    HRESULT                 hres;
    CurrentThreadGuard      previousThread;

    if ( id != -1 )
    {
        hres = g_dbgMgr->system->SetCurrentThreadId( id );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres ); 
    }

    MEMOFFSET_64 offset;
    hres =  g_dbgMgr->registers->GetFrameOffset( &offset );

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugRegisters::GetFrameOffset", hres ); 

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

size_t getRegisterNumber( THREAD_ID id )
{
    HRESULT  hres;
    CurrentThreadGuard  previousThread;

    hres = g_dbgMgr->system->SetCurrentThreadId( id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres ); 

    ULONG number;
    hres = g_dbgMgr->registers->GetNumberRegisters( &number );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugRegisters::GetNumberRegisters", hres ); 

    return size_t(number);
}

///////////////////////////////////////////////////////////////////////////////

size_t getRegsiterIndex( THREAD_ID id, const std::wstring &name )
{
    HRESULT  hres;
    ULONG  index;
    CurrentThreadGuard  previousThread;

    hres = g_dbgMgr->system->SetCurrentThreadId( id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres ); 

    hres = g_dbgMgr->registers->GetIndexByNameWide( name.c_str(), &index );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugRegisters2::GetIndexByNameWide", hres ); 

    return index;
}

///////////////////////////////////////////////////////////////////////////////

CPURegType getRegisterType( THREAD_ID id, size_t index )
{
    HRESULT  hres;
    CurrentThreadGuard  previousThread;

    hres = g_dbgMgr->system->SetCurrentThreadId( id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres );

    if ( index >= getRegisterNumber( id ) )
        throw IndexException(index);

    DEBUG_VALUE  dbgvalue = {};
    hres = g_dbgMgr->registers->GetValue( index, &dbgvalue );

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugRegisters::GetValue", hres ); 

    switch ( dbgvalue.Type )
    {
    case DEBUG_VALUE_INT8: return RegInt8;
    case DEBUG_VALUE_INT16: return RegInt16;
    case DEBUG_VALUE_INT32: return RegInt32;
    case DEBUG_VALUE_INT64: return RegInt64;
    case DEBUG_VALUE_FLOAT32: return RegFloat32;
    case DEBUG_VALUE_FLOAT64: return RegFloat64;
    case DEBUG_VALUE_FLOAT80: return RegFloat80;
    case DEBUG_VALUE_FLOAT128: return RegFloat128;
    case DEBUG_VALUE_VECTOR64: return RegVector64;
    case DEBUG_VALUE_VECTOR128: return RegVector128;
    }

    throw DbgException( L"Unknown regsiter type" ); 
}

///////////////////////////////////////////////////////////////////////////////

std::wstring getRegisterName( THREAD_ID id, size_t index )
{
    HRESULT      hres;
    CurrentThreadGuard  previousThread;

    hres = g_dbgMgr->system->SetCurrentThreadId( id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres );
    
    wchar_t  regName[0x100];

    hres = g_dbgMgr->registers->GetDescriptionWide( index, regName, 0x100, NULL, NULL );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugRegisters2::GetDescriptionWide", hres ); 

    return std::wstring( regName );
}

///////////////////////////////////////////////////////////////////////////////

void getRegisterValue( THREAD_ID id, size_t index, void* buffer, size_t bufferSize )
{
    HRESULT  hres;
    CurrentThreadGuard  previousThread;

    hres = g_dbgMgr->system->SetCurrentThreadId( id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres );

    if ( index >= getRegisterNumber( id ) )
        throw IndexException(index);

    DEBUG_VALUE  dbgvalue = {};
    hres = g_dbgMgr->registers->GetValue( index, &dbgvalue );

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugRegisters::GetValue", hres ); 

    switch ( dbgvalue.Type )
    {
    case DEBUG_VALUE_INT8: 
        if ( bufferSize < sizeof(char) )
            throw DbgException( L"Insufficient buffer size" ); 
        *(char*)buffer = dbgvalue.I8;
        return;

    case DEBUG_VALUE_INT16: 
        if ( bufferSize < sizeof(short) )
            throw DbgException( L"Insufficient buffer size" ); 
        *(short*)buffer = dbgvalue.I16;
        return;

    case DEBUG_VALUE_INT32: 
        if ( bufferSize < sizeof(long) )
            throw DbgException( L"Insufficient buffer size" ); 
        *(long*)buffer = dbgvalue.I32;
        return;

    case DEBUG_VALUE_INT64: 
        if ( bufferSize < sizeof(long long) )
            throw DbgException( L"Insufficient buffer size" ); 
        *(long long*)buffer = dbgvalue.I64;
        return;

    case DEBUG_VALUE_FLOAT32: 
        if ( bufferSize < sizeof(float) )
            throw DbgException( L"Insufficient buffer size" ); 
        *(float*)buffer = dbgvalue.F32;
        return;

    case DEBUG_VALUE_FLOAT64: 
        if ( bufferSize < sizeof(double) )
            throw DbgException( L"Insufficient buffer size" ); 
        *(double*)buffer = dbgvalue.F64;
        return;

    case DEBUG_VALUE_FLOAT80:
        if ( bufferSize < sizeof(dbgvalue.F80Bytes) )
            throw DbgException( L"Insufficient buffer size" ); 
        memcpy_s( buffer, bufferSize, dbgvalue.F80Bytes, sizeof(dbgvalue.F80Bytes) );
        return;

    case DEBUG_VALUE_FLOAT128:
        if ( bufferSize < sizeof(dbgvalue.F128Bytes) )
            throw DbgException( L"Insufficient buffer size" ); 
        memcpy_s( buffer, bufferSize, dbgvalue.F128Bytes, sizeof(dbgvalue.F128Bytes) );
        return;

    case DEBUG_VALUE_VECTOR64:
        if ( bufferSize < sizeof(dbgvalue.VI64) )
            throw DbgException( L"Insufficient buffer size" ); 
        memcpy_s( buffer, bufferSize, dbgvalue.VI64, sizeof(dbgvalue.VI64) );
        return;

    case DEBUG_VALUE_VECTOR128:
        if ( bufferSize < 2*sizeof(dbgvalue.VI64) )
            throw DbgException( L"Insufficient buffer size" ); 
        memcpy_s( buffer, bufferSize, dbgvalue.VI64, 2*sizeof(dbgvalue.VI64) );
        return;
     }

    throw DbgException( L"Unknown regsiter type" ); 
}

///////////////////////////////////////////////////////////////////////////////

unsigned long long loadMSR( THREAD_ID id, size_t msrIndex )
{
    HRESULT  hres;
    CurrentThreadGuard  previousThread;

    hres = g_dbgMgr->system->SetCurrentThreadId( id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres );

    ULONG64     value;

    hres = g_dbgMgr->dataspace->ReadMsr( msrIndex, &value );
    if ( FAILED( hres ) )
         throw DbgEngException( L"IDebugDataSpaces::ReadMsr", hres );

    return value;
}

///////////////////////////////////////////////////////////////////////////////

void setMSR( THREAD_ID id, size_t msrIndex, unsigned long long value )
{
    HRESULT  hres;
    CurrentThreadGuard  previousThread;

    hres = g_dbgMgr->system->SetCurrentThreadId( id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres );

    hres = g_dbgMgr->dataspace->WriteMsr(msrIndex, value);
    if ( FAILED( hres ) )
         throw DbgEngException( L"IDebugDataSpaces::WriteMsr", hres );
}

///////////////////////////////////////////////////////////////////////////////

CPUType getCPUType( THREAD_ID id )
{
    HRESULT  hres;
    CurrentThreadGuard  previousThread;

    hres = g_dbgMgr->system->SetCurrentThreadId( id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres );

    ULONG       processorType;

    hres = g_dbgMgr->control->GetActualProcessorType( &processorType );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetActualProcessorType", hres );

    switch( processorType )
    {
    case IMAGE_FILE_MACHINE_I386:
        return CPU_I386;

    case IMAGE_FILE_MACHINE_AMD64:
        return CPU_AMD64;
    }

    throw DbgException( L"Unknown processor type" );
}

///////////////////////////////////////////////////////////////////////////////

CPUType getCPUMode( THREAD_ID id )
{
    HRESULT  hres;
    CurrentThreadGuard  previousThread;

    hres = g_dbgMgr->system->SetCurrentThreadId( id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres );

    ULONG       processorType;

    hres = g_dbgMgr->control->GetEffectiveProcessorType( &processorType );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetActualProcessorType", hres );

    switch( processorType )
    {
    case IMAGE_FILE_MACHINE_I386:
        return CPU_I386;

    case IMAGE_FILE_MACHINE_AMD64:
        return CPU_AMD64;
    }

    throw DbgException( L"Unknown processor type" );
}

///////////////////////////////////////////////////////////////////////////////

void disasmAssemblay( MEMOFFSET_64 offset, const std::wstring &instruction, MEMOFFSET_64 &nextOffset )
{
    HRESULT     hres;

    hres = g_dbgMgr->control->AssembleWide( offset, instruction.c_str(), &nextOffset );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::Assemble", hres );
}

///////////////////////////////////////////////////////////////////////////////

void disasmDisassembly( MEMOFFSET_64 offset, std::wstring &instruction, MEMOFFSET_64 &nextOffset, MEMOFFSET_64 &ea )
{
    HRESULT     hres;
    wchar_t     buffer[0x100];
    ULONG       disasmSize = 0;
    ULONG64     endOffset = 0;
    
    hres = 
        g_dbgMgr->control->DisassembleWide(
            offset,
            DEBUG_DISASM_EFFECTIVE_ADDRESS,
            buffer,
            sizeof(buffer)/sizeof(wchar_t),
            &disasmSize,
            &nextOffset );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::Disassemble", hres );

    hres = g_dbgMgr->control->GetDisassembleEffectiveOffset(&ea);
    if ( FAILED( hres ) )
        ea = 0;

    instruction = std::wstring( buffer, disasmSize - 2);
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getNearInstruction( MEMOFFSET_64 offset, LONG delta )
{
    HRESULT  hres;
    ULONG64  nearOffset;

    hres = g_dbgMgr->control->GetNearInstruction( offset, delta, &nearOffset );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetNearInstruction", hres );

    return nearOffset;
}

///////////////////////////////////////////////////////////////////////////////

EXTENSION_ID loadExtension(const std::wstring &extPath )
{
    HRESULT  hres;
    ULONG64  handle = 0;

    hres = g_dbgMgr->control->AddExtensionWide( extPath.c_str(), 0, &handle );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::AddExtension", hres );

    return handle;
}

///////////////////////////////////////////////////////////////////////////////

void removeExtension( EXTENSION_ID extHandle )
{
    g_dbgMgr->control->RemoveExtension( extHandle );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring callExtension( EXTENSION_ID extHandle, const std::wstring command, const std::wstring  &params  )
{
    HRESULT  hres;
    OutputReader    outReader( g_dbgMgr->client );

    hres = g_dbgMgr->control->CallExtensionWide( extHandle, command.c_str(), params.c_str() );

    if ( FAILED( hres ) )
        throw  DbgEngException( L"IDebugControl::CallExtension", hres ); 
        
    return outReader.Line();
}

///////////////////////////////////////////////////////////////////////////////


} // kdlib namespace end 





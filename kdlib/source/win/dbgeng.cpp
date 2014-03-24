#include "stdafx.h"

#include <vector>

#include <comutil.h>

#include <boost/smart_ptr/scoped_array.hpp>

#include "kdlib/dbgengine.h"
#include "win/exceptions.h"
#include "win/dbgmgr.h"

#include "moduleimp.h"

namespace  kdlib {

///////////////////////////////////////////////////////////////////////////////

class SwitchThreadContext {

public:

    SwitchThreadContext( THREAD_DEBUG_ID id ) {

        HRESULT      hres;

        if ( id != -1 || id == GetCurrentThreadId() )
        {
            m_previousId = -1;
            return;
        }

        hres = g_dbgMgr->system->GetCurrentThreadId( &m_previousId );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugSystemObjects::GetCurrentThreadId", hres ); 

        ULONG  registerNumber = 0;
        hres = g_dbgMgr->registers->GetNumberRegisters(&registerNumber);
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugRegister::GetNumberRegisters", hres ); 

        m_regValues.resize(registerNumber);

        hres = g_dbgMgr->registers->GetValues2(DEBUG_REGSRC_EXPLICIT, registerNumber, NULL, 0, &m_regValues[0] );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugRegister::GetNumberRegisters", hres ); 

        hres = g_dbgMgr->system->SetCurrentThreadId( id );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres ); 

    }

    ~SwitchThreadContext() {

        HRESULT      hres;

        if ( m_previousId == -1 )
            return;

        hres = g_dbgMgr->system->SetCurrentThreadId( m_previousId );

        if ( SUCCEEDED(hres) )
        {
            g_dbgMgr->registers->SetValues2(DEBUG_REGSRC_EXPLICIT, static_cast<ULONG>(m_regValues.size()), NULL, 0, &m_regValues[0] );
        }
    }

private:

    THREAD_DEBUG_ID  m_previousId;

    std::vector<DEBUG_VALUE>  m_regValues;
};


///////////////////////////////////////////////////////////////////////////////

bool initialize()
{
    g_dbgMgr.set( new DebugManager() );
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void uninitialize()
{
    ModuleImp::clearModuleCache();

    g_dbgMgr.reset();
}

///////////////////////////////////////////////////////////////////////////////

std::string DbgWideException::getCStrDesc( const std::wstring &desc )
{
    return std::string( _bstr_t( desc.c_str() ) );
}

///////////////////////////////////////////////////////////////////////////////

static void setInitialBreakOption()
{
    ULONG   opt;
    HRESULT hres = g_dbgMgr->control->GetEngineOptions( &opt );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetEngineOptions", hres  );

    opt |= DEBUG_ENGOPT_INITIAL_BREAK;
    hres = g_dbgMgr->control->SetEngineOptions( opt );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::SetEngineOptions", hres );
}

///////////////////////////////////////////////////////////////////////////////

PROCESS_DEBUG_ID startProcess( const std::wstring  &processName,  bool debugChildren )
{
    HRESULT     hres;

    setInitialBreakOption();

    std::vector< std::wstring::value_type >      cmdLine( processName.size() + 1 );
    wcscpy_s( &cmdLine[0], cmdLine.size(), processName.c_str() );

    hres = g_dbgMgr->client->CreateProcessWide(
        0,
        &cmdLine[0],
        ( debugChildren ? DEBUG_PROCESS : DEBUG_ONLY_THIS_PROCESS ) | DETACHED_PROCESS 
        );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient4::CreateProcessWide", hres );

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

void terminateProcess( PROCESS_DEBUG_ID processId )
{
    HRESULT     hres;

    if ( processId != -1 )
    {
        hres = g_dbgMgr->system->SetCurrentProcessId(processId);
        if ( FAILED(hres) )
            throw DbgEngException( L"IDebugSystemObjects::SetCurrentProcessId", hres );
    }

    hres = g_dbgMgr->client->TerminateCurrentProcess();
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient::TerminateCurrentProcess", hres );

    ModuleImp::onProcessExit();

    hres = g_dbgMgr->client->DetachCurrentProcess();
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient::DetachCurrentProcess", hres );

}

///////////////////////////////////////////////////////////////////////////////

PROCESS_DEBUG_ID attachProcess( PROCESS_ID pid )
{
    HRESULT     hres;

    setInitialBreakOption();

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
            throw DbgEngException( L"IDebugSystemObjects::SetCurrentProcessId", hres );
    }

    ModuleImp::onProcessExit();

    hres = g_dbgMgr->client->DetachCurrentProcess();
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient::DetachCurrentProcess", hres );

}

///////////////////////////////////////////////////////////////////////////////

void detachAllProcesses()
{
    HRESULT     hres;

    hres = g_dbgMgr->client->DetachProcesses();

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugClient::DetachProcesses", hres );

    ModuleImp::clearModuleCache();
}

///////////////////////////////////////////////////////////////////////////////

void terminateAllProcesses()
{
    HRESULT  hres;

    hres = g_dbgMgr->client->TerminateProcesses();
    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugClient::TerminateProcesses", hres );

    hres = g_dbgMgr->client->DetachProcesses();
    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugClient::DetachProcesses", hres );

    ModuleImp::clearModuleCache();
}

///////////////////////////////////////////////////////////////////////////////

void loadDump( const std::wstring &fileName )
{
    HRESULT     hres;

    hres = g_dbgMgr->client->OpenDumpFileWide( fileName.c_str(), NULL );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient4::OpenDumpFileWide", hres );

    hres = g_dbgMgr->control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::WaitForEvent", hres );
}

///////////////////////////////////////////////////////////////////////////////

void writeDump( const std::wstring &fileName, bool smallDump )
{
    HRESULT     hres;

    ULONG       debugClass, debugQualifier;

    hres = g_dbgMgr->control->GetDebuggeeType( &debugClass, &debugQualifier );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetDebuggeeType", hres );

    hres = g_dbgMgr->client->WriteDumpFileWide(
        fileName.c_str(), 
        NULL,
        smallDump ? DEBUG_DUMP_SMALL : ( debugClass == DEBUG_CLASS_KERNEL ? DEBUG_DUMP_FULL : DEBUG_DUMP_DEFAULT ),
        DEBUG_FORMAT_DEFAULT,
        NULL );

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugClient4::WriteDumpFileWide", hres );
}

///////////////////////////////////////////////////////////////////////////////////

bool isLocalKernelDebuggerEnabled()
{
    HRESULT hres = g_dbgMgr->client->IsKernelDebuggerEnabled();
    if ( ( hres != S_OK ) && ( hres != S_FALSE ) )
        throw DbgEngException( L"IDebugClient::IsKernelDebuggerEnabled", hres );
    return hres == S_OK;
}

///////////////////////////////////////////////////////////////////////////////////

void attachKernel( const std::wstring &connectOptions )
{
    setInitialBreakOption();

    HRESULT hres = 
        g_dbgMgr->client->AttachKernelWide(
            connectOptions.empty() ? DEBUG_ATTACH_LOCAL_KERNEL : DEBUG_ATTACH_KERNEL_CONNECTION,
            connectOptions.empty() ? NULL : connectOptions.c_str());
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient::AttachKernel", hres );

    hres = g_dbgMgr->control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::WaitForEvent", hres );
}

///////////////////////////////////////////////////////////////////////////////////

bool isDumpAnalyzing()
{
    HRESULT         hres;
    ULONG           debugClass, debugQualifier;

    hres = g_dbgMgr->control->GetDebuggeeType( &debugClass, &debugQualifier );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetDebuggeeType", hres );

    return debugQualifier >= DEBUG_DUMP_SMALL;
}

///////////////////////////////////////////////////////////////////////////////////

bool isKernelDebugging()
{
    HRESULT     hres;
    ULONG       debugClass, debugQualifier;

    hres = g_dbgMgr->control->GetDebuggeeType( &debugClass, &debugQualifier );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetDebuggeeType", hres );

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

    ULONG  executionStatus;

    hres = g_dbgMgr->control->GetExecutionStatus( &executionStatus );
    if ( FAILED( hres ) )
        throw  DbgEngException( L"IDebugControl::GetExecutionStatus", hres ); 

    if ( executionStatus != DEBUG_STATUS_BREAK && executionStatus != DEBUG_STATUS_NO_DEBUGGEE )
       g_dbgMgr->control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);

    return std::wstring( outReader.Line() ); 
}

///////////////////////////////////////////////////////////////////////////////////

NumVariant evaluate( const std::wstring  &expression, bool cplusplus )
{
    HRESULT             hres;
    ULONG64             value = 0;

    DEBUG_VALUE  debugValue = {};
    ULONG        remainderIndex = 0;
    ULONG        expresionSyntax;
    
    hres = g_dbgMgr->control->GetExpressionSyntax( &expresionSyntax );
    if ( FAILED(hres) )
    {
        throw  DbgEngException( L"IDebugControl3::GetExpressionSyntax", hres );
    }

    hres = g_dbgMgr->control->SetExpressionSyntax( cplusplus ? DEBUG_EXPR_CPLUSPLUS : DEBUG_EXPR_MASM );
    if ( FAILED(hres) )
    {
        throw  DbgEngException( L"IDebugControl3::GetExpressionSyntax", hres );
    }

    hres = g_dbgMgr->control->EvaluateWide( 
        expression.c_str(), 
        DEBUG_VALUE_INVALID,
        &debugValue,
        &remainderIndex );

    if ( FAILED( hres ) )
    {
        g_dbgMgr->control->SetExpressionSyntax( expresionSyntax );
        throw  DbgEngException( L"IDebugControl::Evaluate", hres );
    }

    NumVariant   var;

    switch( debugValue.Type )
    {
    case DEBUG_VALUE_INT8:
        var =  NumVariant( debugValue.I8 );
        break;
        
    case DEBUG_VALUE_INT16:
        var =  NumVariant( debugValue.I16 );
        break;
        
    case DEBUG_VALUE_INT32:
        var = NumVariant( debugValue.I32 );
        break;
        
    case DEBUG_VALUE_INT64:
        var =  NumVariant( debugValue.I64 );
        break;

    case DEBUG_VALUE_FLOAT32: 
        var =  NumVariant( debugValue.F32 );
        break;

    case DEBUG_VALUE_FLOAT64: 
        var =  NumVariant( debugValue.F64 );
        break;

    default:
        g_dbgMgr->control->SetExpressionSyntax( expresionSyntax );
        throw DbgException("unsupported type");
    } 

    g_dbgMgr->control->SetExpressionSyntax( expresionSyntax );

    return var;
}

///////////////////////////////////////////////////////////////////////////////////

ExecutionStatus targetExecutionStatus()
{
    HRESULT     hres;

    ULONG    currentStatus;

    hres = g_dbgMgr->control->GetExecutionStatus( &currentStatus );

    if ( FAILED( hres ) )
        throw  DbgEngException( L"IDebugControl::GetExecutionStatus", hres ); 

    return ConvertDbgEngineExecutionStatus( currentStatus );
}

///////////////////////////////////////////////////////////////////////////////////

ExecutionStatus targetChangeStatus( ULONG status )
{
    HRESULT     hres;

    hres = g_dbgMgr->control->SetExecutionStatus( status );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::SetExecutionStatus", hres );

    ULONG    currentStatus;

    do {
        hres = g_dbgMgr->control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
        if ( FAILED( hres ) )
        {
            hres = g_dbgMgr->control->GetExecutionStatus( &currentStatus );
            if ( FAILED( hres ) )
                throw  DbgEngException( L"IDebugControl::GetExecutionStatus", hres ); 

            if ( currentStatus == DEBUG_STATUS_NO_DEBUGGEE )
                return DebugStatusNoDebuggee;
            
            throw DbgEngException( L"IDebugControl::WaitForEvent", hres );
        }

        hres = g_dbgMgr->control->GetExecutionStatus( &currentStatus );

        if ( FAILED( hres ) )
            throw  DbgEngException( L"IDebugControl::GetExecutionStatus", hres ); 

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

    hres = g_dbgMgr->control->GetSystemVersionStringWide( DEBUG_SYSVERSTR_BUILD, &buffer[0], static_cast<ULONG>(buffer.size()), NULL );
    if ( FAILED( hres ) )
         throw DbgEngException( L"IDebugControl::GetSystemVersion", hres );

    systemInfo.buildDescription = std::wstring( &buffer[0] );

}

///////////////////////////////////////////////////////////////////////////////

void getSystemCrashInfo( SystemCrashInfo &crashInfo )
{
    HRESULT  hres;

    crashInfo = SystemCrashInfo();

    hres = g_dbgMgr->control->ReadBugCheckData(
            (PULONG)&crashInfo.code,
            (PULONG64)&crashInfo.parameters[0],
            (PULONG64)&crashInfo.parameters[1],
            (PULONG64)&crashInfo.parameters[2],
            (PULONG64)&crashInfo.parameters[3]);

    if ( FAILED( hres ) )
        throw DbgEngException(L"IDebugControl::ReadBugCheckData", hres);

    crashInfo.paramterCount = 4;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long getNumberProcesses()
{
    HRESULT  hres;
    ULONG  number;

    hres = g_dbgMgr->system->GetNumberProcesses( &number );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetNumberProcesses", hres );

    return number;
}

///////////////////////////////////////////////////////////////////////////////

PROCESS_DEBUG_ID getCurrentProcessId()
{
    HRESULT      hres;
    ULONG        id;

    hres = g_dbgMgr->system->GetCurrentProcessId( &id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessId", hres ); 

    return  PROCESS_DEBUG_ID(id);
}

///////////////////////////////////////////////////////////////////////////////

PROCESS_DEBUG_ID getProcessIdBySystemId( PROCESS_ID pid )
{
    HRESULT  hres;
    ULONG  id;

    if ( pid == -1 )
    {
       return getCurrentProcessId();
    }

    hres = g_dbgMgr->system->GetProcessIdBySystemId( pid, &id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetProcessIdBySystemId", hres );

    return id;
}

///////////////////////////////////////////////////////////////////////////////

PROCESS_DEBUG_ID getProcessIdByIndex(unsigned long index)
{
    HRESULT  hres;

    if ( index >= getNumberProcesses() )
        throw IndexException( index );

    ULONG  processId = -1;

    hres = g_dbgMgr->system->GetProcessIdsByIndex( index, 1, &processId, NULL );
    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugSystemObjects::GetProcessIdsByIndex failed", hres );

    return THREAD_DEBUG_ID(processId);
}

///////////////////////////////////////////////////////////////////////////////

PROCESS_DEBUG_ID getProcessIdByOffset( MEMOFFSET_64 offset )
{
    HRESULT  hres;
    ULONG  id;

    hres = g_dbgMgr->system->GetProcessIdByDataOffset( offset, &id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetProcessIdByDataOffset", hres );

    return id;
}

///////////////////////////////////////////////////////////////////////////////

PROCESS_ID getProcessSystemId( PROCESS_DEBUG_ID id )
{
    HRESULT  hres;
    ULONG  systemId;

    if ( id == -1 )
    {
        hres = g_dbgMgr->system->GetCurrentProcessSystemId( &systemId );

        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessSystemId", hres );

        return systemId;
    }

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

    hres = g_dbgMgr->system->GetCurrentProcessSystemId( &systemId );

    g_dbgMgr->system->SetCurrentProcessId( old_id );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessSystemId", hres );

    return systemId;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getProcessOffset( PROCESS_DEBUG_ID id )
{
    HRESULT  hres;
    MEMOFFSET_64  offset;

    if ( id == -1 )
    {
        hres = g_dbgMgr->system->GetCurrentProcessDataOffset( &offset );

        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessSystemId", hres );

        return offset;
    }

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

    hres = g_dbgMgr->system->GetCurrentProcessDataOffset( &offset );

    g_dbgMgr->system->SetCurrentProcessId( old_id );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessSystemId", hres );

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

void setCurrentProcessById(PROCESS_DEBUG_ID id)
{
    HRESULT  hres;

    if ( isKernelDebugging() )
        throw DbgException("setCurrentProcessById - only for user mode");

    hres = g_dbgMgr->system->SetCurrentProcessId(id);
    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugSystemObjects::SetCurrentProcessId", hres );
}

///////////////////////////////////////////////////////////////////////////////

void setCurrentProcessByOffset( MEMOFFSET_64 offset )
{
    if ( isKernelDebugging() )
        throw DbgException("setCurrentProcessByOffset - only for user mode");

    setCurrentProcessById(  getProcessIdByOffset(offset) );
}

///////////////////////////////////////////////////////////////////////////////

void setImplicitProcess(MEMOFFSET_64 offset)
{
    HRESULT  hres;

    hres = g_dbgMgr->system->SetImplicitProcessDataOffset(offset);
    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugSystemObjects::SetImplicitProcessDataOffset", hres );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getCurrentProcess()
{
    if ( isKernelDebugging() )
    {
        return getImplicitProcessOffset();
    }
    else
    {
        return getProcessOffset();
    }
}


///////////////////////////////////////////////////////////////////////////////

void setCurrentProcess( MEMOFFSET_64  offset )
{
    if ( isKernelDebugging() )
    {
        return setImplicitProcess(offset);
    }
    else
    {
        return setCurrentProcessByOffset(offset);
    }
}

///////////////////////////////////////////////////////////////////////////////

std::wstring getCurrentProcessExecutableName()
{
    const ULONG bufChars = (MAX_PATH * 2);

    boost::scoped_array< WCHAR > exeName( new WCHAR[bufChars] );
    memset(&exeName[0], 0, bufChars * sizeof(WCHAR));

    ULONG tmp;
    HRESULT hres = g_dbgMgr->system->GetCurrentProcessExecutableNameWide(&exeName[0], bufChars, &tmp);
    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessExecutableNameWide", hres );

    return std::wstring( &exeName[0] );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getImplicitProcessOffset()
{
    HRESULT  hres;
    MEMOFFSET_64  offset;

    hres = g_dbgMgr->system->GetImplicitProcessDataOffset(&offset);
    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugSystemObjects::GetImplicitProcessDataOffset", hres );

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long getNumberThreads()
{
    HRESULT     hres;
    ULONG       number;

    hres = g_dbgMgr->system->GetNumberThreads( &number );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetNumberThreads", hres ); 
        
    return number;
}

///////////////////////////////////////////////////////////////////////////////

THREAD_DEBUG_ID getCurrentThreadId()
{
    HRESULT      hres;
    ULONG        id;

    hres = g_dbgMgr->system->GetCurrentThreadId( &id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentThreadId", hres ); 
        
     return THREAD_DEBUG_ID(id);
}

///////////////////////////////////////////////////////////////////////////////

THREAD_DEBUG_ID getThreadIdByOffset( MEMOFFSET_64 offset )
{
    HRESULT  hres;
    ULONG  id;

    hres = g_dbgMgr->system->GetThreadIdByDataOffset( offset, &id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetThreadIdBySystemId", hres );

    return THREAD_DEBUG_ID(id);
}

///////////////////////////////////////////////////////////////////////////////

THREAD_DEBUG_ID getThreadIdBySystemId( THREAD_ID tid )
{
    HRESULT  hres;
    ULONG  id;

    if ( tid == -1 )
        return getCurrentThreadId();

    hres = g_dbgMgr->system->GetThreadIdBySystemId( tid, &id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetThreadIdBySystemId", hres );

    return THREAD_DEBUG_ID(id);
}


///////////////////////////////////////////////////////////////////////////////

THREAD_DEBUG_ID getThreadIdByIndex(unsigned long index)
{
    HRESULT  hres;

    if ( index >= getNumberThreads() )
        throw IndexException( index );

    ULONG  threadId = -1;

    hres = g_dbgMgr->system->GetThreadIdsByIndex( index, 1, &threadId, NULL );
    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugSystemObjects::GetThreadIdsByIndex failed", hres );

    return THREAD_DEBUG_ID(threadId);
}

///////////////////////////////////////////////////////////////////////////////

THREAD_ID getThreadSystemId( THREAD_DEBUG_ID id )
{
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;
    ULONG  systemId;

    hres = g_dbgMgr->system->GetCurrentThreadSystemId( &systemId );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentThreadSystemId", hres );

    return systemId;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getThreadOffset( THREAD_DEBUG_ID id )
{
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;
    MEMOFFSET_64  offset;

    hres = g_dbgMgr->system->GetCurrentThreadDataOffset( &offset );

    if ( FAILED( hres ) )
       throw DbgEngException( L"IDebugSystemObjects::GetCurrentThreadSystemId", hres );
    
    return offset;
}

///////////////////////////////////////////////////////////////////////////////

void setCurrentThreadById(THREAD_DEBUG_ID id)
{
    HRESULT  hres;

    hres = g_dbgMgr->system->SetCurrentThreadId( id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres );
}

///////////////////////////////////////////////////////////////////////////////

void setCurrentThreadByOffset(MEMOFFSET_64 offset)
{
   setCurrentThreadById( getThreadIdByOffset(offset) );
}

///////////////////////////////////////////////////////////////////////////////

void setImplicitThread(MEMOFFSET_64 offset)
{   
    HRESULT  hres;

    hres = g_dbgMgr->system->SetImplicitThreadDataOffset(offset);
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::SetImplicitThreadDataOffset", hres );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getImplicitThreadOffset()
{
    HRESULT  hres;
    MEMOFFSET_64  offset;

    hres = g_dbgMgr->system->GetImplicitThreadDataOffset(&offset);
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::SetImplicitThreadDataOffset", hres );

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getCurrentThread()
{
    if ( isKernelDebugging() )
    {
        return getImplicitThreadOffset();
    }
    else
    {
        return getThreadOffset();
    }
}

///////////////////////////////////////////////////////////////////////////////

void setCurrentThread(MEMOFFSET_64 offset)
{
    if ( isKernelDebugging() )
    {
        setImplicitThread(offset);
    }
    else
    {
        setCurrentThreadByOffset(offset);
    }
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

BREAKPOINT_ID hardwareBreakPointSet( MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType )
{
    HRESULT  hres;

    IDebugBreakpoint  *bp;
    hres = g_dbgMgr->control->AddBreakpoint(
        DEBUG_BREAKPOINT_DATA,
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

    hres = bp->SetDataParameters(static_cast<ULONG>(size), accessType);
    if (S_OK != hres)
    {
        g_dbgMgr->control->RemoveBreakpoint(bp);
        throw DbgEngException(L"IDebugBreakpoint::SetDataParameters", hres);
    }

    bpFlags |= DEBUG_BREAKPOINT_ENABLED | DEBUG_BREAKPOINT_ADDER_ONLY | DEBUG_BREAKPOINT_GO_ONLY;
    hres = bp->SetFlags(bpFlags);
    if (S_OK != hres)
    {
        g_dbgMgr->control->RemoveBreakpoint(bp);
        throw DbgEngException(L"IDebugBreakpoint::SetFlags", hres);
    }

    ULONG  breakId;
    hres = bp->GetId(&breakId);
    if (S_OK != hres)
    {
        g_dbgMgr->control->RemoveBreakpoint(bp);
        throw DbgEngException(L"IDebugBreakpoint::GetId", hres);
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
    SwitchThreadContext threadContext(id);

    MEMOFFSET_64  offset;
    HRESULT  hres;

    hres =  g_dbgMgr->registers->GetInstructionOffset( &offset );
    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugRegisters::GetInstructionOffset", hres ); 

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getStackOffset( THREAD_ID id )
{
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;
    MEMOFFSET_64 offset;
    hres =  g_dbgMgr->registers->GetStackOffset( &offset );

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugRegisters::GetStackOffset", hres ); 

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getFrameOffset( THREAD_ID id )
{
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;
    MEMOFFSET_64 offset;
    hres =  g_dbgMgr->registers->GetFrameOffset( &offset );

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugRegisters::GetFrameOffset", hres ); 

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long getRegisterNumber( THREAD_ID id )
{
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;
    ULONG  number;

    hres = g_dbgMgr->registers->GetNumberRegisters( &number );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugRegisters::GetNumberRegisters", hres ); 

    return number;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long getRegsiterIndex( THREAD_ID id, const std::wstring &name )
{
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;
    ULONG  index;

    hres = g_dbgMgr->registers->GetIndexByNameWide( name.c_str(), &index );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugRegisters2::GetIndexByNameWide", hres ); 

    return index;
}

///////////////////////////////////////////////////////////////////////////////

CPURegType getRegisterType( THREAD_ID id, unsigned long index )
{
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;

    if ( index >= getRegisterNumber( id ) )
        throw IndexException(index);

    DEBUG_VALUE  dbgvalue = {};
    hres = g_dbgMgr->registers->GetValue( static_cast<ULONG>(index), &dbgvalue );

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

    throw DbgException( "Unknown regsiter type" ); 
}

///////////////////////////////////////////////////////////////////////////////

std::wstring getRegisterName( THREAD_ID id, unsigned long index )
{
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;
    wchar_t  regName[0x100];

    hres = g_dbgMgr->registers->GetDescriptionWide( static_cast<ULONG>(index), regName, 0x100, NULL, NULL );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugRegisters2::GetDescriptionWide", hres ); 

    return std::wstring( regName );
}

///////////////////////////////////////////////////////////////////////////////

void getRegisterValue( THREAD_ID id, unsigned long index, void* buffer, size_t bufferSize )
{
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;

    if ( index >= getRegisterNumber( id ) )
        throw IndexException(index);

    DEBUG_VALUE  dbgvalue = {};
    hres = g_dbgMgr->registers->GetValue(index, &dbgvalue );

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugRegisters::GetValue", hres ); 

    switch ( dbgvalue.Type )
    {
    case DEBUG_VALUE_INT8: 
        if ( bufferSize < sizeof(unsigned char) )
            throw DbgException( "Insufficient buffer size" ); 
        *(unsigned char*)buffer = dbgvalue.I8;
        return;

    case DEBUG_VALUE_INT16: 
        if ( bufferSize < sizeof(unsigned short) )
            throw DbgException( "Insufficient buffer size" ); 
        *(unsigned short*)buffer = dbgvalue.I16;
        return;

    case DEBUG_VALUE_INT32: 
        if ( bufferSize < sizeof(unsigned long) )
            throw DbgException( "Insufficient buffer size" ); 
        *(unsigned long*)buffer = dbgvalue.I32;
        return;

    case DEBUG_VALUE_INT64: 
        if ( bufferSize < sizeof(unsigned long long) )
            throw DbgException( "Insufficient buffer size" ); 
        *(unsigned long long*)buffer = dbgvalue.I64;
        return;

    case DEBUG_VALUE_FLOAT32: 
        if ( bufferSize < sizeof(float) )
            throw DbgException( "Insufficient buffer size" ); 
        *(float*)buffer = dbgvalue.F32;
        return;

    case DEBUG_VALUE_FLOAT64: 
        if ( bufferSize < sizeof(double) )
            throw DbgException( "Insufficient buffer size" ); 
        *(double*)buffer = dbgvalue.F64;
        return;

    case DEBUG_VALUE_FLOAT80:
        if ( bufferSize < sizeof(dbgvalue.F80Bytes) )
            throw DbgException( "Insufficient buffer size" ); 
        memcpy_s( buffer, bufferSize, dbgvalue.F80Bytes, sizeof(dbgvalue.F80Bytes) );
        return;

    case DEBUG_VALUE_FLOAT128:
        if ( bufferSize < sizeof(dbgvalue.F128Bytes) )
            throw DbgException( "Insufficient buffer size" ); 
        memcpy_s( buffer, bufferSize, dbgvalue.F128Bytes, sizeof(dbgvalue.F128Bytes) );
        return;

    case DEBUG_VALUE_VECTOR64:
        if ( bufferSize < sizeof(dbgvalue.VI64) )
            throw DbgException( "Insufficient buffer size" ); 
        memcpy_s( buffer, bufferSize, dbgvalue.VI64, sizeof(dbgvalue.VI64) );
        return;

    case DEBUG_VALUE_VECTOR128:
        if ( bufferSize < 2*sizeof(dbgvalue.VI64) )
            throw DbgException( "Insufficient buffer size" ); 
        memcpy_s( buffer, bufferSize, dbgvalue.VI64, 2*sizeof(dbgvalue.VI64) );
        return;
     }

    throw DbgException( "Unknown regsiter type" ); 
}

///////////////////////////////////////////////////////////////////////////////

unsigned long long loadMSR( THREAD_ID id, unsigned long msrIndex )
{
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;
    ULONG64     value;

    hres = g_dbgMgr->dataspace->ReadMsr( msrIndex, &value );
    if ( FAILED( hres ) )
         throw DbgEngException( L"IDebugDataSpaces::ReadMsr", hres );

    return value;
}

///////////////////////////////////////////////////////////////////////////////

void setMSR( THREAD_ID id, unsigned long msrIndex, unsigned long long value )
{
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;

    hres = g_dbgMgr->dataspace->WriteMsr(msrIndex, value);
    if ( FAILED( hres ) )
         throw DbgEngException( L"IDebugDataSpaces::WriteMsr", hres );
}

///////////////////////////////////////////////////////////////////////////////

unsigned long getNumberFrames(THREAD_ID id)
{  
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;
    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME> dbgFrames(filledFrames);

    hres = g_dbgMgr->control->GetStackTrace( 0, 0, 0, &dbgFrames[0], filledFrames, &filledFrames);
    if (S_OK != hres)
        throw DbgEngException( L"IDebugControl::GetStackTrace", hres );

    return filledFrames;
}

///////////////////////////////////////////////////////////////////////////////

void getStackFrame( THREAD_ID id, unsigned long frameIndex, MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp )
{
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;
    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME> dbgFrames(filledFrames);

    hres = g_dbgMgr->control->GetStackTrace( 0, 0, 0, &dbgFrames[0], filledFrames, &filledFrames);
    if (S_OK != hres)
        throw DbgEngException( L"IDebugControl::GetStackTrace", hres );

    if ( frameIndex >= filledFrames )
        throw IndexException( frameIndex );

    ip = dbgFrames[frameIndex].InstructionOffset;
    ret = dbgFrames[frameIndex].ReturnOffset;
    fp = dbgFrames[frameIndex].FrameOffset;
    sp = dbgFrames[frameIndex].StackOffset;
}

///////////////////////////////////////////////////////////////////////////////

CPUType getCPUType( THREAD_ID id )
{
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;
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

    throw DbgException( "Unknown processor type" );
}

///////////////////////////////////////////////////////////////////////////////

CPUType getCPUMode( THREAD_ID id )
{
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;
    ULONG  processorType;

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

    throw DbgException( "Unknown processor type" );
}

///////////////////////////////////////////////////////////////////////////////

void setCPUMode( THREAD_ID id, CPUType mode )
{
    SwitchThreadContext  threadContext(id);

    HRESULT  hres;
    ULONG  processorMode;

    switch( mode )
    {
    case CPU_I386:
        processorMode = IMAGE_FILE_MACHINE_I386;
        break;

    case CPU_AMD64:
        processorMode = IMAGE_FILE_MACHINE_AMD64;
        break;

    default:
        DbgException( "Unknown processor type" );
    }

    hres =  g_dbgMgr->control->SetEffectiveProcessorType( processorMode );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::SetEffectiveProcessorType", hres );
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

std::wstring getExtensionSearchPath()
{
    ULONG chars = MAX_PATH;
    for (; ; )
    {
        std::vector< wchar_t > rawPath(chars + 1, L'\0');
        HRESULT hres = 
            g_dbgMgr->advanced->Request(
                DEBUG_REQUEST_GET_EXTENSION_SEARCH_PATH_WIDE,
                NULL,
                0,
                &rawPath[0],
                chars * sizeof(wchar_t),
                NULL);
        if (S_OK == hres)
        {
            std::wstring result = &rawPath[0];
            return result;
        }

        if (S_FALSE == hres)
        {
            // The method was successful. However, the output would not fit in the 
            // output buffer OutBuffer, so truncated output was returned
            chars *= 2; 
            continue;
        }

        throw DbgEngException( L"IDebugAdvanced::Request", hres );
    }
}

///////////////////////////////////////////////////////////////////////////////

EXTENSION_ID loadExtension(const std::wstring &extPath )
{
    HRESULT  hres;
    ULONG64  handle = 0;

    std::vector< wchar_t > rawPath(MAX_PATH + 1, L'\0');
    DWORD ret = 
        ::SearchPath(
            getExtensionSearchPath().c_str(),
            extPath.c_str(),
            L".dll",
            MAX_PATH,
            &rawPath[0],
            NULL);
    if (!ret)
        throw DbgWideException( std::wstring(L"extension not found: ") + extPath );

    struct _scoped_lib
    {
        _scoped_lib(const wchar_t *wsz) : m_hmod(::LoadLibrary(wsz)) {}
        ~_scoped_lib() { if (m_hmod) ::FreeLibrary(m_hmod);}
        HMODULE m_hmod;
    } scoped_lib(&rawPath[0]);
    
    if (!scoped_lib.m_hmod)
    {
        std::wstringstream  sstr;
        sstr << L"failed to load extension with error " << std::dec << GetLastError();
        throw DbgWideException( sstr.str() );
    }

    hres = g_dbgMgr->control->AddExtensionWide( extPath.c_str(), 0, &handle );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::AddExtension", hres );

    // inderect call of dbgeng!ExtensionInfo::Load
    FARPROC dummy = NULL;
    g_dbgMgr->control->GetExtensionFunctionWide(handle, L"dummy", &dummy);

    return handle;
}


///////////////////////////////////////////////////////////////////////////////

EXTENSION_ID getExtension(const std::wstring &extPath )
{
    HRESULT  hres;
    ULONG64  handle = 0;

    hres = g_dbgMgr->control->GetExtensionByPathWide( extPath.c_str(), &handle );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetExtensionByPath", hres );

    return handle;
}

///////////////////////////////////////////////////////////////////////////////

void removeExtension( EXTENSION_ID extHandle )
{
    g_dbgMgr->control->RemoveExtension( extHandle );
}

///////////////////////////////////////////////////////////////////////////////

void removeExtension(const std::wstring &extPath )
{
    HRESULT  hres;
   
    EXTENSION_ID   extid = getExtension(extPath);

    hres = g_dbgMgr->control->RemoveExtension( extid );
    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugControl::RemoveExtension", hres );
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

EventType getLastEventType()
{
    HRESULT  hres;

    ULONG  eventType;
    ULONG  processId;
    ULONG  threadId;
        
    hres = 
        g_dbgMgr->control->GetLastEventInformation(
            &eventType,
            &processId,
            &threadId,
            NULL,
            0,
            NULL,
            NULL,
            0,
            NULL);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::GetLastEventInformation", hres);

    switch (eventType)
    {
    case DEBUG_EVENT_BREAKPOINT:
        return EventTypeBreakpoint;
    case DEBUG_EVENT_EXCEPTION:
        return EventTypeException;
    case DEBUG_EVENT_CREATE_THREAD:
        return EventTypeCreateThread;
    case DEBUG_EVENT_EXIT_THREAD:
        return EventTypeExitThread;
    case DEBUG_EVENT_CREATE_PROCESS:
        return EventTypeCreateProcess;
    case DEBUG_EVENT_EXIT_PROCESS:
        return EventTypeExitProcess;
    case DEBUG_EVENT_LOAD_MODULE:
        return EventTypeLoadModule;
    case DEBUG_EVENT_UNLOAD_MODULE:
        return EventTypeUnloadModule;
    case DEBUG_EVENT_SYSTEM_ERROR:
        return EventTypeSystemError;
    case DEBUG_EVENT_SESSION_STATUS:
        return EventTypeSessionStatus;
    case DEBUG_EVENT_CHANGE_DEBUGGEE_STATE:
        return EventTypeChangeDebuggeeState;
    case DEBUG_EVENT_CHANGE_ENGINE_STATE:
        return EventTypeChangeEngineState;
    case DEBUG_EVENT_CHANGE_SYMBOL_STATE:
        return EventTypeChangeSymbolState;
    }

    throw DbgException( "unknow wvwnrt type");
}

///////////////////////////////////////////////////////////////////////////////

PROCESS_DEBUG_ID getLastEventProcessId()
{
    HRESULT  hres;

    ULONG  eventType;
    ULONG  processId;
    ULONG  threadId;
        
    hres = 
        g_dbgMgr->control->GetLastEventInformation(
            &eventType,
            &processId,
            &threadId,
            NULL,
            0,
            NULL,
            NULL,
            0,
            NULL);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::GetLastEventInformation", hres);

    return PROCESS_DEBUG_ID(processId);
}

///////////////////////////////////////////////////////////////////////////////

THREAD_DEBUG_ID getLastEventThreadId()
{
    HRESULT  hres;

    ULONG  eventType;
    ULONG  processId;
    ULONG  threadId;
        
    hres = 
        g_dbgMgr->control->GetLastEventInformation(
            &eventType,
            &processId,
            &threadId,
            NULL,
            0,
            NULL,
            NULL,
            0,
            NULL);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::GetLastEventInformation", hres);

    return THREAD_DEBUG_ID(threadId);
}

///////////////////////////////////////////////////////////////////////////////

ExceptionInfo  getLastException()
{
    HRESULT hres;

    ULONG  eventType;
    ULONG  processId;
    ULONG  threadId;
    DEBUG_LAST_EVENT_INFO_EXCEPTION  lastException;
    ULONG  retLength = sizeof(lastException);

    if ( getLastEventType() != EventTypeException )
        throw DbgException("Last event is not exception");

    hres = 
        g_dbgMgr->control->GetLastEventInformation(
            &eventType,
            &processId,
            &threadId,
            &lastException,
            sizeof(lastException),
            &retLength,
            NULL,
            0,
            NULL);

    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::GetLastEventInformation", hres);

    ExceptionInfo  excinfo = {};

    excinfo.firstChance = lastException.FirstChance != 0;

    excinfo.exceptionCode = lastException.ExceptionRecord.ExceptionCode;
    excinfo.exceptionFlags = lastException.ExceptionRecord.ExceptionFlags;
    excinfo.exceptionRecord = lastException.ExceptionRecord.ExceptionRecord;
    excinfo.exceptionAddress = lastException.ExceptionRecord.ExceptionAddress;
    excinfo.parameterCount = lastException.ExceptionRecord.NumberParameters;

    for (ULONG i = 0; i < lastException.ExceptionRecord.NumberParameters; ++i)
        excinfo.parameters[i] = lastException.ExceptionRecord.ExceptionInformation[i];

    return excinfo;
}

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end 





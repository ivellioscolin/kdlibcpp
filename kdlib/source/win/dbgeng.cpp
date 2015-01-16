#include "stdafx.h"

#include <vector>

#include <comutil.h>

#include <boost/smart_ptr/scoped_array.hpp>

#include "kdlib/dbgengine.h"
#include "kdlib/stack.h"

#include "win/exceptions.h"
#include "win/dbgmgr.h"

#include "autoswitch.h"
#include "moduleimp.h"
#include "processmon.h"
#include "threadctx.h"

namespace  kdlib {

///////////////////////////////////////////////////////////////////////////////

class ThreadAutoSwitch {

public:

    ThreadAutoSwitch( THREAD_DEBUG_ID id )
    {

        HRESULT      hres;

        if ( id == -1 || id == getCurrentThreadId() )
        {
            m_previousId = -1;
            return;
        }

        g_dbgMgr->setQuietNotiification(true);

        hres = g_dbgMgr->system->GetCurrentThreadId( &m_previousId );
        if ( FAILED( hres ) )
        {
            g_dbgMgr->setQuietNotiification(false);
            throw DbgEngException( L"IDebugSystemObjects::GetCurrentThreadId", hres ); 
        }

        ULONG  registerNumber = 0;
        hres = g_dbgMgr->registers->GetNumberRegisters(&registerNumber);
        if ( FAILED( hres ) )
        {
            g_dbgMgr->setQuietNotiification(false);
            throw DbgEngException( L"IDebugRegister::GetNumberRegisters", hres ); 
        }

        m_regValues.resize(registerNumber);

        m_savedRegCtx = false;

        hres = g_dbgMgr->registers->GetValues2(DEBUG_REGSRC_EXPLICIT, registerNumber, NULL, 0, &m_regValues[0] );
        m_savedRegCtx = SUCCEEDED(hres);


        m_savedLocalContext = false;

        hres = g_dbgMgr->symbols->GetScope(0ULL, NULL, &m_localContext, sizeof(m_localContext) );
        m_savedLocalContext = SUCCEEDED(hres);

        hres = g_dbgMgr->system->SetCurrentThreadId( id );
        if ( FAILED( hres ) )
        {
            g_dbgMgr->setQuietNotiification(false);
            throw DbgEngException( L"IDebugSystemObjects::SetCurrentThreadId", hres ); 
        }
    }

    ~ThreadAutoSwitch() {

        HRESULT      hres;

        if ( m_previousId == -1 )
            return;

        hres = g_dbgMgr->system->SetCurrentThreadId( m_previousId );

        if ( SUCCEEDED(hres) )
        {
            if ( m_savedRegCtx )
                g_dbgMgr->registers->SetValues2(DEBUG_REGSRC_EXPLICIT, static_cast<ULONG>(m_regValues.size()), NULL, 0, &m_regValues[0] );

            if ( m_savedLocalContext )
                g_dbgMgr->symbols->SetScope(0ULL, NULL, &m_localContext, sizeof(m_localContext) );
        }

        g_dbgMgr->setQuietNotiification(false);
    }

private:

    THREAD_DEBUG_ID  m_previousId;

    std::vector<DEBUG_VALUE>  m_regValues;

    bool  m_savedRegCtx;

    CONTEXT_STORAGE  m_localContext;

    bool m_savedLocalContext;
};

///////////////////////////////////////////////////////////////////////////////

ProcessAutoSwitch::ProcessAutoSwitch(PROCESS_DEBUG_ID id)
{
    HRESULT  hres;

    if (id == -1 || id == getCurrentProcessId())
    {
        m_previousId = -1;
        return;
    }

    hres = g_dbgMgr->system->GetCurrentProcessId(&m_previousId);
    if (FAILED(hres))
        throw DbgEngException(L"IDebugSystemObjects::GetCurrentProcessId", hres);

    hres = g_dbgMgr->system->SetCurrentProcessId(id);
    if (FAILED(hres))
        throw DbgEngException(L"IDebugSystemObjects::SetCurrentProcessId", hres);
}

ProcessAutoSwitch::~ProcessAutoSwitch()
{
    if (m_previousId == -1)
        return;
        
    g_dbgMgr->system->SetCurrentProcessId(m_previousId);
}

///////////////////////////////////////////////////////////////////////////////

bool initialize()
{
    if ( g_dbgMgr != 0  )
         throw DbgException("pykd is already initialized");

    g_dbgMgr.set( new DebugManager() );

    ProcessMonitor::init();

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool remote_initialize( const std::wstring& remoteOptions )
{
    if ( g_dbgMgr != 0  )
         throw DbgException("pykd is already initialized");

    g_dbgMgr.set( new DebugManager( remoteOptions ) );

    ProcessMonitor::init();

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool isInintilized()
{
    return g_dbgMgr != 0;
}

///////////////////////////////////////////////////////////////////////////////

void uninitialize()
{
    if ( g_dbgMgr ==  0  )
         throw DbgException("pykd is not initialized");

    ProcessMonitor::deinit();

    g_dbgMgr.reset();
}

///////////////////////////////////////////////////////////////////////////////

ExecutionStatus waitForEvent()
{
    HRESULT   hres;

    if ( !g_dbgMgr->isRemoteInitialized() )
    {
        ULONG    currentStatus;

        do {

            hres = g_dbgMgr->control->GetExecutionStatus( &currentStatus );

            if ( FAILED( hres ) )
                throw  DbgEngException( L"IDebugControl::GetExecutionStatus", hres ); 

            if ( currentStatus == DEBUG_STATUS_BREAK )
                return DebugStatusBreak;

            if ( currentStatus == DEBUG_STATUS_NO_DEBUGGEE )
                return DebugStatusNoDebuggee;

            hres = g_dbgMgr->control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
            if ( FAILED( hres ) )
            {
                HRESULT  hres1 = g_dbgMgr->control->GetExecutionStatus( &currentStatus );
                if ( FAILED( hres1 ) )
                    throw  DbgEngException( L"IDebugControl::GetExecutionStatus", hres1 ); 

                if ( currentStatus == DEBUG_STATUS_NO_DEBUGGEE )
                    return DebugStatusNoDebuggee;
            
                throw DbgEngException( L"IDebugControl::WaitForEvent", hres );
            }

        } while( TRUE );
    }
    else
    {

        do {

            ULONG    currentStatus;

            hres = g_dbgMgr->control->GetExecutionStatus( &currentStatus );
    
            if ( FAILED( hres ) )
                throw  DbgEngException( L"IDebugControl::GetExecutionStatus", hres ); 

            if ( currentStatus == DEBUG_STATUS_GO || currentStatus == DEBUG_STATUS_STEP_OVER || currentStatus == DEBUG_STATUS_STEP_INTO )
            {
                Sleep(100);
                continue;
            }

            if ( currentStatus == DEBUG_STATUS_NO_DEBUGGEE )
                return DebugStatusNoDebuggee;

            if ( currentStatus == DEBUG_STATUS_BREAK )
                return DebugStatusBreak;

            throw DbgException( "Unknown debugger state" );

        } while( TRUE );
    }
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
    if ( !isInintilized() )
        initialize();

    HRESULT     hres;

    setInitialBreakOption();

    std::vector< std::wstring::value_type >      cmdLine( processName.size() + 1 );
    wcscpy_s( &cmdLine[0], cmdLine.size(), processName.c_str() );

    STARTUPINFO  startupInfo = {};
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION  processInfo = {};

    BOOL  createResult = CreateProcessW( 
        NULL,
        &cmdLine[0],
        NULL, 
        NULL,
        FALSE, 
        DETACHED_PROCESS | CREATE_SUSPENDED,
        NULL,
        NULL,
        &startupInfo,
        &processInfo );

    if ( !createResult )
        throw Win32Exception( "Failed to start process");

    hres = g_dbgMgr->client->AttachProcess( 0, processInfo.dwProcessId,  DEBUG_ATTACH_DEFAULT ); //DEBUG_ATTACH_EXISTING );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient::AttachProcess", hres );

    hres = g_dbgMgr->control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::WaitForEvent", hres );

    PROCESS_DEBUG_ID  processId = getProcessIdBySystemId(processInfo.dwProcessId);

    setCurrentProcessById(processId);

    if ( debugChildren )
    {
        hres = g_dbgMgr->client->RemoveProcessOptions( DEBUG_PROCESS_ONLY_THIS_PROCESS );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugControl::RemoveProcessOptions", hres );
    }

    ProcessMonitor::processStart(processId);

    ResumeThread(processInfo.hThread);

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);


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

    ProcessMonitor::processStop(processId, ProcessTerminate, 0);

    hres = g_dbgMgr->client->DetachCurrentProcess();
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient::DetachCurrentProcess", hres );

    if ( ProcessMonitor::getNumberProcesses() == 0 )
        g_dbgMgr->ChangeEngineState( DEBUG_CES_EXECUTION_STATUS, DEBUG_STATUS_NO_DEBUGGEE);

}

///////////////////////////////////////////////////////////////////////////////

PROCESS_DEBUG_ID attachProcess( PROCESS_ID pid )
{
    if ( !isInintilized() )
        initialize();

    HRESULT     hres;

    setInitialBreakOption();

    hres = g_dbgMgr->client->AttachProcess( 0, pid, 0 );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient::AttachProcess", hres );

    hres = g_dbgMgr->control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::WaitForEvent", hres );

    PROCESS_DEBUG_ID  processId = getProcessIdBySystemId(pid);

    setCurrentProcessById(processId);

    ProcessMonitor::processStart(processId);

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

    ProcessMonitor::processStop( processId, ProcessDetach, 0 );

    hres = g_dbgMgr->client->DetachCurrentProcess();
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient::DetachCurrentProcess", hres );

    if ( ProcessMonitor::getNumberProcesses() == 0 )
        g_dbgMgr->ChangeEngineState( DEBUG_CES_EXECUTION_STATUS, DEBUG_STATUS_NO_DEBUGGEE);

}

///////////////////////////////////////////////////////////////////////////////

void detachAllProcesses()
{
    HRESULT     hres;

    hres = g_dbgMgr->client->DetachProcesses();

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugClient::DetachProcesses", hres );

    ProcessMonitor::processAllStop();

    g_dbgMgr->ChangeEngineState( DEBUG_CES_EXECUTION_STATUS, DEBUG_STATUS_NO_DEBUGGEE);
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

    ProcessMonitor::processAllStop();

    g_dbgMgr->ChangeEngineState( DEBUG_CES_EXECUTION_STATUS, DEBUG_STATUS_NO_DEBUGGEE);
}

///////////////////////////////////////////////////////////////////////////////

PROCESS_DEBUG_ID loadDump( const std::wstring &fileName )
{
    if ( !isInintilized() )
        initialize();

    HRESULT     hres;

    hres = g_dbgMgr->client->OpenDumpFileWide( fileName.c_str(), NULL );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugClient4::OpenDumpFileWide", hres );

    hres = g_dbgMgr->control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::WaitForEvent", hres );

    ULONG processId = -1;
    hres = g_dbgMgr->system->GetCurrentProcessId( &processId );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessId", hres );

    ProcessMonitor::processStart(processId);

    return processId;
}


///////////////////////////////////////////////////////////////////////////////

void closeDump( PROCESS_DEBUG_ID processId )
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

    ProcessMonitor::processStop(processId, ProcessDetach, 0);

    if ( ProcessMonitor::getNumberProcesses() == 0 )
        g_dbgMgr->ChangeEngineState( DEBUG_CES_EXECUTION_STATUS, DEBUG_STATUS_NO_DEBUGGEE);

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

PROCESS_DEBUG_ID attachKernel( const std::wstring &connectOptions )
{
    if ( !isInintilized() )
        initialize();

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

    ULONG processId = -1;
    hres = g_dbgMgr->system->GetCurrentProcessId( &processId );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessId", hres );

    ProcessMonitor::processStart(processId);

    return processId;
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

std::wstring debugCommand( const std::wstring &command, bool suppressOutput )
{
    HRESULT         hres;

    if ( suppressOutput )
    {
        OutputReader    outReader;

        CComQIPtr<IDebugControl4>  control = outReader.getClient();

        hres = control->ExecuteWide( DEBUG_OUTCTL_THIS_CLIENT, command.c_str(), 0 );

        if ( FAILED( hres ) )
            throw  DbgEngException( L"IDebugControl::Execute", hres ); 

        return outReader.Line();
    }

    hres = g_dbgMgr->control->ExecuteWide( DEBUG_OUTCTL_ALL_CLIENTS, command.c_str(), 0 );

    if ( FAILED( hres ) )
        throw  DbgEngException( L"IDebugControl::Execute", hres ); 

    return std::wstring();
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

    return waitForEvent();
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
    //HRESULT  hres;
    //ULONG  number;

    //hres = g_dbgMgr->system->GetNumberProcesses( &number );
    //if ( FAILED( hres ) )
    //    throw DbgEngException( L"IDebugSystemObjects::GetNumberProcesses", hres );

    //return number;

    return ProcessMonitor::getNumberProcesses();
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
    ProcessAutoSwitch  processSwitch(id);

    HRESULT  hres;
    ULONG  systemId;

    hres = g_dbgMgr->system->GetCurrentProcessSystemId( &systemId );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessSystemId", hres );

    return systemId;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getProcessOffset( PROCESS_DEBUG_ID id )
{
    ProcessAutoSwitch  processSwitch(id);

    HRESULT  hres;
    MEMOFFSET_64  offset;

    hres = g_dbgMgr->system->GetCurrentProcessDataOffset( &offset );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentProcessSystemId", hres );

     return offset;
}

///////////////////////////////////////////////////////////////////////////////

std::wstring getProcessExecutableName(PROCESS_DEBUG_ID id)
{
    ProcessAutoSwitch  processSwitch(id);

    const ULONG bufChars = (MAX_PATH * 2);

    boost::scoped_array< WCHAR > exeName(new WCHAR[bufChars]);
    memset(&exeName[0], 0, bufChars * sizeof(WCHAR));
    
    HRESULT  hres;
    ULONG tmp;
    hres = g_dbgMgr->system->GetCurrentProcessExecutableNameWide(&exeName[0], bufChars, &tmp);
    if (FAILED(hres))
        throw DbgEngException(L"IDebugSystemObjects::GetCurrentProcessExecutableNameWide", hres);

    return std::wstring(&exeName[0]);
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

unsigned long getNumberThreads(PROCESS_DEBUG_ID  processId)
{
    ProcessAutoSwitch  autoProcess(processId);

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

THREAD_DEBUG_ID getThreadIdByOffset(MEMOFFSET_64 offset, PROCESS_DEBUG_ID processId)
{
    ProcessAutoSwitch  autoProcess(processId);

    HRESULT  hres;
    ULONG  id;

    hres = g_dbgMgr->system->GetThreadIdByDataOffset( offset, &id );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetThreadIdBySystemId", hres );

    return THREAD_DEBUG_ID(id);
}

///////////////////////////////////////////////////////////////////////////////

THREAD_DEBUG_ID getThreadIdBySystemId(THREAD_ID tid, PROCESS_DEBUG_ID processId)
{
    ProcessAutoSwitch  autoProcess(processId);

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

THREAD_DEBUG_ID getThreadIdByIndex(unsigned long index, PROCESS_DEBUG_ID processId)
{
    ProcessAutoSwitch autoProcess(processId);

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

THREAD_ID getThreadSystemId(THREAD_DEBUG_ID id, PROCESS_DEBUG_ID  processId)
{
    ThreadAutoSwitch  threadContext(id);
    ProcessAutoSwitch  autoProcess(processId);

    HRESULT  hres;
    ULONG  systemId;

    hres = g_dbgMgr->system->GetCurrentThreadSystemId( &systemId );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSystemObjects::GetCurrentThreadSystemId", hres );

    return systemId;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getThreadOffset( THREAD_DEBUG_ID id, PROCESS_DEBUG_ID  processId)
{
    ThreadAutoSwitch  threadContext(id);
    ProcessAutoSwitch  autoProcess(processId);

    HRESULT  hres;
    MEMOFFSET_64  offset;

    hres = g_dbgMgr->system->GetCurrentThreadDataOffset( &offset );

    if ( FAILED( hres ) )
       throw DbgEngException( L"IDebugSystemObjects::GetCurrentThreadSystemId", hres );
    
    return offset;
}

///////////////////////////////////////////////////////////////////////////////

void registerEventsCallback( DebugEventsCallback *callback )
{
    ProcessMonitor::registerEventsCallback(callback);
}

///////////////////////////////////////////////////////////////////////////////

void removeEventsCallback( DebugEventsCallback *callback )
{
    ProcessMonitor::removeEventsCallback(callback);
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getInstructionOffset()
{
    MEMOFFSET_64  offset;
    HRESULT  hres;

    hres =  g_dbgMgr->registers->GetInstructionOffset( &offset );
    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugRegisters::GetInstructionOffset", hres ); 

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getStackOffset()
{
    HRESULT  hres;
    MEMOFFSET_64 offset;
    hres =  g_dbgMgr->registers->GetStackOffset( &offset );

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugRegisters::GetStackOffset", hres ); 

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getFrameOffset()
{
    HRESULT  hres;
    MEMOFFSET_64 offset;
    hres =  g_dbgMgr->registers->GetFrameOffset( &offset );

    if ( FAILED(hres) )
        throw DbgEngException( L"IDebugRegisters::GetFrameOffset", hres ); 

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long getRegisterNumber()
{
    HRESULT  hres;
    ULONG  number;

    hres = g_dbgMgr->registers->GetNumberRegisters( &number );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugRegisters::GetNumberRegisters", hres ); 

    return number;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long getRegsiterIndex( const std::wstring &name )
{
    HRESULT  hres;
    ULONG  index;

    hres = g_dbgMgr->registers->GetIndexByNameWide( name.c_str(), &index );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugRegisters2::GetIndexByNameWide", hres ); 

    return index;
}

///////////////////////////////////////////////////////////////////////////////

CPURegType getRegisterType( unsigned long index )
{
    HRESULT  hres;

    if ( index >= getRegisterNumber() )
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

std::wstring getRegisterName( unsigned long index )
{
    HRESULT  hres;
    wchar_t  regName[0x100];

    hres = g_dbgMgr->registers->GetDescriptionWide( static_cast<ULONG>(index), regName, 0x100, NULL, NULL );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugRegisters2::GetDescriptionWide", hres ); 

    return std::wstring( regName );
}

///////////////////////////////////////////////////////////////////////////////

void getRegisterValue(unsigned long index, void* buffer, size_t bufferSize )
{
    HRESULT  hres;

    if ( index >= getRegisterNumber() )
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

unsigned long long loadMSR(unsigned long msrIndex )
{
    HRESULT  hres;
    ULONG64     value;

    hres = g_dbgMgr->dataspace->ReadMsr( msrIndex, &value );
    if ( FAILED( hres ) )
         throw DbgEngException( L"IDebugDataSpaces::ReadMsr", hres );

    return value;
}

///////////////////////////////////////////////////////////////////////////////

void setMSR(unsigned long msrIndex, unsigned long long value )
{
    HRESULT  hres;

    hres = g_dbgMgr->dataspace->WriteMsr(msrIndex, value);
    if ( FAILED( hres ) )
         throw DbgEngException( L"IDebugDataSpaces::WriteMsr", hres );
}
///////////////////////////////////////////////////////////////////////////////

CPUType getCPUType()
{
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

CPUType getCPUMode()
{
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

void setCPUMode(CPUType mode )
{
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

void switchCPUMode()
{
    switch ( getCPUMode() )
    {
    case CPU_I386:
        setCPUMode(CPU_AMD64);
        return;
    
    case CPU_AMD64:
        setCPUMode(CPU_I386);
        return;
    }

    DbgException( "can not switch CPU mode");
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
    OutputReader    outReader;

    CComQIPtr<IDebugControl4>  control = outReader.getClient();

    hres = control->CallExtensionWide( extHandle, command.c_str(), params.c_str() );

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

bool IsKernelDebuggeeType()
{
    ULONG debugClass, debugQualifier;
    HRESULT hres = g_dbgMgr->control->GetDebuggeeType( &debugClass, &debugQualifier );

    if (S_OK != hres)
        throw DbgEngException( L"IDebugControl::GetDebuggeeType", hres );   

    return (debugClass == DEBUG_CLASS_KERNEL);
}

///////////////////////////////////////////////////////////////////////////////

void ReadWow64Context( WOW64_CONTEXT& wow64Context )
{
    // 
    //  *** undoc ***
    // !wow64exts.r
    // http://www.woodmann.com/forum/archive/index.php/t-11162.html
    // http://www.nynaeve.net/Code/GetThreadWow64Context.cpp
    // 

    HRESULT     hres;

    ULONG64 teb64Address;

    if (IsKernelDebuggeeType())
    {
        DEBUG_VALUE  debugValue = {};
        ULONG        remainderIndex = 0;

        hres = 
            g_dbgMgr->control->EvaluateWide( 
                L"@@C++(#FIELD_OFFSET(nt!_KTHREAD, Teb))",
                DEBUG_VALUE_INT64,
                &debugValue,
                &remainderIndex );
        if ( FAILED( hres ) )
            throw  DbgEngException( L"IDebugControl::Evaluate", hres );
            
        ULONG64 tebOffset = debugValue.I64;

        hres = g_dbgMgr->system->GetImplicitThreadDataOffset(&teb64Address);
        if (S_OK != hres)
            throw DbgEngException( L"IDebugSystemObjects::GetImplicitThreadDataOffset", hres);

        ULONG readedBytes;

        hres = 
            g_dbgMgr->dataspace->ReadVirtual( 
                teb64Address + tebOffset,
                &teb64Address,
                sizeof(teb64Address),
                &readedBytes);
        if (FAILED(hres) )
            throw MemoryException(teb64Address + tebOffset);
    }
    else
    {
        hres = g_dbgMgr->system->GetImplicitThreadDataOffset(&teb64Address);
        if (S_OK != hres)
            throw DbgEngException( L"IDebugSystemObjects::GetImplicitThreadDataOffset", hres);
    }

    // ? @@C++(#FIELD_OFFSET(nt!_TEB64, TlsSlots))
    // hardcoded in !wow64exts.r (6.2.8250.0)
    static const ULONG teb64ToTlsOffset = 0x01480;
    static const ULONG WOW64_TLS_CPURESERVED = 1;
    ULONG64 cpuAreaAddress;
    ULONG readedBytes;

    hres = g_dbgMgr->dataspace->ReadVirtual( 
            teb64Address + teb64ToTlsOffset + (sizeof(ULONG64) * WOW64_TLS_CPURESERVED),
            &cpuAreaAddress,
            sizeof(cpuAreaAddress),
            &readedBytes);

    if (S_OK != hres)
        throw MemoryException(teb64Address + teb64ToTlsOffset + (sizeof(ULONG64) * WOW64_TLS_CPURESERVED));

    // CPU Area is:
    // +00 unknown ULONG
    // +04 WOW64_CONTEXT struct
    static const ULONG cpuAreaToWow64ContextOffset = sizeof(ULONG);

    hres = 
        g_dbgMgr->dataspace->ReadVirtual(
            cpuAreaAddress + cpuAreaToWow64ContextOffset,
            &wow64Context,
            sizeof(wow64Context),
            &readedBytes);

    if (S_OK != hres)
        throw MemoryException(cpuAreaAddress + cpuAreaToWow64ContextOffset);
}

///////////////////////////////////////////////////////////////////////////////

template < typename T, HRESULT(STDMETHODCALLTYPE IDebugSystemObjects4::*getThread1)(T*), HRESULT(STDMETHODCALLTYPE IDebugSystemObjects4::*getThread2)(T*) >
struct IsThreadChangedImpl
{
    static bool Do()
    {
        HRESULT hres;

        T thread1;
        hres = (g_dbgMgr->system->*getThread1)(&thread1);
        if (S_OK != hres)
            throw DbgEngException( L"IDebugSystemObjects::getThread1", hres );

        T thread2;
        hres = (g_dbgMgr->system->*getThread2)(&thread2);
        if (S_OK != hres)
            throw DbgEngException( L"IDebugSystemObjects::getThread2", hres );

        return (thread1 != thread2);
    }
};

typedef IsThreadChangedImpl<ULONG64, &IDebugSystemObjects4::GetCurrentThreadDataOffset, &IDebugSystemObjects4::GetImplicitThreadDataOffset> IsKernelModeThreadChanged;
typedef IsThreadChangedImpl<ULONG, &IDebugSystemObjects4::GetEventThread, &IDebugSystemObjects4::GetCurrentThreadId>                        IsUserModeThreadChanged;

bool IsThreadChanged()
{
    if (IsKernelDebuggeeType())
        return IsKernelModeThreadChanged::Do();

    return IsUserModeThreadChanged::Do();
}

///////////////////////////////////////////////////////////////////////////////

void getStackTrace(std::vector<FrameDesc> &stackTrace)
{

    HRESULT  hres;

    ULONG  processorType;

    hres = g_dbgMgr->control->GetActualProcessorType( &processorType );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetActualProcessorType", hres );

    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME> dbgFrames(filledFrames);

    if ( processorType == IMAGE_FILE_MACHINE_I386 )
    {
        g_dbgMgr->setQuietNotiification(true);

        hres = 
            g_dbgMgr->control->GetStackTrace(
                0,
                0,
                0,
                &dbgFrames[0],
                filledFrames,
                &filledFrames);

        g_dbgMgr->setQuietNotiification(false);

        if (S_OK != hres)
            throw DbgEngException( L"IDebugControl::GetStackTrace", hres );
    }
    else if ( processorType == IMAGE_FILE_MACHINE_AMD64 )
    {
        ULONG  effproc;

        hres = g_dbgMgr->control->GetEffectiveProcessorType( &effproc );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugControl::GetEffectiveProcessorType", hres );

        if ( effproc == IMAGE_FILE_MACHINE_I386 && IsThreadChanged() )
        {
            WOW64_CONTEXT  wow64Context;

            ReadWow64Context( wow64Context);

            g_dbgMgr->setQuietNotiification(true);

            hres = 
                g_dbgMgr->control->GetContextStackTrace( 
                    &wow64Context,
                    sizeof(wow64Context),
                    &dbgFrames[0],
                    filledFrames,
                    NULL, 
                    filledFrames*sizeof(wow64Context),
                    sizeof(wow64Context),
                    &filledFrames );

            g_dbgMgr->setQuietNotiification(false);

            if (S_OK != hres)
                throw DbgEngException( L"IDebugControl::GetContextStackTrace", hres );
        }
        else
        {
            g_dbgMgr->setQuietNotiification(true);

            hres = 
                g_dbgMgr->control->GetStackTrace(
                    0,
                    0,
                    0,
                    &dbgFrames[0],
                    filledFrames,
                    &filledFrames);

            g_dbgMgr->setQuietNotiification(false);

            if (S_OK != hres)
                throw DbgEngException( L"IDebugControl::GetStackTrace", hres );
        }
    }

    stackTrace.resize(filledFrames);
    for ( ULONG i = 0; i < filledFrames; ++i )
    {
        stackTrace[i].instructionOffset = dbgFrames[i].InstructionOffset;
        stackTrace[i].returnOffset = dbgFrames[i].ReturnOffset;
        stackTrace[i].frameOffset = dbgFrames[i].FrameOffset;
        stackTrace[i].stackOffset = dbgFrames[i].StackOffset;
    }
}

///////////////////////////////////////////////////////////////////////////////

StackFramePtr getCurrentStackFrame()
{
    HRESULT  hres;
    DEBUG_STACK_FRAME  stackFrame = {};

    g_dbgMgr->setQuietNotiification(true);

    hres = g_dbgMgr->symbols->GetScope(NULL, &stackFrame, NULL, 0 );

    g_dbgMgr->setQuietNotiification(false);

    if ( FAILED(hres) )
        throw DbgEngException(L"IDebugSymbols::GetScope", hres);

    return getStackFrame(stackFrame.InstructionOffset, stackFrame.ReturnOffset, stackFrame.FrameOffset, stackFrame.StackOffset);
}

///////////////////////////////////////////////////////////////////////////////

unsigned long getCurrentStackFrameNumber()
{
    HRESULT  hres;
    DEBUG_STACK_FRAME  stackFrame = {};

    g_dbgMgr->setQuietNotiification(true);

    hres = g_dbgMgr->symbols->GetScope(NULL, &stackFrame, NULL, 0 );

    g_dbgMgr->setQuietNotiification(false);

    if ( FAILED(hres) )
        throw DbgEngException(L"IDebugSymbols::GetScope", hres);

    std::vector<FrameDesc>  stackTrace;
    getStackTrace(stackTrace);

    for ( size_t t = 0; t < stackTrace.size(); t++)
    {
        if ( stackTrace[t].instructionOffset == stackFrame.InstructionOffset &&
            stackTrace[t].returnOffset == stackFrame.ReturnOffset &&
            stackTrace[t].frameOffset == stackFrame.FrameOffset &&
            stackTrace[t].stackOffset == stackFrame.StackOffset )
                return t; 
    }

    return ~0;
}

///////////////////////////////////////////////////////////////////////////////

void setCurrentStackFrame( StackFramePtr& stackFrame)
{
    HRESULT  hres;

    DEBUG_STACK_FRAME  debugStackFrame = {};

    debugStackFrame.InstructionOffset = stackFrame->getIP();
    debugStackFrame.ReturnOffset = stackFrame->getRET();
    debugStackFrame.FrameOffset = stackFrame->getFP();
    debugStackFrame.StackOffset = stackFrame->getSP();

    hres = g_dbgMgr->symbols->SetScope( 0, &debugStackFrame, NULL, 0);
    if ( FAILED(hres) )
        throw DbgEngException(L"IDebugSymbols::SetScope", hres);
}

///////////////////////////////////////////////////////////////////////////////

void setCurrentStackFrameByIndex(unsigned long frameIndex)
{
    HRESULT  hres;

    hres = g_dbgMgr->symbols->SetScopeFrameByIndex(frameIndex);
    if (FAILED(hres))
        throw DbgEngException(L"IDebugSymbols::SetScopeFrameByIndex", hres);
}

///////////////////////////////////////////////////////////////////////////////

void resetCurrentStackFrame()
{
    HRESULT  hres;

    hres = g_dbgMgr->symbols->ResetScope();
    if ( FAILED(hres) )
        throw DbgEngException(L"IDebugSymbols::ResetScope", hres);
}

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end 





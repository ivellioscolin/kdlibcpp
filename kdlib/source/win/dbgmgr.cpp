#include "stdafx.h"

#include "dbgmgr.h"
#include "moduleimp.h"

#include "win/exceptions.h"
#include "win/breakpointimp.h"

namespace kdlib {

DebugManagerWrapper   g_dbgMgr;

///////////////////////////////////////////////////////////////////////////////

DebugManager::DebugManager()
{
    m_previousExecutionStatus = DebugStatusNoChange;

    CoInitialize(NULL);

    HRESULT  hres = DebugCreate( __uuidof(IDebugClient4), (void **)&client );
    if ( FAILED( hres ) )
        throw DbgEngException(L"DebugCreate", hres);

    control = CComQIPtr<IDebugControl4>(client);
    system = CComQIPtr<IDebugSystemObjects4>(client);
    dataspace = CComQIPtr<IDebugDataSpaces4>(client);
    symbols = CComQIPtr<IDebugSymbols3>(client);
    advanced = CComQIPtr<IDebugAdvanced2>(client);
    registers = CComQIPtr<IDebugRegisters2>(client);

    client->SetEventCallbacksWide( this );
}

///////////////////////////////////////////////////////////////////////////////

DebugManager::~DebugManager()
{
    CoUninitialize();
}

///////////////////////////////////////////////////////////////////////////////

void DebugManager::registerEventsCallback( DebugEventsCallback *callback )
{
     boost::recursive_mutex::scoped_lock l(m_callbacksLock);
     m_callbacks.push_back( callback );
}

///////////////////////////////////////////////////////////////////////////////

void DebugManager::removeEventsCallback( DebugEventsCallback *callback )
{
    boost::recursive_mutex::scoped_lock l(m_callbacksLock);
    m_callbacks.remove( callback );
}

///////////////////////////////////////////////////////////////////////////////

inline
ULONG ConvertCallbackResult( DebugCallbackResult result )
{
    switch( result )
    {
    case DebugCallbackBreak:
        return DEBUG_STATUS_BREAK;

    case DebugCallbackProceed:
        return DEBUG_STATUS_GO_HANDLED;

    default:
        assert( 0 );

    case DebugCallbackNoChange:
        return DEBUG_STATUS_NO_CHANGE;
    }
}

///////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DebugManager::Breakpoint( IDebugBreakpoint2 *bp ) 
{
    DebugCallbackResult  result = DebugCallbackNoChange;

    HRESULT hres;
    ULONG id;

    hres = bp->GetId( &id );
    if ( FAILED( hres ) )
        return DEBUG_STATUS_NO_CHANGE;

    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    result =  BaseBreakpoint::onBreakpoint(id);

    EventsCallbackList::iterator  it;
    for ( it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
    {
        DebugCallbackResult  ret = (*it)->onBreakpoint(id);
        result = ret != DebugCallbackNoChange ? ret : result;
    }

    return ConvertCallbackResult( result );
}

///////////////////////////////////////////////////////////////////////////////

ExecutionStatus ConvertDbgEngineExecutionStatus( ULONG status )
{
    switch( status )
    {
    case DEBUG_STATUS_NO_CHANGE:
        return DebugStatusNoChange;

    case DEBUG_STATUS_GO:
    case DEBUG_STATUS_GO_HANDLED:
    case DEBUG_STATUS_GO_NOT_HANDLED:
    case DEBUG_STATUS_STEP_OVER:
    case DEBUG_STATUS_STEP_INTO:
    case DEBUG_STATUS_STEP_BRANCH:
    case DEBUG_STATUS_IGNORE_EVENT:
        return DebugStatusGo;

    case DEBUG_STATUS_BREAK:
        return DebugStatusBreak;

    case DEBUG_STATUS_NO_DEBUGGEE:
        return DebugStatusNoDebuggee;
    }

    NOT_IMPLEMENTED();
}

HRESULT STDMETHODCALLTYPE DebugManager::ChangeEngineState(
    __in ULONG Flags,
    __in ULONG64 Argument )
{
    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    if ( ( ( Flags & DEBUG_CES_EXECUTION_STATUS ) != 0 ) &&
         ( ( Argument & DEBUG_STATUS_INSIDE_WAIT ) == 0 ) &&
         (ULONG)Argument != m_previousExecutionStatus )
    {
        if ( m_previousExecutionStatus == DEBUG_STATUS_NO_DEBUGGEE &&
             (ULONG)Argument != DEBUG_STATUS_GO )
                return S_OK;

        ExecutionStatus  executionStatus = ConvertDbgEngineExecutionStatus( (ULONG)Argument );

        EventsCallbackList::iterator  it = m_callbacks.begin();

        for ( ; it != m_callbacks.end(); ++it )
        {
            (*it)->onExecutionStatusChange( executionStatus );
        }

        m_previousExecutionStatus = (ULONG)Argument;
    }

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DebugManager::Exception(
    __in PEXCEPTION_RECORD64 Exception,
    __in ULONG FirstChance )
{
    DebugCallbackResult  result = DebugCallbackNoChange;

    ExceptionInfo   excinfo = {};

    excinfo.firstChance = FirstChance != 0;

    excinfo.exceptionCode = Exception->ExceptionCode;
    excinfo.exceptionFlags = Exception->ExceptionFlags;
    excinfo.exceptionRecord = Exception->ExceptionRecord;
    excinfo.exceptionAddress = Exception->ExceptionAddress;
    excinfo.parameterCount = Exception->NumberParameters;

    for (ULONG i = 0; i < Exception->NumberParameters; ++i)
        excinfo.parameters[i] = Exception->ExceptionInformation[i];

    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it;
    for ( it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
    {
        DebugCallbackResult  ret = (*it)->onException( excinfo );
        result = ret != DebugCallbackNoChange ? ret : result;
    }

    return ConvertCallbackResult( result );
}

///////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DebugManager::LoadModule(
    __in ULONG64 ImageFileHandle,
    __in ULONG64 BaseOffset,
    __in ULONG ModuleSize,
    __in PCWSTR ModuleName,
    __in PCWSTR ImageName,
    __in ULONG CheckSum,
    __in ULONG TimeDateStamp
    )
{
    DebugCallbackResult  result = DebugCallbackNoChange;

    std::wstring  moduleName = ModuleName ? std::wstring(ModuleName) : std::wstring();

    ModuleImp::onModuleLoad(BaseOffset);

    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it;
    for ( it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
    {
        DebugCallbackResult  ret = (*it)->onModuleLoad(BaseOffset, moduleName);
        result = ret != DebugCallbackNoChange ? ret : result;
    }

    return ConvertCallbackResult( result );
}

///////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DebugManager::UnloadModule(
    __in PCWSTR ImageBaseName,
    __in ULONG64 BaseOffset
    )
{
    DebugCallbackResult  result = DebugCallbackNoChange;

    ModulePtr mod = loadModule(BaseOffset);

    ModuleImp::onModuleUnload(BaseOffset);

    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it;
    for ( it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
    {
        DebugCallbackResult  ret = (*it)->onModuleUnload(BaseOffset, mod->getName());
        result = ret != DebugCallbackNoChange ? ret : result;
    }

    return ConvertCallbackResult( result );
}


///////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DebugManager::CreateProcess(
    __in ULONG64 ImageFileHandle,
    __in ULONG64 Handle,
    __in ULONG64 BaseOffset,
    __in ULONG ModuleSize,
    __in PCWSTR ModuleName,
    __in PCWSTR ImageName,
    __in ULONG CheckSum,
    __in ULONG TimeDateStamp,
    __in ULONG64 InitialThreadHandle,
    __in ULONG64 ThreadDataOffset,
    __in ULONG64 StartOffset
    )
{
    DebugCallbackResult  result = DebugCallbackNoChange;

    ModuleImp::onProcessStart(BaseOffset);

    return ConvertCallbackResult( result );
}

///////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE DebugManager::ExitProcess(
    __in ULONG ExitCode
    )
{
    DebugCallbackResult  result = DebugCallbackNoChange;

    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it = m_callbacks.begin();

    for ( ; it != m_callbacks.end(); ++it )
    {
        DebugCallbackResult  ret = (*it)->onProcessExit( getCurrentProcessId(), ProcessExit, ExitCode );
        result = ret != DebugCallbackNoChange ? ret : result;
    }

    return ConvertCallbackResult( result );
}

///////////////////////////////////////////////////////////////////

} //kdlib namespace end

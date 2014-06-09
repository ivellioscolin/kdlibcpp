#pragma once

#include <list>
#include <string>

#include <dbgeng.h>
#include <dbghelp.h>
#include <windows.h>
#include <atlbase.h>

#include <boost/thread/recursive_mutex.hpp>

#include "kdlib/dbgcallbacks.h"

#include "exceptions.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class DebugManager : private DebugBaseEventCallbacksWide
{

public:

    DebugManager();
    DebugManager( const std::wstring& remoteOptions );

    ~DebugManager();

public:

     CComPtr<IDebugClient5>         client;
     CComPtr<IDebugControl4>        control;
     CComPtr<IDebugSystemObjects4>  system;
     CComPtr<IDebugDataSpaces4>     dataspace;
     CComPtr<IDebugSymbols3>        symbols;
     CComPtr<IDebugAdvanced2>       advanced;
     CComPtr<IDebugRegisters2>      registers;

public:

    void registerEventsCallback( DebugEventsCallback *callback );
    void removeEventsCallback( DebugEventsCallback *callback );

private:

    typedef std::list<DebugEventsCallback*>  EventsCallbackList;

    boost::recursive_mutex      m_callbacksLock;
    EventsCallbackList          m_callbacks;
    ULONG                       m_previousExecutionStatus;

    STDMETHOD_(ULONG, AddRef)() { return 1; }
    STDMETHOD_(ULONG, Release)() { return 1; }

    // IDebugEventCallbacks impls
    STDMETHOD(GetInterestMask)(
        __out PULONG Mask 
        )
    {
        *Mask = 0;
        *Mask |= DEBUG_EVENT_BREAKPOINT;
        *Mask |= DEBUG_EVENT_CHANGE_ENGINE_STATE;
        *Mask |= DEBUG_EVENT_EXCEPTION;
        *Mask |= DEBUG_EVENT_LOAD_MODULE;
        *Mask |= DEBUG_EVENT_UNLOAD_MODULE;
        *Mask |= DEBUG_EVENT_CREATE_PROCESS;
        *Mask |= DEBUG_EVENT_EXIT_PROCESS;

        return S_OK;
    }

    STDMETHOD(Breakpoint)(
        __in IDebugBreakpoint2 *bp
    );

    STDMETHOD(ChangeEngineState)(
        __in ULONG Flags,
        __in ULONG64 Argument );

    STDMETHOD(Exception)(
        __in PEXCEPTION_RECORD64 Exception,
        __in  ULONG FirstChance );

    STDMETHOD(LoadModule)(
        __in ULONG64 ImageFileHandle,
        __in ULONG64 BaseOffset,
        __in ULONG ModuleSize,
        __in PCWSTR ModuleName,
        __in PCWSTR ImageName,
        __in ULONG CheckSum,
        __in ULONG TimeDateStamp
        );

    STDMETHOD(UnloadModule)(
        __in PCWSTR ImageBaseName,
        __in ULONG64 BaseOffset
        );

    STDMETHOD(CreateProcess)(
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
        );

    STDMETHOD(ExitProcess)(
        __in ULONG ExitCode
        );
    
};


class DebugManagerWrapper {

public:

    void set( DebugManager*  dbgMgr ) {
        m_dbgMgr = dbgMgr;
    }

    void reset() {
        delete m_dbgMgr;
        m_dbgMgr = 0;
    }

    DebugManager* operator->() {
        if (!m_dbgMgr)
            throw DbgException("pykd is not initialized");

        return m_dbgMgr;
    }

    bool operator==(DebugManager* ptr) {
        return m_dbgMgr == ptr;
    }

    bool operator!=(DebugManager* ptr) {
        return !(m_dbgMgr == ptr);
    }

private:

    DebugManager*  m_dbgMgr;
};


extern DebugManagerWrapper   g_dbgMgr;

///////////////////////////////////////////////////////////////////////////////

class OutputReader : public IDebugOutputCallbacksWide, private boost::noncopyable {

public:

    explicit OutputReader( IDebugClient5 *client ) 
    {
        HRESULT   hres;

        m_client = client;

        hres = m_client->GetOutputCallbacksWide( &m_previousCallback );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugClient::GetOutputCallbacks", hres );

        hres = m_client->SetOutputCallbacksWide( this );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugClient::GetOutputCallbacks", hres);
    }

    ~OutputReader() 
    {
        m_client->SetOutputCallbacksWide( m_previousCallback );
    }

    const std::wstring&
    Line() const {
        return  m_readLine;
    }

private:

     // IUnknown.
    STDMETHOD(QueryInterface)(
        __in REFIID InterfaceId,
        __out PVOID* Interface ) {
        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)() {
        return 1L;
    }


    STDMETHOD_(ULONG, Release)() {
        return 0L;
    }

   STDMETHOD(Output)(
        __in ULONG Mask,
        __in PCWSTR Text )
   {
        if ( Mask == DEBUG_OUTPUT_NORMAL )
        {
            m_readLine += std::wstring( Text );
        }
       return S_OK;
   }

private:

    std::wstring                         m_readLine;

    CComPtr<IDebugOutputCallbacksWide>   m_previousCallback;

    CComPtr<IDebugClient5>              m_client;
};

///////////////////////////////////////////////////////////////////////////////

ExecutionStatus ConvertDbgEngineExecutionStatus( ULONG status );

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end


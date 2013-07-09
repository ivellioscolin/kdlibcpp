#pragma once

#include <list>

#include <dbgeng.h>
#include <dbghelp.h>
#include <windows.h>
#include <atlbase.h>

#include <boost/thread/recursive_mutex.hpp>

#include "kdlib/dbgcallbacks.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class DebugManager : private DebugBaseEventCallbacks
{

public:

    DebugManager();
    ~DebugManager();

public:

     CComPtr<IDebugClient4>         client;
     CComPtr<IDebugControl4>        control;
     CComPtr<IDebugSystemObjects2>  system;
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
        return S_OK;
    }

    STDMETHOD(Breakpoint)(
        __in IDebugBreakpoint *bp
    );

    STDMETHOD(ChangeEngineState)(
        __in ULONG Flags,
        __in ULONG64 Argument );
};

extern DebugManager*  g_dbgMgr;

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end


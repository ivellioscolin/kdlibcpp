#pragma once

#include <dbgeng.h>
#include <dbghelp.h>
#include <windows.h>
#include <atlbase.h>

#include <map>

#include <boost/thread/recursive_mutex.hpp>

#include "kdlib/breakpoint.h"

namespace kdlib {


class BaseBreakpoint : public Breakpoint 
{
public:

    BaseBreakpoint( BreakpointCallback* callback = 0 ) :
      m_callback(callback)
      {}

    ~BaseBreakpoint();

    void insertBreakpoint();

    static void clearBreakpointMap();

    static DebugCallbackResult onBreakpoint( BREAKPOINT_ID bpId );

protected: 

    BreakpointCallback*  m_callback;

    CComPtr<IDebugBreakpoint>  m_breakpoint;

    static boost::recursive_mutex  m_breakMapLock;

    static std::map<BREAKPOINT_ID, BaseBreakpoint*>  m_breakMap;
};


class SoftwareBreakpoint : public BaseBreakpoint
{
public:

    explicit SoftwareBreakpoint( MEMOFFSET_64 offset, BreakpointCallback* callback = 0 );
};

class HardwareBreakpoint : public BaseBreakpoint
{
public:

    explicit HardwareBreakpoint( MEMOFFSET_64 offset, size_t size = 0, ACCESS_TYPE accessType = 0, BreakpointCallback* callback = 0 );
};


}
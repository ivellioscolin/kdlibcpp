#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "kdlib/dbgtypedef.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

BREAKPOINT_ID softwareBreakPointSet( MEMOFFSET_64 offset );
BREAKPOINT_ID hardwareBreakPointSet( MEMOFFSET_64 offset, size_t size = 0, ACCESS_TYPE accessType = 0 );
void breakPointRemove( BREAKPOINT_ID id );

///////////////////////////////////////////////////////////////////////////////

class Breakpoint;
typedef boost::shared_ptr<Breakpoint>  BreakpointPtr;

struct BreakpointCallback {
    virtual DebugCallbackResult onBreakpoint() = 0;
};

class Breakpoint : public boost::noncopyable
{
public:

    static BreakpointPtr setBreakpoint( MEMOFFSET_64 offset, BreakpointCallback* callback = 0 );

    static BreakpointPtr setHardwareBreakpoint( MEMOFFSET_64 offset, size_t size = 0, ACCESS_TYPE accessType = 0, BreakpointCallback* callback = 0 );
};

///////////////////////////////////////////////////////////////////////////////

} // namespace kdlib

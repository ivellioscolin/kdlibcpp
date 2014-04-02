
#pragma once

#include <boost/shared_ptr.hpp>

#include <kdlib/dbgtypedef.h>
#include <kdlib/breakpoint.h>
#include <kdlib/dbgengine.h>

namespace kdlib {

class BreakpointInfo;
typedef boost::shared_ptr<BreakpointInfo>  BreakpointInfoPtr;


class BreakpointInfo {

public:

    BreakpointInfo( MEMOFFSET_64  offset, Breakpoint* callback = 0 );

    BreakpointInfo( MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType, Breakpoint* callback = 0 );

    void remove();

    DebugCallbackResult hit() {
        return m_callback->onHit();
    }

    MEMOFFSET_64  getOffset() const {
        return m_offset;
    }

    BreakpointType  getType() const {
        return m_breakpointType;
    }

     Breakpoint*  getCallback() {
         return  m_callback;
     }

private:

    MEMOFFSET_64  m_offset;
    BreakpointType  m_breakpointType;
    Breakpoint*  m_callback;
};

};


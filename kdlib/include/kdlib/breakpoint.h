#pragma once 

#include <boost/smart_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "kdlib/dbgtypedef.h"

namespace kdlib {

//////////////////////////////////////////////////////////////////////////////

enum BreakpointType {

    SoftwareBreakpoint,
    DataAccessBreakpoint
};


#define BREAKPOINT_UNSET  (~0ULL)

//////////////////////////////////////////////////////////////////////////////

class Breakpoint;
typedef boost::shared_ptr<Breakpoint>  BreakpointPtr;

///////////////////////////////////////////////////////////////////////////////

class Breakpoint {

public:

    virtual DebugCallbackResult  onHit() = 0;

};

///////////////////////////////////////////////////////////////////////////////


class BaseBreakpoint : public Breakpoint {

public:

    BaseBreakpoint() : m_id(BREAKPOINT_UNSET)
    {}

    virtual ~BaseBreakpoint();

    virtual DebugCallbackResult  onHit() {
        return DebugCallbackBreak;
    }

    void remove();

    void set( MEMOFFSET_64 offset );

    void set( MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType );

protected:

    BREAKPOINT_ID  m_id;

};

/////////////////////////////////////////////////////////////////////////////

template<typename BreakpointType>
inline
BreakpointPtr
setBp( MEMOFFSET_64 offset )
{
    BreakpointType  *bp = new BreakpointType();
    bp->set(offset);
    return BreakpointPtr(bp);
}

template<typename BreakpointType>
inline
BreakpointPtr
setBp(MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType )
{
    BreakpointType  *bp = new BreakpointType();
    bp->set(offset, size, accessType);
    return BreakpointPtr(bp);
}

///////////////////////////////////////////////////////////////////////////////

//breakpoints

BREAKPOINT_ID softwareBreakPointSet( MEMOFFSET_64 offset, Breakpoint *callback =0 );
BREAKPOINT_ID hardwareBreakPointSet( MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType, Breakpoint *callback =0 );
void breakPointRemove( BREAKPOINT_ID id );

////////////////////////////////////////////////////////////////////////////////

} // kdlib

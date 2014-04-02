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

    virtual void remove() = 0;

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

    void set( MEMOFFSET_64 offset );

    void set( MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType );

    virtual void remove();

private:

    BREAKPOINT_ID  m_id;

};

///////////////////////////////////////////////////////////////////////////////

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

BREAKPOINT_ID softwareBreakPointSet( MEMOFFSET_64 offset);
BREAKPOINT_ID hardwareBreakPointSet( MEMOFFSET_64 offset, size_t size = 0, ACCESS_TYPE accessType = 0 );
void breakPointRemove( BREAKPOINT_ID id );


////////////////////////////////////////////////////////////////////////////////











//class Breakpoint  {
//
//public:
//    virtual DebugCallbackResult  onHit() = 0;
//    virtual MEMOFFSET_64  getOffset() const = 0;
//    virtual void remove() = 0;
//
//private:
//
//    template<typename BreakpointType> friend  BreakpointPtr setBp( MEMOFFSET_64 offset );
//
//};
//
//////////////////////////////////////////////////////////////////////////////////
//
//class BreakpointAdapter : public Breakpoint 
//{
//public:
//
//    explicit BreakpointAdapter( BreakpointPtr originalBp ) 
//        : m_ptr( originalBp )
//        {}
//
//protected: 
//    
//    virtual DebugCallbackResult  onHit() {
//        return m_ptr->onHit();
//    }
//
//    virtual MEMOFFSET_64  getOffset() const {
//        return  m_ptr->getOffset();
//    }
//
//    virtual void remove() {
//        m_ptr->remove();
//    }
//
//    BreakpointPtr   m_ptr;
//};
//
////////////////////////////////////////////////////////////////////////////////
//
//template<typename BreakpointType>
//inline
//BreakpointPtr
//setBp( MEMOFFSET_64 offset )
//{
//    BreakpointType  bp = new BreakpointType(offset);
//    return  BreakpointAdapter( bp->shared_from_this() );
//}
//
//////////////////////////////////////////////////////////////////////////////////
//
//class SoftwareBreakpoint : public Breakpoint, private boost::noncopyable, public boost::enable_shared_from_this<SoftwareBreakpoint>
//{
//
//protected:
//
//    explicit SoftwareBreakpoint( MEMOFFSET_64 offset );
//
//public:
//    
//    virtual ~SoftwareBreakpoint() {
//
//    }
//
//protected:
//
//    virtual DebugCallbackResult  onHit() {
//       return DebugCallbackBreak;
//    }
//
//    virtual MEMOFFSET_64  getOffset() const {
//        return m_offset;
//    }
//
//    virtual void remove();
//
//    MEMOFFSET_64  m_offset;
//    BREAKPOINT_ID  m_id;
//
//};

////////////////////////////////////////////////////////////////////////////////

//class HardwareBreakpoint : public Breakpoint, private boost::noncopyable 
//{
//protected:
//
//    HardwareBreakpoint( MEMOFFSET_64 offset, size_t size = 0, ACCESS_TYPE accessType = 0 );
//
//public:
//
//    virtual ~HardwareBreakpoint();
//
//protected:
//    virtual DebugCallbackResult  onHit() {
//       return DebugCallbackBreak;
//    }
//};



//////////////////////////////////////////////////////////////////////////////

} // kdlib

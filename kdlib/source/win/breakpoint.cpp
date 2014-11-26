#include "stdafx.h"

#include <boost/enable_shared_from_this.hpp>

#include "win/dbgmgr.h"

#include "processmon.h"

namespace kdlib {


class BaseBreakpointImpl : public Breakpoint,  public boost::enable_shared_from_this<BaseBreakpointImpl>
{
public:

    BaseBreakpointImpl(MEMOFFSET_64 offset, BreakpointCallback *callback =0) :
        m_id(BREAKPOINT_UNSET),
        m_offset(offset),
        m_callback(callback)
        {}

    virtual BREAKPOINT_ID getId() const {
        return m_id;
    }

    virtual MEMOFFSET_64 getOffset() const {
        return m_offset;
    }

    virtual BreakpointCallback* getCallback() {
        return m_callback;
    }

protected:

    BREAKPOINT_ID  m_id;
    MEMOFFSET_64  m_offset;
    BreakpointCallback*  m_callback;
};



class SoftwareBreakpointImpl : public BaseBreakpointImpl
{

public:

    SoftwareBreakpointImpl(MEMOFFSET_64 offset, BreakpointCallback *callback =0) : BaseBreakpointImpl(offset,callback){}

    virtual void set();

    virtual void remove();

    virtual BreakpointType getType() const {
        return SoftwareBreakpoint;
    }

};

class HardwareBreakpointImpl : public BaseBreakpointImpl
{
public:

    HardwareBreakpointImpl(MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType, BreakpointCallback *callback) 
        : BaseBreakpointImpl(offset,callback),
        m_size(size), 
        m_accessType(accessType)
        {}

    virtual void set();

    virtual void remove();

    virtual BreakpointType getType() const {
        return DataAccessBreakpoint;
    }

private:

    size_t  m_size;
    ACCESS_TYPE  m_accessType;
};

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr softwareBreakPointSet(MEMOFFSET_64 offset, BreakpointCallback *callback)
{
    BreakpointPtr  bp = BreakpointPtr( new SoftwareBreakpointImpl(offset,callback) );
    bp->set();
    return bp;
}

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr hardwareBreakPointSet(MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType, BreakpointCallback *callback)
{
    BreakpointPtr  bp = BreakpointPtr( new HardwareBreakpointImpl(offset, size, accessType, callback) );
    bp->set();
    return bp;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long getNumberBreakpoints()
{
    return ProcessMonitor::getNumberBreakpoints();
}

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr getBreakpointByIndex(unsigned long index)
{
    return ProcessMonitor::getBreakpointByIndex(index);
}

///////////////////////////////////////////////////////////////////////////////

void SoftwareBreakpointImpl::set()
{
    HRESULT  hres;

    IDebugBreakpoint  *bp;
    hres = g_dbgMgr->control->AddBreakpoint(
        DEBUG_BREAKPOINT_CODE,
        DEBUG_ANY_ID,
        &bp);

    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::AddBreakpoint", hres);

    hres = bp->SetOffset(m_offset);
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

    BreakpointPtr  ptr = shared_from_this();
    m_id = ProcessMonitor::insertBreakpoint(ptr);
}

///////////////////////////////////////////////////////////////////////////////

void SoftwareBreakpointImpl::remove()
{
    HRESULT  hres;
    ULONG  bpNumber;

    hres =  g_dbgMgr->control->GetNumberBreakpoints(&bpNumber);
    if ( FAILED(hres) )
        throw DbgEngException(L"IDebugControl::GetNumberBreakpoints", hres);

    for ( ULONG i = 0; i < bpNumber; ++i )
    {
        IDebugBreakpoint  *bp;
        hres = g_dbgMgr->control->GetBreakpointByIndex(i, &bp );
        if ( FAILED(hres) )
            throw DbgEngException(L"IDebugControl::GetBreakpointByIndex", hres);

        MEMOFFSET_64  offset;
        hres = bp->GetOffset(&offset);

        if ( FAILED(hres) )
            throw DbgEngException(L"IDebugControl::GetOffset", hres);

        if ( offset != m_offset )
            continue;

        ULONG breakType, procType;
        hres = bp->GetType( &breakType, &procType );
        if ( FAILED(hres) )
            throw DbgEngException(L"IDebugBreakpoint::GetType", hres );

        if ( breakType != DEBUG_BREAKPOINT_CODE )
            continue;

        hres= g_dbgMgr->control->RemoveBreakpoint(bp);

        if ( FAILED(hres) )
            throw DbgEngException( L"IDebugBreakpoint::RemoveBreakpoint", hres);

        ProcessMonitor::removeBreakpoint(m_id);
        m_id = BREAKPOINT_UNSET;

        break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void HardwareBreakpointImpl::set()
{
    HRESULT  hres;

    IDebugBreakpoint  *bp;
    hres = g_dbgMgr->control->AddBreakpoint(
        DEBUG_BREAKPOINT_DATA,
        DEBUG_ANY_ID,
        &bp);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::AddBreakpoint", hres);

    hres = bp->SetOffset(m_offset);
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

    hres = bp->SetDataParameters(static_cast<ULONG>(m_size), m_accessType);
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
}

///////////////////////////////////////////////////////////////////////////////

void HardwareBreakpointImpl::remove()
{
    HRESULT  hres;
    ULONG  bpNumber;

    hres =  g_dbgMgr->control->GetNumberBreakpoints(&bpNumber);
    if ( FAILED(hres) )
        throw DbgEngException(L"IDebugControl::GetNumberBreakpoints", hres);

    for ( ULONG i = 0; i < bpNumber; ++i )
    {
        IDebugBreakpoint  *bp;
        hres = g_dbgMgr->control->GetBreakpointByIndex(i, &bp );
        if ( FAILED(hres) )
            throw DbgEngException(L"IDebugControl::GetBreakpointByIndex", hres);

        MEMOFFSET_64  offset;
        hres = bp->GetOffset(&offset);

        if ( FAILED(hres) )
            throw DbgEngException(L"IDebugControl::GetOffset", hres);

        if ( offset != m_offset )
            continue;

        ULONG breakType, procType;
        hres = bp->GetType( &breakType, &procType );
        if ( FAILED(hres) )
            throw DbgEngException(L"IDebugBreakpoint::GetType", hres );

        if ( breakType != DEBUG_BREAKPOINT_DATA )
            continue;

        hres= g_dbgMgr->control->RemoveBreakpoint(bp);

        if ( FAILED(hres) )
            throw DbgEngException( L"IDebugBreakpoint::RemoveBreakpoint", hres);

        break;
    }
}

///////////////////////////////////////////////////////////////////////////////

} // namespace kdlib

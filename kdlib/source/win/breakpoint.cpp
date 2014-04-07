#include "stdafx.h"

#include "win/dbgmgr.h"

#include "breakpointimpl.h"
#include "processmon.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

BreakpointInfo::BreakpointInfo( MEMOFFSET_64  offset, Breakpoint* callback ) :
    m_offset(offset),
    m_breakpointType(SoftwareBreakpoint),
    m_callback(callback)
{
    HRESULT  hres;

    IDebugBreakpoint  *bp;
    hres = g_dbgMgr->control->AddBreakpoint(
        DEBUG_BREAKPOINT_CODE,
        DEBUG_ANY_ID,
        &bp);

    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::AddBreakpoint", hres);

    hres = bp->SetOffset(offset);
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
}

///////////////////////////////////////////////////////////////////////////////

BreakpointInfo::BreakpointInfo( MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType, Breakpoint* callback ) :
    m_offset(offset),
    m_breakpointType(DataAccessBreakpoint),
    m_callback(callback)
{
    HRESULT  hres;

    IDebugBreakpoint  *bp;
    hres = g_dbgMgr->control->AddBreakpoint(
        DEBUG_BREAKPOINT_DATA,
        DEBUG_ANY_ID,
        &bp);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::AddBreakpoint", hres);

    hres = bp->SetOffset(offset);
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

    hres = bp->SetDataParameters(static_cast<ULONG>(size), accessType);
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

void BreakpointInfo::remove()
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

        if ( breakType == DEBUG_BREAKPOINT_CODE && m_breakpointType != SoftwareBreakpoint )
            continue;

        hres= g_dbgMgr->control->RemoveBreakpoint(bp);

        if ( FAILED(hres) )
            throw DbgEngException( L"IDebugBreakpoint::RemoveBreakpoint", hres);

        break;
    }
}

///////////////////////////////////////////////////////////////////////////////

BREAKPOINT_ID softwareBreakPointSet( MEMOFFSET_64 offset, Breakpoint *callback )
{
    BreakpointInfoPtr  bp = BreakpointInfoPtr(new BreakpointInfo(offset, callback) );
    return ProcessMonitor::insertBreakpoint(bp);
}

///////////////////////////////////////////////////////////////////////////////

BREAKPOINT_ID hardwareBreakPointSet( MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType, Breakpoint *callback )
{
    BreakpointInfoPtr  bp = BreakpointInfoPtr(new BreakpointInfo(offset, size, accessType, callback) );
    return ProcessMonitor::insertBreakpoint(bp);
}

///////////////////////////////////////////////////////////////////////////////

void breakPointRemove( BREAKPOINT_ID id )
{
    ProcessMonitor::removeBreakpoint(id);
}

///////////////////////////////////////////////////////////////////////////////

BaseBreakpoint::~BaseBreakpoint()
{
    remove();
}

///////////////////////////////////////////////////////////////////////////////

void BaseBreakpoint::set( MEMOFFSET_64 offset )
{
    remove();
    BreakpointInfoPtr  bp = BreakpointInfoPtr(new BreakpointInfo(offset, this) );
    m_id = ProcessMonitor::insertBreakpoint(bp);
}

///////////////////////////////////////////////////////////////////////////////

void BaseBreakpoint::set( MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType )
{
    remove();
    BreakpointInfoPtr  bp = BreakpointInfoPtr(new BreakpointInfo(offset, size, accessType, this) );
    m_id = ProcessMonitor::insertBreakpoint(bp);
}


///////////////////////////////////////////////////////////////////////////////

void BaseBreakpoint::remove()
{
    if ( m_id != BREAKPOINT_UNSET )
    {
        breakPointRemove(m_id);
        m_id = BREAKPOINT_UNSET;
    }
}


///////////////////////////////////////////////////////////////////////////////

} // namespace kdlib

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

BREAKPOINT_ID softwareBreakPointSet( MEMOFFSET_64 offset )
{
    BreakpointInfoPtr  bp = BreakpointInfoPtr(new BreakpointInfo(offset) );
    return ProcessMonitor::insertBreakpoint(bp);
}

///////////////////////////////////////////////////////////////////////////////

BREAKPOINT_ID hardwareBreakPointSet( MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType )
{
    BreakpointInfoPtr  bp = BreakpointInfoPtr(new BreakpointInfo(offset, size, accessType) );
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














//BREAKPOINT_ID softwareBreakPointSet( MEMOFFSET_64 offset )
//{
//    HRESULT  hres;
//
//    IDebugBreakpoint  *bp;
//    hres = g_dbgMgr->control->AddBreakpoint(
//        DEBUG_BREAKPOINT_CODE,
//        DEBUG_ANY_ID,
//        &bp);
//
//    if (S_OK != hres)
//        throw DbgEngException(L"IDebugControl::AddBreakpoint", hres);
//
//    hres = bp->SetOffset(offset);
//    if (S_OK != hres)
//    {
//        g_dbgMgr->control->RemoveBreakpoint(bp);
//        throw DbgEngException(L"IDebugBreakpoint::SetOffset", hres);
//    }
//
//    ULONG bpFlags;
//    hres = bp->GetFlags(&bpFlags);
//    if (S_OK != hres)
//    {
//        g_dbgMgr->control->RemoveBreakpoint(bp);
//        throw DbgEngException(L"IDebugBreakpoint::GetFlags", hres);
//    }
//    
//    bpFlags |= DEBUG_BREAKPOINT_ENABLED | DEBUG_BREAKPOINT_ADDER_ONLY | DEBUG_BREAKPOINT_GO_ONLY;
//    hres = bp->SetFlags(bpFlags);
//    if (S_OK != hres)
//    {
//        g_dbgMgr->control->RemoveBreakpoint(bp);
//        throw DbgEngException( L"IDebugBreakpoint::SetFlags", hres);
//    }
//
//    return ProcessMonitor::insertSoftwareBreakpoint(offset);
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void breakPointRemove( BREAKPOINT_ID id )
//{
//    PROCESS_DEBUG_ID  processId;
//    MEMOFFSET_64  bpOffset;
//    bool  isSoftware;
//
//    ProcessMonitor::getBreakpoint( id, processId, bpOffset, software );
//
//    ProcessMonitor::removeBreakpoint( id );
//
//}
//
/////////////////////////////////////////////////////////////////////////////////
//

















//
//SoftwareBreakpoint::SoftwareBreakpoint( MEMOFFSET_64 offset )
//{
//    HRESULT  hres;
//
//    IDebugBreakpoint  *bp;
//    hres = g_dbgMgr->control->AddBreakpoint(
//        DEBUG_BREAKPOINT_CODE,
//        DEBUG_ANY_ID,
//        &bp);
//
//    if (S_OK != hres)
//        throw DbgEngException(L"IDebugControl::AddBreakpoint", hres);
//
//    hres = bp->SetOffset(offset);
//    if (S_OK != hres)
//    {
//        g_dbgMgr->control->RemoveBreakpoint(bp);
//        throw DbgEngException(L"IDebugBreakpoint::SetOffset", hres);
//    }
//
//    ULONG bpFlags;
//    hres = bp->GetFlags(&bpFlags);
//    if (S_OK != hres)
//    {
//        g_dbgMgr->control->RemoveBreakpoint(bp);
//        throw DbgEngException(L"IDebugBreakpoint::GetFlags", hres);
//    }
//    
//    bpFlags |= DEBUG_BREAKPOINT_ENABLED | DEBUG_BREAKPOINT_ADDER_ONLY | DEBUG_BREAKPOINT_GO_ONLY;
//    hres = bp->SetFlags(bpFlags);
//    if (S_OK != hres)
//    {
//        g_dbgMgr->control->RemoveBreakpoint(bp);
//        throw DbgEngException( L"IDebugBreakpoint::SetFlags", hres);
//    }
//
//    m_id = ProcessMonitor::insertBp( shared_from_this() );
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void SoftwareBreakpoint::remove()
//{
//    PROCESS_DEBUG_ID  processId = ProcessMonitor::getProcessByBp(m_id);
//
//    if ( processId == -1 )
//        return;
//
//    ProcessMonitor::removeBp(m_id);
//
//    setCurrentProcess(processId);
//
//    HRESULT  hres;
//    ULONG  bpNumber;
//
//    hres =  g_dbgMgr->control->GetNumberBreakpoints(&bpNumber);
//    if ( FAILED(hres) )
//        throw DbgEngException(L"IDebugControl::GetNumberBreakpoints", hres);
//
//    for ( ULONG i = 0; i < bpNumber; ++i )
//    {
//        IDebugBreakpoint  *bp;
//        hres = g_dbgMgr->control->GetBreakpointByIndex(i, &bp );
//        if ( FAILED(hres) )
//            throw DbgEngException(L"IDebugControl::GetBreakpointByIndex", hres);
//
//        MEMOFFSET_64  offset;
//        hres = bp->GetOffset(&offset);
//
//        if ( FAILED(hres) )
//            throw DbgEngException(L"IDebugControl::GetOffset", hres);
//
//        if ( offset != m_offset )
//            continue;
//
//        ULONG breakType, procType;
//        hres = bp->GetType( &breakType, &procType );
//        if ( FAILED(hres) )
//            throw DbgEngException(L"IDebugBreakpoint::GetType", hres );
//
//        if ( breakType != DEBUG_BREAKPOINT_CODE )
//            continue;
//
//        hres= g_dbgMgr->control->RemoveBreakpoint(bp);
//
//        if ( FAILED(hres) )
//            throw DbgEngException( L"IDebugBreakpoint::RemoveBreakpoint", hres);
//
//        break;
//    }
//
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//BREAKPOINT_ID softwareBreakPointSet( MEMOFFSET_64 offset )
//{
//    SoftwareBreakpoint  bp = new SoftwareBreakpoint(offset);
//    bp->getId();
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void breakPointRemove( BREAKPOINT_ID id )
//{
//    PROCESS_DEBUG_ID  processId = ProcessMonitor::getProcessByBp(m_id);
//
//    if ( processId == -1 )
//        return;
//
//    ProcessMonitor::removeBp(m_id);
//
//    setCurrentProcess(processId);
//}
//
/////////////////////////////////////////////////////////////////////////////////









































/////////////////////////////////////////////////////////////////////////////////
//
//void hardwareBreakPointSet( MEMOFFSET_64 offset, size_t size = 0, ACCESS_TYPE accessType = 0 )
//{
//
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void softwareBreakPointRemove( MEMOFFSET_64 offset )
//{
//
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void hardwareBreakPointSet( MEMOFFSET_64 offset )
//{
//
//}
//
/////////////////////////////////////////////////////////////////////////////////

//void softwareBreakPointRemoveAll()
//{
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void hardwareBreakPointRemoveAll()
//{
//}
///////////////////////////////////////////////////////////////////////////////
//
//BREAKPOINT_ID softwareBreakPointSet( MEMOFFSET_64 offset )
//{
//    HRESULT  hres;
//
//    IDebugBreakpoint  *bp;
//    hres = g_dbgMgr->control->AddBreakpoint(
//        DEBUG_BREAKPOINT_CODE,
//        DEBUG_ANY_ID,
//        &bp);
//
//    if (S_OK != hres)
//        throw DbgEngException(L"IDebugControl::AddBreakpoint", hres);
//
//    hres = bp->SetOffset(offset);
//    if (S_OK != hres)
//    {
//        g_dbgMgr->control->RemoveBreakpoint(bp);
//        throw DbgEngException(L"IDebugBreakpoint::SetOffset", hres);
//    }
//
//    ULONG bpFlags;
//    hres = bp->GetFlags(&bpFlags);
//    if (S_OK != hres)
//    {
//        g_dbgMgr->control->RemoveBreakpoint(bp);
//        throw DbgEngException(L"IDebugBreakpoint::GetFlags", hres);
//    }
//    
//    bpFlags |= DEBUG_BREAKPOINT_ENABLED | DEBUG_BREAKPOINT_ADDER_ONLY | DEBUG_BREAKPOINT_GO_ONLY;
//    hres = bp->SetFlags(bpFlags);
//    if (S_OK != hres)
//    {
//        g_dbgMgr->control->RemoveBreakpoint(bp);
//        throw DbgEngException( L"IDebugBreakpoint::SetFlags", hres);
//    }
//
//    ULONG  breakId;
//    hres = bp->GetId(&breakId);
//    if (S_OK != hres)
//    {
//        g_dbgMgr->control->RemoveBreakpoint(bp);
//        throw DbgEngException( L"IDebugBreakpoint::GetId", hres);
//    }
//
//    return breakId;
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//BREAKPOINT_ID hardwareBreakPointSet( MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType )
//{
//    HRESULT  hres;
//
//    IDebugBreakpoint  *bp;
//    hres = g_dbgMgr->control->AddBreakpoint(
//        DEBUG_BREAKPOINT_DATA,
//        DEBUG_ANY_ID,
//        &bp);
//    if (S_OK != hres)
//        throw DbgEngException(L"IDebugControl::AddBreakpoint", hres);
//
//    hres = bp->SetOffset(offset);
//    if (S_OK != hres)
//    {
//        g_dbgMgr->control->RemoveBreakpoint(bp);
//        throw DbgEngException(L"IDebugBreakpoint::SetOffset", hres);
//    }
//
//    ULONG bpFlags;
//    hres = bp->GetFlags(&bpFlags);
//    if (S_OK != hres)
//    {
//        g_dbgMgr->control->RemoveBreakpoint(bp);
//        throw DbgEngException(L"IDebugBreakpoint::GetFlags", hres);
//    }
//
//    hres = bp->SetDataParameters(static_cast<ULONG>(size), accessType);
//    if (S_OK != hres)
//    {
//        g_dbgMgr->control->RemoveBreakpoint(bp);
//        throw DbgEngException(L"IDebugBreakpoint::SetDataParameters", hres);
//    }
//
//    bpFlags |= DEBUG_BREAKPOINT_ENABLED | DEBUG_BREAKPOINT_ADDER_ONLY | DEBUG_BREAKPOINT_GO_ONLY;
//    hres = bp->SetFlags(bpFlags);
//    if (S_OK != hres)
//    {
//        g_dbgMgr->control->RemoveBreakpoint(bp);
//        throw DbgEngException(L"IDebugBreakpoint::SetFlags", hres);
//    }
//
//    ULONG  breakId;
//    hres = bp->GetId(&breakId);
//    if (S_OK != hres)
//    {
//        g_dbgMgr->control->RemoveBreakpoint(bp);
//        throw DbgEngException(L"IDebugBreakpoint::GetId", hres);
//    }
//
//    return breakId;
//}
//
///////////////////////////////////////////////////////////////////////////////////
//
//void breakPointRemove( BREAKPOINT_ID id )
//{
//    IDebugBreakpoint *bp;
//    HRESULT hres = g_dbgMgr->control->GetBreakpointById(id, &bp);
//    if (S_OK != hres)
//        throw DbgEngException(L"IDebugControl::GetBreakpointById", hres);
//
//    hres = g_dbgMgr->control->RemoveBreakpoint(bp);
//    if (S_OK != hres)
//        throw DbgEngException(L"IDebugControl::RemoveBreakpoint", hres);
//}
//
///////////////////////////////////////////////////////////////////////////////////
//
//SoftwareBreakpoint::SoftwareBreakpoint( MEMOFFSET_64 offset ) :
//    m_offset(offset)
//{
//    softwareBreakPointSet( offset );
//
//    ProcessInfoPtr  processInfo = ProcessMonitor::getProcessInfo();
//    if ( processInfo )
//        processInfo->insertSoftwareBp(this);
//}
//
///////////////////////////////////////////////////////////////////////////////////
//
//SoftwareBreakpoint::~SoftwareBreakpoint()
//{
//
//}
//
///////////////////////////////////////////////////////////////////////////////////
//
//


/////////////////////////////////////////////////////////////////////////////////
//
//void Breakpoint::addSoftwareBp( Breakpoint* bp )
//{
//    ProcessInfoPtr  processInfo = ProcessMonitor::getProcessInfo();
//    if ( processInfo )
//        processInfo->insertSoftwareBp(bp);
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void Breakpoint::removeSoftwareBp( Breakpoint* bp )
//{
//    
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//SoftwareBreakpoint::SoftwareBreakpoint( MEMOFFSET_64 offset ) : m_offset(offset)
//{
//    softwareBreakPointSet(offset);
//}
//
/////////////////////////////////////////////////////////////////////////////////

} // namespace kdlib

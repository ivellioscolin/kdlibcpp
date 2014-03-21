#include "stdafx.h"

#include "win/breakpointimp.h"
#include "win/dbgmgr.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

boost::recursive_mutex  BaseBreakpoint::m_breakMapLock;
std::map<BREAKPOINT_ID, BaseBreakpoint*> BaseBreakpoint::m_breakMap;

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr Breakpoint::setBreakpoint( MEMOFFSET_64 offset, BreakpointCallback* callback )
{
    return BreakpointPtr( new SoftwareBreakpoint(offset,callback) );
}

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr Breakpoint::setHardwareBreakpoint( MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType, BreakpointCallback* callback )
{
    return BreakpointPtr( new HardwareBreakpoint(offset, size, accessType, callback) );
}

///////////////////////////////////////////////////////////////////////////////

BaseBreakpoint::~BaseBreakpoint()
{
    HRESULT  hres;

    ULONG  breakId;
    hres = m_breakpoint->GetId(&breakId);
    if ( SUCCEEDED(hres) )
    {
        boost::recursive_mutex::scoped_lock  l(m_breakMapLock);
        std::map<BREAKPOINT_ID, BaseBreakpoint*>::iterator it = m_breakMap.find(breakId);
        if ( it != m_breakMap.end() )
            m_breakMap.erase(it);
    }
}

///////////////////////////////////////////////////////////////////////////////

void BaseBreakpoint::clearBreakpointMap()
{
    m_breakMap.clear();
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult BaseBreakpoint::onBreakpoint( BREAKPOINT_ID bpId )
{
    boost::recursive_mutex::scoped_lock  l(m_breakMapLock);
    std::map<BREAKPOINT_ID, BaseBreakpoint*>::iterator it = m_breakMap.find(bpId);
    if ( it != m_breakMap.end() )
    {
        if ( it->second->m_callback )
            return it->second->m_callback->onBreakpoint();
    }

    return DebugCallbackBreak;
}

///////////////////////////////////////////////////////////////////////////////

void BaseBreakpoint::insertBreakpoint()
{
    HRESULT  hres;

    ULONG  breakId;
    hres = m_breakpoint->GetId(&breakId);
    if (S_OK != hres)
        throw DbgEngException( L"IDebugBreakpoint::GetId", hres);

    boost::recursive_mutex::scoped_lock  l(m_breakMapLock);
    m_breakMap[breakId] = this; 
}

///////////////////////////////////////////////////////////////////////////////

SoftwareBreakpoint::SoftwareBreakpoint( MEMOFFSET_64 offset, BreakpointCallback* callback) 
    : BaseBreakpoint(callback)
{
    HRESULT  hres;

    hres = g_dbgMgr->control->AddBreakpoint(
        DEBUG_BREAKPOINT_CODE,
        DEBUG_ANY_ID,
        &m_breakpoint);

    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::AddBreakpoint", hres);

    hres = m_breakpoint->SetOffset(offset);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugBreakpoint::SetOffset", hres);
    
    ULONG bpFlags;
    hres = m_breakpoint->GetFlags(&bpFlags);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugBreakpoint::GetFlags", hres);

    bpFlags |= DEBUG_BREAKPOINT_ENABLED | DEBUG_BREAKPOINT_ADDER_ONLY | DEBUG_BREAKPOINT_GO_ONLY;
    hres = m_breakpoint->SetFlags(bpFlags);
    if (S_OK != hres)
        throw DbgEngException( L"IDebugBreakpoint::SetFlags", hres);

    insertBreakpoint();
}

///////////////////////////////////////////////////////////////////////////////

HardwareBreakpoint::HardwareBreakpoint( MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType, BreakpointCallback* callback)
    : BaseBreakpoint(callback)
{
    HRESULT  hres;

    hres = g_dbgMgr->control->AddBreakpoint(
        DEBUG_BREAKPOINT_DATA,
        DEBUG_ANY_ID,
        &m_breakpoint);

    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::AddBreakpoint", hres);

    hres = m_breakpoint->SetOffset(offset);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugBreakpoint::SetOffset", hres);

    ULONG bpFlags;
    hres = m_breakpoint->GetFlags(&bpFlags);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugBreakpoint::GetFlags", hres);

    hres = m_breakpoint->SetDataParameters(static_cast<ULONG>(size), accessType);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugBreakpoint::SetDataParameters", hres);

    bpFlags |= DEBUG_BREAKPOINT_ENABLED | DEBUG_BREAKPOINT_ADDER_ONLY | DEBUG_BREAKPOINT_GO_ONLY;
    hres = m_breakpoint->SetFlags(bpFlags);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugBreakpoint::SetFlags", hres);
}

///////////////////////////////////////////////////////////////////////////////

BREAKPOINT_ID softwareBreakPointSet( MEMOFFSET_64 offset )
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

    ULONG  breakId;
    hres = bp->GetId(&breakId);
    if (S_OK != hres)
    {
        g_dbgMgr->control->RemoveBreakpoint(bp);
        throw DbgEngException( L"IDebugBreakpoint::GetId", hres);
    }

    return breakId;
}

///////////////////////////////////////////////////////////////////////////////

BREAKPOINT_ID hardwareBreakPointSet( MEMOFFSET_64 offset, size_t size, ACCESS_TYPE accessType )
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

    ULONG  breakId;
    hres = bp->GetId(&breakId);
    if (S_OK != hres)
    {
        g_dbgMgr->control->RemoveBreakpoint(bp);
        throw DbgEngException(L"IDebugBreakpoint::GetId", hres);
    }

    return breakId;
}

///////////////////////////////////////////////////////////////////////////////

void breakPointRemove( BREAKPOINT_ID id )
{
    IDebugBreakpoint *bp;
    HRESULT hres = g_dbgMgr->control->GetBreakpointById(id, &bp);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::GetBreakpointById", hres);

    hres = g_dbgMgr->control->RemoveBreakpoint(bp);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::RemoveBreakpoint", hres);
}

///////////////////////////////////////////////////////////////////////////////

}
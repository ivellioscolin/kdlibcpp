#include "stdafx.h"

#include <map>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/atomic.hpp>

#include "processmon.h"

namespace kdlib
{

///////////////////////////////////////////////////////////////////////////////

class ProcessInfo;
typedef boost::shared_ptr<ProcessInfo>  ProcessInfoPtr;

///////////////////////////////////////////////////////////////////////////////

class ProcessInfo {

public:

    ModulePtr getModule(MEMOFFSET_64  offset);
    void insertModule( ModulePtr& module);
    void removeModule(MEMOFFSET_64  offset );

    void insertBreakpoint(BREAKPOINT_ID bpId,  BreakpointInfoPtr& breakpoint);
    BreakpointInfoPtr  removeBreakpoint(BREAKPOINT_ID bpId);

    DebugCallbackResult breakpointHit( MEMOFFSET_64 offset, BreakpointType breakpointType);

private:

    typedef std::map<MEMOFFSET_64, ModulePtr> ModuleMap;
    ModuleMap  m_moduleMap;
    boost::recursive_mutex  m_moduleLock;


    typedef std::map<BREAKPOINT_ID, BreakpointInfoPtr>  BreakpointMap;
    BreakpointMap  m_breakpointMap;
    typedef std::map<MEMOFFSET_64, BreakpointInfoPtr>  BreakpointMap;
    BreakpointMap  m_softBpOffsetMap;
    BreakpointMap  m_accessBpOffsetMap;

    boost::recursive_mutex  m_breakpointLock;
};

///////////////////////////////////////////////////////////////////////////////

class ProcessMonitorImpl {

public:

    ProcessMonitorImpl() : m_bpUnique(0x80000000)
    {}

public:

    void processStart( PROCESS_DEBUG_ID id );
    void processStop( PROCESS_DEBUG_ID id );
    void processAllStop();

    void moduleLoad( PROCESS_DEBUG_ID id, MEMOFFSET_64  offset );
    void moduleUnload( PROCESS_DEBUG_ID id, MEMOFFSET_64  offset );
    DebugCallbackResult breakpointHit( PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, BreakpointType breakpointType);

    ModulePtr getModule( MEMOFFSET_64  offset, PROCESS_DEBUG_ID id );
    void insertModule( ModulePtr& module, PROCESS_DEBUG_ID id );

    BREAKPOINT_ID insertBreakpoint( BreakpointInfoPtr& breakpoint, PROCESS_DEBUG_ID id = -1 );
    void removeBreakpoint( BREAKPOINT_ID  bpid );

private:

    ProcessInfoPtr  getProcess( PROCESS_DEBUG_ID id );

    boost::recursive_mutex  m_lock;

    typedef std::map<PROCESS_DEBUG_ID, ProcessInfoPtr>  ProcessMap;
    ProcessMap  m_processMap;

    boost::atomic<unsigned long long>  m_bpUnique;

};

std::auto_ptr<ProcessMonitorImpl>  g_procmon;

///////////////////////////////////////////////////////////////////////////////

class CurrentProcessGuard {

public:

    CurrentProcessGuard() :
        m_procId( getCurrentProcessId() )
        {}

    ~CurrentProcessGuard() {
        setCurrentProcessById(m_procId);
    }

private:

    PROCESS_DEBUG_ID  m_procId;
};

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::init()
{
    g_procmon.reset( new ProcessMonitorImpl );
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::deinit()
{
    g_procmon.reset(0);
}

/////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::processStart( PROCESS_DEBUG_ID id )
{
    g_procmon->processStart(id);
}

/////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::processStop( PROCESS_DEBUG_ID id )
{
    g_procmon->processStart(id);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::processAllStop()
{
    g_procmon->processAllStop();
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::moduleLoad( PROCESS_DEBUG_ID id, MEMOFFSET_64 offset )
{
    g_procmon->moduleLoad(id, offset);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::moduleUnload( PROCESS_DEBUG_ID id, MEMOFFSET_64  offset )
{
    g_procmon->moduleUnload(id, offset);
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitor::breakpointHit( PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, BreakpointType breakpointType)
{
    return g_procmon->breakpointHit(id, offset, breakpointType);
}

///////////////////////////////////////////////////////////////////////////////

ModulePtr ProcessMonitor::getModule( MEMOFFSET_64  offset, PROCESS_DEBUG_ID id )
{
    if ( id == -1 )
        id = getCurrentProcessId();

    return g_procmon->getModule(offset,id);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::insertModule( ModulePtr& module, PROCESS_DEBUG_ID id )
{
    if ( id == -1 )
        id = getCurrentProcessId();

    return g_procmon->insertModule(module, id);
}

///////////////////////////////////////////////////////////////////////////////

BREAKPOINT_ID ProcessMonitor::insertBreakpoint( BreakpointInfoPtr& breakpoint, PROCESS_DEBUG_ID id )
{
    if ( id == -1 )
        id = getCurrentProcessId();

    return g_procmon->insertBreakpoint( breakpoint, id );
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::removeBreakpoint( BREAKPOINT_ID  bpid )
{
    g_procmon->removeBreakpoint(bpid);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::processStart( PROCESS_DEBUG_ID id )
{
    ProcessInfoPtr  proc = ProcessInfoPtr( new ProcessInfo() );
    boost::recursive_mutex::scoped_lock l(m_lock);
    m_processMap[id] = proc;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::processStop( PROCESS_DEBUG_ID id )
{
    boost::recursive_mutex::scoped_lock l(m_lock);
    m_processMap.erase(id);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::processAllStop()
{
    boost::recursive_mutex::scoped_lock l(m_lock);
    m_processMap.clear();
}

///////////////////////////////////////////////////////////////////////////////

ModulePtr ProcessMonitorImpl::getModule( MEMOFFSET_64  offset, PROCESS_DEBUG_ID id )
{
    ProcessInfoPtr  processInfo = getProcess(id);

    ModulePtr  module;

    if ( processInfo )
        module = processInfo->getModule(offset);

    return module;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::moduleLoad( PROCESS_DEBUG_ID id, MEMOFFSET_64  offset )
{
    ProcessInfoPtr  processInfo = getProcess(id);

    if ( processInfo )
    {
        processInfo->removeModule( offset );
        loadModule(offset);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::moduleUnload( PROCESS_DEBUG_ID id, MEMOFFSET_64  offset )
{
    ProcessInfoPtr  processInfo = getProcess(id);

    if ( processInfo )
        processInfo->removeModule( offset );
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitorImpl::breakpointHit( PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, BreakpointType breakpointType)
{
    ProcessInfoPtr  processInfo = getProcess(id);
    if ( processInfo )
        return processInfo->breakpointHit( offset, breakpointType );

    return DebugCallbackNoChange;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::insertModule( ModulePtr& module, PROCESS_DEBUG_ID id )
{
    ProcessInfoPtr  processInfo = getProcess(id);
    if ( processInfo )
        return processInfo->insertModule(module);
}

///////////////////////////////////////////////////////////////////////////////

BREAKPOINT_ID ProcessMonitorImpl::insertBreakpoint( BreakpointInfoPtr& breakpoint, PROCESS_DEBUG_ID id )
{
    ProcessInfoPtr  processInfo = getProcess(id);

    BREAKPOINT_ID  bpId = -1;

    if ( processInfo )
    {
        bpId = ++m_bpUnique;
        processInfo->insertBreakpoint(bpId, breakpoint);
    }

    return bpId;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::removeBreakpoint( BREAKPOINT_ID  bpid )
{
    boost::recursive_mutex::scoped_lock l(m_lock);

    for ( ProcessMap::iterator  it = m_processMap.begin(); it != m_processMap.end(); ++it )
    {
        BreakpointInfoPtr  bp = it->second->removeBreakpoint(bpid);
        if ( bp )
        {
            if ( it->first != getCurrentProcessId() )
            {
                CurrentProcessGuard  g;
                setCurrentProcessById( it->first );
                bp->remove();
            }
            else
            {
                bp->remove();
            }
        }
    }

}

///////////////////////////////////////////////////////////////////////////////

ProcessInfoPtr ProcessMonitorImpl::getProcess( PROCESS_DEBUG_ID id )
{
    boost::recursive_mutex::scoped_lock l(m_lock);
    return  m_processMap[id];
}

///////////////////////////////////////////////////////////////////////////////

ModulePtr ProcessInfo::getModule(MEMOFFSET_64  offset)
{
    boost::recursive_mutex::scoped_lock l(m_moduleLock);
    return m_moduleMap[ offset ];
}

///////////////////////////////////////////////////////////////////////////////

void ProcessInfo::insertModule( ModulePtr& module)
{
    boost::recursive_mutex::scoped_lock l(m_moduleLock);
    m_moduleMap[ module->getBase() ] = module;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessInfo::removeModule(MEMOFFSET_64  offset )
{
    boost::recursive_mutex::scoped_lock l(m_moduleLock);
    m_moduleMap.erase(offset);
}

///////////////////////////////////////////////////////////////////////////////

void  ProcessInfo::insertBreakpoint(BREAKPOINT_ID bpId,  BreakpointInfoPtr& breakpoint)
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);
    
    m_breakpointMap[bpId] = breakpoint;

    switch( breakpoint->getType() )
    {
    case SoftwareBreakpoint:
        m_softBpOffsetMap[breakpoint->getOffset()] = breakpoint;
        break;
    case DataAccessBreakpoint:
        m_accessBpOffsetMap[breakpoint->getOffset()] = breakpoint;
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////

BreakpointInfoPtr  ProcessInfo::removeBreakpoint(BREAKPOINT_ID bpId)
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);

    BreakpointInfoPtr breakpoint = m_breakpointMap[bpId];
    if ( breakpoint )
    {
        m_breakpointMap.erase(bpId);

        switch( breakpoint->getType() )
        {
        case SoftwareBreakpoint:
            m_softBpOffsetMap.erase( breakpoint->getOffset() );
            break;
        case DataAccessBreakpoint:
            m_accessBpOffsetMap.erase( breakpoint->getOffset() );
            break;
        }
    }

    return breakpoint;
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessInfo::breakpointHit( MEMOFFSET_64 offset, BreakpointType breakpointType)
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);

    BreakpointInfoPtr breakpoint;

    switch( breakpointType )
    {
    case SoftwareBreakpoint:
        breakpoint = m_softBpOffsetMap[offset];
        break;
    case DataAccessBreakpoint:
        breakpoint = m_accessBpOffsetMap[offset];
        break;
    }
    
    if ( !breakpoint )
        return DebugCallbackNoChange;
        
    if ( !breakpoint->getCallback() )
        return DebugCallbackBreak;

    return breakpoint->getCallback()->onHit();
}

///////////////////////////////////////////////////////////////////////////////











///////////////////////////////////////////////////////////////////////////////
//
//ProcessInfoPtr ProcessMonitor::getProcessInfo( PROCESS_DEBUG_ID id)
//{
//    if ( id == -1 )
//        id = getCurrentProcessId();
//
//    boost::recursive_mutex::scoped_lock  l(m_processLock);
//
//    ProcessInfoPtr  procInfo = m_processMap[id];
//
//    if ( !procInfo )
//    {
//        procInfo = ProcessInfoPtr( new ProcessInfo() );
//
//        m_processMap.insert( std::make_pair(id, procInfo) );
//    }
//
//    return procInfo;
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//ModulePtr ProcessInfo::getModule(MEMOFFSET_64  offset)
//{
//    boost::recursive_mutex::scoped_lock  l(m_moduleLock);
//    return m_moduleMap[offset];
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void ProcessInfo::insertModule(ModulePtr& module)
//{
//    MEMOFFSET_64  offset = module->getBase();
//    boost::recursive_mutex::scoped_lock  l(m_moduleLock);
//    m_moduleMap[offset] = module;
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void ProcessInfo::insertSoftwareBp(Breakpoint* bp)
//{
//    MEMOFFSET_64  offset = bp->getOffset();
//    m_softBpMap[offset] = bp;
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void ProcessInfo::moduleLoad(MEMOFFSET_64  offset)
//{
//    insertModule( loadModule(offset) );
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void ProcessInfo::moduleUnload(MEMOFFSET_64  offset)
//{
//    boost::recursive_mutex::scoped_lock  l(m_moduleLock);
//    m_moduleMap.erase(offset);
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//DebugCallbackResult ProcessInfo::breakpointHit(MEMOFFSET_64 offset, bool hardware)
//{
//    boost::recursive_mutex::scoped_lock l(m_bpLock);
//
//    Breakpoint  *bp = hardware ? m_hardBpMap[offset] : m_softBpMap[offset];
//
//    if ( bp )
//        return bp->onHit();
//
//    return DebugCallbackNoChange;
//}
//
/////////////////////////////////////////////////////////////////////////////////


} //namesapce kdlib


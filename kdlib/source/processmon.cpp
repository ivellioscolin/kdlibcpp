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

    void insertBreakpoint(BREAKPOINT_ID bpId,  BreakpointPtr& breakpoint);
    BreakpointPtr removeBreakpoint(BREAKPOINT_ID bpId);
    unsigned long getNumberBreakpoints();
    BreakpointPtr getBreakpointByIndex(unsigned long index);
    BreakpointPtr getBreakpointById(BREAKPOINT_ID  bpid );

    DebugCallbackResult breakpointHit( MEMOFFSET_64 offset, BreakpointType breakpointType);

private:

    typedef std::map<MEMOFFSET_64, ModulePtr> ModuleMap;
    ModuleMap  m_moduleMap;
    boost::recursive_mutex  m_moduleLock;


    typedef std::map<BREAKPOINT_ID, BreakpointPtr>  BreakpointIdMap;
    BreakpointIdMap  m_breakpointMap;

    typedef std::map<MEMOFFSET_64, BreakpointPtr>  BreakpointOffsetMap;
    BreakpointOffsetMap  m_softBpOffsetMap;
    BreakpointOffsetMap  m_accessBpOffsetMap;

    boost::recursive_mutex  m_breakpointLock;
};

///////////////////////////////////////////////////////////////////////////////

class ProcessMonitorImpl {

public:

    ProcessMonitorImpl() : m_bpUnique(0x80000000)
    {}

public:

    DebugCallbackResult processStart(PROCESS_DEBUG_ID id);
    DebugCallbackResult processStop(PROCESS_DEBUG_ID id, ProcessExitReason reason, unsigned int ExitCode);
    void processAllStop();
    unsigned int getNumberProcesses();

    DebugCallbackResult moduleLoad(PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, const std::wstring& moduleName);
    DebugCallbackResult moduleUnload(PROCESS_DEBUG_ID id, MEMOFFSET_64  offset, const std::wstring& moduleName);
    DebugCallbackResult breakpointHit( PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, BreakpointType breakpointType);
    void currentThreadChange(THREAD_DEBUG_ID threadid);
    void executionStatusChange(ExecutionStatus status);
    void changeLocalScope();
    DebugCallbackResult  exceptionHit(const ExceptionInfo& excinfo);

    ModulePtr getModule( MEMOFFSET_64  offset, PROCESS_DEBUG_ID id );
    void insertModule( ModulePtr& module, PROCESS_DEBUG_ID id );

    BREAKPOINT_ID insertBreakpoint( BreakpointPtr& breakpoint, PROCESS_DEBUG_ID id);
    void removeBreakpoint( BREAKPOINT_ID  bpid );
    unsigned long getNumberBreakpoints(PROCESS_DEBUG_ID processid);
    BreakpointPtr getBreakpointByIndex(unsigned long index, PROCESS_DEBUG_ID processid);
    BreakpointPtr getBreakpointById(BREAKPOINT_ID bpid);

    void registerEventsCallback(DebugEventsCallback *callback);
    void removeEventsCallback(DebugEventsCallback *callback);

private:

    ProcessInfoPtr  getProcess( PROCESS_DEBUG_ID id );

    boost::recursive_mutex  m_lock;

    typedef std::map<PROCESS_DEBUG_ID, ProcessInfoPtr>  ProcessMap;
    ProcessMap  m_processMap;

    boost::atomic<unsigned long long>  m_bpUnique;

private:

    typedef std::list<DebugEventsCallback*>  EventsCallbackList;

    boost::recursive_mutex      m_callbacksLock;
    EventsCallbackList          m_callbacks;
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

void ProcessMonitor::registerEventsCallback(DebugEventsCallback *callback)
{
    g_procmon->registerEventsCallback(callback);
}

/////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::removeEventsCallback(DebugEventsCallback *callback)
{
    g_procmon->removeEventsCallback(callback);
}

/////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitor::processStart(PROCESS_DEBUG_ID id)
{
    return g_procmon->processStart(id);
}

/////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitor::processStop(PROCESS_DEBUG_ID id, ProcessExitReason reason, unsigned int exitCode)
{
    return g_procmon->processStop(id, reason, exitCode);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::processAllStop()
{
    g_procmon->processAllStop();
}

///////////////////////////////////////////////////////////////////////////////

unsigned int ProcessMonitor::getNumberProcesses()
{
    return g_procmon->getNumberProcesses();
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitor::moduleLoad(PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, const std::wstring& moduleName)
{
    return g_procmon->moduleLoad(id, offset, moduleName);
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitor::moduleUnload(PROCESS_DEBUG_ID id, MEMOFFSET_64  offset, const std::wstring& moduleName)
{
    return g_procmon->moduleUnload(id, offset, moduleName);
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitor::breakpointHit( PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, BreakpointType breakpointType)
{
    return g_procmon->breakpointHit(id, offset, breakpointType);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::currentThreadChange(THREAD_DEBUG_ID id)
{
    g_procmon->currentThreadChange(id);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::executionStatusChange(ExecutionStatus status)
{
    g_procmon->executionStatusChange(status);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::changeLocalScope()
{
    g_procmon->changeLocalScope();
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitor::exceptionHit(const ExceptionInfo& excinfo)
{
    return g_procmon->exceptionHit(excinfo);
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

BREAKPOINT_ID ProcessMonitor::insertBreakpoint( BreakpointPtr& breakpoint, PROCESS_DEBUG_ID id )
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

unsigned long ProcessMonitor::getNumberBreakpoints(PROCESS_DEBUG_ID processid)
{
    if ( processid == -1 )
        processid = getCurrentProcessId();

    return g_procmon->getNumberBreakpoints(processid);
}

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr ProcessMonitor::getBreakpointByIndex(unsigned long index, PROCESS_DEBUG_ID processid)
{
    if ( processid == -1 )
        processid = getCurrentProcessId();

    return g_procmon->getBreakpointByIndex(index, processid);
}

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr ProcessMonitor::getBreakpointById(BREAKPOINT_ID bpid )
{
    return g_procmon->getBreakpointById(bpid);
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitorImpl::processStart(PROCESS_DEBUG_ID id)
{
    {
        ProcessInfoPtr  proc = ProcessInfoPtr(new ProcessInfo());
        boost::recursive_mutex::scoped_lock l(m_lock);
        m_processMap[id] = proc;
    }

    DebugCallbackResult  result = DebugCallbackNoChange;

    return result;
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitorImpl::processStop(PROCESS_DEBUG_ID id, ProcessExitReason reason, unsigned int exitCode)
{
    {
        boost::recursive_mutex::scoped_lock l(m_lock);
        m_processMap.erase(id);
    }

    DebugCallbackResult  result = DebugCallbackNoChange;

    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it = m_callbacks.begin();

    for (; it != m_callbacks.end(); ++it)
    {
        DebugCallbackResult  ret = (*it)->onProcessExit(id, reason, exitCode);
        result = ret != DebugCallbackNoChange ? ret : result;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::processAllStop()
{
    boost::recursive_mutex::scoped_lock l(m_lock);
    m_processMap.clear();
}

///////////////////////////////////////////////////////////////////////////////

unsigned int ProcessMonitorImpl::getNumberProcesses()
{
    boost::recursive_mutex::scoped_lock l(m_lock);
    return m_processMap.size();
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

DebugCallbackResult ProcessMonitorImpl::moduleLoad(PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, const std::wstring& moduleName)
{
    DebugCallbackResult  result = DebugCallbackNoChange;

    ProcessInfoPtr  processInfo = getProcess(id);

    if ( processInfo )
    {
        processInfo->removeModule( offset );
        loadModule(offset);
    }

    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it;
    for (it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
    {
        DebugCallbackResult  ret = (*it)->onModuleLoad(offset, moduleName);
        result = ret != DebugCallbackNoChange ? ret : result;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitorImpl::moduleUnload(PROCESS_DEBUG_ID id, MEMOFFSET_64  offset, const std::wstring &moduleName)
{
    DebugCallbackResult  result = DebugCallbackNoChange;

    ProcessInfoPtr  processInfo = getProcess(id);

    if ( processInfo )
        processInfo->removeModule( offset );

    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it;
    for ( it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
    {
        DebugCallbackResult  ret = (*it)->onModuleUnload(offset, moduleName );
        result = ret != DebugCallbackNoChange ? ret : result;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitorImpl::breakpointHit( PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, BreakpointType breakpointType)
{
    DebugCallbackResult  result = DebugCallbackNoChange;

    ProcessInfoPtr  processInfo = getProcess(id);
    if ( processInfo )
        result = processInfo->breakpointHit( offset, breakpointType );

    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it;
    for (it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
    {
        DebugCallbackResult  ret = (*it)->onBreakpoint(id);
        result = ret != DebugCallbackNoChange ? ret : result;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::currentThreadChange(THREAD_DEBUG_ID threadid)
{
    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it = m_callbacks.begin();

    for (; it != m_callbacks.end(); ++it)
    {
        (*it)->onCurrentThreadChange(threadid);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::executionStatusChange(ExecutionStatus status)
{
    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it = m_callbacks.begin();

    for (; it != m_callbacks.end(); ++it)
    {
        (*it)->onExecutionStatusChange(status);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::changeLocalScope()
{
    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it = m_callbacks.begin();

    for (; it != m_callbacks.end(); ++it)
    {
        (*it)->onChangeLocalScope();
    }
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult  ProcessMonitorImpl::exceptionHit(const ExceptionInfo& excinfo)
{
    DebugCallbackResult  result = DebugCallbackNoChange;

    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it;
    for (it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
    {
        DebugCallbackResult  ret = (*it)->onException(excinfo);
        result = ret != DebugCallbackNoChange ? ret : result;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::insertModule( ModulePtr& module, PROCESS_DEBUG_ID id )
{
    ProcessInfoPtr  processInfo = getProcess(id);
    if ( processInfo )
        return processInfo->insertModule(module);
}

///////////////////////////////////////////////////////////////////////////////

BREAKPOINT_ID ProcessMonitorImpl::insertBreakpoint( BreakpointPtr& breakpoint, PROCESS_DEBUG_ID id )
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

/////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::removeBreakpoint( BREAKPOINT_ID  bpid )
{
    boost::recursive_mutex::scoped_lock l(m_lock);

    for ( ProcessMap::iterator  it = m_processMap.begin(); it != m_processMap.end(); ++it )
    {
        BreakpointPtr  bp = it->second->removeBreakpoint(bpid);
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

unsigned long ProcessMonitorImpl::getNumberBreakpoints(PROCESS_DEBUG_ID processid)
{
    unsigned long  breakpointNumber = 0;

    ProcessInfoPtr  processInfo = getProcess(processid);
    if ( processInfo )
    {
        return processInfo->getNumberBreakpoints();
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr ProcessMonitorImpl::getBreakpointByIndex(unsigned long index, PROCESS_DEBUG_ID processid)
{

    BreakpointPtr bp;
    ProcessInfoPtr  processInfo = getProcess(processid);
    if ( processInfo )
    {
        return processInfo->getBreakpointByIndex(index);
    }

    return bp;
}

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr ProcessMonitorImpl::getBreakpointById(BREAKPOINT_ID bpid)
{
    BreakpointPtr  foundBp;

    boost::recursive_mutex::scoped_lock l(m_lock);

    for ( ProcessMap::iterator  it = m_processMap.begin(); it != m_processMap.end(); ++it )
    {
        foundBp = it->second->getBreakpointById(bpid);
        if (foundBp)
            break;
    }

    return foundBp;
}

///////////////////////////////////////////////////////////////////////////////

ProcessInfoPtr ProcessMonitorImpl::getProcess( PROCESS_DEBUG_ID id )
{
    boost::recursive_mutex::scoped_lock l(m_lock);

    ProcessMap::iterator  it  = m_processMap.find(id);

    if ( it != m_processMap.end() )
        return it->second;

    ProcessInfoPtr  proc = ProcessInfoPtr( new ProcessInfo() );
    m_processMap[id] = proc;

    return proc;
}


///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::registerEventsCallback(DebugEventsCallback *callback)
{
    boost::recursive_mutex::scoped_lock l(m_callbacksLock);
    m_callbacks.push_back(callback);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::removeEventsCallback(DebugEventsCallback *callback)
{
    boost::recursive_mutex::scoped_lock l(m_callbacksLock);
    m_callbacks.remove(callback);
}

///////////////////////////////////////////////////////////////////////////////

ModulePtr ProcessInfo::getModule(MEMOFFSET_64  offset)
{
    boost::recursive_mutex::scoped_lock l(m_moduleLock);

    ModuleMap::iterator it = m_moduleMap.find(offset);

    if ( it != m_moduleMap.end() )
        return it->second;

    for ( ModuleMap::iterator it = m_moduleMap.begin(); it != m_moduleMap.end(); ++it )
    {
        if ( it->second->getBase() <= offset && offset < it->second->getEnd() )
        {
           return it->second;
        }
    }

    return ModulePtr();
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

void  ProcessInfo::insertBreakpoint(BREAKPOINT_ID bpId,  BreakpointPtr& breakpoint)
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

/////////////////////////////////////////////////////////////////////////////

BreakpointPtr  ProcessInfo::removeBreakpoint(BREAKPOINT_ID bpId)
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);

    BreakpointIdMap::iterator   it = m_breakpointMap.find(bpId);
    if (it == m_breakpointMap.end() )
        return BreakpointPtr();

    BreakpointPtr breakpoint = it->second;

    m_breakpointMap.erase(it);

    switch( breakpoint->getType() )
    {
    case SoftwareBreakpoint:
        m_softBpOffsetMap.erase( breakpoint->getOffset() );
        break;
    case DataAccessBreakpoint:
        m_accessBpOffsetMap.erase( breakpoint->getOffset() );
        break;
    }

    BreakpointCallback*  callback = breakpoint->getCallback();
    if ( callback != 0 )
        callback->onRemove();

    return breakpoint;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long ProcessInfo::getNumberBreakpoints()
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);

    return m_breakpointMap.size();
}

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr ProcessInfo::getBreakpointByIndex(unsigned long index)
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);

    if ( index >= m_breakpointMap.size() )
        return BreakpointPtr();

    BreakpointIdMap::iterator  it = m_breakpointMap.begin();
    std::advance( it, index );
    return it->second;
}

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr ProcessInfo::getBreakpointById(BREAKPOINT_ID  bpid )
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);

    BreakpointIdMap::iterator   it = m_breakpointMap.find(bpid);
    if (it == m_breakpointMap.end() )
        return BreakpointPtr();

    return it->second;
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessInfo::breakpointHit( MEMOFFSET_64 offset, BreakpointType breakpointType)
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);

    BreakpointPtr breakpoint;

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
        
    BreakpointCallback*  callback = breakpoint->getCallback();
    if ( callback == 0 )
        return DebugCallbackBreak;

    return callback->onHit();
}

/////////////////////////////////////////////////////////////////////////////


} //namesapce kdlib


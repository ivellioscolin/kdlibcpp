#include "stdafx.h"

#include "processmon.h"

namespace kdlib
{

///////////////////////////////////////////////////////////////////////////////

ProcessMonitor::ProcessMap  ProcessMonitor::m_processMap;
boost::recursive_mutex  ProcessMonitor::m_processLock;

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::init()
{
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::deinit()
{
    processAllStop();
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::processStart( PROCESS_DEBUG_ID id )
{
    ProcessInfoPtr  procInfo = ProcessInfoPtr( new ProcessInfo() );

    boost::recursive_mutex::scoped_lock  l(m_processLock);

    m_processMap.insert( std::make_pair(id, procInfo) );
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::processStop( PROCESS_DEBUG_ID id )
{
    boost::recursive_mutex::scoped_lock  l(m_processLock);

    m_processMap.erase(id);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::processAllStop()
{
    boost::recursive_mutex::scoped_lock  l(m_processLock);

    m_processMap.clear();
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::moduleLoad( PROCESS_DEBUG_ID id, MEMOFFSET_64 offset )
{

    ProcessInfoPtr  processInfo = getProcessInfo(id);
    if ( processInfo )
    {
        processInfo->moduleLoad(offset);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::moduleUnload( PROCESS_DEBUG_ID id, MEMOFFSET_64 offset )
{
    ProcessInfoPtr  processInfo = getProcessInfo(id);
    if ( processInfo )
    {
        processInfo->moduleUnload(offset);
    }
}

///////////////////////////////////////////////////////////////////////////////

ProcessInfoPtr ProcessMonitor::getProcessInfo( PROCESS_DEBUG_ID id)
{
    if ( id == -1 )
        id = getCurrentProcessId();

    boost::recursive_mutex::scoped_lock  l(m_processLock);

    ProcessInfoPtr  procInfo = m_processMap[id];

    if ( !procInfo )
    {
        procInfo = ProcessInfoPtr( new ProcessInfo() );

        m_processMap.insert( std::make_pair(id, procInfo) );
    }

    return procInfo;
}

///////////////////////////////////////////////////////////////////////////////

ModulePtr ProcessInfo::getModule(MEMOFFSET_64  offset)
{
    boost::recursive_mutex::scoped_lock  l(m_moduleLock);
    return m_moduleMap[offset];
}

///////////////////////////////////////////////////////////////////////////////

void ProcessInfo::insertModule(ModulePtr& module)
{
    MEMOFFSET_64  offset = module->getBase();
    boost::recursive_mutex::scoped_lock  l(m_moduleLock);
    m_moduleMap[offset] = module;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessInfo::moduleLoad(MEMOFFSET_64  offset)
{
    insertModule( loadModule(offset) );
}

///////////////////////////////////////////////////////////////////////////////

void ProcessInfo::moduleUnload(MEMOFFSET_64  offset)
{
    boost::recursive_mutex::scoped_lock  l(m_moduleLock);
    m_moduleMap.erase(offset);
}

///////////////////////////////////////////////////////////////////////////////


} //namesapce kdlib


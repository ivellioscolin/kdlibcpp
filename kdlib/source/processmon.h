#pragma once

#include <map>

#include <boost/thread/recursive_mutex.hpp>

#include "kdlib/dbgtypedef.h"
#include "kdlib/module.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class ProcessInfo {

public:

    ModulePtr getModule(MEMOFFSET_64  offset);
    void insertModule(ModulePtr& module);

    void moduleLoad(MEMOFFSET_64  offset);
    void moduleUnload(MEMOFFSET_64  offset);

private:

    typedef std::map<MEMOFFSET_64, ModulePtr> ModuleMap;
    ModuleMap  m_moduleMap;
    boost::recursive_mutex  m_moduleLock;
};

typedef boost::shared_ptr<ProcessInfo> ProcessInfoPtr;

///////////////////////////////////////////////////////////////////////////////

class ProcessMonitor {

public:

    static void init();
    static void deinit();
    static void processStart( PROCESS_DEBUG_ID id );
    static void processStop( PROCESS_DEBUG_ID id );
    static void processAllStop();

    static ProcessInfoPtr getProcessInfo( PROCESS_DEBUG_ID id = -1);

    static void moduleLoad( PROCESS_DEBUG_ID id, MEMOFFSET_64  offset );
    static void moduleUnload( PROCESS_DEBUG_ID id, MEMOFFSET_64  offset );


private:

    typedef std::map<PROCESS_DEBUG_ID, ProcessInfoPtr> ProcessMap;

    static ProcessMap  m_processMap;
    static boost::recursive_mutex  m_processLock;

};

///////////////////////////////////////////////////////////////////////////////

} // namespace kdlib

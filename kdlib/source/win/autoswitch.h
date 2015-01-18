#pragma once

#include <vector>

#include "kdlib/dbgtypedef.h"

#include "win/dbgmgr.h"
#include "win/threadctx.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class ContextAutoRestore {

public:

    ContextAutoRestore();

    virtual ~ContextAutoRestore();

protected:

    THREAD_DEBUG_ID    m_currentThread;
    PROCESS_DEBUG_ID   m_currentProcess;
    bool  m_savedRegCtx;
    std::vector<DEBUG_VALUE>  m_regValues;
    bool m_savedLocalContext;
    CONTEXT_STORAGE  m_localContext;



    //    bool  m_savedRegCtx;
    //
    //    CONTEXT_STORAGE  m_localContext;
    //
    //    bool m_savedLocalContext;
};


///////////////////////////////////////////////////////////////////////////////

} // namesapce kdlib


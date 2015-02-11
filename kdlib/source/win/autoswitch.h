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
    DEBUG_STACK_FRAME  m_currentFrame;
};


///////////////////////////////////////////////////////////////////////////////

} // namesapce kdlib


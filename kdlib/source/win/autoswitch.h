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

    bool  m_quietState;

    THREAD_DEBUG_ID    m_currentThread;
    PROCESS_DEBUG_ID   m_currentProcess;
    SYSTEM_DEBUG_ID    m_currentSystem;

    bool  m_savedRegCtx;
    std::vector<DEBUG_VALUE>  m_regValues;

    bool m_savedLocalContext;
    CONTEXT_STORAGE  m_localContext;

    bool m_savedCurrentFrame;
    DEBUG_STACK_FRAME  m_currentFrame;
};


///////////////////////////////////////////////////////////////////////////////

} // namesapce kdlib


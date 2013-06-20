#include "stdafx.h"

#include "kdlib/cpucontext.h"
#include "kdlib/dbgengine.h"
#include "kdlib/exceptions.h"

namespace kdlib {

/////////////////////////////////////////////////////////////

CPUContext::CPUContext( size_t index )
{
    if ( index == -1 )
    {
        index = getCurrentThreadId();
    }
    else
    {
        if ( index >= getNumberThreads() )
            throw IndexException(index);
    }

    m_contextIndex = index;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 CPUContext::getIP()
{
    return  getInstructionOffset( m_contextIndex );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 CPUContext::getSP()
{
     return  getStackOffset( m_contextIndex );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 CPUContext::getFP()
{
    return  getFrameOffset( m_contextIndex );
}

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end

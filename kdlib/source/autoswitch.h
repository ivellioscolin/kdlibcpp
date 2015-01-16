#pragma once

#include "kdlib/dbgtypedef.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class ProcessAutoSwitch {

public:

    ProcessAutoSwitch(PROCESS_DEBUG_ID id);

    ~ProcessAutoSwitch();

private:

    PROCESS_DEBUG_ID     m_previousId;
};

///////////////////////////////////////////////////////////////////////////////

} // namesapce kdlib


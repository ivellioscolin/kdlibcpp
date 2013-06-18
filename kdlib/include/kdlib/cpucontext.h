#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "kdlib/dbgtypedef.h"
#include "kdlib/variant.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class CPUContext;
typedef boost::shared_ptr<CPUContext>  CPUContextPtr;

CPUContextPtr loadCPUCurrentContext();
CPUContextPtr loadCPUContextByIndex( size_t index );

inline CPUContextPtr loadCPUContext() {
    return loadCPUCurrentContext();
}

inline CPUContextPtr loadCPUContext( size_t index ) {
    return loadCPUContextByIndex( index );
}

class CPUContext : private boost::noncopyable {

public:

    CPUType getCPUType();
    CPUType getCPUMode();
    NumVariant getRegisterByName( const std::wstring &name);
    NumVariant getRegisterByIndex( size_t index );
    
    MEMOFFSET_64 getIP();
    MEMOFFSET_64 getSP();
};

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end

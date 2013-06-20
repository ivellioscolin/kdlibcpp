#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "kdlib/dbgtypedef.h"
#include "kdlib/variant.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class CPUContext;
typedef boost::shared_ptr<CPUContext>  CPUContextPtr;

class CPUContext : private boost::noncopyable {

public:

    CPUType getCPUType();
    CPUType getCPUMode();
    NumVariant getRegisterByName( const std::wstring &name);
    NumVariant getRegisterByIndex( size_t index );
    
    MEMOFFSET_64 getIP();
    MEMOFFSET_64 getSP();
    MEMOFFSET_64 getFP();

protected:

    friend CPUContextPtr loadCPUCurrentContext();
    friend CPUContextPtr loadCPUContextByIndex( size_t index );

    CPUContext( size_t index = -1 );

    size_t  m_contextIndex;
};


inline CPUContextPtr loadCPUCurrentContext() {
   return CPUContextPtr( new CPUContext() );
}

inline CPUContextPtr loadCPUContextByIndex( size_t index ) {
    return CPUContextPtr( new CPUContext(index) );
}

inline CPUContextPtr loadCPUContext() {
    return loadCPUCurrentContext();
}

inline CPUContextPtr loadCPUContext( size_t index ) {
    return loadCPUContextByIndex(index);
}
///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end

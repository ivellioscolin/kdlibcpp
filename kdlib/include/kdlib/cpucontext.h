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
    void setCPUMode( CPUType mode );
    void switchCPUMode();

    NumVariant getRegisterByName( const std::wstring &name);
    NumVariant getRegisterByIndex( unsigned long index );
    std::wstring getRegisterName( unsigned long index );

    MEMOFFSET_64 getIP();
    MEMOFFSET_64 getSP();
    MEMOFFSET_64 getFP();

    unsigned long getStackLength();
    void getStackFrame( unsigned long frameIndex, MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp );
    void getCurrentStackFrame( unsigned long frameIndex, MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp ) {
        getStackFrame( 0, ip, ret, fp, sp );
    }

    unsigned long long loadMSR( unsigned long msrIndex );
    void setMSR( unsigned long msrIndex, unsigned long long value );

protected:

    friend CPUContextPtr loadCPUCurrentContext();
    friend CPUContextPtr loadCPUContextByIndex( unsigned long index );

    CPUContext( unsigned long index = -1 );

    THREAD_ID  m_contextIndex;
};


inline CPUContextPtr loadCPUCurrentContext() {
   return CPUContextPtr( new CPUContext() );
}

inline CPUContextPtr loadCPUContextByIndex( unsigned long index ) {
    return CPUContextPtr( new CPUContext(index) );
}

inline CPUContextPtr loadCPUContext() {
    return loadCPUCurrentContext();
}

inline CPUContextPtr loadCPUContext( unsigned long index ) {
    return loadCPUContextByIndex(index);
}
///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end

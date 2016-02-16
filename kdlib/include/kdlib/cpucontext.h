#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "kdlib/dbgtypedef.h"
#include "kdlib/variant.h"
#include "kdlib/stack.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class CPUContext;
typedef boost::shared_ptr<CPUContext>  CPUContextPtr;

class  CPUContext : private boost::noncopyable {

public:

    static CPUContextPtr loadCPUCurrentContext();

public:

    virtual CPUType getCPUType() = 0;
    virtual CPUType getCPUMode() = 0;

    virtual NumVariant getRegisterByName( const std::wstring &name) = 0;
    virtual NumVariant getRegisterByIndex( unsigned long index ) = 0;
    virtual std::wstring getRegisterName( unsigned long index ) = 0;
    virtual unsigned long getRegisterNumber() = 0;
    virtual DataAccessorPtr getRegisterAccessor(unsigned long regId) = 0;

    virtual MEMOFFSET_64 getIP() = 0;
    virtual MEMOFFSET_64 getSP() = 0;
    virtual MEMOFFSET_64 getFP() = 0;

    virtual StackPtr getStack() = 0;
};

inline CPUContextPtr loadCPUContext() {
    return CPUContext::loadCPUCurrentContext();
}

StackPtr getStackFromContext(CPUContextPtr &ctx);
StackFramePtr getStackFrameFromContext(CPUContextPtr &ctx);

///////////////////////////////////////////////////////////////////////////////

NumVariant getRegisterByName(const std::wstring& regName);
NumVariant getRegisterByIndex(unsigned long regIndex);

///////////////////////////////////////////////////////////////////////////////


} // kdlib namespace end

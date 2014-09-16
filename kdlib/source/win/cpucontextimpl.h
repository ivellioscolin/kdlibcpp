#pragma once

#include "kdlib/cpucontext.h"

#include "threadctx.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class CPUContextImpl : public CPUContext
{

public:

    CPUContextImpl();

protected:

    virtual CPUType getCPUType() {
        return m_cpuType;
    }

    virtual CPUType getCPUMode() {
        return m_cpuMode;
    }

    virtual NumVariant getRegisterByName( const std::wstring &name);

    virtual NumVariant getRegisterByIndex( unsigned long index );

    virtual std::wstring getRegisterName( unsigned long index );

    virtual unsigned long getRegisterNumber() {
        return m_values.size();
    }

    virtual MEMOFFSET_64 getIP() {
        return m_ip;
    }

    virtual MEMOFFSET_64 getSP() {
        return m_sp;
    }

    virtual MEMOFFSET_64 getFP() {
        return m_fp;
    }


    virtual unsigned long long loadMSR( unsigned long msrIndex ) {
        NOT_IMPLEMENTED();
    }

    virtual void setMSR( unsigned long msrIndex, unsigned long long value ) {
        NOT_IMPLEMENTED();
    }

protected:

    CPUType  m_cpuType;
    CPUType  m_cpuMode;

    std::vector< std::pair<std::wstring, NumVariant> >  m_values;
    
    MEMOFFSET_64  m_ip;
    MEMOFFSET_64  m_sp;
    MEMOFFSET_64  m_fp;

};

///////////////////////////////////////////////////////////////////////////////

}

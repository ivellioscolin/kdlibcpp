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
        NOT_IMPLEMENTED();
    }

    virtual CPUType getCPUMode() {
        NOT_IMPLEMENTED();
    }

    virtual void setCPUMode( CPUType mode ) {
        NOT_IMPLEMENTED();
    }

    virtual void switchCPUMode() {
        NOT_IMPLEMENTED();
    }

    virtual NumVariant getRegisterByName( const std::wstring &name) {
        NOT_IMPLEMENTED();
    }

    virtual NumVariant getRegisterByIndex( unsigned long index ) {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getRegisterName( unsigned long index ) {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getIP() {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getSP() {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getFP() {
        NOT_IMPLEMENTED();
    }

    virtual unsigned long getStackLength();

    virtual void getStackFrame( unsigned long frameIndex, MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp );
    
    virtual void getCurrentStackFrame( unsigned long frameIndex, MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp ) {
        NOT_IMPLEMENTED();
    }

    virtual unsigned long long loadMSR( unsigned long msrIndex ) {
        NOT_IMPLEMENTED();
    }

    virtual void setMSR( unsigned long msrIndex, unsigned long long value ) {
        NOT_IMPLEMENTED();
    }


private:

    ULONG  m_processorType;
    bool  m_wow64;

    union {
      CONTEXT_X86     m_ctx_x86;
      WOW64_CONTEXT   m_ctx_wow64;
      CONTEXT_X64     m_ctx_amd64;
    };

    void ReadWow64Context();

    void* getContext();
    
    unsigned long getContextSize();
};

///////////////////////////////////////////////////////////////////////////////

}

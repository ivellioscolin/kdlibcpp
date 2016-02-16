#pragma once

#include "kdlib/cpucontext.h"
#include "kdlib/memaccess.h"

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

    virtual DataAccessorPtr getRegisterAccessor(unsigned long regId) {
        NOT_IMPLEMENTED();
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

    virtual StackPtr getStack() {
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

class CPUx86Context : public CPUContext
{
public:

    CPUx86Context();

    explicit CPUx86Context(const CONTEXT_X86&  context) :
        m_context(context)
    {}

    virtual CPUType getCPUType() {
        return CPU_I386;
    }

    virtual CPUType getCPUMode() {
        return CPU_I386;
    }

    virtual NumVariant getRegisterByName(const std::wstring &name) {
        NOT_IMPLEMENTED();
    }
    virtual NumVariant getRegisterByIndex(unsigned long index) {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getRegisterName(unsigned long index) {
        NOT_IMPLEMENTED();
    }

    virtual unsigned long getRegisterNumber() {
        NOT_IMPLEMENTED();
    }

    virtual DataAccessorPtr getRegisterAccessor(unsigned long regId) {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getIP()
    {
        return addr64(m_context.Eip);
    }

    virtual MEMOFFSET_64 getSP() 
    {
        return addr64(m_context.Esp);
    }

    virtual MEMOFFSET_64 getFP() 
    {
        return addr64(m_context.Ebp);
    }

    virtual StackPtr getStack();

private:

    CONTEXT_X86   m_context;
};

///////////////////////////////////////////////////////////////////////////////

class CPUx64Context : public CPUContext
{
public:

    CPUx64Context();

    explicit CPUx64Context(const CONTEXT_X64&  context) :
        m_context(context)
    {}

    virtual CPUType getCPUType() {
        return CPU_AMD64;
    }

    virtual CPUType getCPUMode() {
        return CPU_AMD64;
    }

    virtual NumVariant getRegisterByName(const std::wstring &name) {
        NOT_IMPLEMENTED();
    }
    virtual NumVariant getRegisterByIndex(unsigned long index) {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getRegisterName(unsigned long index) {
        NOT_IMPLEMENTED();
    }

    virtual unsigned long getRegisterNumber() {
        NOT_IMPLEMENTED();
    }

    virtual DataAccessorPtr getRegisterAccessor(unsigned long regId) {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getIP()
    {
        return addr64(m_context.Rip);
    }

    virtual MEMOFFSET_64 getSP()
    {
        return addr64(m_context.Rsp);
    }

    virtual MEMOFFSET_64 getFP()
    {
        return addr64(m_context.Rbp);
    }
    virtual StackPtr getStack();


private:

    CONTEXT_X64   m_context;
};

///////////////////////////////////////////////////////////////////////////////

class CPUWOW64Context : public CPUContext
{
public:

    CPUWOW64Context();

    explicit CPUWOW64Context(const WOW64_CONTEXT&  context) :
        m_context(context)
    {}

    virtual CPUType getCPUType() {
        return CPU_AMD64;
    }

    virtual CPUType getCPUMode() {
        return CPU_I386;
    }

    virtual NumVariant getRegisterByName(const std::wstring &name) {
        NOT_IMPLEMENTED();
    }
    virtual NumVariant getRegisterByIndex(unsigned long index) {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getRegisterName(unsigned long index) {
        NOT_IMPLEMENTED();
    }

    virtual unsigned long getRegisterNumber() {
        NOT_IMPLEMENTED();
    }

    virtual DataAccessorPtr getRegisterAccessor(unsigned long regId) {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getIP()
    {
        return addr64(m_context.Eip);
    }

    virtual MEMOFFSET_64 getSP()
    {
        return addr64(m_context.Esp);
    }

    virtual MEMOFFSET_64 getFP()
    {
        return addr64(m_context.Ebp);
    }
    virtual StackPtr getStack();


private:

    WOW64_CONTEXT   m_context;
};

///////////////////////////////////////////////////////////////////////////////
}

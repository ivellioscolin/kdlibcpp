#pragma once

#include <kdlib/stack.h>

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class StackImpl : public Stack
{

public:

    StackImpl(const std::vector<StackFramePtr>& frames) :
        m_stackTrace(frames)
        {}

    unsigned long getFrameCount()
    {
        return static_cast<unsigned long>(m_stackTrace.size());
    }

    StackFramePtr getFrame(unsigned long index) {
        if (index < m_stackTrace.size())
            return m_stackTrace[index];
        throw IndexException(index);
    }

private:

    std::vector<StackFramePtr>  m_stackTrace;
};

///////////////////////////////////////////////////////////////////////////////

class StackFrameImpl : public StackFrame
{
public:

    StackFrameImpl(unsigned long number, MEMOFFSET_64 ip, MEMOFFSET_64 ret, MEMOFFSET_64 fp, MEMOFFSET_64 sp, const CPUContextPtr &cpuCtx) :
        m_number(number),
        m_ip(ip),
        m_ret(ret),
        m_fp(fp),
        m_sp(sp),
        m_cpuContext(cpuCtx)
        {}

    unsigned long getNumber() const
    {
        return m_number;
    }

    virtual MEMOFFSET_64 getIP() const 
    {
        return m_ip;
    }

    virtual MEMOFFSET_64 getRET() const
    {
        return m_ret;
    }

    virtual MEMOFFSET_64  getFP() const
    {
        return m_fp;
    }

    virtual MEMOFFSET_64  getSP() const
    {
        return m_sp;
    }

    virtual CPUContextPtr getCPUContext()
    {
        return m_cpuContext;
    }

    virtual unsigned long getTypedParamCount();
    virtual TypedVarPtr getTypedParam(unsigned long index);
    virtual std::wstring  getTypedParamName(unsigned long index);
    virtual TypedVarPtr getTypedParam(const std::wstring& paramName);

    virtual unsigned long getLocalVarCount();
    virtual TypedVarPtr getLocalVar(unsigned long index);
    virtual TypedVarPtr getLocalVar(const std::wstring& paramName);
    virtual std::wstring  getLocalVarName(unsigned long index);

    virtual unsigned long getStaticVarCount() {
        NOT_IMPLEMENTED();
    }

    virtual TypedVarPtr getStaticVar(unsigned long index) {
        NOT_IMPLEMENTED();
    }

    virtual TypedVarPtr getStaticVar(const std::wstring& paramName) {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring  getStaticVarName(unsigned long index) {
        NOT_IMPLEMENTED();
    }
public:

    SymbolPtrList getLocalVars();
    SymbolPtrList getParams();
    SymbolPtrList getBlockLocalVars(SymbolPtr& sym);
    MEMOFFSET_64 getOffset(unsigned long regRel, MEMOFFSET_REL relOffset);

    MEMOFFSET_64  m_ip;
    MEMOFFSET_64  m_ret;
    MEMOFFSET_64  m_fp;
    MEMOFFSET_64  m_sp;
    CPUContextPtr  m_cpuContext;
    unsigned long  m_number;

};

///////////////////////////////////////////////////////////////////////////////

}


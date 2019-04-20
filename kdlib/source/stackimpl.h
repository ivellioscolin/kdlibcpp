#pragma once

#include <kdlib/stack.h>

#include <boost/enable_shared_from_this.hpp>

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

class StackFrameImpl : public StackFrame, public boost::enable_shared_from_this<StackFrameImpl>
{
public:

    StackFrameImpl(unsigned long number, MEMOFFSET_64 ip, MEMOFFSET_64 ret, MEMOFFSET_64 fp, 
        MEMOFFSET_64 sp, const CPUContextPtr &cpuCtx, unsigned long inlineIndex = 0) :
        m_number(number),
        m_ip(ip),
        m_ret(ret),
        m_fp(fp),
        m_sp(sp),
        m_cpuContext(cpuCtx),
        m_inlineIndex(inlineIndex)
        {}

    unsigned long getNumber() const override
    {
        return m_number;
    }

    MEMOFFSET_64 getIP() const override
    {
        return m_ip;
    }

    MEMOFFSET_64 getRET() const override
    {
        return m_ret;
    }

    MEMOFFSET_64  getFP() const override
    {
        return m_fp;
    }

    MEMOFFSET_64  getSP() const override
    {
        return m_sp;
    }

    CPUContextPtr getCPUContext() override
    {
        return m_cpuContext;
    }

    unsigned long getTypedParamCount() override;
    TypedVarPtr getTypedParam(unsigned long index) override;
    std::wstring  getTypedParamName(unsigned long index) override;
    TypedVarPtr getTypedParam(const std::wstring& paramName) override;
    bool findParam(const std::wstring& paramName) override;

    unsigned long getLocalVarCount() override;
    TypedVarPtr getLocalVar(unsigned long index) override;
    TypedVarPtr getLocalVar(const std::wstring& paramName) override;
    std::wstring  getLocalVarName(unsigned long index) override;
    bool findLocalVar(const std::wstring& varName) override;

    unsigned long getStaticVarCount() override;
    TypedVarPtr getStaticVar(unsigned long index) override;
    TypedVarPtr getStaticVar(const std::wstring& paramName) override;
    std::wstring  getStaticVarName(unsigned long index) override;
    bool findStaticVar(const std::wstring& varName) override;

    void switchTo()  override {
        setCurrentStackFrame(shared_from_this());
    }

    bool isInline() override {
        return m_inlineIndex > 0;
    }

    TypedVarPtr getFunction() override;
    std::wstring getSymbol(bool showDisplacement = true) override;
    void getSourceLine(std::wstring& fileName, unsigned long& lineNo) override;

private:

    SymbolPtrList getLocalVars();
    SymbolPtrList getParams();
    SymbolPtrList getStaticVars();

    SymbolPtrList getBlockLocalVars(SymbolPtr& sym);
    SymbolPtrList getBlockStaticVars(SymbolPtr& sym);

    MEMOFFSET_64 getOffset(unsigned long regRel, MEMOFFSET_REL relOffset);

    MEMOFFSET_64  m_ip;
    MEMOFFSET_64  m_ret;
    MEMOFFSET_64  m_fp;
    MEMOFFSET_64  m_sp;
    CPUContextPtr  m_cpuContext;
    unsigned long  m_number;
    unsigned long  m_inlineIndex;
};

///////////////////////////////////////////////////////////////////////////////

}


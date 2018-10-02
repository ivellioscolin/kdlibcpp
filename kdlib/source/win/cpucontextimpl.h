#pragma once

#include <map>

#include "kdlib/cpucontext.h"

#include "threadctx.h"

namespace kdlib {


///////////////////////////////////////////////////////////////////////////////

template <typename CONTEXT_TYPE>
class CPUContextImpl : public CPUContext
{
public:
    using RawContextType = typename CONTEXT_TYPE;

    virtual CPUType getCPUType() {
        return m_cpuType;
    }

    virtual CPUType getCPUMode() {
        return m_cpuMode;
    }

    virtual void restore() {
        auto hres = g_dbgMgr->advanced->SetThreadContext(&m_context, sizeof(m_context));
        if (FAILED(hres))
            throw DbgEngException(L"IDebugAdvanced::SetThreadContext", hres);
    }

    virtual NumVariant getRegisterByName(const std::wstring &name) {
        NOT_IMPLEMENTED();
    }
    virtual void setRegisterByName(const std::wstring &name, const NumVariant& value) {
        NOT_IMPLEMENTED();
    }
    virtual void setRegisterByIndex(unsigned long index, const NumVariant& value) {
        NOT_IMPLEMENTED();
    }
    virtual unsigned long getRegisterNumber() {
        NOT_IMPLEMENTED();
    }

    virtual void setIP(MEMOFFSET_64 ip) {
        NOT_IMPLEMENTED();
    }
    virtual void setSP(MEMOFFSET_64 sp) {
        NOT_IMPLEMENTED();
    }
    virtual void setFP(MEMOFFSET_64 fp) {
        NOT_IMPLEMENTED();
    }

protected:
    CPUContextImpl(CPUType cpuType, CPUType cpuMode, const CONTEXT_TYPE *context = nullptr)
        : m_cpuType{ cpuType }, m_cpuMode{ cpuMode }
    {
        if (context)
        {
            static_assert(std::is_pod<CONTEXT_TYPE>::value, "std::is_pod<CONTEXT_TYPE>::value");
            memcpy(&m_context, context, sizeof(CONTEXT_TYPE));
        }
        else
        {
            auto hres = g_dbgMgr->advanced->GetThreadContext(&m_context, sizeof(m_context));
            if (FAILED(hres))
                throw DbgEngException(L"IDebugAdvanced::GetThreadContext", hres);
        }
    }

protected:
    CONTEXT_TYPE m_context;

private:
    const CPUType m_cpuType;
    const CPUType m_cpuMode;
};

///////////////////////////////////////////////////////////////////////////////

class CPUContextAmd64 : public CPUContextImpl<CONTEXT_X64>
{
    typedef CPUContextImpl<CONTEXT_X64> Base;
public:
    CPUContextAmd64() :
        Base{ CPU_AMD64, CPU_AMD64 }
    {
    }

    explicit CPUContextAmd64(const CONTEXT_X64 &context) :
        Base{ CPU_AMD64, CPU_AMD64, &context }
    {
    }

    virtual NumVariant getRegisterByIndex(unsigned long index); 

    virtual std::wstring getRegisterName(unsigned long index);

    virtual MEMOFFSET_64 getIP() {
        return m_context.Rip;
    }

    virtual MEMOFFSET_64 getSP() {
        return m_context.Rsp;
    }

    virtual MEMOFFSET_64 getFP() {
        return m_context.Rbp;
    }
};

///////////////////////////////////////////////////////////////////////////////

class CPUContextI386 : public CPUContextImpl<CONTEXT_X86>
{
    typedef CPUContextImpl<CONTEXT_X86> Base;
public:
    CPUContextI386() :
        Base{ CPU_I386, CPU_I386 }
    {
    }

    CPUContextI386(const CONTEXT_X86 &context) :
        Base{ CPU_I386, CPU_I386, &context }
    {
    }

    virtual NumVariant getRegisterByIndex(unsigned long index);

    virtual std::wstring getRegisterName(unsigned long index);

    virtual MEMOFFSET_64 getIP() {
        return m_context.Eip;
    }

    virtual MEMOFFSET_64 getSP() {
        return m_context.Esp;
    }

    virtual MEMOFFSET_64 getFP() {
        return m_context.Ebp;
    }
};

///////////////////////////////////////////////////////////////////////////////

class CPUContextWOW64 : public CPUContextImpl<WOW64_CONTEXT>
{
    typedef CPUContextImpl<WOW64_CONTEXT> Base;
public:
    CPUContextWOW64() :
        Base{ CPU_AMD64, CPU_I386 }
    {
    }

    explicit CPUContextWOW64(const WOW64_CONTEXT &context) :
        Base{ CPU_AMD64, CPU_I386, &context }
    {
    }

    virtual NumVariant getRegisterByIndex(unsigned long index);
    
    virtual std::wstring getRegisterName(unsigned long index);

    virtual MEMOFFSET_64 getIP() {
        return m_context.Eip;
    }

    virtual MEMOFFSET_64 getSP() {
        return m_context.Esp;
    }

    virtual MEMOFFSET_64 getFP() {
        return m_context.Ebp;
    }
};

///////////////////////////////////////////////////////////////////////////////

class CPUContextArm64 : public CPUContextImpl<CONTEXT_ARM64>
{
    typedef CPUContextImpl<CONTEXT_ARM64> Base;
public:
    CPUContextArm64() :
        Base{ CPU_ARM64, CPU_ARM64 }
    {
    }

    explicit CPUContextArm64(const CONTEXT_ARM64 &context) :
        Base{ CPU_ARM64, CPU_ARM64, &context }
    {
    }

    virtual NumVariant getRegisterByIndex(unsigned long index);

    virtual std::wstring getRegisterName(unsigned long index);

    virtual MEMOFFSET_64 getIP() {
        return m_context.Pc;
    }

    virtual MEMOFFSET_64 getSP() {
        return m_context.Sp;
    }

    virtual MEMOFFSET_64 getFP() {
        return m_context.Fp;
    }
};

///////////////////////////////////////////////////////////////////////////////

class CPUContextArm : public CPUContextImpl<CONTEXT_ARM>
{
    typedef CPUContextImpl<CONTEXT_ARM> Base;
public:
    CPUContextArm() :
        Base{ CPU_ARM, CPU_ARM }
    {
    }

    explicit CPUContextArm(const CONTEXT_ARM &context) :
        Base{ CPU_ARM, CPU_ARM, &context }
    {
    }

    virtual NumVariant getRegisterByIndex(unsigned long index);

    virtual std::wstring getRegisterName(unsigned long index);

    virtual MEMOFFSET_64 getIP() {
        return m_context.Pc;
    }

    virtual MEMOFFSET_64 getSP() {
        return m_context.Sp;
    }

    virtual MEMOFFSET_64 getFP() {
        return m_context.R11;
    }
};

///////////////////////////////////////////////////////////////////////////////

} // namespace kdlib

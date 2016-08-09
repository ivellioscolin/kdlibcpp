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

    virtual void setRegisterByName( const std::wstring &name, const NumVariant& value) {
        NOT_IMPLEMENTED();
    }

    virtual NumVariant getRegisterByIndex( unsigned long index );

    virtual void setRegisterByIndex( unsigned long index, const NumVariant& value) {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getRegisterName( unsigned long index );

    virtual unsigned long getRegisterNumber() {
        return m_values.size();
    }

    virtual void restore();

    virtual MEMOFFSET_64 getIP() {
        return m_ip;
    }

    virtual void setIP(MEMOFFSET_64 ip) {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getSP() {
        return m_sp;
    }

    virtual void setSP(MEMOFFSET_64 sp) {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getFP() {
        return m_fp;
    }

    virtual void setFP(MEMOFFSET_64 fp) {
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

class CPUContextAmd64 : public CPUContext
{
public:
    CPUContextAmd64(const CONTEXT_X64& ctx) :
        m_context(ctx)
    {}

    virtual CPUType getCPUType() {
        return CPU_AMD64;
    }

    virtual CPUType getCPUMode() {
        return CPU_AMD64;
    }

    virtual void restore() {
        NOT_IMPLEMENTED();
    }


    virtual NumVariant getRegisterByName(const std::wstring &name) {
        NOT_IMPLEMENTED();
    }

    virtual void setRegisterByName( const std::wstring &name, const NumVariant& value) {
        NOT_IMPLEMENTED();
    }

    virtual NumVariant getRegisterByIndex(unsigned long index); 

    virtual void setRegisterByIndex( unsigned long index, const NumVariant& value) {
        NOT_IMPLEMENTED();
    }


    virtual std::wstring getRegisterName(unsigned long index);

    virtual unsigned long getRegisterNumber() {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getIP() {
        NOT_IMPLEMENTED();
    }

    virtual void setIP(MEMOFFSET_64 ip) {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getSP() {
        NOT_IMPLEMENTED();
    }

    virtual void setSP(MEMOFFSET_64 sp) {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getFP() {
        NOT_IMPLEMENTED();
    }

    virtual void setFP(MEMOFFSET_64 fp) {
        NOT_IMPLEMENTED();
    }

private:

    CONTEXT_X64  m_context;
};

///////////////////////////////////////////////////////////////////////////////

class CPUContextI386: public CPUContext
{
public:
    CPUContextI386(const CONTEXT_X86& ctx) :
        m_context(ctx)
    {}


    virtual CPUType getCPUType() {
        return CPU_I386;
    }

    virtual CPUType getCPUMode() {
        return CPU_I386;
    }

    virtual void restore() {
        NOT_IMPLEMENTED();
    }


    virtual NumVariant getRegisterByName(const std::wstring &name) {
        NOT_IMPLEMENTED();
    }

    virtual void setRegisterByName( const std::wstring &name, const NumVariant& value) {
        NOT_IMPLEMENTED();
    }

    virtual NumVariant getRegisterByIndex(unsigned long index);

    virtual void setRegisterByIndex( unsigned long index, const NumVariant& value) {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getRegisterName(unsigned long index);

    virtual unsigned long getRegisterNumber() {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getIP() {
        NOT_IMPLEMENTED();
    }

    virtual void setIP(MEMOFFSET_64 ip) {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getSP() {
        NOT_IMPLEMENTED();
    }

    virtual void setSP(MEMOFFSET_64 sp) {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getFP() {
        NOT_IMPLEMENTED();
    }

    virtual void setFP(MEMOFFSET_64 fp) {
        NOT_IMPLEMENTED();
    }

private:

    CONTEXT_X86  m_context;
};

///////////////////////////////////////////////////////////////////////////////

class CPUContextWOW64 : public CPUContext
{
public:
    CPUContextWOW64(const WOW64_CONTEXT& ctx) :
        m_context(ctx)
    {}


    virtual CPUType getCPUType() {
        return CPU_AMD64;
    }

    virtual CPUType getCPUMode() {
        return CPU_I386;
    }

    virtual void restore() {
        NOT_IMPLEMENTED();
    }


    virtual NumVariant getRegisterByName(const std::wstring &name) {
        NOT_IMPLEMENTED();
    }

    virtual void setRegisterByName( const std::wstring &name, const NumVariant& value) {
        NOT_IMPLEMENTED();
    }

    virtual NumVariant getRegisterByIndex(unsigned long index);

    virtual void setRegisterByIndex( unsigned long index, const NumVariant& value) {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getRegisterName(unsigned long index);

    virtual unsigned long getRegisterNumber() {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getIP() {
        NOT_IMPLEMENTED();
    }

    virtual void setIP(MEMOFFSET_64 ip) {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getSP() {
        NOT_IMPLEMENTED();
    }

    virtual void setSP(MEMOFFSET_64 sp) {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getFP() {
        NOT_IMPLEMENTED();
    }

    virtual void setFP(MEMOFFSET_64 fp) {
        NOT_IMPLEMENTED();
    }

private:

    WOW64_CONTEXT  m_context;
};

///////////////////////////////////////////////////////////////////////////////


}

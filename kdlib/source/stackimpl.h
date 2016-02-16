#pragma once

#include <boost/noncopyable.hpp>

#include <kdlib/stack.h>
#include <kdlib/cpucontext.h>

namespace kdlib {


///////////////////////////////////////////////////////////////////////////////

class  StackFrameImpl : public StackFrame
{
public:

    TypedVarPtr getFunction();

    unsigned long getTypedParamCount();

    TypedVarPtr getTypedParam(unsigned long index);

    std::wstring  getTypedParamName(unsigned long index);

    TypedVarPtr getTypedParam(const std::wstring& paramName);

    unsigned long getLocalVarCount();

    TypedVarPtr getLocalVar(unsigned long index);

    TypedVarPtr getLocalVar(const std::wstring& paramName);

    std::wstring  getLocalVarName(unsigned long index);

    unsigned long getStaticVarCount() {
        NOT_IMPLEMENTED();
    }

    TypedVarPtr getStaticVar(unsigned long index) {
        NOT_IMPLEMENTED();
    }

    TypedVarPtr getStaticVar(const std::wstring& paramName) {
        NOT_IMPLEMENTED();
    }

    std::wstring  getStaticVarName(unsigned long index) {
        NOT_IMPLEMENTED();
    }

protected:

    SymbolPtrList  getParams();

    SymbolPtrList  getLocalVars();

    SymbolPtrList  getBlockLocalVars(SymbolPtr&  sym);

    MEMOFFSET_64 getOffset(unsigned long regRel, MEMOFFSET_REL relOffset);

};

///////////////////////////////////////////////////////////////////////////////

class StackFrameSimple : public StackFrameImpl, public boost::noncopyable
{

public:
    StackFrameSimple(MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp) :
        m_ip(ip), m_ret(ret), m_fp(fp), m_sp(sp)
    {}

    DataAccessorPtr getRegisterAccessor(unsigned long regId) {
        return getRegisterAccessor(regId);
    }

    MEMOFFSET_64 getIP() const {
        return m_ip;
    }

    MEMOFFSET_64 getRET() const {
        return m_ret;
    }

    MEMOFFSET_64  getFP() const {
        return m_fp;
    }

    MEMOFFSET_64  getSP() const {
        return m_sp;
    }

protected:

    MEMOFFSET_64  m_ip;
    MEMOFFSET_64  m_ret;
    MEMOFFSET_64  m_fp;
    MEMOFFSET_64  m_sp;
};


///////////////////////////////////////////////////////////////////////////////

class StackFrameWithContext : public StackFrameSimple
{
public:

    StackFrameWithContext(MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp, const CPUContextPtr& ctx) :
        StackFrameSimple(ip, ret, fp, sp),
        m_context(ctx)
        {}

    DataAccessorPtr getRegisterAccessor(unsigned long regId) {
        return m_context->getRegisterAccessor(regId);
    }

protected:

    CPUContextPtr  m_context;
};

///////////////////////////////////////////////////////////////////////////////

class StackImpl : public Stack, private boost::noncopyable
{
public:

    StackImpl(const std::vector<StackFramePtr>& frames) :
        m_frames(frames)
        {}

        unsigned long getFrameCount()
        {
            return static_cast<unsigned long>(m_frames.size());
        }
    
        StackFramePtr getFrame(unsigned long frameNumber)
        {
            if (frameNumber >= m_frames.size())
                throw IndexException(frameNumber);
    
            return m_frames[frameNumber];
        }

protected:

    std::vector<StackFramePtr> m_frames;

};


///////////////////////////////////////////////////////////////////////////////

}

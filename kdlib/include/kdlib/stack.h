#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <kdlib/cpucontext.h>
#include <kdlib/typedvar.h>
#include <kdlib/exceptions.h>

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class Stack;
typedef boost::shared_ptr<Stack>  StackPtr;

class StackFrame;
typedef boost::shared_ptr<StackFrame>  StackFramePtr;

class LocalScope;
typedef boost::shared_ptr<LocalScope>  LocalScopePtr;

///////////////////////////////////////////////////////////////////////////////

class LocalScope {

    friend LocalScopePtr getLocalScope();

public:

    unsigned long getLocalVarCount() {
        NOT_IMPLEMENTED();
    }

    TypedVarPtr getLocalVar( unsigned long index ) {
        NOT_IMPLEMENTED();
    }

    TypedVarPtr getLocalVar( const std::wstring& paramName ) {
        NOT_IMPLEMENTED();
    }

    std::wstring  getLocalVarName( unsigned long index ) {
        NOT_IMPLEMENTED();
    }

    unsigned long getChildScopeCount() {
        NOT_IMPLEMENTED();
    }

    LocalScopePtr getChildScope( unsigned long index ) {
        NOT_IMPLEMENTED();
    }

    bool isCurrent() {
        NOT_IMPLEMENTED();
    }
};

///////////////////////////////////////////////////////////////////////////////

class StackFrame {

    friend StackFramePtr getStackFrame();
    friend StackFramePtr getStackFrame( MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp );

public:

    MEMOFFSET_64 getIP() {
         return m_ip;
    }

    MEMOFFSET_64 getRET() {
        return m_ret;
    }

    MEMOFFSET_64  getFP() {
        return m_fp;
    }

    MEMOFFSET_64  getSP() {
        return m_sp;
    }

    TypedVarPtr getFunction();

    unsigned long getTypedParamCount();

    TypedVarPtr getTypedParam( unsigned long index );

    std::wstring  getTypedParamName( unsigned long index );

    TypedVarPtr getTypedParam( const std::wstring& paramName );

    LocalScopePtr getLocalScope() {
        NOT_IMPLEMENTED();
    }

protected:

    StackFrame( MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp ) :
        m_ip(ip), m_ret(ret), m_fp(fp), m_sp(sp)
        {}

    MEMOFFSET_64 getOffset( unsigned long regRel, MEMOFFSET_REL relOffset );

     MEMOFFSET_64  m_ip;
     MEMOFFSET_64  m_ret;
     MEMOFFSET_64  m_fp;
     MEMOFFSET_64  m_sp;
};

///////////////////////////////////////////////////////////////////////////////

class Stack {

    friend StackPtr getStack();
    friend StackPtr getStack( CPUContextPtr& cpuContext );

public:

    unsigned long getFrameCount() {
       return m_cpuContext->getStackLength();
    }

    StackFramePtr getFrame( unsigned long frameNumber ) {
        MEMOFFSET_64 ip, ret, fp, sp;
        m_cpuContext->getStackFrame( frameNumber, ip, ret, fp, sp );
        return getStackFrame( ip, ret, fp, sp );
    }

protected:

    Stack() : m_cpuContext( loadCPUContext() )
    {}

    Stack( const CPUContextPtr& cpuContext ) : m_cpuContext( cpuContext )
    {}

    CPUContextPtr  m_cpuContext;
};

///////////////////////////////////////////////////////////////////////////////

StackPtr getStack();

StackPtr getStack( CPUContextPtr& cpuContext );

inline
StackFramePtr getStackFrame() {
    return getStack()->getFrame(0);
}

StackFramePtr getStackFrame( MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp );

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end


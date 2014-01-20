#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
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

///////////////////////////////////////////////////////////////////////////////

class StackFrame : public boost::enable_shared_from_this<StackFrame>, public boost::noncopyable {

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

    unsigned long getLocalVarCount();

    TypedVarPtr getLocalVar( unsigned long index );

    TypedVarPtr getLocalVar( const std::wstring& paramName );

    std::wstring  getLocalVarName( unsigned long index );

    unsigned long getStaticVarCount() {
        NOT_IMPLEMENTED();
    }

    TypedVarPtr getStaticVar( unsigned long index ) {
        NOT_IMPLEMENTED();
    }

    TypedVarPtr getStaticVar( const std::wstring& paramName ) {
        NOT_IMPLEMENTED();
    }

    std::wstring  getStaticVarName( unsigned long index ) {
        NOT_IMPLEMENTED();
    }

protected:

    StackFrame( MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp ) :
        m_ip(ip), m_ret(ret), m_fp(fp), m_sp(sp)
        {}

     SymbolPtrList  getLocalVars();

     SymbolPtrList  getBlockLocalVars(SymbolPtr&  sym);

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


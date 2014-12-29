#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

#include <kdlib/dbgengine.h>
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

    //friend StackFramePtr getStackFrame();
    friend StackFramePtr getStackFrame( MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp );

public:

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

public:

    unsigned long getFrameCount() {
        return static_cast<unsigned long>( m_stackTrace.size() );
    }

    StackFramePtr getFrame( unsigned long frameNumber ) 
    {
        return getStackFrame( 
                m_stackTrace[frameNumber].instructionOffset,
                m_stackTrace[frameNumber].returnOffset,
                m_stackTrace[frameNumber].frameOffset,
                m_stackTrace[frameNumber].stackOffset );
    }

protected:

    Stack() 
    {
       getStackTrace(m_stackTrace);
    }

    std::vector<FrameDesc>  m_stackTrace;
};

///////////////////////////////////////////////////////////////////////////////

inline
StackPtr getStack() {
    return StackPtr( new Stack() );
}

StackFramePtr getCurrentStackFrame();

unsigned long getCurrentStackFrameNumber();

void setCurrentStackFrame(StackFramePtr& stackFrame);

void setCurrentStackFrameByIndex(unsigned long frameIndex);

void resetCurrentStackFrame();

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end


#include "stdafx.h"

#include "kdlib\stack.h"
#include "kdlib\module.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

StackPtr getStack()
{
    return StackPtr( new Stack() );
}

StackPtr getStack( CPUContextPtr& cpuContext )
{
    return StackPtr( new Stack( cpuContext ) );
}

StackFramePtr getStackFrame( MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp )
{
    return StackFramePtr( new StackFrame(ip,ret,fp,sp) );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr StackFrame::getFunction()
{
    ModulePtr mod = loadModule(m_ip);

    return mod->getFunctionByAddr(m_ip);
}

///////////////////////////////////////////////////////////////////////////////

unsigned long StackFrame::getTypedParamCount()
{
    ModulePtr mod = loadModule(m_ip);

    TypedVarPtr func = mod->getFunctionByAddr(m_ip);

    return func->getElementCount();
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr StackFrame::getTypedParam( unsigned long index )
{
    ModulePtr mod = loadModule(m_ip);

    TypedVarPtr func = mod->getFunctionByAddr(m_ip);

    MEMOFFSET_REL relOffset = func->getElementOffset(index);

    RELREG_ID regRel = func->getElementOffsetRelativeReg(index);

    MEMOFFSET_64  offset = getOffset( regRel, relOffset );

    return loadTypedVar( func->getType()->getElement(index), offset );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring  StackFrame::getTypedParamName( unsigned long index )
{
    ModulePtr mod = loadModule(m_ip);

    TypedVarPtr func = mod->getFunctionByAddr(m_ip);

    return func->getElementName(index);
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr StackFrame::getTypedParam(  const std::wstring& paramName )
{
    ModulePtr mod = loadModule(m_ip);

    TypedVarPtr func = mod->getFunctionByAddr(m_ip);

    MEMOFFSET_REL relOffset = func->getElementOffset(paramName);

    RELREG_ID regRel = func->getElementOffsetRelativeReg(paramName);

    MEMOFFSET_64  offset = getOffset( regRel, relOffset );

    size_t  paramIndex = func->getElementIndex( paramName );

    return loadTypedVar( func->getType()->getElement(paramIndex), offset );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 StackFrame::getOffset( unsigned long regRel, MEMOFFSET_REL relOffset )
{
    switch( regRel )
    {
    case rriInstructionPointer:
        return (MEMOFFSET_64)( m_ip + relOffset );

    case rriStackFrame:
        return (MEMOFFSET_64)( m_fp + relOffset );

    case rriStackPointer:
        return (MEMOFFSET_64)( m_sp + relOffset );
    }

    throw DbgException( "unknown relative offset" );
}

///////////////////////////////////////////////////////////////////////////////


}; // kdlib namespace end

#include "stdafx.h"

#include "kdlib\stack.h"
#include "kdlib\module.h"

namespace kdlib {

typedef std::pair<MEMOFFSET_64, MEMOFFSET_64>   DebugRange;

namespace {

///////////////////////////////////////////////////////////////////////////////

DebugRange getFuncDebugRange(SymbolPtr& sym)
{
    SymbolPtrList lstFuncDebugStart;
    SymbolPtrList lstFuncDebugEnd;

    try
    {
        lstFuncDebugStart = sym->findChildren(SymTagFuncDebugStart);
        if ( lstFuncDebugStart.empty() )
            return std::make_pair(0, 0);

        lstFuncDebugEnd = sym->findChildren(SymTagFuncDebugEnd);
        if ( lstFuncDebugEnd.empty() )
            return std::make_pair(0, 0);

        return std::make_pair( (*lstFuncDebugStart.begin())->getVa(), (*lstFuncDebugEnd.begin())->getVa());
    }
    catch (const SymbolException&)
    {
    }

    return std::make_pair(0, 0);
}

DebugRange getBlockRange(SymbolPtr& sym)
{
    MEMOFFSET_64   blockBegin = sym->getVa();
    MEMOFFSET_64   blockEnd = blockBegin + sym->getSize();

    return std::make_pair(blockBegin, blockEnd);
}

bool inDebugRange( const DebugRange& range, MEMOFFSET_64 offset)
{
    return range.first <= offset && range.second >= offset;
}

///////////////////////////////////////////////////////////////////////////////

}

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

TypedVarPtr StackFrame::getTypedParam( const std::wstring& paramName )
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

unsigned long StackFrame::getLocalVarCount()
{
    return static_cast<unsigned long>(getLocalVars().size());
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr StackFrame::getLocalVar( unsigned long index )
{
    SymbolPtrList  vars = getLocalVars();

    SymbolPtrList::iterator it = vars.begin();
    std::advance( it, index );

    if ( it == vars.end() )
        throw IndexException(index);

    SymbolPtr  sym = *it;

    MEMOFFSET_REL relOffset = sym->getOffset();

    RELREG_ID regRel = sym->getRegRealativeId();

    MEMOFFSET_64  offset = getOffset( regRel, relOffset );

    return loadTypedVar( loadType(sym), offset);
}

///////////////////////////////////////////////////////////////////////////////

std::wstring  StackFrame::getLocalVarName( unsigned long index )
{
    SymbolPtrList  vars = getLocalVars();

    SymbolPtrList::iterator it = vars.begin();
    std::advance( it, index );

    if ( it == vars.end() )
        throw IndexException(index);

    return (*it)->getName();
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr StackFrame::getLocalVar( const std::wstring& paramName )
{
    SymbolPtrList  vars = getLocalVars();

    SymbolPtrList::iterator it = vars.begin();
    for ( ; it != vars.end(); ++it )
    {
        if ( (*it)->getName() == paramName )
        {
            SymbolPtr  sym = *it;

            MEMOFFSET_REL relOffset = sym->getOffset();

            RELREG_ID regRel = sym->getRegRealativeId();

            MEMOFFSET_64  offset = getOffset( regRel, relOffset );

            return loadTypedVar( loadType(sym), offset);
        }
    }

    throw SymbolException(L"symbol not found");
}

///////////////////////////////////////////////////////////////////////////////

SymbolPtrList  StackFrame::getLocalVars()
{
    ModulePtr mod  = loadModule(m_ip);

    MEMDISPLACEMENT displacemnt;
    SymbolPtr symFunc = mod->getSymbolByVa( m_ip, SymTagFunction, &displacemnt );

    SymbolPtrList  lst;

    DebugRange  debugRange = getFuncDebugRange(symFunc);

    if ( !inDebugRange(debugRange, m_ip) )
        return lst;

    // find var in current scope
    SymbolPtrList symList = symFunc->findChildren(SymTagData);

    SymbolPtrList::iterator it;
    for ( it = symList.begin(); it != symList.end(); it++ )
    {
        if ( (*it)->getDataKind() == DataIsLocal  )
            lst.push_back( *it );
    }

   // find inners scopes
    SymbolPtrList scopeList = symFunc->findChildren(SymTagBlock);
    SymbolPtrList::iterator itScope = scopeList.begin();

    for (; itScope != scopeList.end(); ++itScope)
    {
        SymbolPtrList  innerVars = getBlockLocalVars(*itScope);
 
        lst.insert( lst.end(), innerVars.begin(), innerVars.end() );
    }

    return lst;
}

///////////////////////////////////////////////////////////////////////////////

SymbolPtrList  StackFrame::getBlockLocalVars(SymbolPtr& sym)
{
    SymbolPtrList  lst;

    DebugRange  debugRange = getBlockRange(sym);

    if ( !inDebugRange(debugRange, m_ip) )
        return lst;

    // find var in current scope
    SymbolPtrList symList = sym->findChildren(SymTagData);

    SymbolPtrList::iterator it;
    for ( it = symList.begin(); it != symList.end(); it++ )
    {
        if ( (*it)->getDataKind() == DataIsLocal  )
            lst.push_back( *it );
    }

   // find inners scopes
    SymbolPtrList scopeList = sym->findChildren(SymTagBlock);
    SymbolPtrList::iterator itScope = scopeList.begin();

    for (; itScope != scopeList.end(); ++itScope)
    {
        SymbolPtrList  innerVars = getBlockLocalVars(*itScope);

        lst.insert( lst.end(), innerVars.begin(), innerVars.end() );
    }

    return lst;
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

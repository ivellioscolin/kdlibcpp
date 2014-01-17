#include "stdafx.h"

#include <kdlib/module.h>

#include "scopeimp.h"

namespace  kdlib {

///////////////////////////////////////////////////////////////////////////////

unsigned long FunctionScope::getVarCount()
{
    ModulePtr mod = loadModule(m_frame->getIP());

    TypedVarPtr func = mod->getFunctionByAddr(m_frame->getIP());

    SymbolPtr  localScope = mod->getSymbolScope(func->getAddress());

    SymbolPtrList  paramLst = localScope->findChildren( SymTagData );

    SymbolPtrList::iterator itVar = paramLst.begin();

    unsigned long count = 0;

    for ( count = 0; itVar != paramLst.end(); ++itVar )
    {
        if ( (*itVar)->getDataKind() == DataIsLocal )
        {
            count++;
        }
    };

    return count;
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr FunctionScope::getVarByIndex( unsigned long index )
{
    ModulePtr mod = loadModule(m_frame->getIP());

    TypedVarPtr func = mod->getFunctionByAddr(m_frame->getIP());

    SymbolPtr  localScope = mod->getSymbolScope(func->getAddress());

    SymbolPtrList  paramLst = localScope->findChildren( SymTagData );

    SymbolPtrList::iterator it = paramLst.begin();

    for ( unsigned long i = 0; it != paramLst.end(); ++it )
    {
        SymbolPtr  sym = *it;

        if ( sym->getDataKind() == DataIsLocal )
        {
            if ( i == index )
            {
                MEMOFFSET_REL relOffset = sym->getOffset();
    
                RELREG_ID regRel = sym->getRegRealativeId();
            
                MEMOFFSET_64  offset = m_frame->getOffset( regRel, relOffset );

                return loadTypedVar( loadType(sym), offset);
            }
            else
            {
                ++i;
            }
        }
    };

    throw IndexException( index );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr FunctionScope::getVarByName( const std::wstring& paramName )
{
    ModulePtr mod = loadModule(m_frame->getIP());

    TypedVarPtr func = mod->getFunctionByAddr(m_frame->getIP());

    SymbolPtr  localScope = mod->getSymbolScope(func->getAddress());

    SymbolPtr  sym = localScope->getChildByName(paramName);

    if ( !sym )
         SymbolException(L"local var not found");

    if ( sym->getSymTag() != SymTagData || sym->getDataKind() != DataIsLocal )
         SymbolException(L"local var not found");

    MEMOFFSET_REL relOffset = sym->getOffset();
    
    RELREG_ID regRel = sym->getRegRealativeId();
            
    MEMOFFSET_64  offset = m_frame->getOffset( regRel, relOffset );

    return loadTypedVar( loadType(sym), offset);
}

///////////////////////////////////////////////////////////////////////////////

unsigned long FunctionScope::getChildScopeCount()
{
    NOT_IMPLEMENTED();
}

///////////////////////////////////////////////////////////////////////////////

TypedVarScopePtr FunctionScope::getChildScope( unsigned long index )
{
    NOT_IMPLEMENTED();
}

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end

#pragma once

#include <kdlib/scope.h>
#include <kdlib/stack.h>
#include <kdlib/symengine.h>
#include <kdlib/exceptions.h>

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class TypedVarScopeImp : public TypedVarScope {

public:

    unsigned long getVarCount() {
        NOT_IMPLEMENTED();
    }

    TypedVarPtr getVarByIndex( unsigned long index ) {
        NOT_IMPLEMENTED();
    }

    TypedVarPtr getVarByName( const std::wstring& paramName ) {
        NOT_IMPLEMENTED();
    }

    std::wstring  getVarName( unsigned long index ) {
        NOT_IMPLEMENTED();
    }

    unsigned long getChildScopeCount() {
        NOT_IMPLEMENTED();
    }

    TypedVarScopePtr getChildScope( unsigned long index ) {
        NOT_IMPLEMENTED();
    }

    bool isCurrent() {
        NOT_IMPLEMENTED();
    }
};

///////////////////////////////////////////////////////////////////////////////

//class SymbolScope : public TypedVarScopeImp {
//
//public:
//
//    SymbolScope(const SymbolPtr&  symbol ) : m_symbol(symbol )
//    {
//        if ( m_symbol->getSymTag() != SymTagBlock )
//            throw DbgException("Wrong symbol for scope");
//    }
//
//protected:
//
//    virtual unsigned long getChildScopeCount();
//
//    SymbolPtr  m_symbol;
//
//};

///////////////////////////////////////////////////////////////////////////////

class FunctionScope : public TypedVarScopeImp {

public:

    FunctionScope( const StackFramePtr& frame ) :
        m_frame(frame)
    {}

protected:


    virtual unsigned long getVarCount();

    virtual unsigned long getChildScopeCount();

    virtual TypedVarPtr getVarByIndex( unsigned long index );

    virtual TypedVarPtr getVarByName( const std::wstring& paramName );

    TypedVarScopePtr FunctionScope::getChildScope( unsigned long index );

    StackFramePtr  m_frame;
};

///////////////////////////////////////////////////////////////////////////////

} // namespace kdlib

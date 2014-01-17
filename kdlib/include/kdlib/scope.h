#pragma once

#include <boost/shared_ptr.hpp>

#include <kdlib/exceptions.h>

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class TypedVar;
typedef boost::shared_ptr<TypedVar>  TypedVarPtr;

class TypedVarScope;
typedef boost::shared_ptr<TypedVarScope> TypedVarScopePtr;

class TypedVarScope {

public:
    virtual unsigned long getVarCount() = 0;
    virtual TypedVarPtr getVarByIndex( unsigned long index ) = 0;
    virtual TypedVarPtr getVarByName( const std::wstring& paramName ) = 0;
    virtual std::wstring  getVarName( unsigned long index ) = 0;
    virtual unsigned long getChildScopeCount() = 0;
    virtual TypedVarScopePtr getChildScope( unsigned long index ) = 0;
};

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end


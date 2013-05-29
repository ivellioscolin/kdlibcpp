#pragma once

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "kdlib/typeinfo.h"
#include "kdlib/variant.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class TypedVar;
typedef boost::shared_ptr<TypedVar>  TypedVarPtr;

TypedVarPtr loadTypedVar( const std::wstring &varName );

TypedVarPtr loadTypedVar( const std::wstring &typeName, MEMOFFSET_64 addr );

TypedVarPtr loadTypedVar( const TypeInfoPtr &typeInfo, MEMOFFSET_64 addr );


class TypedVar : private boost::noncopyable, public NumBehavior {

    friend TypedVarPtr loadTypedVar( const std::wstring &varName );

    friend TypedVarPtr loadTypedVar( const std::wstring &typeName, MEMOFFSET_64 addr );

    friend TypedVarPtr loadTypedVar( const TypeInfoPtr &typeInfo, MEMOFFSET_64 addr );

public:
    
    virtual MEMOFFSET_64 getAddress() const = 0;
    virtual size_t getSize() const = 0;
    virtual TypedVarPtr getElement( const std::wstring& fieldName ) = 0;
    virtual TypedVarPtr getElement( size_t index ) = 0;
    virtual MEMOFFSET_32 getElementOffset(const std::wstring& fieldName ) = 0;
    virtual MEMOFFSET_32 getElementOffset(size_t index ) = 0;
    virtual size_t getElementCount() = 0;
    virtual std::wstring getElementName( size_t index ) = 0;

    virtual TypeInfoPtr getType() const = 0;
    virtual NumVariant getValue() const = 0;

protected:


    TypedVar() 
    {}

    virtual ~TypedVar() 
    {}
};

///////////////////////////////////////////////////////////////////////////////

} // end kdlib namespace

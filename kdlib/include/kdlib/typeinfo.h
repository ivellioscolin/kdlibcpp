#pragma once

#include <string>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "kdlib/symengine.h"
#include "kdlib/variant.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class TypeInfo;
typedef boost::shared_ptr<TypeInfo>     TypeInfoPtr;

TypeInfoPtr loadType( const std::wstring &symName );
TypeInfoPtr loadType( SymbolPtr &symbol );
TypeInfoPtr loadType( SymbolPtr &symbolScope, const std::wstring &symbolName ); 

size_t getSymbolSize( const std::wstring &name );
MEMOFFSET_64 getSymbolOffset( const std::wstring &name );

class TypeInfo : private boost::noncopyable, public NumBehavior {

    friend TypeInfoPtr loadType( const std::wstring &symName );
    friend TypeInfoPtr loadType( SymbolPtr &symbol );
    friend TypeInfoPtr loadType( SymbolPtr &symbolScope, const std::wstring &symbolName ); 
    friend size_t getSymbolSize( const std::wstring &name );

public:

    virtual std::wstring getName() = 0;
    virtual size_t getSize() = 0;
    virtual size_t getPtrSize() = 0;

    virtual TypeInfoPtr ptrTo() = 0;
    virtual TypeInfoPtr deref() = 0;
    virtual TypeInfoPtr arrayOf( size_t size ) = 0;

    virtual bool isArray() = 0;
    virtual bool isPointer() = 0;
    virtual bool isVoid() = 0;
    virtual bool isBase() = 0;
    virtual bool isUserDefined() = 0;
    virtual bool isConstant() = 0;
    virtual bool isEnum() = 0;
    virtual bool isBitField() = 0;
    virtual bool isFunction() = 0;

    virtual BITOFFSET getBitOffset() = 0;
    virtual BITOFFSET getBitWidth() = 0;
    virtual TypeInfoPtr getBitType() = 0;

    virtual TypeInfoPtr getElement( const std::wstring &name ) = 0;
    virtual TypeInfoPtr getElement( size_t index ) = 0;
    virtual MEMOFFSET_32 getElementOffset( const std::wstring &name ) = 0;
    virtual MEMOFFSET_32 getElementOffset( size_t index ) = 0;
    virtual std::wstring getElementName( size_t index ) = 0;
    virtual size_t getElementIndex( const std::wstring &name ) = 0;
    virtual size_t getElementCount() = 0;
    virtual  MEMOFFSET_64 getElementVa( const std::wstring &name ) = 0;
    virtual  MEMOFFSET_64 getElementVa( size_t index ) = 0;
    
    virtual bool isStaticMember( const std::wstring &name ) = 0;
    virtual bool isStaticMember( size_t index ) = 0;

    virtual bool isVirtualMember( const std::wstring &name ) = 0;
    virtual bool isVirtualMember( size_t index ) = 0;

    virtual NumVariant getValue() const = 0;

    virtual void getVirtualDisplacement( const std::wstring& fieldName, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize ) = 0;
    virtual void getVirtualDisplacement( size_t fieldIndex, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize ) = 0;

protected:

    static bool isBaseType( const std::wstring &typeName );
    static bool isComplexType( const std::wstring &typeName );

    static TypeInfoPtr getBaseTypeInfo( const std::wstring &typeName, size_t ptrSize );
    static TypeInfoPtr getBaseTypeInfo( SymbolPtr &symbolScope );

    static TypeInfoPtr getComplexTypeInfo( const std::wstring &typeName, SymbolPtr &symbolScope );
    static TypeInfoPtr getRecursiveComplexType( const std::wstring &typeName, TypeInfoPtr &lowestType, std::wstring &suffix, size_t ptrSize );

    TypeInfo() 
    {}

    virtual ~TypeInfo() 
    {}
};

///////////////////////////////////////////////////////////////////////////////

}; // kdlib namespace end

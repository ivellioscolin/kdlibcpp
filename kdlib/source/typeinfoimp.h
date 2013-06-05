#pragma once

#include <kdlib/typeinfo.h>
#include <kdlib/exceptions.h>

#include "udtfield.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////


inline size_t getPtrSizeBySymbol( SymbolPtr &typeSym )
{
    SymTags symTag = typeSym->getSymTag();
    if ( symTag != SymTagPointerType )
        return (typeSym->getMachineType() == machine_AMD64) ? 8 : 4;

    return typeSym->getSize();
}


///////////////////////////////////////////////////////////////////////////////

class TypeInfoImp : public TypeInfo, public boost::enable_shared_from_this<TypeInfoImp>
{
protected:

    virtual std::wstring getName() {
        NOT_IMPLEMENTED();
    }

    virtual size_t getSize() {
        NOT_IMPLEMENTED();
    }

    virtual size_t getPtrSize() {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr ptrTo();

    virtual TypeInfoPtr deref() {
        throw TypeException( getName(), L"type can not be dereffered" );
    }

    virtual TypeInfoPtr arrayOf( size_t size );

    virtual bool isArray() {
        return false;
    }

    virtual bool isPointer() {
        return false;
    }

    virtual bool isVoid() {
        return false;
    }

    virtual bool isBase() {
        return false;
    }

    virtual bool isUserDefined() {
        return false;
    }

    virtual bool isEnum() {
        return false;
    }

    virtual bool isBitField() {
        return false;
    }

    virtual bool isConstant() {
        return m_constant;
    }

    virtual TypeInfoPtr getElement( const std::wstring &name ) {
         throw TypeException( getName(), L" type has no fields or array elements");
    }

    virtual TypeInfoPtr getElement( size_t index ) {
         throw TypeException( getName(), L" type has no fields or array elements");
    }

    virtual MEMOFFSET_32 getElementOffset( const std::wstring &name ) {
         throw TypeException( getName(), L" type has no fields or array elements");
    }

    virtual MEMOFFSET_32 getElementOffset( size_t index ) {
         throw TypeException( getName(), L" type has no fields or array elements");
    }

    virtual size_t getElementCount() {
        throw TypeException( getName(), L" type has no fields or array elements");
    }

    virtual std::wstring getElementName( size_t index ) {
         NOT_IMPLEMENTED();
    }

    virtual size_t getElementIndex( const std::wstring &name ) {
         NOT_IMPLEMENTED();
    }

    virtual BITOFFSET getBitOffset() {
         throw TypeException( getName(), L" type is not a bit field");
    }

    virtual BITOFFSET getBitWidth() {
         throw TypeException( getName(), L" type is not a bit field");
    }

    virtual TypeInfoPtr getBitType() {
         throw TypeException( getName(), L" type is not a bit field");
    }
    
    virtual  MEMOFFSET_64 getElementVa( const std::wstring &name ) {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    virtual  MEMOFFSET_64 getElementVa( size_t index ) {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    virtual bool isStaticMember( const std::wstring &name ) {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    virtual bool isStaticMember( size_t index ) {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    virtual bool isVirtualMember( const std::wstring &name ) {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    virtual bool isVirtualMember( size_t index ) {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    virtual void getVirtualDisplacement( const std::wstring& fieldName, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize ) {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    virtual void getVirtualDisplacement( size_t fieldIndex, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize ) {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

public:

    void setConstant( const NumVariant& constVal )
    {
        m_constant = true;
        m_constantValue = constVal;
    }

protected:

    TypeInfoImp() :
        m_constant( false )
        {}

    bool   m_constant;

    NumVariant  m_constantValue;

    virtual NumVariant getValue() const 
    {
        if ( !m_constant )
        {
            std::wstring typeName = const_cast<TypeInfoImp&>(*this).getName();
            throw TypeException( typeName, L"this type is not a constant and has not a value" );
        }

        return  m_constantValue;
    }
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoFields : public TypeInfoImp 
{

protected:

    virtual TypeInfoPtr getElement( const std::wstring &name );

    virtual TypeInfoPtr getElement( size_t index );

    virtual MEMOFFSET_32 getElementOffset( const std::wstring &name );

    virtual MEMOFFSET_32 getElementOffset( size_t index );

    virtual std::wstring getElementName( size_t index );

    virtual size_t getElementIndex( const std::wstring &name );

    virtual std::wstring getName() {
        return m_name;
    }

    virtual size_t getElementCount();

    virtual  MEMOFFSET_64 getElementVa( const std::wstring &name );
    virtual  MEMOFFSET_64 getElementVa( size_t index );
    virtual bool isStaticMember( const std::wstring &name );
    virtual bool isStaticMember( size_t index );
    virtual bool isVirtualMember( const std::wstring &name );
    virtual bool isVirtualMember( size_t index );

protected:

    TypeInfoFields( const std::wstring &name ) :
        m_fields( name ),
        m_name( name ),
        m_fieldsGot(false)
        {}

    FieldCollection  m_fields;

    std::wstring  m_name;

    virtual void getFields() = 0;

private:

    bool m_fieldsGot;

    void checkFields()
    {
        if ( !m_fieldsGot )
        {
            getFields();
            m_fieldsGot = true;
        }
    }

};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoUdt : public TypeInfoFields
{
public:

    TypeInfoUdt(SymbolPtr &symbol) :
        TypeInfoFields( symbol->getName() ),
        m_symbol( symbol )
        {}

protected:


    virtual size_t getSize() {
        return  m_symbol->getSize();
    }

    virtual size_t getPtrSize() {
        return getPtrSizeBySymbol( m_symbol );
    }

    virtual bool isUserDefined() {
        return true;
    }

protected:

    SymbolPtr  m_symbol;

    virtual void getFields();

    virtual void getVirtualDisplacement( const std::wstring& fieldName, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize );

    virtual void getVirtualDisplacement( size_t fieldIndex, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize );

    void getFields( 
        SymbolPtr &rootSym, 
        SymbolPtr &baseVirtualSym,
        MEMOFFSET_32 startOffset = 0,
        MEMOFFSET_32 virtualBasePtr = 0,
        size_t virtualDispIndex = 0,
        size_t m_virtualDispSize = 0 );

    void getVirtualFields();

    
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoEnum : public TypeInfoFields 
{
public:

    TypeInfoEnum( SymbolPtr& symbol ) :
        TypeInfoFields( symbol->getName() ),
        m_symbol( symbol )
        {}

protected:

    virtual bool isEnum() 
    {
        return true;
    }

    virtual size_t getPtrSize() {
        return getPtrSizeBySymbol( m_symbol );
    }

    virtual void getFields();

    SymbolPtr  m_symbol;
};

///////////////////////////////////////////////////////////////////////////////

template<typename T>
class TypeInfoBaseWrapper : public TypeInfoImp 
{
public:

    TypeInfoBaseWrapper( const std::wstring & name, size_t ptrSize ) :
        m_name( name ),
        m_ptrSize( ptrSize )
        {}

protected:

    virtual std::wstring getName() {
        return m_name;
    }

    virtual size_t getSize() {
        return sizeof(T);
    }

    virtual bool isBase() {
        return true;
    }

    size_t getPtrSize() {
        return m_ptrSize;
    }

protected:

    std::wstring  m_name;
    size_t  m_ptrSize;

};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoVoid: public TypeInfoImp 
{
public:

    TypeInfoVoid( size_t ptrSize )
    {}

protected:

    virtual bool isVoid() {
        return true;
    }

    virtual std::wstring getName() {
        return L"Void";
    }

    virtual size_t getSize() {
        throw TypeException( L"Void", L"This type has no size" );
    }

    size_t getPtrSize() {
        return m_ptrSize;
    }

private:

    size_t  m_ptrSize;

};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoVtbl: public TypeInfoImp
{
public:
    TypeInfoVtbl( SymbolPtr &symbol ) : m_symbol( symbol ) {}


protected:

    virtual std::wstring getName() {
        return m_symbol->getName();
    }

    SymbolPtr   m_symbol;
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoBitField : public TypeInfoImp 
{
public:

    TypeInfoBitField( SymbolPtr &symbol );

protected:

    virtual bool isBitField() {
        return true;
    }

    virtual std::wstring getName();

    virtual size_t getSize() {
        return m_bitType->getSize();
    }

    virtual BITOFFSET getBitOffset() {
        return m_bitPos;
    }

    virtual BITOFFSET getBitWidth() {
        return m_bitWidth;
    }

    virtual TypeInfoPtr getBitType() {
         return m_bitType;
    }

private:

    TypeInfoPtr  m_bitType;
    BITOFFSET  m_bitWidth;
    BITOFFSET  m_bitPos;
    size_t  m_size;

};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoReference : public TypeInfoImp
{
protected:

    virtual std::wstring getName();

};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoPointer : public TypeInfoReference 
{
public:

    TypeInfoPointer( const TypeInfoPtr &derefType, size_t ptrSize ) :
        m_derefType( derefType ),
        m_ptrSize( ptrSize )
        {}

    TypeInfoPointer( SymbolPtr& symbol )
    {
        m_derefType = loadType( symbol->getType() );
        m_ptrSize = symbol->getSize();
    }

    virtual TypeInfoPtr deref() {
        return m_derefType;
    }

    const std::wstring getDerefName() const {
        return m_derefType->getName();
    }

protected:

    virtual bool isPointer() {
        return true;
    }

    virtual size_t getSize() {
        return m_ptrSize;
    }

    size_t getPtrSize() {
        return m_ptrSize;
    }


protected:

    TypeInfoPtr  m_derefType;
    size_t  m_ptrSize;
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoArray : public TypeInfoReference 
{

public:

    TypeInfoArray( const TypeInfoPtr &derefType, size_t count ) :
        m_derefType( derefType ),
        m_count( count ),
        m_ptrSize( derefType->getPtrSize() )
        {}

    TypeInfoArray( SymbolPtr& symbol )
    {
        m_derefType = loadType( symbol->getType() );
        m_count = symbol->getCount();
        m_ptrSize = getPtrSizeBySymbol(symbol);
    }

    TypeInfoPtr getDerefType() {
        return m_derefType;
    }

protected:

    virtual bool isArray() 
    {
        return true;
    }

    virtual size_t getElementCount() {
        return m_count;
    }

    virtual size_t getSize() {
        return m_derefType->getSize() * m_count;
    }

    size_t getPtrSize() {
        return m_ptrSize;
    }

    TypeInfoPtr getElement( size_t index );

protected:

    TypeInfoPtr  m_derefType;
    size_t  m_count;
    size_t  m_ptrSize;
};

///////////////////////////////////////////////////////////////////////////////

} // namespace kdlib
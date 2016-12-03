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

    virtual std::wstring str() {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getName() {
        NOT_IMPLEMENTED();
    }

    virtual std::pair<std::wstring, std::wstring> splitName() {
        return std::pair<std::wstring, std::wstring>(getName(), std::wstring());
    }

    virtual size_t getSize() {
        NOT_IMPLEMENTED();
    }

    virtual size_t getPtrSize() {
        return ptrSize();
    }

    virtual TypeInfoPtr ptrTo( size_t ptrSize = 0 );

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

    virtual bool isFunction() {
        return false;
    }

    virtual bool isVtbl() {
        return false;
    }

    virtual bool isNoType() {
        return false;
    }

    virtual bool isConstant() {
        return m_constant;
    }

    virtual bool isVirtual() {
        throw TypeException( getName(), L"type is not class method" );
    }

    virtual TypeInfoPtr getElement( const std::wstring &name ) {
         throw TypeException( getName(), L" type has no fields or array elements");
    }

    virtual TypeInfoPtr getElement( size_t index ) {
         throw TypeException( getName(), L" type has no fields or array elements");
    }

    virtual MEMOFFSET_REL getElementOffset( const std::wstring &name ) {
         throw TypeException( getName(), L" type has no fields or array elements");
    }

    virtual MEMOFFSET_REL getElementOffset( size_t index ) {
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

    virtual TypeInfoPtr getMethod( const std::wstring &name, const std::wstring&  prototype)  {
        throw TypeException( getName(), L"type has no methods" ); 
    }

    virtual TypeInfoPtr getMethod( const std::wstring &name, TypeInfoPtr prototype) {
        throw TypeException( getName(), L"type has no methods" ); 
    }

    virtual TypeInfoPtr getMethod( size_t index ) {
        throw TypeException( getName(), L"type has no methods" ); 
    }

    virtual size_t getMethodsCount() {
        throw TypeException( getName(), L"type has no methods" ); 
    }

    virtual TypeInfoPtr getBaseClass( const std::wstring& className) {
        throw TypeException( getName(), L"type has no base class" ); 
    }

    virtual TypeInfoPtr getBaseClass( size_t index ) {
        throw TypeException( getName(), L"type has no base class" ); 
    }

    virtual size_t getBaseClassesCount() {
        throw TypeException( getName(), L"type has no base class" ); 
    }

    virtual bool isMethodMember( const std::wstring &name ) {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    virtual bool isMethodMember( size_t index ) {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    virtual void getVirtualDisplacement( const std::wstring& fieldName, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize ) {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    virtual void getVirtualDisplacement( size_t fieldIndex, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize ) {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    virtual CallingConventionType getCallingConvention() {
        throw TypeException( getName(), L"type is not a function" ); 
    }

    virtual bool hasThis() {
        throw TypeException( getName(), L"type is not a function" ); 
    }

    virtual TypeInfoPtr getReturnType() {
        throw TypeException( getName(), L"type is not a function" ); 
    }

    virtual void appendField(const std::wstring &fieldName, TypeInfoPtr &fieldType ) {
        throw TypeException( getName(), L"type is not editable" ); 
    }

    virtual TypeInfoPtr getClassParent() {
        throw TypeException( getName(), L"has no class parent" ); 
    }

    virtual size_t getAlignReq() {
        throw TypeException( getName(), L"has no alignment-requirement" );
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
    std::wstring print();

    virtual TypeInfoPtr getElement( const std::wstring &name );

    virtual TypeInfoPtr getElement( size_t index );

    virtual MEMOFFSET_REL getElementOffset( const std::wstring &name );

    virtual MEMOFFSET_REL getElementOffset( size_t index );

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

    virtual size_t getAlignReq();

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
    virtual std::wstring str();

    virtual size_t getSize() {
        return  m_symbol->getSize();
    }

    virtual size_t getPtrSize() {
        return getPtrSizeBySymbol( m_symbol );
    }

    virtual bool isUserDefined() {
        return true;
    }

    virtual TypeInfoPtr getClassParent();

    virtual TypeInfoPtr getMethod( const std::wstring &name, const std::wstring&  prototype=L"");
    virtual TypeInfoPtr getMethod( size_t index );
    virtual size_t getMethodsCount();

    virtual TypeInfoPtr getBaseClass( const std::wstring& className);
    virtual TypeInfoPtr getBaseClass( size_t index );
    virtual size_t getBaseClassesCount();

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

    virtual std::wstring str();

    virtual size_t getSize() {
        return  m_symbol->getSize();
    }

    virtual bool isEnum() {
        return true;
    }

    virtual TypeInfoPtr getClassParent();

protected:

    virtual size_t getPtrSize() {
        return getPtrSizeBySymbol( m_symbol );
    }

    virtual void getFields();

    SymbolPtr  m_symbol;
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoFunctionPrototype : public TypeInfoImp
{

public:

protected:

    virtual std::wstring str() {
        return getName();
    }


    virtual bool isFunction() {
        return true;
    }

    virtual size_t getSize() {
        return getPtrSize();
    }

    virtual size_t getElementCount() {
        return m_args.size();
    }

    virtual TypeInfoPtr getElement( size_t index ) {
        if ( index < m_args.size() )
            return m_args[index];
        throw IndexException(index);
    }

    virtual std::wstring getName();

    virtual std::pair<std::wstring, std::wstring> splitName();

protected:

    typedef std::vector< TypeInfoPtr > Args;
    Args m_args;
};

///////////////////////////////////////////////////////////////////////////////


class TypeInfoSymbolFunctionPrototype : public TypeInfoFunctionPrototype
{
public:

    TypeInfoSymbolFunctionPrototype( SymbolPtr& symbol );

protected:

    virtual size_t getPtrSize() {
        return getPtrSizeBySymbol( m_symbol );
    }

    virtual size_t getElementCount() {
        return m_args.size();
    }

    virtual TypeInfoPtr getElement( size_t index );

    virtual CallingConventionType getCallingConvention();

    virtual TypeInfoPtr getReturnType();

    virtual bool hasThis() {
        return m_hasThis;
    }

    virtual size_t getSize() {
        return m_symbol->getSize();
    }

    virtual TypeInfoPtr getClassParent();

    virtual size_t getAlignReq() {
        return getPtrSize();
    }

private:
    SymbolPtr m_symbol;

    bool m_hasThis;
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoSymbolFunction : public TypeInfoSymbolFunctionPrototype
{
public:

    TypeInfoSymbolFunction(const SymbolPtr& symbol) :
        TypeInfoSymbolFunctionPrototype(symbol->getType()),
        m_symbol(symbol)
    {}

    virtual NumVariant getValue() const 
    {
        try 
        {
            return m_symbol->getVa();
        }
        catch(SymbolException&)
        {}

        throw TypeException(L"function has no body");
    }

    virtual bool isVirtual()
    {
        return m_symbol->isVirtual();
    }


private:

    SymbolPtr m_symbol;

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

    virtual std::wstring str() {
        return m_name;
    }

    virtual std::wstring getName() {
        return m_name;
    }

    virtual size_t getSize() {
        return sizeof(T);
    }

    virtual bool isBase() {
        return true;
    }

    virtual size_t getPtrSize() {
        return m_ptrSize;
    }

    virtual size_t getAlignReq() {
        return getSize();
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

    virtual std::wstring str() {
        return getName();
    }


    virtual bool isVoid() {
        return true;
    }

    virtual std::wstring getName() {
        return L"Void";
    }

    virtual size_t getSize() {
        throw TypeException( getName(), L"This type has no size" );
    }

    virtual size_t getPtrSize() {
        return m_ptrSize;
    }

private:

    size_t  m_ptrSize;
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoNoType : public TypeInfoImp 
{
protected:
    virtual std::wstring str() {
        return getName();
    }

    virtual std::wstring getName() {
        return L"NoType";
    }

    virtual bool isNoType() {
        return true;
    }

    virtual size_t getPtrSize() {
        throw TypeException( getName(), L"Don't exist pointers to this type" );
    }
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoVtbl: public TypeInfoImp
{
public:

    TypeInfoVtbl( SymbolPtr &symbol ) : m_symbol( symbol ) {}

    virtual std::wstring str() {
        return getName();
    }

    virtual std::wstring getName() {
        return L"VTable";
    }

    virtual size_t getAlignReq() {
        return getPtrSizeBySymbol( m_symbol );
    }

    virtual bool isVtbl() {
        return true;
    }

    virtual size_t getElementCount();

    virtual size_t getSize();

protected:
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

    virtual TypeInfoPtr getClassParent();

    virtual size_t getAlignReq() {
        return getSize();
    }

private:

    SymbolPtr  m_symbol;
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
        m_symbol = symbol;
        m_derefType = loadType( symbol->getType() );
        m_ptrSize = symbol->getSize();
    }

    virtual TypeInfoPtr deref() {
        return m_derefType;
    }

    const std::wstring getDerefName() const {
        return m_derefType->getName();
    }

    virtual TypeInfoPtr getClassParent();

    virtual size_t getAlignReq() {
        return getSize();
    }

protected:

    virtual std::wstring str() {
        return L"ptr to " + m_derefType->getName();
    }

    virtual bool isPointer() {
        return true;
    }

    virtual size_t getSize() {
        return m_ptrSize;
    }

    virtual size_t getPtrSize() {
        return m_ptrSize;
    }


protected:

    SymbolPtr  m_symbol;
    TypeInfoPtr  m_derefType;
    size_t  m_ptrSize;
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoArray : public TypeInfoReference 
{

public:

    TypeInfoArray( const TypeInfoPtr &derefType, size_t count ) 
    {
        size_t maxCount = SIZE_MAX / derefType->getSize();
        if (count > maxCount)
            throw TypeException(L"Failed to create too long array");

        m_derefType = derefType;
        m_count = count;
        m_ptrSize = derefType->getPtrSize();
    }

    TypeInfoArray( SymbolPtr& symbol )
    {
        m_symbol = symbol;
        m_derefType = loadType( symbol->getType() );
        m_count = symbol->getCount();
        m_ptrSize = getPtrSizeBySymbol(symbol);
    }

    TypeInfoPtr getDerefType() {
        return m_derefType;
    }

    virtual TypeInfoPtr getClassParent();

    virtual size_t getAlignReq() {
        return getDerefType()->getAlignReq();
    }

protected:

    virtual std::wstring str() {
        return getName();
    }

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

    virtual size_t getPtrSize() {
        return m_ptrSize;
    }

    TypeInfoPtr getElement( size_t index );

protected:

    SymbolPtr  m_symbol;
    TypeInfoPtr  m_derefType;
    size_t  m_count;
    size_t  m_ptrSize;
};

///////////////////////////////////////////////////////////////////////////////

} // namespace kdlib
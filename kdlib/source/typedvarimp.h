#pragma once

#include <kdlib/typedvar.h>
#include <kdlib/exceptions.h>
#include <kdlib/memaccess.h>

namespace kdlib {


///////////////////////////////////////////////////////////////////////////////

class VarDataProvider;
typedef boost::shared_ptr<VarDataProvider> VarDataProviderPtr;

class VarDataProvider
{
public:

    virtual unsigned char readByte() const = 0;
    virtual char readSignByte() const = 0;
    virtual unsigned short readWord() const = 0;
    virtual short readSignWord() const = 0;
    virtual unsigned long readDWord() const = 0;
    virtual long readSignDWord() const = 0;
    virtual unsigned long long readQWord() const = 0;
    virtual long long readSignQWord() const = 0;
    virtual float readFloat() const = 0;
    virtual double readDouble() const = 0;
    virtual MEMOFFSET_64 getAddress() const = 0;
    virtual MEMOFFSET_64 readPtr4() const = 0;
    virtual MEMOFFSET_64 readPtr8() const = 0;
    virtual std::wstring asString() const = 0;

protected:
    virtual ~VarDataProvider()
    {}
};

///////////////////////////////////////////////////////////////////////////////

class VarDataMemoryProvider : public VarDataProvider 
{

public:

    VarDataMemoryProvider( MEMOFFSET_64 offset ) :
        m_offset( offset )
        {}

    virtual unsigned char readByte() const {
        return ptrByte(m_offset);
    }

    virtual char readSignByte() const {
        return ptrSignByte(m_offset);
    }

    virtual unsigned short readWord() const {
        return ptrWord(m_offset);
    }

    virtual short readSignWord() const {
        return ptrSignWord(m_offset);
    }

    virtual unsigned long  readDWord() const {
        return ptrDWord(m_offset); 
    }

    virtual long readSignDWord() const {
        return ptrSignDWord(m_offset); 
    }

    virtual unsigned long long readQWord() const {
        return ptrQWord(m_offset); 
    }

    virtual long long readSignQWord() const {
        return ptrSignQWord(m_offset); 
    }

    virtual float readFloat() const {
        return ptrSingleFloat(m_offset);
    }

    virtual double readDouble() const {
        return ptrDoubleFloat(m_offset);
    }

    virtual MEMOFFSET_64 getAddress() const {
        return m_offset;
    }

    virtual MEMOFFSET_64 readPtr4() const {
        return addr64(ptrDWord(m_offset));
    }

    virtual MEMOFFSET_64 readPtr8() const {
        return addr64(ptrQWord(m_offset));
    }

    virtual std::wstring asString() const {
        std::wstringstream sstr;
        sstr << L"at 0x" << std::hex << m_offset;
        return sstr.str();
    }

protected:

    MEMOFFSET_64  m_offset;

};

///////////////////////////////////////////////////////////////////////////////

class TypedVarImp : public TypedVar
{

protected:


    virtual std::wstring str() 
    {
        NOT_IMPLEMENTED();
    }

    virtual NumVariant getValue() const
    {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getAddress() const 
    {
        return m_varData->getAddress();
    }

    virtual size_t getSize() const
    {
        return m_typeInfo->getSize();
    }

    virtual TypeInfoPtr getType() const 
    {
       return m_typeInfo;
    }

    virtual TypedVarPtr deref() 
    {
        throw TypeException( m_typeInfo->getName(), L"object can not be derefernced" );
    }

    virtual TypedVarPtr getElement( const std::wstring& fieldName ) 
    {
        throw TypeException( m_typeInfo->getName(), L" type has no named fields");
    }

    virtual TypedVarPtr getElement( size_t index ) 
    {
        throw TypeException( m_typeInfo->getName(), L" type has no fields or array elements");
    }

    virtual MEMOFFSET_REL getElementOffset( const std::wstring& fieldName ) 
    {
        throw TypeException( m_typeInfo->getName(), L" type has no named fields");
    }

    virtual MEMOFFSET_REL getElementOffset( size_t index ) 
    {
        throw TypeException( m_typeInfo->getName(), L" type has no fields or array elements");
    }

    virtual unsigned long getElementOffsetRelative(const std::wstring& fieldName ) 
    {
        throw TypeException( m_typeInfo->getName(), L" type has no named fields");
    }

    virtual unsigned long getElementOffsetRelative(size_t index ) 
    {
        throw TypeException( m_typeInfo->getName(), L" type has no fields or array elements");
    }

    virtual size_t getElementCount()
    {
        throw TypeException( m_typeInfo->getName(), L" type has no fields or array elements");
    }

    virtual std::wstring getElementName( size_t index ) 
    {
        throw TypeException( m_typeInfo->getName(), L" type has no named fields");
    }

protected:

    TypedVarImp( const TypeInfoPtr& typeInfo, const VarDataProviderPtr &varData ) :
        m_typeInfo( typeInfo ),
        m_varData( varData )
        {}

    TypeInfoPtr  m_typeInfo;

    VarDataProviderPtr  m_varData;
};

///////////////////////////////////////////////////////////////////////////////

class TypedVarBase : public TypedVarImp
{
public:

    TypedVarBase( const TypeInfoPtr& typeInfo, const VarDataProviderPtr &varData ) :
        TypedVarImp( typeInfo, varData )
        {}

    std::wstring printValue() const;

protected:
    
    virtual NumVariant getValue() const;

    virtual std::wstring str();

};

///////////////////////////////////////////////////////////////////////////////

class TypedVarUdt :  public TypedVarImp
{
public:

    TypedVarUdt( const TypeInfoPtr& typeInfo, const VarDataProviderPtr &varData ) :
        TypedVarImp( typeInfo, varData )
        {}

protected:

    virtual NumVariant getValue() const {
        return NumVariant( m_varData->getAddress() );
    }

    virtual TypedVarPtr getElement( const std::wstring& fieldName );

    virtual  TypedVarPtr getElement( size_t index );

    virtual MEMOFFSET_REL getElementOffset( const std::wstring& fieldName ) {
        return m_typeInfo->getElementOffset( fieldName );
    }

    virtual MEMOFFSET_REL getElementOffset( size_t index ) {
        return m_typeInfo->getElementOffset( index );
    }

    virtual size_t getElementCount() {
        return m_typeInfo->getElementCount();
    }

    virtual std::wstring getElementName( size_t index ) {
        return m_typeInfo->getElementName( index );
    }

    MEMDISPLACEMENT getVirtualBaseDisplacement( const std::wstring &fieldName );
    MEMDISPLACEMENT getVirtualBaseDisplacement( size_t index );

    virtual std::wstring str();

    std::wstring printFieldValue( const TypeInfoPtr& typeInfo, const TypedVarPtr& var );
};


///////////////////////////////////////////////////////////////////////////////

class TypedVarPointer : public TypedVarImp
{
public:

    TypedVarPointer( const TypeInfoPtr& typeInfo, VarDataProviderPtr &varData ) :
        TypedVarImp( typeInfo, varData )
        {}


public:

    virtual NumVariant getValue() const {
        return NumVariant( getSize() == 4 ? m_varData->readPtr4() : m_varData->readPtr8() );
    }

    virtual TypedVarPtr deref();

    virtual TypedVarPtr getElement( const std::wstring& fieldName ) {
        TypedVarPtr  derefPtr = deref();
        return derefPtr->getElement( fieldName );
    }

    virtual  TypedVarPtr getElement( size_t index ) {
        TypedVarPtr  derefPtr = deref();
        return derefPtr->getElement( index );
    }

    virtual std::wstring str();


    std::wstring printValue() const;
};

///////////////////////////////////////////////////////////////////////////////

class TypedVarArray : public TypedVarImp
{
public:

    TypedVarArray( const TypeInfoPtr& typeInfo, VarDataProviderPtr &varData ) :
        TypedVarImp( typeInfo, varData )
        {}

    virtual NumVariant getValue() const {
       // return NumVariant( m_typeInfo->getPtrSize() == 4 ?  m_varData->readPtr4() : m_varData->readPtr8() );
        return NumVariant(m_varData->getAddress() );
    }

    virtual size_t getElementCount() {
        return m_typeInfo->getElementCount();
    }

    virtual  TypedVarPtr getElement( size_t index );

    virtual std::wstring str();
};

///////////////////////////////////////////////////////////////////////////////

class TypedVarBitField : public TypedVarImp
{
public:
    TypedVarBitField( const TypeInfoPtr& typeInfo, VarDataProviderPtr &varData ) :
      TypedVarImp( typeInfo, varData )
      {}

    virtual NumVariant getValue() const;
};

///////////////////////////////////////////////////////////////////////////////

class TypedVarEnum : public TypedVarImp
{
public:
    TypedVarEnum( const TypeInfoPtr& typeInfo, VarDataProviderPtr &varData ) :
      TypedVarImp( typeInfo, varData )
      {}

    virtual NumVariant getValue() const {
        return NumVariant( m_varData->readDWord() );
    }


    virtual std::wstring str();

    std::wstring printValue();
};

///////////////////////////////////////////////////////////////////////////////

class TypedVarFunction : public TypedVarImp
{
public:

    TypedVarFunction( const TypeInfoPtr& typeInfo, VarDataProviderPtr &varData ) :
        TypedVarImp( typeInfo, varData )
    {}


    virtual NumVariant getValue() const {
        return NumVariant( getAddress() );
    }

    virtual size_t getElementCount() {
        return m_typeInfo->getElementCount();
    }

    virtual std::wstring str();
};

///////////////////////////////////////////////////////////////////////////////

class SymbolFunction : public TypedVarFunction
{
public:

    SymbolFunction( SymbolPtr& symbol );

    MEMOFFSET_REL getElementOffset( size_t index );

    unsigned long getElementOffsetRelative(size_t index );

    virtual size_t getSize() const {
        return m_symbol->getSize();
    }
protected:

    SymbolPtr  m_symbol;
};

///////////////////////////////////////////////////////////////////////////////

} // namespace kdlib

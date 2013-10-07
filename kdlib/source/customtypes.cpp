#include "stdafx.h"

#include "kdlib\typeinfo.h"

#include "typeinfoimp.h"
#include "udtfield.h"

namespace kdlib {

/////////////////////////////////////////////////////////////////////////////////

class CustomBase : public TypeInfoFields {

protected:
    
    CustomBase( const std::wstring &name, size_t align = 0 ) :
        TypeInfoFields(name)
        {
            m_align = align ? align : ptrSize();
            m_size = 0;
        }

    void throwIfFiledExist(const std::wstring &fieldName);

    void throwIfTypeRecursive(TypeInfoPtr type);

protected:

    virtual size_t getSize() {
        return m_size;
    }

    virtual void getFields() {}

protected:

    size_t  m_align;
    MEMOFFSET_32  m_size;

};


/////////////////////////////////////////////////////////////////////////////////

class CustomStruct : public CustomBase 
{
public:
    
    CustomStruct( const std::wstring &name, size_t align = 0) :
        CustomBase( name, align )
        {}

private:

    virtual void appendField(const std::wstring &fieldName, TypeInfoPtr &fieldType );
};

///////////////////////////////////////////////////////////////////////////////

class CustomUnion : public CustomBase 
{
public:
    
    CustomUnion( const std::wstring &name ) :
        CustomBase( name )
        {}

private:

    virtual void appendField(const std::wstring &fieldName, TypeInfoPtr &fieldType );
};

///////////////////////////////////////////////////////////////////////////////

class CustomUdtField : public TypeField
{
public:

    CustomUdtField( const TypeInfoPtr &typeInfo, const std::wstring& name ) :
        TypeField( name ),
        m_type( typeInfo )
        {}

    void setOffset( MEMOFFSET_32 offset ) {
      m_offset = offset;
    }

private:

    virtual TypeInfoPtr getTypeInfo() {
        return m_type;
    }

    TypeInfoPtr  m_type;
};

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr defineStruct( const std::wstring &structName, size_t align )
{
    return TypeInfoPtr( new CustomStruct(structName, align) );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr defineUnion( const std::wstring& unionName )
{
    return TypeInfoPtr( new CustomUnion(unionName) );
}

///////////////////////////////////////////////////////////////////////////////

void CustomBase::throwIfFiledExist(const std::wstring &fieldName)
{
    bool fieldExist = false;
    try
    {
        m_fields.lookup(fieldName);
        fieldExist = true;
    }
    catch (const TypeException&)
    {
    }

    if (fieldExist)
        throw TypeException(getName(), L"duplicate field name: " + fieldName);
}

////////////////////////////////////////////////////////////////////////////////

void CustomBase::throwIfTypeRecursive(TypeInfoPtr type)
{
    if ( type.get() == this )
        throw TypeException(getName(), L"recursive type definition");

    if ( !type->isUserDefined() )
        return;

    try
    {
        size_t  fieldsCount = type->getElementCount();
        for (size_t i = 0; i < fieldsCount; ++i)
        {
            TypeInfoPtr fieldType = type->getElement(i);
            if ( fieldType.get() == this )
                throw TypeException(getName(), L"recursive type definition");

            throwIfTypeRecursive(fieldType);
        }
    }
    catch (const TypeException &)
    {}
}

////////////////////////////////////////////////////////////////////////////////

void CustomStruct::appendField(const std::wstring &fieldName, TypeInfoPtr &fieldType )
{
    if ( !fieldType )
         throw DbgException( "typeInfo can not be None" );

    throwIfFiledExist(fieldName);
    throwIfTypeRecursive(fieldType);

    CustomUdtField  *field = new CustomUdtField( fieldType, fieldName );

    size_t  fieldSize = fieldType->getSize();
    MEMOFFSET_32  offset = m_size;
    size_t  align = fieldSize < m_align ? fieldSize : m_align;

    if (align)
        offset += (MEMOFFSET_32)( offset % align > 0 ? align - offset % align : 0 );

    field->setOffset( offset );

    m_size = (MEMOFFSET_32)( offset + fieldSize );

    m_fields.push_back( TypeFieldPtr( field ) );
}

////////////////////////////////////////////////////////////////////////////////

void CustomUnion::appendField(const std::wstring &fieldName, TypeInfoPtr &fieldType )
{
    if ( !fieldType )
         throw DbgException( "typeInfo can not be None" );

    throwIfFiledExist(fieldName);
    throwIfTypeRecursive(fieldType);

    CustomUdtField  *field = new CustomUdtField( fieldType, fieldName );

    size_t  fieldSize = fieldType->getSize();

    m_size = fieldSize > m_size ? fieldSize : m_size;

    m_fields.push_back( TypeFieldPtr( field ) );
}

////////////////////////////////////////////////////////////////////////////////

} // kdlib namespace



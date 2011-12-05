#pragma once

#include <string>

#include "diawrapper.h"

namespace pykd {

///////////////////////////////////////////////////////////////////////////////////

class TypeInfo;
typedef boost::shared_ptr<TypeInfo>  TypeInfoPtr;

///////////////////////////////////////////////////////////////////////////////////

class TypeInfo : boost::noncopyable {

public:

    static 
    TypeInfoPtr  getTypeInfo( pyDia::SymbolPtr &symScope, const std::string &symName );

    static 
    TypeInfoPtr  getTypeInfo( pyDia::SymbolPtr &symbol );

    static
    TypeInfoPtr  getBaseTypeInfo( const std::string &name );

    static
    TypeInfoPtr  getBaseTypeInfo( pyDia::SymbolPtr &symbol ); 

public:

    virtual std::string getName() = 0;

    virtual ULONG getSize() = 0;

    virtual TypeInfoPtr getField( const std::string &fieldName ) = 0;

    virtual bool isBasicType() {
        return false;
    }

    virtual bool isPointer() {
        return false;
    }

    virtual bool isArray() {
        return false;
    }

    virtual bool isUserDefined() {
        return false;
    }

    virtual ULONG getCount() {
        throw DbgException( "there is no element" );   
    }

    virtual TypeInfoPtr getElementType() {
        throw DbgException( "there is no element" );   
    }

    ULONG getOffset() {
        return m_offset;
    }

    void setOffset( ULONG offset ) {
        m_offset = offset;
    }

protected:

    std::string getComplexName();

    ULONG   m_offset;
};

///////////////////////////////////////////////////////////////////////////////////

class BaseTypeInfo : public TypeInfo 
{
public:

    BaseTypeInfo( pyDia::SymbolPtr &symbol ) :
      m_dia( symbol )
      {}
        
protected:

    virtual std::string getName() {
        return m_dia->getBasicTypeName( m_dia->getBaseType() );
    }

    virtual ULONG getSize() {
        return (ULONG)m_dia->getSize();
    }

    virtual TypeInfoPtr getField( const std::string &fieldName ) {
        throw DbgException( "there is no such field" );
    }

    virtual bool isBasicType() {
        return true;
    }

    pyDia::SymbolPtr    m_dia;

};

///////////////////////////////////////////////////////////////////////////////////

template<typename T>
class TypeInfoWrapper : public TypeInfo
{
public:    
    TypeInfoWrapper( const std::string &name ) :
      m_name(name)
      {}

private:

    virtual std::string getName() {
        return m_name;
    }

    virtual ULONG getSize() {
        return sizeof(T);
    }

    virtual TypeInfoPtr getField( const std::string &fieldName ) {
        throw DbgException( "there is no such field" );
    }

    virtual bool isBasicType() {
        return true;
    }

    std::string     m_name;

};

///////////////////////////////////////////////////////////////////////////////////

class UdtTypeInfo : public TypeInfo 
{
public:

    UdtTypeInfo ( pyDia::SymbolPtr &symbol ) :
      m_dia( symbol )
      {}

protected:

    virtual std::string getName() {
        return m_dia->getName();
    }

    virtual ULONG getSize() {
        return (ULONG)m_dia->getSize();
    }

    virtual TypeInfoPtr getField( const std::string &fieldName ) {
        pyDia::SymbolPtr  field = m_dia->getChildByName( fieldName );
        TypeInfoPtr  ti = TypeInfo::getTypeInfo( m_dia, fieldName );
        ti->setOffset( field->getOffset() );
        return ti;
    }

    virtual bool isUserDefined() {
        return true;
    }

    pyDia::SymbolPtr    m_dia;
};
  
///////////////////////////////////////////////////////////////////////////////////

class PointerTypeInfo : public TypeInfo {

public:

    PointerTypeInfo( pyDia::SymbolPtr &symbol  );

    PointerTypeInfo( pyDia::SymbolPtr &symScope, const std::string &symName );

    virtual std::string getName();

    virtual ULONG getSize();

    virtual TypeInfoPtr getField( const std::string &fieldName ) {
        throw DbgException( "there is no such field" );
    }

    virtual bool isPointer() {
        return true;
    }

    TypeInfoPtr getDerefType() {
        return m_derefType;
    }


private:
    
    TypeInfoPtr     m_derefType;

    ULONG           m_size;
};

///////////////////////////////////////////////////////////////////////////////////

class ArrayTypeInfo : public TypeInfo {

public:

    ArrayTypeInfo( pyDia::SymbolPtr &symbol  );

    ArrayTypeInfo( pyDia::SymbolPtr &symScope, const std::string &symName, ULONG count );

    virtual std::string getName();

    virtual ULONG getSize();

    virtual TypeInfoPtr getField( const std::string &fieldName ) {
        throw DbgException( "there is no such field" );
    }

    virtual bool isArray() {
        return true;
    }

    virtual ULONG getCount() {
        return m_count;
    }

    virtual TypeInfoPtr getElementType() {
        return m_derefType;
    }

    TypeInfoPtr getDerefType() {
        return m_derefType;
    }



private:

    TypeInfoPtr     m_derefType;

    ULONG           m_count;
};

///////////////////////////////////////////////////////////////////////////////////

}; // namespace pykd

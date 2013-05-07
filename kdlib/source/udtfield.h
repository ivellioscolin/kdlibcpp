#pragma once 

#include <string>
#include <vector>

#include <boost/smart_ptr.hpp>

#include "kdlib/dbgengine.h"
#include "kdlib/typeinfo.h"
#include "kdlib/exceptions.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class UdtField;
typedef boost::shared_ptr<UdtField>  UdtFieldPtr;

class UdtField
{

public:

    const std::wstring& getName() const
    {
        return m_name;
    }

    const std::wstring& getVirtualBaseClassName() const 
    {
        return m_virtualBaseName;
    }

    MEMOFFSET_32 getOffset() const
    {
        if ( m_staticOffset != 0 )
            throw TypeException( m_name, L"static field has only virtual address" );

        return m_offset;
    }

    bool isVirtualMember() const 
    {
        return m_virtualBasePtr != 0;
    }

    bool isStaticMember() const
    {
        return m_staticOffset != 0;
    }

    MEMOFFSET_64 getStaticOffset() const
    {
        if ( m_staticOffset == 0 )
            throw TypeException( m_name, L"this is not a static field" );

        return m_staticOffset;
    }

    void getVirtualDisplacement( MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize )
    {
       virtualBasePtr = m_virtualBasePtr;
       virtualDispIndex = m_virtualDispIndex;
       virtualDispSize = m_virtualDispSize;
    }

    virtual TypeInfoPtr getTypeInfo() = 0;

protected:

    UdtField( const std::wstring  &name ) :
         m_name( name ),
         m_offset( 0 ),
         m_staticOffset( 0 ),
         m_virtualBasePtr( 0 ),
         m_virtualDispIndex( 0 ),
         m_virtualDispSize( 0 )
         {}

    std::wstring  m_name;

    std::wstring  m_virtualBaseName;

    MEMOFFSET_32  m_offset;

    MEMOFFSET_64  m_staticOffset;

    MEMOFFSET_32  m_virtualBasePtr;
    size_t  m_virtualDispIndex;
    size_t  m_virtualDispSize;


};

///////////////////////////////////////////////////////////////////////////////////

class SymbolUdtField : public UdtField 
{
public:

    static UdtFieldPtr getField( 
        const SymbolPtr &sym, 
        const std::wstring& name,
        MEMOFFSET_32 offset,
        MEMOFFSET_32 virtualBasePtr, 
        size_t virtualDispIndex, 
        size_t virtualDispSize )
    {
        SymbolUdtField *p = new SymbolUdtField( sym, name );
        p->m_offset = offset;
        p->m_virtualBasePtr = virtualBasePtr;
        p->m_virtualDispIndex = virtualDispIndex;
        p->m_virtualDispSize = virtualDispSize;
        return UdtFieldPtr(p);
    }

    static UdtFieldPtr getStaticField(
        const SymbolPtr &sym, 
        const std::wstring& name,
        MEMOFFSET_64 offset )
    {
        SymbolUdtField *p = new SymbolUdtField( sym, name );
        p->m_staticOffset = offset;
        return UdtFieldPtr(p);
    }

public:

    SymbolPtr& getSymbol() {
        return m_symbol;
    }

private:

    SymbolUdtField( const SymbolPtr &sym, const std::wstring& name ) :
        UdtField( name ),
        m_symbol( sym )
        {}

    virtual TypeInfoPtr getTypeInfo();

    SymbolPtr  m_symbol;
};

///////////////////////////////////////////////////////////////////////////////

class EnumField : public UdtField 
{
public:
    EnumField( const SymbolPtr &sym ) :
        UdtField( sym->getName() ),
        m_symbol( sym )
        {}

private:

    virtual TypeInfoPtr getTypeInfo() {
        return loadType(m_symbol);
    }

    SymbolPtr  m_symbol;
};

///////////////////////////////////////////////////////////////////////////////

class FieldCollection 
{
public:

    FieldCollection( const std::wstring &name ) :
      m_name( name )
      {}

    const UdtFieldPtr &lookup(size_t index) const;
    const UdtFieldPtr &lookup(const std::wstring &name) const;

    UdtFieldPtr &lookup(size_t index);
    UdtFieldPtr &lookup(const std::wstring &name);

    void push_back( const UdtFieldPtr& field ) {
        m_fields.push_back( field );
    }

    size_t count() const {
        return m_fields.size();
    }

private:

    typedef std::vector<UdtFieldPtr>  FieldList;
    FieldList  m_fields;
    std::wstring  m_name;
};

///////////////////////////////////////////////////////////////////////////////

} // end kdlib namespace


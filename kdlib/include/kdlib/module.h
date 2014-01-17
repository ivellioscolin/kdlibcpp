#pragma once

#include <string>


#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>

#include "kdlib/dbgtypedef.h"
#include "kdlib/dbgengine.h"
#include "kdlib/symengine.h"
#include "kdlib/typeinfo.h"
#include "kdlib/variant.h"
#include "kdlib/typedvar.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class Module;
typedef boost::shared_ptr<Module>  ModulePtr;

ModulePtr loadModule( const std::wstring &name );
ModulePtr loadModule( MEMOFFSET_64 offset );

void splitSymName( const std::wstring &fullName, std::wstring &moduleName, std::wstring &symbolName );

typedef std::pair< std::wstring, MEMOFFSET_64 > SymbolOffset;
typedef std::list< SymbolOffset > SymbolOffsetList;

class Module : private boost::noncopyable, public NumBehavior {
    
public:

    friend ModulePtr loadModule( const std::wstring &name ) {
        return ModulePtr( new Module( name ) );
    }

    friend ModulePtr loadModule( MEMOFFSET_64 offset ) {
        return  ModulePtr( new Module( offset ) );
    }

    std::wstring  getName() {
        return m_name;
    }

    MEMOFFSET_64  getBase() const {
        return m_base;
    }

    MEMOFFSET_64  getEnd() const {
        return m_base + m_size;
    }

    size_t getSize() const {
        return m_size;
    }

    std::wstring getImageName() const {
        return m_imageName;
    }

    unsigned long getCheckSum() const {
        return m_checkSum;
    }

    unsigned long getTimeDataStamp() const {
        return m_timeDataStamp;
    }

    std::wstring getSymFile();

    void
    reloadSymbols();


    MEMOFFSET_64 getSymbolVa( const std::wstring &symbolName );
    MEMOFFSET_32 getSymbolRva( const std::wstring &symbolName );

    SymbolPtr getSymbolByVa( MEMOFFSET_64 offset, MEMDISPLACEMENT* displacement = 0 );
    SymbolPtr getSymbolByRva( MEMOFFSET_32 offset, MEMDISPLACEMENT* displacement = 0 );
    SymbolPtr getSymbolByName( const std::wstring &symbolName );
    SymbolPtr getSymbolScope();
    SymbolPtr getSymbolScope(MEMOFFSET_64 offset);

    size_t getSymbolSize( const std::wstring &symName );


    TypeInfoPtr getTypeByName( const std::wstring &typeName ) {
        return loadType( getSymbolScope(), typeName );
    }

    TypedVarPtr getTypedVarByAddr( MEMOFFSET_64 offset );
    TypedVarPtr getTypedVarByName( const std::wstring &symName );
    TypedVarPtr getTypedVarByTypeName( const std::wstring &typeName, MEMOFFSET_64 addr );
    TypedVarPtr containingRecord( MEMOFFSET_64 offset, const std::wstring &typeName,  const std::wstring &fieldName );
    TypedVarPtr getFunctionByAddr( MEMOFFSET_64 offset );

    TypedVarList loadTypedVarList( MEMOFFSET_64 addr, const std::wstring &typeName, const std::wstring &fieldName );
    TypedVarList loadTypedVarArray( MEMOFFSET_64 addr, const std::wstring &typeName, size_t count );

    SymbolOffsetList  enumSymbols( const std::wstring  &mask = L"*" );

    std::wstring findSymbol( MEMOFFSET_64 offset, MEMDISPLACEMENT &displacement );


    std::wstring getSourceFile( MEMOFFSET_64 offset );

    void getSourceLine( MEMOFFSET_64 offset, std::wstring &fileName, unsigned long &lineno, long &displacement );

protected:

    Module( const std::wstring &name );

    Module( MEMOFFSET_64 offset );

    SymbolSessionPtr& getSymSession();

    virtual NumVariant getValue() const {
        return NumVariant( m_base );
    }

    std::wstring  m_name;
    std::wstring  m_imageName;
    MEMOFFSET_64  m_base;
    size_t  m_size;
    unsigned long  m_timeDataStamp;
    unsigned long  m_checkSum;
    SymbolSessionPtr  m_symSession;

};

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end


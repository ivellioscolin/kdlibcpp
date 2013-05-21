#pragma once

#include <string>


#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>

#include "kdlib/dbgtypedef.h"
#include "kdlib/dbgengine.h"
#include "kdlib/symengine.h"
#include "kdlib/typeinfo.h"
#include "kdlib/variant.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class Module;
typedef boost::shared_ptr<Module>  ModulePtr;

ModulePtr loadModule( const std::wstring &name );
ModulePtr loadModule( MEMOFFSET_64 offset );

class Module : private boost::noncopyable, public NumVariantGetter {
    
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

    std::wstring getSymFile();

    void
    reloadSymbols();


    MEMOFFSET_64 getSymbolVa( const std::wstring &symbolName );
    MEMOFFSET_32 getSymbolRva( const std::wstring &symbolName );

    SymbolPtr getSymbolByVa( MEMOFFSET_64 offset );
    SymbolPtr getSymbolByRva( MEMOFFSET_32 offset );
    SymbolPtr getSymbolByName( const std::wstring &symbolName );
    SymbolPtr getSymbolScope();

    //operator MEMOFFSET_64() const  {
    //    return m_base;
    //}

    TypeInfoPtr getTypeByName( const std::wstring &typeName ) {
        return loadType( getSymbolScope(), typeName );
    }

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


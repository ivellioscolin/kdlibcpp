#include "stdafx.h"

#include <boost/regex.hpp>

#include "kdlib/module.h"
#include "kdlib/memaccess.h"
#include "kdlib/exceptions.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

Module::Module(const std::wstring &moduleName )
{
    m_base = findModuleBase( moduleName );
    m_name = moduleName;
    m_imageName = getModuleImageName( m_base );
    m_timeDataStamp = getModuleTimeStamp( m_base );
    m_checkSum = getModuleCheckSum( m_base );
    m_size = getModuleSize( m_base );
}

///////////////////////////////////////////////////////////////////////////////

Module::Module(MEMOFFSET_64 offset )
{
    m_base = findModuleBase( addr64(offset) );
    m_name = getModuleName( m_base );
    m_imageName = getModuleImageName( m_base );
    m_timeDataStamp = getModuleTimeStamp( m_base );
    m_checkSum = getModuleCheckSum( m_base );
    m_size = getModuleSize( m_base );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 Module::getSymbolVa( const std::wstring& symbolName )
{
    return m_base + getSymbolRva(symbolName);
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_32 Module::getSymbolRva( const std::wstring& symbolName )
{
    SymbolPtr  sym = getSymbolScope()->getChildByName( symbolName );
    return sym->getRva();
}

///////////////////////////////////////////////////////////////////////////////

SymbolPtr Module::getSymbolScope()
{
    return getSymSession()->getSymbolScope();
}

///////////////////////////////////////////////////////////////////////////////

SymbolSessionPtr& Module::getSymSession()
{
    if (m_symSession)
        return m_symSession;

    //SymCacheModuleKey cacheKey = { m_name, m_size, m_timeDataStamp, m_checkSum };
    //if ( findSymCacheEntry( cacheKey, m_symSession ) )
    //{
    //    if ( !m_symSession )
    //        throw SymbolException( "failed to load symbol file" );

    //    return m_symSession;
    //}

    try
    {
        m_symSession = loadSymbolFile( m_base, m_imageName);
        if (m_symSession)
        {
           // insertSymCacheEntry( m_base, cacheKey, m_symSession );
            return m_symSession;
        }
    }
    catch(const SymbolException &)
    {}

    // TODO: read image file path and load using IDiaReadExeAtOffsetCallback

    try
    {
        std::wstring symfile = getModuleSymbolFileName(m_base);
        if (!symfile.empty() )
        {
            m_symSession = loadSymbolFile(symfile, m_base);
        }

        if (m_symSession)
        {
            //insertSymCacheEntry( m_base, cacheKey, m_symSession );
            return m_symSession;
        }
    }
    catch(const DbgException&)
    {}

    try
    {
        m_symSession = loadSymbolFromExports(m_base);
        if (m_symSession)
        {
           // insertSymCacheEntry( m_base, cacheKey, m_symSession );
            return m_symSession;
        }
    }
    catch(const DbgException&)
    {}

    //insertSymCacheEntry( m_base, cacheKey, SymbolSessionPtr() );
    throw SymbolException( L"failed to load symbol file" );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring Module::getSymFile()
{
    try {
       
        return getSymSession()->getSymbolFileName();

    } 
    catch( const SymbolException& )
    {}

    return std::wstring();
}

///////////////////////////////////////////////////////////////////////////////

void Module::reloadSymbols()
{
    //SymCacheModuleKey cacheKey = { m_name, m_size, m_timeDataStamp, m_checkSum };
    //eraseSymCacheEntry( cacheKey );

    m_symSession.reset();
    getSymSession();
}

///////////////////////////////////////////////////////////////////////////////

SymbolPtr Module::getSymbolByVa( MEMOFFSET_64 offset )
{
    offset = addr64(offset);

    if ( offset < m_base || offset > getEnd() )
        throw DbgException( L"address is out of the module space" );

    return getSymbolByRva( (MEMOFFSET_32)(offset - m_base ) );
}

///////////////////////////////////////////////////////////////////////////////

SymbolPtr Module::getSymbolByRva( MEMOFFSET_32 rva )
{
   return getSymSession()->findByRva( rva, SymTagNull, NULL );
}

///////////////////////////////////////////////////////////////////////////////

SymbolPtr Module::getSymbolByName( const std::wstring &symbolName )
{ 
    return getSymbolScope()->getChildByName( symbolName );
}

///////////////////////////////////////////////////////////////////////////////

size_t Module::getSymbolSize( const std::wstring &symName )
{
    TypeInfoPtr typeInfo = getTypeByName( symName );

    return typeInfo->getSize();
}

///////////////////////////////////////////////////////////////////////////////

SymbolOffsetList Module::enumSymbols( const std::wstring  &mask )
{
    SymbolOffsetList offsetLst;

    SymbolPtrList  symlst = getSymbolScope()->findChildren( SymTagData, mask );

    for ( SymbolPtrList::iterator it = symlst.begin(); it != symlst.end(); ++it )
    {
        if ( (*it)->getDataKind() != DataIsConstant )
        {
            offsetLst.push_back( SymbolOffset( (*it)->getName(), (*it)->getVa() ) );
        }
    }

    symlst = getSymbolScope()->findChildren( SymTagFunction, mask );

    for ( SymbolPtrList::iterator it = symlst.begin(); it != symlst.end(); ++it )
    {
        offsetLst.push_back( SymbolOffset( (*it)->getName(), (*it)->getVa() ) );
    }

    return offsetLst;
}

///////////////////////////////////////////////////////////////////////////////

std::wstring Module::findSymbol( MEMOFFSET_64 offset, MEMDISPLACEMENT &displacement )
{
    displacement = 0;
    
    SymbolPtr  sym = getSymSession()->findByRva( (MEMDISPLACEMENT)(offset - m_base ), SymTagNull, &displacement );

    return sym->getName();
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr Module::getTypedVarByAddr( MEMOFFSET_64 offset )
{
    offset = addr64(offset);

    if ( offset < m_base || offset >= m_base + m_size )
        throw SymbolException(L"offset dont has to module");

    SymbolPtr symVar = getSymSession()->findByRva( (MEMOFFSET_32)(offset - m_base ) );

    TypeInfoPtr typeInfo = loadType( symVar->getType() );

    return loadTypedVar( typeInfo, offset );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr Module::getTypedVarByName( const std::wstring &symName )
{
    SymbolPtr  symVar = getSymbolScope()->getChildByName( symName );

    TypeInfoPtr typeInfo = loadType( symVar->getType() );

    if ( LocIsConstant != symVar->getLocType() )
    {
        MEMOFFSET_64  offset = getSymbolVa( symName );

        return loadTypedVar( typeInfo, offset );
    }

    TODO( "constant support");
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr Module::getTypedVarByTypeName( const std::wstring &typeName, MEMOFFSET_64 offset )
{
    offset = addr64(offset);

    if ( offset < m_base || offset >= m_base + m_size )
        throw SymbolException(L"offset dont has to module");

    TypeInfoPtr typeInfo = getTypeByName( typeName );

    return loadTypedVar(typeInfo, offset );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr Module::containingRecord( MEMOFFSET_64 offset, const std::wstring &typeName,  const std::wstring &fieldName )
{
    offset = addr64(offset);

    if ( offset < m_base || offset >= m_base + m_size )
        throw SymbolException(L"offset dont has to module");

    TypeInfoPtr typeInfo = getTypeByName( typeName );

    return kdlib::containingRecord( offset, typeInfo, fieldName );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarList Module::loadTypedVarList( MEMOFFSET_64 offset, const std::wstring &typeName, const std::wstring &fieldName )
{
    offset = addr64(offset);

    if ( offset < m_base || offset >= m_base + m_size )
        throw SymbolException(L"offset dont has to module");

    TypeInfoPtr typeInfo = getTypeByName( typeName );

    return kdlib::loadTypedVarList( offset, typeName, fieldName );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarList Module::loadTypedVarArray( MEMOFFSET_64 offset, const std::wstring &typeName, size_t count )
{
    offset = addr64(offset);

    if ( offset < m_base || offset >= m_base + m_size )
        throw SymbolException(L"offset dont has to module");

    TypeInfoPtr typeInfo = getTypeByName( typeName );

    return kdlib::loadTypedVarArray( offset, typeName, count );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring Module::getSourceFile( MEMOFFSET_64 offset )
{
    std::wstring  fileName;
    unsigned long  lineno;
    long  displacement;

    getSourceLine( offset, fileName, lineno, displacement );

    return fileName;
}

///////////////////////////////////////////////////////////////////////////////

void Module::getSourceLine( MEMOFFSET_64 offset, std::wstring &fileName, unsigned long &lineno, long &displacement )
{
    offset = addr64(offset);

    if ( offset < m_base || offset >= m_base + m_size )
        throw SymbolException(L"offset dont has to module");

    getSymSession()->getSourceLine( offset, fileName, lineno, displacement );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 findModuleBySymbol( const std::wstring &symbolName )
{
    std::vector<MEMOFFSET_64>   moduleList = getModuleBasesList();

    std::vector<MEMOFFSET_64>::const_iterator it;
    for ( it = moduleList.begin(); it != moduleList.end(); ++it )
    {
        ModulePtr  module = loadModule( *it );

        try {

            TypeInfoPtr typeInfo = module->getTypeByName( symbolName );
            return *it;
        } 
        catch( SymbolException& )
        {}
    }        

    std::wstringstream   sstr;
    sstr << L"failed to find module for symbol: " << symbolName;
    throw SymbolException( sstr.str() );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring getSourceFile( MEMOFFSET_64 offset )
{
    //if ( offset == 0 )
    //    offset = getRegInstructionPointer();
    //else
    //    offset = addr64( offset );

    ModulePtr module = loadModule( offset );
    
    return module->getSourceFile( offset );
}

///////////////////////////////////////////////////////////////////////////////

void getSourceLine( std::wstring &fileName, unsigned long &lineno, long &displacement, MEMOFFSET_64 offset )
{
    //if ( offset == 0 )
    //    offset = getRegInstructionPointer();
    //else
    //    offset = addr64( offset );

    ModulePtr module = loadModule( offset );
    
    module->getSourceLine( offset, fileName, lineno, displacement );
}

///////////////////////////////////////////////////////////////////////////////

static const boost::wregex moduleSymMatch(L"^(?:([^!]*)!)?([^!]+)$"); 

void splitSymName( const std::wstring &fullName, std::wstring &moduleName, std::wstring &symbolName )
{
    boost::wsmatch   matchResult;

    if ( !boost::regex_match( fullName, matchResult, moduleSymMatch ) )
    {
        std::wstringstream   sstr;
        sstr << L"invalid symbol name: " << fullName;
        throw kdlib::SymbolException( sstr.str() );
    }

    symbolName = std::wstring( matchResult[2].first, matchResult[2].second );

    if ( matchResult[1].matched )
    {
        moduleName = std::wstring( matchResult[1].first, matchResult[1].second );
    }
    else
    {
        moduleName = L"";
    }
}

///////////////////////////////////////////////////////////////////////////////

}; // kdlib namespace end

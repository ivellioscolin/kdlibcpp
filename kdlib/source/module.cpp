#include "stdafx.h"

#include <boost/regex.hpp>

#include "kdlib/memaccess.h"
#include "kdlib/exceptions.h"

#include "moduleimp.h"
#include "processmon.h"

namespace kdlib {


///////////////////////////////////////////////////////////////////////////////

ModulePtr loadModule( const std::wstring &name )
{
    return ModuleImp::getModule(name);
}

///////////////////////////////////////////////////////////////////////////////

ModulePtr loadModule( MEMOFFSET_64 offset )
{
    return ModuleImp::getModule(offset);
}

///////////////////////////////////////////////////////////////////////////////

ModulePtr ModuleImp::getModule( const std::wstring &name )
{
    return getModule( findModuleBase(name) );
}

///////////////////////////////////////////////////////////////////////////////

ModulePtr ModuleImp::getModule( MEMOFFSET_64 offset )
{
    ProcessInfoPtr  processInfo = ProcessMonitor::getProcessInfo();

    if ( !processInfo )
        throw DbgException( "failed to get module");
    
    ModulePtr  module = processInfo->getModule(offset);

    if ( !module )
    {
        module = ModulePtr( new ModuleImp(offset) );

        processInfo->insertModule(module);
    }

    return module;
}

///////////////////////////////////////////////////////////////////////////////

ModuleImp::ModuleImp(const std::wstring &moduleName )
{
    m_base = findModuleBase( moduleName );
    m_name = moduleName;
    fillFields();
}

///////////////////////////////////////////////////////////////////////////////

ModuleImp::ModuleImp(MEMOFFSET_64 offset )
{
    m_base = findModuleBase( addr64(offset) );
    m_name = getModuleName( m_base );
    fillFields();
}

///////////////////////////////////////////////////////////////////////////////

void ModuleImp::fillFields()
{
    m_imageName = getModuleImageName( m_base );
    m_timeDataStamp = getModuleTimeStamp( m_base );
    m_checkSum = getModuleCheckSum( m_base );
    m_size = getModuleSize( m_base );
    m_isUnloaded = isModuleUnloaded( m_base );
    m_isUserMode = isModuleUserMode( m_base );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 ModuleImp::getSymbolVa( const std::wstring& symbolName )
{
    return m_base + getSymbolRva(symbolName);
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_32 ModuleImp::getSymbolRva( const std::wstring& symbolName )
{
    SymbolPtr  sym = getSymbolScope()->getChildByName( symbolName );
    return sym->getRva();
}

///////////////////////////////////////////////////////////////////////////////

SymbolPtr ModuleImp::getSymbolScope()
{
    return getSymSession()->getSymbolScope();
}


///////////////////////////////////////////////////////////////////////////////

SymbolSessionPtr& ModuleImp::getSymSession()
{
    if (m_symSession)
        return m_symSession;

    try
    {
        m_symSession = loadSymbolFile( m_base, m_imageName);
        if (m_symSession)
        {
            return m_symSession;
        }
    }
    catch(const SymbolException &)
    {}

    try
    {
        std::wstring symfile = getModuleSymbolFileName(m_base);
        if (!symfile.empty() )
        {
            m_symSession = loadSymbolFile(symfile, m_base);
        }

        if (m_symSession)
        {
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
            return m_symSession;
        }
    }
    catch(const DbgException&)
    {}

    throw SymbolException( L"failed to load symbol file" );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring ModuleImp::getSymFile()
{
    try {
       
        return getSymSession()->getSymbolFileName();

    } 
    catch( const SymbolException& )
    {}

    return std::wstring();
}

///////////////////////////////////////////////////////////////////////////////

void ModuleImp::reloadSymbols()
{
    m_symSession.reset();
    getSymSession();
}

///////////////////////////////////////////////////////////////////////////////

SymbolPtr ModuleImp::getSymbolByVa( MEMOFFSET_64 offset,  MEMDISPLACEMENT* displacemnt )
{
    return getSymbolByVa( offset, SymTagNull, displacemnt );
}

///////////////////////////////////////////////////////////////////////////////

SymbolPtr ModuleImp::getSymbolByVa( MEMOFFSET_64 offset, unsigned long  symTag, MEMDISPLACEMENT* displacement )
{
    offset = addr64(offset);

    if ( offset < m_base || offset > getEnd() )
        throw DbgException( "address is out of the module space" );

    return getSymbolByRva( (MEMOFFSET_32)(offset - m_base ), symTag, displacement );
}

///////////////////////////////////////////////////////////////////////////////

SymbolPtr ModuleImp::getSymbolByRva( MEMOFFSET_32 rva, MEMDISPLACEMENT* displacemnt  )
{
   return getSymbolByRva( rva, SymTagNull, displacemnt );
}

///////////////////////////////////////////////////////////////////////////////

SymbolPtr ModuleImp::getSymbolByRva( MEMOFFSET_32 rva, unsigned long symTag, MEMDISPLACEMENT* displacement )
{
   return getSymSession()->findByRva( rva, symTag, displacement );
} 

///////////////////////////////////////////////////////////////////////////////

SymbolPtr ModuleImp::getSymbolByName( const std::wstring &symbolName )
{ 
    return getSymbolScope()->getChildByName( symbolName );
}

///////////////////////////////////////////////////////////////////////////////

size_t ModuleImp::getSymbolSize( const std::wstring &symName )
{
    TypeInfoPtr typeInfo = getTypeByName( symName );

    return typeInfo->getSize();
}

///////////////////////////////////////////////////////////////////////////////

SymbolOffsetList ModuleImp::enumSymbols( const std::wstring  &mask )
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

std::wstring ModuleImp::findSymbol( MEMOFFSET_64 offset, MEMDISPLACEMENT &displacement )
{
    displacement = 0;
    std::wstring name;

    while ( name.empty() )
    {
        SymbolPtr  sym = getSymSession()->findByRva( (MEMDISPLACEMENT)(offset - m_base ), SymTagNull, &displacement );
        name = sym->getName();
        if ( !name.empty() )
            break;

        offset = offset - displacement - 1;
    }

    return name;
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr ModuleImp::getTypedVarByAddr( MEMOFFSET_64 offset )
{
    offset = addr64(offset);

    if ( offset < m_base || offset >= m_base + m_size )
        throw SymbolException(L"offset dont has to module");

    SymbolPtr symVar = getSymSession()->findByRva( (MEMOFFSET_32)(offset - m_base ) );
    return loadTypedVar( symVar );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr ModuleImp::getTypedVarByName( const std::wstring &symName )
{
    SymbolPtr  symVar = getSymbolScope()->getChildByName( symName );

    return loadTypedVar( symVar );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr ModuleImp::getTypedVarByTypeName( const std::wstring &typeName, MEMOFFSET_64 offset )
{
    TypeInfoPtr typeInfo = getTypeByName( typeName );

    return loadTypedVar( typeInfo, offset );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr ModuleImp::containingRecord( MEMOFFSET_64 offset, const std::wstring &typeName,  const std::wstring &fieldName )
{
    offset = addr64(offset);

    TypeInfoPtr typeInfo = getTypeByName( typeName );

    return kdlib::containingRecord( offset, typeInfo, fieldName );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr ModuleImp::getFunctionByAddr( MEMOFFSET_64 offset )
{
    offset = addr64(offset);

    MEMDISPLACEMENT  displacement;
    SymbolPtr  symVar = getSymSession()->findByRva( (MEMDISPLACEMENT)(offset - m_base ), SymTagFunction, &displacement );

    if ( !symVar || displacement < 0 )
         throw SymbolException(L"failed to find function symbols by address");

    size_t  funcLength = symVar->getSize();

    if ( static_cast<size_t>(displacement) > funcLength )
         throw SymbolException(L"failed to find function symbols by address");

    return  loadTypedVar( symVar );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarList ModuleImp::loadTypedVarList( MEMOFFSET_64 offset, const std::wstring &typeName, const std::wstring &fieldName )
{
    offset = addr64(offset);

    TypeInfoPtr typeInfo = getTypeByName( typeName );

    return kdlib::loadTypedVarList( offset, typeInfo, fieldName );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarList ModuleImp::loadTypedVarArray( MEMOFFSET_64 offset, const std::wstring &typeName, size_t count )
{
    offset = addr64(offset);

    TypeInfoPtr typeInfo = getTypeByName( typeName );

    return kdlib::loadTypedVarArray( offset, typeInfo, count );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring ModuleImp::getSourceFile( MEMOFFSET_64 offset )
{
    std::wstring  fileName;
    unsigned long  lineno;
    long  displacement;

    getSourceLine( offset, fileName, lineno, displacement );

    return fileName;
}

///////////////////////////////////////////////////////////////////////////////

void ModuleImp::getSourceLine( MEMOFFSET_64 offset, std::wstring &fileName, unsigned long &lineno, long &displacement )
{
    offset = addr64(offset);

    if ( offset < m_base || offset >= m_base + m_size )
        throw SymbolException(L"offset dont has to module");

    getSymSession()->getSourceLine( offset, fileName, lineno, displacement );
}

///////////////////////////////////////////////////////////////////////////////

std::string ModuleImp::getVersionInfo( const std::string &value )
{
    return getModuleVersionInfo( m_base, value );
}

///////////////////////////////////////////////////////////////////////////////

void ModuleImp::getFixedFileInfo( FixedFileInfo &fixedFileInfo )
{
    return getModuleFixedFileInfo( m_base, fixedFileInfo );
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

            SymbolPtr typeInfo = module->getSymbolByName( symbolName );
            return *it;
        } 
        catch( SymbolException& )
        {}

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
    ModulePtr module = loadModule( offset );
    
    return module->getSourceFile( offset );
}

///////////////////////////////////////////////////////////////////////////////

void getSourceLine( std::wstring &fileName, unsigned long &lineno, long &displacement, MEMOFFSET_64 offset )
{
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

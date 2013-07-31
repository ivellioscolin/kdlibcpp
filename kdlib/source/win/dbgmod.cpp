


///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "kdlib/dbgengine.h"
#include "win/exceptions.h"
#include "win/dbgmgr.h"

#include <vector>

namespace  kdlib {

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 findModuleBase( const std::wstring &moduleName )
{
    HRESULT     hres;
    ULONG64     base;

    hres = g_dbgMgr->symbols->GetModuleByModuleNameWide( moduleName.c_str(), 0, NULL, &base );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSymbol::GetModuleByModuleName", hres ); 

    return base;
}

///////////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 findModuleBase( MEMOFFSET_64 offset )
{
    HRESULT     hres;
    ULONG64     base;
    ULONG       moduleIndex;

    hres = g_dbgMgr->symbols->GetModuleByOffset( offset, 0, &moduleIndex, &base );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSymbol::GetModuleByOffset", hres );

    return base;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long getNumberModules()
{
    ULONG  loaded;
    ULONG  unloaded;
 
    HRESULT  hres = g_dbgMgr->symbols->GetNumberModules( &loaded, &unloaded );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSymbol::GetNumberModules", hres );

    return loaded;
}

///////////////////////////////////////////////////////////////////////////////

std::vector<MEMOFFSET_64> getModuleBasesList()
{
    size_t num = getNumberModules();

    std::vector<MEMOFFSET_64>  moduleList;

    for ( ULONG i = 0; i < num; ++i )
    {
        ULONG64  base = 0;
        HRESULT  hres = g_dbgMgr->symbols->GetModuleByIndex( i, &base );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugSymbol::GetModuleByIndex", hres );

         moduleList.push_back( base );
    }

    return moduleList;
}

///////////////////////////////////////////////////////////////////////////////

//MEMOFFSET_64 findModuleBySymbol( const std::wstring &symbolName )
//{
//    HRESULT     hres;
//    ULONG64     base;
//
//    hres = g_dbgMgr->symbols->GetSymbolModuleWide( ( std::wstring(L"!") + symbolName ).c_str(), &base );
//    if ( FAILED( hres ) )
//    {
//        std::wstringstream   sstr;
//        sstr << L"failed to find module for symbol: " << symbolName;
//        throw SymbolException( sstr.str() );
//    }
//
//    return base;
//}

///////////////////////////////////////////////////////////////////////////////

std::wstring getModuleName( MEMOFFSET_64 baseOffset )
{
    HRESULT  hres;

    std::vector<wchar_t>  moduleName(0x100);

    hres = g_dbgMgr->symbols->GetModuleNameStringWide( 
        DEBUG_MODNAME_MODULE,
        DEBUG_ANY_ID,
        baseOffset,
        &moduleName[0],
        static_cast<ULONG>(moduleName.size()),
        NULL );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSymbol::GetModuleNameString", hres );

    return std::wstring( &moduleName[0] );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring getModuleImageName( MEMOFFSET_64 baseOffset )
{
    HRESULT  hres;
    
    std::vector<wchar_t>  moduleName(0x100);

    hres = g_dbgMgr->symbols->GetModuleNameStringWide( 
        DEBUG_MODNAME_IMAGE,
        DEBUG_ANY_ID,
        baseOffset,
        &moduleName[0],
        static_cast<ULONG>(moduleName.size()),
        NULL );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugSymbol::GetModuleNameString", hres );

    return std::wstring( &moduleName[0] );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_32 getModuleSize( MEMOFFSET_64 baseOffset )
{
    HRESULT  hres;
    DEBUG_MODULE_PARAMETERS     moduleParam = { 0 };

    hres = g_dbgMgr->symbols->GetModuleParameters( 1, &baseOffset, 0, &moduleParam );
    if ( FAILED( hres ) )
         throw DbgEngException( L"IDebugSymbol::GetModuleParameters", hres );

    return moduleParam.Size;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long getModuleTimeStamp( MEMOFFSET_64 baseOffset )
{
    HRESULT  hres;
    DEBUG_MODULE_PARAMETERS     moduleParam = { 0 };

    hres = g_dbgMgr->symbols->GetModuleParameters( 1, &baseOffset, 0, &moduleParam );
    if ( FAILED( hres ) )
         throw DbgEngException( L"IDebugSymbol::GetModuleParameters", hres );

    return moduleParam.TimeDateStamp;
}

///////////////////////////////////////////////////////////////////////////////

unsigned long getModuleCheckSum( MEMOFFSET_64 baseOffset )
{
    HRESULT  hres;
    DEBUG_MODULE_PARAMETERS     moduleParam = { 0 };

    hres = g_dbgMgr->symbols->GetModuleParameters( 1, &baseOffset, 0, &moduleParam );
    if ( FAILED( hres ) )
         throw DbgEngException( L"IDebugSymbol::GetModuleParameters", hres );

    return moduleParam.Checksum;
}

///////////////////////////////////////////////////////////////////////////////

std::wstring getModuleSymbolFileName( MEMOFFSET_64 baseOffset )
{
    HRESULT  hres;
    IMAGEHLP_MODULEW64   moduleInfo = {};

    hres = g_dbgMgr->advanced->GetSymbolInformation(
        DEBUG_SYMINFO_IMAGEHLP_MODULEW64,
        baseOffset,
        0,
        &moduleInfo,
        sizeof(moduleInfo),
        NULL,
        NULL,
        0,
        NULL );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugAdvanced2::GetSymbolInformation", hres );

    if (!*moduleInfo.LoadedPdbName)
    {
        std::wstringstream sstr;
        sstr << L"/f \"" << moduleInfo.ImageName << L"\"";

        hres = g_dbgMgr->symbols->ReloadWide( sstr.str().c_str() );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugSymbols::Reload", hres );

        hres = g_dbgMgr->advanced->GetSymbolInformation(
            DEBUG_SYMINFO_IMAGEHLP_MODULEW64,
            baseOffset,
            0,
            &moduleInfo,
            sizeof(moduleInfo),
            NULL,
            NULL,
            0,
            NULL );

        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugAdvanced2::GetSymbolInformation", hres );
    }

    return std::wstring( moduleInfo.LoadedPdbName );
}

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end 


#include "stdafx.h"

#include <vector>

#include <windows.h>

#include <boost/numeric/conversion/cast.hpp>

#include "kdlib/dbgengine.h"

#include "win/dbgmgr.h"
#include "win/exceptions.h"

using boost::numeric_cast;

namespace  kdlib {

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 addr64( MEMOFFSET_64 offset )
{
    HRESULT     hres;

    ULONG   processorMode;
    hres = g_dbgMgr->control->GetActualProcessorType( &processorMode );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetEffectiveProcessorType", hres );

    switch( processorMode )
    {
    case IMAGE_FILE_MACHINE_I386:
        if ( *( (ULONG*)&offset + 1 ) == 0 )
            return (ULONG64)(LONG)offset;

    case IMAGE_FILE_MACHINE_AMD64:
        break;

    default:
        throw DbgException( "Unknown processor type" );
        break;
    }

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

void readMemory( MEMOFFSET_64 offset, void* buffer, size_t length, bool phyAddr, unsigned long *readed )
{
    if ( readed )
        *readed = 0;

    HRESULT hres;

    if ( phyAddr == false )
    {
        offset = addr64( offset );

        // workitem/10473 workaround
        ULONG64 nextAddress;
        hres = g_dbgMgr->dataspace->GetNextDifferentlyValidOffsetVirtual( offset, &nextAddress );

        DBG_UNREFERENCED_LOCAL_VARIABLE(nextAddress);

        hres = g_dbgMgr->dataspace->ReadVirtual( offset, buffer, numeric_cast<ULONG>(length), readed );
    }
    else
    {
        hres = g_dbgMgr->dataspace->ReadPhysical( offset, buffer,  numeric_cast<ULONG>(length), readed );
    }

    if ( FAILED( hres ) )
        throw MemoryException( offset, phyAddr );
}

///////////////////////////////////////////////////////////////////////////////

bool readMemoryUnsafe( MEMOFFSET_64 offset, void* buffer, size_t length, bool phyAddr, unsigned long *readed  )
{
    HRESULT hres;
    if ( phyAddr == false )
    {
        offset = addr64( offset );

        // workitem/10473 workaround
        ULONG64 nextAddress;
        hres = g_dbgMgr->dataspace->GetNextDifferentlyValidOffsetVirtual( offset, &nextAddress );

        DBG_UNREFERENCED_LOCAL_VARIABLE(nextAddress);

        hres = g_dbgMgr->dataspace->ReadVirtual( offset, buffer,  numeric_cast<ULONG>(length), readed );
    }
    else
    {
        hres = g_dbgMgr->dataspace->ReadPhysical( offset, buffer,  numeric_cast<ULONG>(length), readed );
    }

    return hres == S_OK;
}

///////////////////////////////////////////////////////////////////////////////

bool isVaValid( MEMOFFSET_64 offset )
{
    HRESULT     hres;
    ULONG       offsetInfo;
    
    hres = 
        g_dbgMgr->dataspace->GetOffsetInformation(
            DEBUG_DATA_SPACE_VIRTUAL,
            DEBUG_OFFSINFO_VIRTUAL_SOURCE,
            addr64( offset ),
            &offsetInfo,
            sizeof( offsetInfo ),
            NULL );

    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugDataSpace4::GetOffsetInformation", hres );

    return  offsetInfo != DEBUG_VSOURCE_INVALID;
}

///////////////////////////////////////////////////////////////////////////////

std::string loadCStr( MEMOFFSET_64 offset )
{
   const size_t    maxLength = 0x10000;

    offset = addr64( offset );

    ULONG   strLength = 0;

    HRESULT     hres = 
        g_dbgMgr->dataspace->ReadMultiByteStringVirtual(
            offset,
            maxLength,
            NULL,
            0,
            &strLength );

    if ( FAILED( hres ) )
        throw MemoryException( offset );

    std::vector<char>  buffer(strLength);

    hres = 
        g_dbgMgr->dataspace->ReadMultiByteStringVirtual(
            offset,
            strLength,
            &buffer[0],
            strLength,
            NULL );

    if ( FAILED( hres ) )
        throw MemoryException( offset );
                           
    return std::string( &buffer[0] );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring loadWStr( MEMOFFSET_64 offset )
{
    const size_t    maxLength = 0x10000;
 
    offset = addr64( offset );

    ULONG   strLength = 0;

    HRESULT     hres = 
        g_dbgMgr->dataspace->ReadUnicodeStringVirtualWide(
            offset,
            maxLength,
            NULL,
            0,
            &strLength );

    if ( FAILED( hres ) )
        throw MemoryException( offset );

    std::vector<wchar_t>  buffer(strLength);
        
    hres = 
        g_dbgMgr->dataspace->ReadUnicodeStringVirtualWide(
            offset,
            strLength,
            &buffer[0],
            strLength,
            NULL );
    
    if ( FAILED( hres ) )
        throw MemoryException( offset );
                           
    return std::wstring( &buffer[0] );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 searchMemory( MEMOFFSET_64 beginOffset, unsigned long length, const std::vector<char>& pattern )
{
    if ( pattern.empty() )
        throw DbgException( "searchMemeory: pattern parameter can not have 0 length" );

    beginOffset = addr64(beginOffset);

    ULONG64 foundOffset;
    HRESULT hres = g_dbgMgr->dataspace->SearchVirtual( beginOffset, length, (PVOID)&pattern[0], (ULONG)pattern.size(), 1, &foundOffset );

    if ( FAILED( hres ) )
        return 0LL;

    return foundOffset;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 findMemoryRegion( MEMOFFSET_64 beginOffset, MEMOFFSET_64& regionOffset, unsigned long long &regionLength )
{
    beginOffset = addr64(beginOffset);

    HRESULT  hres =  g_dbgMgr->dataspace->GetNextDifferentlyValidOffsetVirtual( beginOffset, &regionOffset );

    if ( FAILED(hres) )
        throw DbgException( "findMemoryRegion: failed to find next region" );

    regionOffset =  addr64(regionOffset);

    MEMORY_BASIC_INFORMATION64  meminfo = {};

    hres = g_dbgMgr->dataspace->QueryVirtual( regionOffset, &meminfo );

    if ( FAILED(hres) )
       throw MemoryException( regionOffset );

    regionLength = meminfo.RegionSize;

    return regionOffset;
}

///////////////////////////////////////////////////////////////////////////////

kdlib::MemoryProtect getVaProtect( kdlib::MEMOFFSET_64 offset )
{
    offset = addr64(offset);

    HRESULT  hres;
    MEMORY_BASIC_INFORMATION64  meminfo = {};

    hres = g_dbgMgr->dataspace->QueryVirtual( offset, &meminfo );

    if ( FAILED(hres) )
       throw MemoryException( offset );

    return static_cast<MemoryProtect>(meminfo.Protect);
}

///////////////////////////////////////////////////////////////////////////////

}


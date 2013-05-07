#include "stdafx.h"

#include <vector>

#include <windows.h>

#include "kdlib/dbgengine.h"

#include "win/dbgmgr.h"
#include "win/exceptions.h"

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
        throw DbgException( L"Unknown processor type" );
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

        hres = g_dbgMgr->dataspace->ReadVirtual( offset, buffer, length, readed );
    }
    else
    {
        hres = g_dbgMgr->dataspace->ReadPhysical( offset, buffer, length, readed );
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

        hres = g_dbgMgr->dataspace->ReadVirtual( offset, buffer, length, readed );
    }
    else
    {
        hres = g_dbgMgr->dataspace->ReadPhysical( offset, buffer, length, readed );
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

}


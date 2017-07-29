#include "stdafx.h"

#include <memory>

#include <Windows.h>

#include <clrdata.h>
#include <atlbase.h>

#include "kdlib/exceptions.h"
#include "kdlib/dbgengine.h"
#include "kdlib/memaccess.h"



#include "net.h"
#include "win/dbgmgr.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

CLRDataTarget  g_clrDataTarget;


CLRDataTarget::~CLRDataTarget()
{
    if ( m_xclrDataProcess )
    {
        m_xclrDataProcess->Release();
    }

    if ( m_mscordacMod )
    {
        FreeModule(m_mscordacMod);
    }
}

///////////////////////////////////////////////////////////////////////////////

IXCLRDataProcess* CLRDataTarget::get()
{
    if ( m_xclrDataProcess )
    {
        m_xclrDataProcess->AddRef();
        return m_xclrDataProcess;
    }

    MEMOFFSET_64 clrBase = findModuleBase(L"clr");

    std::wstring clrImageName = getModuleImageName(clrBase);

    std::wstring clrDir = clrImageName.substr(0, clrImageName.find_last_of(L'\\'));

    std::wstring mscordac = clrDir + L"\\mscordacwks.dll";

    m_mscordacMod = LoadLibraryW(mscordac.c_str() );
    if ( !m_mscordacMod )
        throw DbgException("failed to load mscordacwks.dll");

    PFN_CLRDataCreateInstance  pfnCLRDataCreateInstance = reinterpret_cast<PFN_CLRDataCreateInstance>(GetProcAddress(m_mscordacMod, "CLRDataCreateInstance"));

    if ( !pfnCLRDataCreateInstance )
        throw DbgException("failed to get CLRDataCreateInstance address");

    IXCLRDataProcess*  xclrDataProcess = NULL;

    HRESULT  hres = pfnCLRDataCreateInstance( __uuidof(IXCLRDataProcess), this, (void**)&m_xclrDataProcess);
    if ( FAILED(hres) )
        throw DbgException("failed CLRDataCreateInstance");

    return m_xclrDataProcess;
}



///////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CLRDataTarget::GetMachineType(
        /* [out] */ ULONG32 *machineType
        )
{
    return  g_dbgMgr->control->GetEffectiveProcessorType(reinterpret_cast<PULONG>(machineType));
}

///////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CLRDataTarget::GetPointerSize(
        /* [out] */ ULONG32 *pointerSize
        )
{
    HRESULT  hres = g_dbgMgr->control->IsPointer64Bit();

    if (FAILED(hres) )
        return hres;

    *pointerSize = hres == S_OK ? 8 : 4;

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CLRDataTarget::GetImageBase(
        /* [string][in] */ LPCWSTR imagePath,
        /* [out] */ CLRDATA_ADDRESS *baseAddress
        )
{
    HRESULT  hres = S_OK;

    *baseAddress = 0;

    for ( ULONG index = 0; hres == S_OK; index++)
    {
        ULONG64  base = 0;
        hres = g_dbgMgr->symbols->GetModuleByIndex(index, &base);
        if (FAILED(hres))
            return hres;

        std::vector<wchar_t> buffer(1000);

        hres = g_dbgMgr->symbols->GetModuleNameStringWide(
            DEBUG_MODNAME_IMAGE,
            index,
            0,
            &buffer[0],
            static_cast<ULONG>(buffer.size()),
            NULL );

        if (FAILED(hres))
            return hres;

        std::wstring  fullImageName( &buffer[0] );
        std::wstring  shortImageName;

        auto  slashPos = fullImageName.find_last_of(L'\\');
        if (slashPos == std::wstring::npos || slashPos == fullImageName.size() - 1)
        {
            shortImageName = fullImageName;
        }
        else
        {
            shortImageName = fullImageName.substr(slashPos + 1, fullImageName.size() - slashPos - 1);
        }

        if ( shortImageName == std::wstring(imagePath) )
            return g_dbgMgr->symbols->GetModuleByIndex(index, baseAddress);
    }

    return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CLRDataTarget::ReadVirtual(
            /* [in] */ CLRDATA_ADDRESS address,
            /* [length_is][size_is][out] */ BYTE *buffer,
            /* [in] */ ULONG32 bytesRequested,
            /* [out] */ ULONG32 *bytesRead
            )
{
    return g_dbgMgr->dataspace->ReadVirtual(
        address, 
        buffer, 
        bytesRequested,
        reinterpret_cast<PULONG>(bytesRead) 
        );
}

///////////////////////////////////////////////////////////////////////////////

}

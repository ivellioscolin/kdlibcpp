#pragma once

#include <clrdata.h>

#include "xclrdata.h"

#include "kdlib/dbgtypedef.h"

namespace kdlib {


class CLRDataTarget : public ICLRDataTarget
{

public:

    CLRDataTarget() : m_xclrDataProcess(0), m_mscordacMod(NULL)
    {}

    ~CLRDataTarget();

    IXCLRDataProcess*  get();

private:

    STDMETHOD_(ULONG, AddRef)() { return 1; }

    STDMETHOD_(ULONG, Release)() { return 1; }

    STDMETHOD(QueryInterface)(
        _In_ REFIID InterfaceId,
        _Out_ PVOID* Interface
        )
    {
        *Interface = NULL;

        if ( IsEqualIID(InterfaceId, IID_IUnknown) || IsEqualIID(InterfaceId, __uuidof(ICLRDataTarget)) )
        {
            AddRef();
            *Interface = this;
            return S_OK;
        }

        *Interface = NULL;

        return E_NOINTERFACE;
    }

    STDMETHOD(GetMachineType)(
        /* [out] */ ULONG32 *machineType
        );
        
    STDMETHOD (GetPointerSize)(
        /* [out] */ ULONG32 *pointerSize
        );
        
    STDMETHOD (GetImageBase)(
        /* [string][in] */ LPCWSTR imagePath,
        /* [out] */ CLRDATA_ADDRESS *baseAddress
        );
        
    STDMETHOD (ReadVirtual)(
            /* [in] */ CLRDATA_ADDRESS address,
            /* [length_is][size_is][out] */ BYTE *buffer,
            /* [in] */ ULONG32 bytesRequested,
            /* [out] */ ULONG32 *bytesRead
            );

    STDMETHOD (WriteVirtual)(
            /* [in] */ CLRDATA_ADDRESS address,
            /* [size_is][in] */ BYTE *buffer,
            /* [in] */ ULONG32 bytesRequested,
            /* [out] */ ULONG32 *bytesWritten
            )
    {
        return E_NOTIMPL;
    }
        
    STDMETHOD (GetTLSValue)(
            /* [in] */ ULONG32 threadID,
            /* [in] */ ULONG32 index,
            /* [out] */ CLRDATA_ADDRESS *value
            )
    {
        return E_NOTIMPL;
    }
        
    STDMETHOD (SetTLSValue)(
            /* [in] */ ULONG32 threadID,
            /* [in] */ ULONG32 index,
            /* [in] */ CLRDATA_ADDRESS value
            )
    {
        return E_NOTIMPL;
    }

    STDMETHOD (GetCurrentThreadID)(
            /* [out] */ ULONG32 *threadID
            ) 
    {
        return E_NOTIMPL;
    }
        
    STDMETHOD (GetThreadContext)(
            /* [in] */ ULONG32 threadID,
            /* [in] */ ULONG32 contextFlags,
            /* [in] */ ULONG32 contextSize,
            /* [size_is][out] */ BYTE *context
            )
    {
        return E_NOTIMPL;
    }
        
    STDMETHOD (SetThreadContext)(
            /* [in] */ ULONG32 threadID,
            /* [in] */ ULONG32 contextSize,
            /* [size_is][in] */ BYTE *context
            ) 
    {
        return E_NOTIMPL;
    }

    STDMETHOD (Request)( 
            /* [in] */ ULONG32 reqCode,
            /* [in] */ ULONG32 inBufferSize,
            /* [size_is][in] */ BYTE *inBuffer,
            /* [in] */ ULONG32 outBufferSize,
            /* [size_is][out] */ BYTE *outBuffer
            ) 
    {
        return E_NOTIMPL;
    }

private:

    IXCLRDataProcess*  m_xclrDataProcess;

    HMODULE  m_mscordacMod;
};

extern CLRDataTarget  g_clrDataTarget;


template<typename T>
HRESULT  clrGetName(T* obj,  std::wstring&  name)
{
    name.clear();

    ULONG32  bufferLength = 0x100;
    std::vector<WCHAR>  buf(bufferLength);
    ULONG32  nameLen = 0;
    HRESULT  hres = obj->GetName(0, bufferLength, &bufferLength, &buf.front() );

    if ( hres == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) )
    {
        buf.resize(bufferLength);
        hres = obj->GetName(0, bufferLength, &bufferLength, &buf.front() );
    }

    if (FAILED(hres))
        return hres;

    name = std::wstring(&buf.front());

    return S_OK;
}

}
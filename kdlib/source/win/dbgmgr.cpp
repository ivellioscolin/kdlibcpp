#include "stdafx.h"
#include "dbgmgr.h"

#include "win/exceptions.h"

namespace kdlib {

DebugManager*  g_dbgMgr = NULL;

///////////////////////////////////////////////////////////////////////////////

DebugManager::DebugManager()
{
    CoInitialize(NULL);

    HRESULT  hres = DebugCreate( __uuidof(IDebugClient4), (void **)&client );
    if ( FAILED( hres ) )
        throw DbgEngException(L"DebugCreate", hres);

    control = CComQIPtr<IDebugControl4>(client);
    system = CComQIPtr<IDebugSystemObjects2>(client);
    dataspace = CComQIPtr<IDebugDataSpaces4>(client);
    symbols = CComQIPtr<IDebugSymbols3>(client);
    advanced = CComQIPtr<IDebugAdvanced2>(client);
}

///////////////////////////////////////////////////////////////////////////////

DebugManager::~DebugManager()
{
    CoUninitialize();
}

///////////////////////////////////////////////////////////////////////////////

} //kdlib namespace end

#pragma once

#include <dbgeng.h>
#include <dbghelp.h>
#include <windows.h>
#include <atlbase.h>

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class DebugManager 
{

public:

    DebugManager();
    ~DebugManager();

public:

     CComPtr<IDebugClient4>         client;
     CComPtr<IDebugControl4>        control;
     CComPtr<IDebugSystemObjects2>  system;
     CComPtr<IDebugDataSpaces4>     dataspace;
     CComPtr<IDebugSymbols3>        symbols;
     CComPtr<IDebugAdvanced2>       advanced;

};

extern DebugManager*  g_dbgMgr;

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end

